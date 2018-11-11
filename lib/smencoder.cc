// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smencoder.hh"
#include "smmath.hh"
#include "smfft.hh"
#include "smdebug.hh"
#include "smmicroconf.hh"
#include "smutils.hh"
#include "smblockutils.hh"
#include "smalignedarray.hh"
#include "smrandom.hh"

#include <math.h>
#include <stdio.h>
#include <assert.h>

#include <complex>
#include <map>
#include <algorithm>
#include <memory>
#include <cinttypes>

using namespace SpectMorph;
using std::vector;
using std::string;
using std::map;
using std::max;
using std::complex;

static double
magnitude (vector<float>::iterator i)
{
  return sqrt (*i * *i + *(i+1) * *(i+1));
}

// wraps phase in range [0:2*pi]
static double
normalize_phase (double phase)
{
  const double inv_2pi = 1.0 / (2.0 * M_PI);
  phase *= inv_2pi;
  phase -= floor (phase);
  return phase * (2.0 * M_PI);
}

#define debug(...) SpectMorph::Debug::debug ("encoder", __VA_ARGS__)

EncoderParams::EncoderParams() :
  mix_freq (0),
  frame_step_ms (0),
  frame_size_ms (0),
  zeropad (0),
  frame_step (0),
  frame_size (0),
  block_size (0),
  fundamental_freq (0),
  param_name_d ({"peak-width", "min-frame-periods", "min-frame-size"}),
  param_name_s ({"window"})
{
}

bool
EncoderParams::load_config (const std::string& filename)
{
  MicroConf cfg (filename);

  if (!cfg.open_ok())
    {
      return false;
    }

  string oneline = "";
  while (cfg.next())
    {
      bool parsed = false;
      for (auto name : param_name_d)    // parse double parameters
        {
          double d;
          string str;

          if (cfg.command (name, d))
            {
              param_value_d[name] = d;
              if (oneline != "")
                oneline += ";";
              if (cfg.command (name, str))
                oneline += name + "=" + str;
              parsed = true;
            }
        }
      for (auto name : param_name_s)    // parse string parameters
        {
          string str;

          if (cfg.command (name, str))
            {
              param_value_s[name] = str;
              if (oneline != "")
                oneline += ";";
              oneline += name + "=" + str;
              parsed = true;
            }
        }
      if (!parsed)
        {
          cfg.die_if_unknown();
        }
    }

#if 0
  printf ("--- config ---\n");
  printf ("ONELINE: %s\n", oneline.c_str());
  for (auto name : param_name_d)
    {
      double d;
      if (get_param (name, d))
        {
          printf ("%s=%f\n", name.c_str(), d);
        }
      else
        {
          printf ("%s=<undef>\n", name.c_str());
        }
    }
  printf ("--- end config ---\n");
#endif

  return true;
}

bool
EncoderParams::get_param (const string& param, double& value) const
{
  if (find (param_name_d.begin(), param_name_d.end(), param) == param_name_d.end())
    {
      fprintf (stderr, "error: encoder parameter '%s' was not defined\n", param.c_str());
      exit (1);
    }

  map<string,double>::const_iterator pi = param_value_d.find (param);
  if (pi == param_value_d.end())
    {
      return false; /* not defined */
    }
  else
    {
      value = pi->second;
      return true;
    }
}

bool
EncoderParams::get_param (const string& param, string& value) const
{
  if (find (param_name_s.begin(), param_name_s.end(), param) == param_name_s.end())
    {
      fprintf (stderr, "error: encoder parameter '%s' was not defined\n", param.c_str());
      exit (1);
    }

  map<string,string>::const_iterator pi = param_value_s.find (param);
  if (pi == param_value_s.end())
    {
      return false; /* not defined */
    }
  else
    {
      value = pi->second;
      return true;
    }
}

/**
 * Constructor which initializes the Encoders parameters.
 */
Encoder::Encoder (const EncoderParams& enc_params)
{
  assert (enc_params.mix_freq > 0);
  assert (enc_params.frame_step_ms > 0);
  assert (enc_params.frame_size_ms > 0);
  assert (enc_params.zeropad > 0);
  assert (enc_params.frame_step > 0);
  assert (enc_params.frame_size > 0);
  assert (enc_params.block_size > 0);
  assert (enc_params.fundamental_freq > 0);

  this->enc_params = enc_params;

  loop_start = -1;
  loop_end = -1;
  loop_type = Audio::LOOP_NONE;
  optimal_attack.attack_start_ms = 0;
  optimal_attack.attack_end_ms = 0;
}

/**
 * This function computes the short-time-fourier-transform (STFT) of the input
 * signal using a window to cut the individual frames out of the sample.
 */
void
Encoder::compute_stft (const WavData& multi_channel_wav_data, int channel, const vector<float>& window)
{
  /* deinterleave multi channel signal */
  vector<float> single_channel_signal;

  const size_t n_channels = multi_channel_wav_data.n_channels();
  for (size_t i = channel; i < multi_channel_wav_data.n_values(); i += n_channels)
    single_channel_signal.push_back (multi_channel_wav_data[i]);

  original_samples = single_channel_signal;

  WavData wav_data (single_channel_signal, 1, multi_channel_wav_data.mix_freq());

  /* encode single channel */
  zero_values_at_start = enc_params.frame_size - enc_params.frame_step / 2;
  vector<float> zero_values (zero_values_at_start);

  wav_data.prepend (zero_values);

  const uint64 n_values = wav_data.n_values();
  const size_t frame_size = enc_params.frame_size;
  const size_t block_size = enc_params.block_size;
  const int    zeropad    = enc_params.zeropad;

  sample_count = n_values;

  vector<double> in (block_size * zeropad), out (block_size * zeropad + 2);

  float *fft_in = FFT::new_array_float (in.size());
  float *fft_out = FFT::new_array_float (in.size());

  for (uint64 pos = 0; pos < n_values; pos += enc_params.frame_step)
    {
      EncoderBlock audio_block;

      /* start with zero block, so the incomplete blocks at end are zeropadded */
      vector<float> block (block_size);

      for (size_t offset = 0; offset < block.size(); offset++)
        {
          if (pos + offset < wav_data.n_values())
            block[offset] = wav_data[pos + offset];
        }
      vector<float> debug_samples (block.begin(), block.end());
      Block::mul (enc_params.block_size, &block[0], &window[0]);

      int j = in.size() - enc_params.frame_size / 2;
      for (vector<float>::const_iterator i = block.begin(); i != block.end(); i++)
        in[(j++) % in.size()] = *i;

      std::copy (in.begin(), in.end(), fft_in);
      FFT::fftar_float (in.size(), fft_in, fft_out);
      std::copy (fft_out, fft_out + in.size(), out.begin());

      out[block_size * zeropad] = out[1];
      out[block_size * zeropad + 1] = 0;
      out[1] = 0;

      audio_block.noise.assign (out.begin(), out.end()); // <- will be overwritten by noise spectrum later on
      audio_block.original_fft.assign (out.begin(), out.end());
      audio_block.debug_samples.assign (debug_samples.begin(), debug_samples.begin() + frame_size);

      audio_blocks.push_back (audio_block);
    }
  FFT::free_array_float (fft_in);
  FFT::free_array_float (fft_out);
}

namespace
{

class QInterpolator
{
  double a, b, c;

public:
  QInterpolator (double y1, double y2, double y3)
  {
    a = (y1 + y3 - 2*y2) / 2;
    b = (y3 - y1) / 2;
    c = y2;
  }
  double
  eval (double x)
  {
    return a * x * x + b * x + c;
  }
  double
  x_max()
  {
    return -b / (2 * a);
  }
};

}

/**
 * This function searches for peaks in the frame ffts. These are stored in frame_tracksels.
 */
void
Encoder::search_local_maxima (const vector<float>& window)
{
  const size_t block_size = enc_params.block_size;
  const size_t frame_size = enc_params.frame_size;
  const int    zeropad    = enc_params.zeropad;
  const double mix_freq   = enc_params.mix_freq;

  // figure out normalization for window
  double window_weight = 0;
  for (size_t i = 0; i < frame_size; i++)
    window_weight += window[i];
  const double window_scale = 2.0 / window_weight;

  // initialize tracksel structure
  frame_tracksels.clear();
  frame_tracksels.resize (audio_blocks.size());

  // find maximum of all values
  double max_mag = 0;
  for (size_t n = 0; n < audio_blocks.size(); n++)
    {
      for (size_t d = 2; d < block_size * zeropad; d += 2)
	{
	  max_mag = max (max_mag, magnitude (audio_blocks[n].noise.begin() + d));
	}
    }

  for (size_t n = 0; n < audio_blocks.size(); n++)
    {
      vector<double> mag_values (audio_blocks[n].noise.size() / 2);
      for (size_t d = 0; d < block_size * zeropad; d += 2)
        mag_values[d / 2] = magnitude (audio_blocks[n].noise.begin() + d);

      for (size_t d = 2; d < block_size * zeropad; d += 2)
	{
#if 0
	  double phase = atan2 (*(audio_blocks[n]->noise.begin() + d),
	                        *(audio_blocks[n]->noise.begin() + d + 1)) / 2 / M_PI;  /* range [-0.5 .. 0.5] */
#endif
          enum { PEAK_NONE, PEAK_SINGLE, PEAK_DOUBLE } peak_type = PEAK_NONE;

          if (mag_values[d/2] > mag_values[d/2-1] && mag_values[d/2] > mag_values[d/2+1])   /* search for peaks in fft magnitudes */
            {
              /* single peak is the common case, where the magnitude of the middle value is
               * larger than the magnitude of the left and right neighbour
               */
              peak_type = PEAK_SINGLE;
            }
          else
            {
              double epsilon_fact = 1.0 + 1e-8;
              if (mag_values[d/2] < mag_values[d/2+1] * epsilon_fact && mag_values[d/2] * epsilon_fact > mag_values[d/2 + 1]
              &&  mag_values[d/2] > mag_values[d/2-1] && mag_values[d/2] > mag_values[d/2+2])
                {
                  /* double peak is a special case, where two values in the spectrum have (almost) equal magnitude
                   * in this case, this magnitude must be larger than the value left and right of the _two_
                   * maximal values in the spectrum
                   */
                  peak_type = PEAK_DOUBLE;
                }
            }

          const double mag2 = db_from_factor (mag_values[d / 2] / max_mag, -100);
          debug ("dbspectrum:%zd %f\n", n, mag2);

          if (peak_type != PEAK_NONE)
            {
              if (mag2 > -90)
                {
                  size_t ds, de;
                  for (ds = d / 2 - 1; ds > 0 && mag_values[ds] < mag_values[ds + 1]; ds--);
                  for (de = d / 2 + 1; de < (mag_values.size() - 1) && mag_values[de] > mag_values[de + 1]; de++);

                  const double normalized_peak_width = (de - ds) * frame_size / double (block_size * zeropad);

                  bool peak_ok;
                  double value;
                  if (enc_params.get_param ("peak-width", value))
                    peak_ok = normalized_peak_width > value;
                  else
                    peak_ok = normalized_peak_width > 2.9;

                  if (peak_ok)
                    {
                      const double mag1 = db_from_factor (mag_values[d / 2 - 1] / max_mag, -100);
                      const double mag3 = db_from_factor (mag_values[d / 2 + 1] / max_mag, -100);
                      //double freq = d / 2 * mix_freq / (block_size * zeropad); /* bin frequency */

                      QInterpolator mag_interp (mag1, mag2, mag3);
                      double x_max = mag_interp.x_max();
                      double tfreq = (d / 2 + x_max) * mix_freq / (block_size * zeropad);

                      double peak_mag_db = mag_interp.eval (x_max);
                      double peak_mag = db_to_factor (peak_mag_db) * max_mag;

                      // use the interpolation formula for the complex values to find the phase
                      QInterpolator re_interp (audio_blocks[n].noise[d-2], audio_blocks[n].noise[d], audio_blocks[n].noise[d+2]);
                      QInterpolator im_interp (audio_blocks[n].noise[d-1], audio_blocks[n].noise[d+1], audio_blocks[n].noise[d+3]);
    /*
                      if (mag2 > -20)
                        printf ("%f %f %f %f %f\n", phase, last_phase[d], phase_diff, phase_diff * mix_freq / (block_size * zeropad) * overlap, tfreq);
    */
                      Tracksel tracksel;
                      tracksel.frame = n;
                      tracksel.d = d;
                      tracksel.freq = tfreq;
                      tracksel.mag = peak_mag * window_scale;
                      tracksel.mag2 = mag2;
                      tracksel.next = 0;
                      tracksel.prev = 0;

                      const double re_mag = re_interp.eval (x_max);
                      const double im_mag = im_interp.eval (x_max);
                      double phase = atan2 (im_mag, re_mag) + 0.5 * M_PI;
                      // correct for the odd-centered analysis
                        {
                          phase -= (frame_size - 1) / 2.0 / mix_freq * tracksel.freq * 2 * M_PI;
                          phase = normalize_phase (phase);
                        }
                      tracksel.phase = phase;

                      // FIXME: need a different criterion here
                      // mag2 > -30 doesn't track all partials
                      // mag2 > -60 tracks lots of junk, too
                      if (mag2 > -90 && tracksel.freq > 10)
                        frame_tracksels[n].push_back (tracksel);

                      if (peak_type == PEAK_DOUBLE)
                        d += 2;
                    }
                }
#if 0
              last_phase[d] = phase;
#endif
            }
	}
    }
}

/// @cond
struct
PeakIndex
{
  double                      freq;
  vector<Tracksel>::iterator  i;
  PeakIndex                  *prev;
  double                      prev_delta;

  PeakIndex (double freq, vector<Tracksel>::iterator i)
    : freq (freq), i (i), prev (0), prev_delta (0)
  {
  }
};
/// @endcond

static bool
partial_index_cmp (const PeakIndex& a, const PeakIndex& b)
{
  return a.freq < b.freq;
}

/**
 * This function links the spectral peaks (contained in the Tracksel structure)
 * of successive frames together by setting the prev and next pointers. It
 * tries to minimize the frequency difference between the peaks that are linked
 * together, while using a threshold of 5% frequency derivation.
 */
void
Encoder::link_partials()
{
  for (size_t n = 0; n + 1 < audio_blocks.size(); n++)
    {
      // build sorted index for this frame
      vector<PeakIndex> current_index;
      for (vector<Tracksel>::iterator i = frame_tracksels[n].begin(); i != frame_tracksels[n].end(); i++)
        current_index.push_back (PeakIndex (i->freq, i));
      sort (current_index.begin(), current_index.end(), partial_index_cmp);

      // build sorted index for next frame
      vector<PeakIndex> next_index;
      for (vector<Tracksel>::iterator i = frame_tracksels[n + 1].begin(); i != frame_tracksels[n + 1].end(); i++)
        next_index.push_back (PeakIndex (i->freq, i));
      sort (next_index.begin(), next_index.end(), partial_index_cmp);

      vector<PeakIndex>::iterator ci = current_index.begin();
      vector<PeakIndex>::iterator ni = next_index.begin();
      if (ni != next_index.end())    // if current or next frame are empty (no peaks) there is nothing to do
        {
          while (ci != current_index.end())
            {
              /*
               * increment ni as long as incrementing it makes ni point to a
               * better (closer) peak below ci's frequency
               */
              vector<PeakIndex>::iterator inc_ni;
              do
                {
                  inc_ni = ni + 1;
                  if (inc_ni < next_index.end() && inc_ni->freq < ci->freq)
                    ni = inc_ni;
                }
              while (ni == inc_ni);

              /*
               * possible candidates for a match are
               * - ni      - which contains the greatest peak with a smaller frequency than ci->freq
               * - ni + 1  - which contains the smallest peak with a greater frequency that ci->freq
               * => choose the candidate which is closer to ci->freq
               */
              vector<PeakIndex>::iterator besti = ni;
              if (ni + 1 < next_index.end() && fabs (ci->freq - (ni + 1)->freq) < fabs (ci->freq - ni->freq))
                besti = ni + 1;

              const double delta = fabs (ci->freq - besti->freq) / ci->freq;
              if (delta < 0.05) /* less than 5% frequency derivation */
                {
                  if (!besti->prev || besti->prev_delta > delta)
                    {
                      besti->prev = &(*ci);
                      besti->prev_delta = delta;
                    }
                }
              ci++;
            }

          /* link best matches (with the smallest frequency derivation) */
          for (ni = next_index.begin(); ni != next_index.end(); ni++)
            {
              if (ni->prev)
                {
                  Tracksel *crosslink_a = &(*ni->prev->i);
                  Tracksel *crosslink_b = &(*ni->i);
                  crosslink_a->next = crosslink_b;
                  crosslink_b->prev = crosslink_a;
                }
            }
        }
    }
}

/**
 * This function validates that the partials found by the peak linking have
 * good quality.
 */
void
Encoder::validate_partials()
{
  map<Tracksel *, bool> processed_tracksel;
  for (size_t n = 0; n < audio_blocks.size(); n++)
    {
      vector<Tracksel>::iterator i, j;
      for (i = frame_tracksels[n].begin(); i != frame_tracksels[n].end(); i++)
	{
	  if (!processed_tracksel[&(*i)])
	    {
	      double biggest_mag = -100;
	      for (Tracksel *t = &(*i); t; t = t->next)
		{
		  biggest_mag = max (biggest_mag, t->mag2);
		  processed_tracksel[t] = true;
		}
	      if (biggest_mag > -90)
		{
		  for (Tracksel *t = &(*i); t; t = t->next)
		    {
		      audio_blocks[t->frame].freqs.push_back (t->freq);
		      audio_blocks[t->frame].mags.push_back (t->mag);
		      audio_blocks[t->frame].phases.push_back (t->phase);
		    }
		}
	    }
	}
    }
}

/**
 * This function subtracts the partials from the audio signal, to get the
 * residue (remaining energy not corresponding to sine frequencies).
 */
void
Encoder::spectral_subtract (const vector<float>& window)
{
  const size_t block_size = enc_params.block_size;
  const size_t frame_size = enc_params.frame_size;
  const size_t zeropad = enc_params.zeropad;

  float *fft_in = FFT::new_array_float (block_size * zeropad);
  float *fft_out = FFT::new_array_float (block_size * zeropad);

  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      AlignedArray<float,16> signal (frame_size);
      for (size_t i = 0; i < audio_blocks[frame].freqs.size(); i++)
	{
          const double freq = audio_blocks[frame].freqs[i];
	  const double mag = audio_blocks[frame].mags[i];
	  const double phase = audio_blocks[frame].phases[i];

          VectorSinParams params;
          params.mix_freq = enc_params.mix_freq;
          params.freq = freq;
          params.phase = phase;
          params.mag = mag;
          params.mode = VectorSinParams::ADD;

          fast_vector_sinf (params, &signal[0], &signal[frame_size]);
	}
      vector<double> out (block_size * zeropad + 2);
      // apply window
      std::fill (fft_in, fft_in + block_size * zeropad, 0);
      for (size_t k = 0; k < frame_size; k++)
        fft_in[k] = window[k] * signal[k];
      // FFT
      FFT::fftar_float (block_size * zeropad, fft_in, fft_out);
      std::copy (fft_out, fft_out + block_size * zeropad, out.begin());
      out[block_size * zeropad] = out[1];
      out[block_size * zeropad + 1] = 0;
      out[1] = 0;

      // subtract spectrum from audio spectrum
      for (size_t d = 0; d < block_size * zeropad; d += 2)
	{
	  double re = out[d], im = out[d + 1];
	  double sub_mag = sqrt (re * re + im * im);
	  debug ("subspectrum:%" PRId64 " %g\n", frame, sub_mag);

	  double mag = magnitude (audio_blocks[frame].noise.begin() + d);
	  debug ("spectrum:%" PRId64 " %g\n", frame, mag);
	  if (mag > 0)
	    {
	      audio_blocks[frame].noise[d] /= mag;
	      audio_blocks[frame].noise[d + 1] /= mag;
	      mag -= sub_mag;
	      if (mag < 0)
		mag = 0;
	      audio_blocks[frame].noise[d] *= mag;
	      audio_blocks[frame].noise[d + 1] *= mag;
	    }
	  debug ("finalspectrum:%" PRId64 " %g\n", frame, mag);
	}
    }
  FFT::free_array_float (fft_in);
  FFT::free_array_float (fft_out);
}

template<class AIter, class BIter>
static double
float_vector_delta (AIter ai, AIter aend, BIter bi)
{
  double d = 0;
  while (ai != aend)
    {
      double dd = *ai++ - *bi++;
      d += dd * dd;
    }
  return d;
}

static void
refine_sine_params_fast (EncoderBlock& audio_block, double mix_freq, int frame, const vector<float>& window)
{
  const size_t frame_size = audio_block.debug_samples.size();

  AlignedArray<float, 16> sin_vec (frame_size);
  AlignedArray<float, 16> cos_vec (frame_size);
  AlignedArray<float, 16> sines (frame_size);
  AlignedArray<float, 16> all_sines (frame_size);

  vector<float> good_freqs;
  vector<float> good_mags;
  vector<float> good_phases;

  // figure out normalization for window
  double window_weight = 0;
  for (size_t i = 0; i < frame_size; i++)
    window_weight += window[i];

  for (size_t i = 0; i < audio_block.freqs.size(); i++)
    {
      VectorSinParams params;

      params.mix_freq = mix_freq;
      params.freq     = audio_block.freqs[i];
      params.mag      = audio_block.mags[i];
      params.phase    = audio_block.phases[i];
      params.mode     = VectorSinParams::ADD;

      fast_vector_sinf (params, &all_sines[0], &all_sines[frame_size]);
    }

  double max_mag;
  size_t partial = 0;
  do
    {
      max_mag = 0;
      // search biggest partial
      for (size_t i = 0; i < audio_block.freqs.size(); i++)
        {
          const double mag = audio_block.mags[i];

          if (mag > max_mag)
            {
              partial = i;
              max_mag = mag;
            }
        }
      // compute reconstruction of that partial
      if (max_mag > 0)
        {
          // remove partial, so we only do each partial once
          double f = audio_block.freqs[partial];

          audio_block.mags[partial] = 0;

          double phase;
          // determine "perfect" phase and magnitude instead of using interpolated fft phase
          double x_re = 0;
          double x_im = 0;

          VectorSinParams params;

          params.mix_freq = mix_freq;
          params.freq = f;
          params.mag = 1;
          params.phase = -((frame_size - 1) / 2.0) * f / mix_freq * 2.0 * M_PI;
          params.phase = normalize_phase (params.phase);
          params.mode = VectorSinParams::REPLACE;

          fast_vector_sincosf (params, &sin_vec[0], &sin_vec[frame_size], &cos_vec[0]);

          params.freq  = f;
          params.mag   = max_mag;
          params.phase = audio_block.phases[partial];
          params.mode  = VectorSinParams::REPLACE;

          fast_vector_sinf (params, &sines[0], &sines[frame_size]);

          for (size_t n = 0; n < frame_size; n++)
            {
              double v = audio_block.debug_samples[n] - all_sines[n] + sines[n];
              v *= window[n];

              // multiply windowed signal with complex exp function from fourier transform:
              //
              //   v * exp (-j * x) = v * (cos (x) - j * sin (x))
              x_re += v * cos_vec[n];
              x_im -= v * sin_vec[n];
            }

          // correct influence of mirrored window (caused by negative frequency component)
          params.mix_freq = mix_freq;
          params.freq = 2 * f;
          params.mag = 1;
          params.phase = -((frame_size - 1) / 2.0) * (2 * f) / mix_freq * 2.0 * M_PI + 0.5 * M_PI;
          params.phase = normalize_phase (params.phase);
          params.mode = VectorSinParams::REPLACE;
          fast_vector_sinf (params, &cos_vec[0], &cos_vec[frame_size]);

          double w2omega = 0;
          for (size_t n = 0; n < frame_size; n++)
            w2omega += window[n] * cos_vec[n];

          x_re *= 2 / (window_weight + w2omega);
          x_im *= 2 / (window_weight - w2omega);

          // compute final magnitude & phase
          double magnitude = sqrt (x_re * x_re + x_im * x_im);
          phase = atan2 (x_im, x_re) + 0.5 * M_PI;
          phase -= (frame_size - 1) / 2.0 / mix_freq * f * 2 * M_PI;
          phase = normalize_phase (phase);

          // restore partial => sines; keep params.freq & params.mix_freq
          params.freq = f;
          params.phase = phase;
          params.mag = magnitude;
          params.mode = VectorSinParams::ADD;
          fast_vector_sinf (params, &sines[0], &sines[frame_size]);

          // store refined freq, mag and phase
          good_freqs.push_back (f);
          good_mags.push_back (magnitude);
          good_phases.push_back (phase);
        }
    }
  while (max_mag > 0);

  audio_block.freqs = good_freqs;
  audio_block.mags = good_mags;
  audio_block.phases = good_phases;
}

static void
remove_small_partials (EncoderBlock& audio_block)
{
  /*
   * this function mainly serves to eliminate side peaks introduced by windowing
   * since these side peaks are typically much smaller than the main peak, we can
   * get rid of them by comparing peaks to the nearest peak, and removing them
   * if the nearest peak is much larger
   */
  vector<double> dbmags;
  for (vector<float>::iterator mi = audio_block.mags.begin(); mi != audio_block.mags.end(); mi++)
    dbmags.push_back (db_from_factor (*mi, -200));

  vector<bool> remove (dbmags.size());

  for (size_t i = 0; i < dbmags.size(); i++)
    {
      for (size_t j = 0; j < dbmags.size(); j++)
        {
          if (i != j)
            {
              double octaves = log (abs (audio_block.freqs[i] - audio_block.freqs[j])) / log (2);
              double mask = -30 - 15 * octaves; /* theoretical values -31 and -18 */
              if (dbmags[j] < dbmags[i] + mask)
                ;
                // remove[j] = true;
            }
        }
    }

  vector<float> good_freqs;
  vector<float> good_mags;
  vector<float> good_phases;

  for (size_t i = 0; i < dbmags.size(); i++)
    {
      if (!remove[i])
        {
          good_freqs.push_back (audio_block.freqs[i]);
          good_mags.push_back (audio_block.mags[i]);
          good_phases.push_back (audio_block.phases[i]);
        }
    }
  audio_block.freqs = good_freqs;
  audio_block.mags = good_mags;
  audio_block.phases = good_phases;
}

/**
 * This function reestimates the magnitudes and phases of the partials found
 * in the previous steps.
 */
void
Encoder::optimize_partials (const vector<float>& window, int optimization_level)
{
  const double mix_freq = enc_params.mix_freq;

  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      if (optimization_level >= 1) // redo FFT estmates, only better
        refine_sine_params_fast (audio_blocks[frame], mix_freq, frame, window);

      remove_small_partials (audio_blocks[frame]);
    }
}

static double
mel_to_hz (double mel)
{
  return 700 * (exp (mel / 1127.0) - 1);
}

static void
approximate_noise_spectrum (int frame,
                            double mix_freq,
                            const vector<double>& spectrum,
			    vector<double>& envelope,
                            double norm)
{
  for (size_t t = 0; t < spectrum.size(); t += 2)
    {
      debug ("noise2red:%d %f\n", frame, sqrt (spectrum[t] * spectrum[t] + spectrum[t + 1] * spectrum[t + 1]));
    }

  size_t bands = envelope.size();
  size_t d = 0;
  for (size_t band = 0; band < envelope.size(); band++)
    {
      double mel_low = 30 + 4000.0 / bands * band;
      double mel_high = 30 + 4000.0 / bands * (band + 1);
      double hz_low = mel_to_hz (mel_low);
      double hz_high = mel_to_hz (mel_high);

      envelope[band] = 0;

      /* skip frequencies which are too low to be in lowest band */
      if (band == 0)
        {
          double f_hz = mix_freq / 2.0 * d / spectrum.size();
          while (f_hz < hz_low)
            {
              d += 2;
              f_hz = mix_freq / 2.0 * d / spectrum.size();
            }
        }
      double f_hz = mix_freq / 2.0 * d / spectrum.size();
      int n_values = 0;
      while (f_hz < hz_high)
        {
          if (d < spectrum.size())
            {
              envelope[band] += (spectrum[d] * spectrum[d] + spectrum[d + 1] * spectrum[d + 1]);
              n_values++;
            }
          d += 2;
          f_hz = mix_freq / 2.0 * d / spectrum.size();
        }
      if (n_values > 0)
        envelope[band] = sqrt (envelope[band] / norm / n_values);
    }
}

static void
xnoise_envelope_to_spectrum (int frame,
                             double mix_freq,
                             const vector<double>& envelope,
			     vector<double>& spectrum,
                             double norm)
{
  vector<int>  band_from_d (spectrum.size());
  vector<int>  band_count (envelope.size());

  size_t bands = envelope.size();
  size_t d = 0;
  /* assign each d to a band */
  std::fill (band_from_d.begin(), band_from_d.end(), -1);
  for (size_t band = 0; band < envelope.size(); band++)
    {
      double mel_low = 30 + 4000.0 / bands * band;
      double mel_high = 30 + 4000.0 / bands * (band + 1);
      double hz_low = mel_to_hz (mel_low);
      double hz_high = mel_to_hz (mel_high);

      /* skip frequencies which are too low to be in lowest band */
      double f_hz = mix_freq / 2.0 * d / spectrum.size();
      if (band == 0)
        {
          while (f_hz < hz_low)
            {
              d += 2;
              f_hz = mix_freq / 2.0 * d / spectrum.size();
            }
        }
      while (f_hz < hz_high && d < spectrum.size())
        {
          if (d < band_from_d.size())
            {
              band_from_d[d] = band;
              band_from_d[d + 1] = band;
            }
          d += 2;
          f_hz = mix_freq / 2.0 * d / spectrum.size();
        }
    }
  /* count bins per band */
  for (size_t band = 0; band < bands; band++)
    {
      for (size_t d = 0; d < spectrum.size(); d += 2)
        {
          size_t b = band_from_d[d];
          if (b == band)
            band_count[b]++;
        }
    }

  for (size_t d = 0; d < spectrum.size(); d += 2)
    {
      int b = band_from_d[d];
      if (b == -1)    /* d is not in a band */
	{
	  spectrum[d] = 0;
	}
      else
	{
	  spectrum[d] = envelope[b] * sqrt (norm);
	}
      spectrum[d+1] = 0;
      debug ("noiseint:%d %f\n", frame, spectrum[d]);
    }
}

/**
 * This function tries to approximate the residual by a spectral envelope
 * for a noise signal.
 */
void
Encoder::approx_noise (const vector<float>& window)
{
  const size_t block_size = enc_params.block_size;
  const size_t frame_size = enc_params.frame_size;
  const size_t zeropad = enc_params.zeropad;

  double sum_w2 = 0;
  for (size_t x = 0; x < frame_size; x++)
    sum_w2 += window[x] * window[x];

  // sum_w2 is the average influence of the window (w[x]^2), multiplied with frame_size
  const double norm = 0.5 * enc_params.mix_freq * sum_w2;

  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      vector<double> noise_envelope (32);
      vector<double> spectrum (audio_blocks[frame].noise.begin(), audio_blocks[frame].noise.end());

      /* A complex FFT would preserve the energy of the input signal exactly; the difference to
       * our (real) FFT is that every value in the complex spectrum occurs twice, once as "positive"
       * frequency, once as "negative" frequency - except for two spectrum values: the value
       * for frequency 0, and the value for frequency mix_freq / 2.
       *
       * To make this FFT energy preserving, we scale those values with a factor of sqrt (2) so
       * that their energy is twice as big (energy == squared value). Then we scale the whole
       * thing with a factor of 0.5, and we get an energy preserving transformation.
       */
      spectrum[0] /= sqrt (2);
      spectrum[spectrum.size() - 2] /= sqrt (2);

      approximate_noise_spectrum (frame, enc_params.mix_freq, spectrum, noise_envelope, norm);

      /// DEBUG CODE {
      const size_t fft_size = block_size * zeropad;
      const double debug_norm = fft_size * 0.5 * sum_w2;

      vector<double> approx_spectrum (fft_size);
      xnoise_envelope_to_spectrum (frame, enc_params.mix_freq, noise_envelope, approx_spectrum, norm);
      for (size_t i = 0; i < approx_spectrum.size(); i += 2)
        debug ("spect_approx:%" PRId64 " %g\n", frame, approx_spectrum[i]);

      double spect_energy = 0;
      for (vector<double>::iterator si = approx_spectrum.begin(); si != approx_spectrum.end(); si++)
        spect_energy += *si * *si / debug_norm;

      double b4_energy = 0;
      for (vector<double>::iterator si = spectrum.begin(); si != spectrum.end(); si++)
        b4_energy += *si * *si / debug_norm;

      double r_energy = 0;
      for (vector<float>::iterator ri = audio_blocks[frame].debug_samples.begin(); ri != audio_blocks[frame].debug_samples.end(); ri++)
        r_energy += *ri * *ri / audio_blocks[frame].debug_samples.size();

      debug ("noiseenergy:%" PRId64 " %f %f %f\n", frame, spect_energy, b4_energy, r_energy);
      /// } DEBUG_CODE
      audio_blocks[frame].noise.assign (noise_envelope.begin(), noise_envelope.end());
    }
}

double
Encoder::attack_error (const vector< vector<double> >& unscaled_signal, const vector<float>& window, const Attack& attack, vector<double>& out_scale)
{
  const size_t frames = unscaled_signal.size();
  double total_error = 0;
  vector<double> decoded_signal (enc_params.frame_size + enc_params.frame_step * frames);
  vector<double> orig_signal (decoded_signal.size());

  for (size_t f = 0; f < frames; f++)
    {
      const vector<double>& frame_signal = unscaled_signal[f];
      size_t zero_values = 0;
      double scale = 1.0;

      for (size_t n = 0; n < frame_signal.size(); n++)
        {
          const double n_ms = f * enc_params.frame_step_ms + n * 1000.0 / enc_params.mix_freq;
          double env;
          if (n_ms < attack.attack_start_ms)
            {
              env = 0;
              zero_values++;

              size_t samples_in_frame = frame_signal.size() - zero_values;
              if (samples_in_frame < (frame_signal.size() / 8))
                {
                  /* if we have very few samples in frame, the partials will
                   * not be reliable, so in this case we cancel out the frame
                   */
                  scale = 0;
                }
              else
                {
                  /* based on an incomplete frame, we boost the partials
                   * to obtain an estimate for one whole frame
                   */
                  scale = frame_signal.size() / double (samples_in_frame);
                }
            }
          else if (n_ms < attack.attack_end_ms)  // during attack
            {
              const double attack_len_ms = attack.attack_end_ms - attack.attack_start_ms;

              env = (n_ms - attack.attack_start_ms) / attack_len_ms;
            }
          else // after attack
            {
              env = 1.0;
            }
          decoded_signal[f * enc_params.frame_step + n] += frame_signal[n] * scale * env * window[n];
          orig_signal[f * enc_params.frame_step + n] = audio_blocks[f].debug_samples[n];
        }
      out_scale[f] = scale;
    }
  for (size_t i = 0; i < decoded_signal.size(); i++)
    {
      double error = orig_signal[i] - decoded_signal[i];
      //printf ("%d %f %f\n", i,orig_signal[i], decoded_signal[i]);
      total_error += error * error;
    }
  return total_error;
}

/**
 * This function computes the optimal attack parameters, by finding the optimal
 * attack envelope (attack_start_ms and attack_end_ms) given the data.
 */
void
Encoder::compute_attack_params (const vector<float>& window)
{
  const double mix_freq   = enc_params.mix_freq;
  const size_t frame_size = enc_params.frame_size;
  const size_t frames = MIN (20, audio_blocks.size());

  vector< vector<double> > unscaled_signal;
  for (size_t f = 0; f < frames; f++)
    {
      const EncoderBlock& audio_block = audio_blocks[f];
      vector<double> frame_signal (frame_size);

      for (size_t partial = 0; partial < audio_block.freqs.size(); partial++)
        {
          const double SA = 0.5;
          double mag   = audio_block.mags[partial] * SA;
          double f     = audio_block.freqs[partial];
          double phase = audio_block.phases[partial];

          // do a phase optimal reconstruction of that partial
          for (size_t n = 0; n < frame_signal.size(); n++)
            {
              frame_signal[n] += sin (phase) * mag;
              phase += f / mix_freq * 2.0 * M_PI;
            }
        }
      unscaled_signal.push_back (frame_signal);
    }

  /* make attack envelope deterministically return the same result for the same input every time */
  Random random;
  random.set_seed (42);

  Attack attack;
  int no_modification = 0;
  double error = 1e7;
  vector<double> scale (frames);

  double zero_values_at_start_ms = zero_values_at_start / mix_freq * 1000;
  attack.attack_start_ms = zero_values_at_start_ms;
  attack.attack_end_ms = zero_values_at_start_ms + 10;
  while (no_modification < 3000)
    {
      double R;
      Attack new_attack = attack;
      if (no_modification < 500)
        R = 100;
      else if (no_modification < 1000)
        R = 20;
      else if (no_modification < 1500)
        R = 1;
      else if (no_modification < 2000)
        R = 0.2;
      else if (no_modification < 2500)
        R = 0.01;
      else
        R = 0.002;

      new_attack.attack_start_ms += random.random_double_range (-R, R);
      new_attack.attack_end_ms += random.random_double_range (-R, R);

      // constrain attack to at least 5ms to avoid clickiness at start
      new_attack.attack_end_ms = max (new_attack.attack_end_ms, new_attack.attack_start_ms + 5);

      if (new_attack.attack_start_ms < new_attack.attack_end_ms &&
          new_attack.attack_start_ms >= zero_values_at_start_ms &&
          new_attack.attack_end_ms < 200)
        {
          const double new_error = attack_error (unscaled_signal, window, new_attack, scale);
#if 0
          printf ("attack=<%f, %f> error=%.17g new_attack=<%f, %f> new_arror=%.17g\n", attack.attack_start_ms, attack.attack_end_ms, error,
                                                                                       new_attack.attack_start_ms, new_attack.attack_end_ms, new_error);
#endif
          if (new_error < error)
            {
              error = new_error;
              attack = new_attack;

              no_modification = 0;
            }
          else
            {
              no_modification++;
            }
        }
    }
  for (size_t f = 0; f < frames; f++)
    {
      for (size_t i = 0; i < audio_blocks[f].mags.size(); i++)
        audio_blocks[f].mags[i] *= scale[f];
    }
  optimal_attack = attack;
}

struct PartialData
{
  float freq;
  float mag;
  float phase;
};

static bool
pd_cmp (const PartialData& p1, const PartialData& p2)
{
  return p1.freq < p2.freq;
}

void
Encoder::sort_freqs()
{
  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      // sort partials by frequency
      vector<PartialData> pvec;

      for (size_t p = 0; p < audio_blocks[frame].freqs.size(); p++)
        {
          PartialData pd;
          pd.freq = audio_blocks[frame].freqs[p];
          pd.mag = audio_blocks[frame].mags[p];
          pd.phase = audio_blocks[frame].phases[p];
          pvec.push_back (pd);
        }
      sort (pvec.begin(), pvec.end(), pd_cmp);

      // replace partial data with sorted partial data
      audio_blocks[frame].freqs.clear();
      audio_blocks[frame].mags.clear();
      audio_blocks[frame].phases.clear();

      for (vector<PartialData>::const_iterator pi = pvec.begin(); pi != pvec.end(); pi++)
        {
          // attack envelope computation produces some partials with mag = 0; we don't need to store these
          if (pi->mag != 0)
            {
              audio_blocks[frame].freqs.push_back (pi->freq);
              audio_blocks[frame].mags.push_back (pi->mag);
              audio_blocks[frame].phases.push_back (pi->phase);
            }
        }
    }
}

/**
 * This function calls all steps necessary for encoding in the right order.
 *
 * \param dhandle a data handle containing the signal to be encoded
 * \param window the analysis window
 * \param optimization_level determines if fast (0), medium (1), or very slow (2) algorithm is used
 * \param attack whether to find the optimal attack parameters
 */
void
Encoder::encode (const WavData& wav_data, int channel, const vector<float>& window, int optimization_level,
                 bool attack, bool track_sines)
{
  compute_stft (wav_data, channel, window);

  if (track_sines)
    {
      search_local_maxima (window);
      link_partials();
      validate_partials();

      optimize_partials (window, optimization_level);

      spectral_subtract (window);
    }
  approx_noise (window);

  if (attack)
    compute_attack_params (window);

  sort_freqs();
}

void
Encoder::set_loop (Audio::LoopType loop_type, int loop_start, int loop_end)
{
  this->loop_type = loop_type;
  this->loop_start = loop_start;
  this->loop_end = loop_end;
}

void
Encoder::set_loop_seconds (Audio::LoopType loop_type, double loop_start_sec, double loop_end_sec)
{
  this->loop_type = loop_type;

  assert (loop_type == Audio::LOOP_FRAME_PING_PONG || loop_type == Audio::LOOP_FRAME_FORWARD);
  loop_start = (loop_start_sec * 1000) / enc_params.frame_step_ms;
  loop_end = (loop_end_sec * 1000) / enc_params.frame_step_ms;
}

static void
convert_freqs_mags_phases (const EncoderBlock& eblock, AudioBlock& ablock, const EncoderParams& enc_params)
{
  const size_t frame_size       = enc_params.frame_size;
  const double mix_freq         = enc_params.mix_freq;
  const double fundamental_freq = enc_params.fundamental_freq;

  ablock.freqs.clear();
  ablock.mags.clear();
  ablock.phases.clear();

  for (size_t i = 0; i < eblock.freqs.size(); i++)
    {
      const uint16_t ifreq  = sm_freq2ifreq (eblock.freqs[i] / fundamental_freq);
      const uint16_t imag   = sm_factor2idb (eblock.mags[i]);

      double xphase = eblock.phases[i];

      // xphase is used (instead of phase) so that restoring the partial with the
      // quantized frequency will produce a minimal error at the center of the frame

      // compute unquantized phase at the center of the frame
      xphase += 2 * M_PI * eblock.freqs[i] / mix_freq * (frame_size - 1) / 2.0;

      // use quantized frequency to compute phase at the beginning of the frame
      xphase -= 2 * M_PI * sm_ifreq2freq (ifreq) * fundamental_freq / mix_freq * (frame_size - 1) / 2.0;

      // => normalize to interval [0..2*pi]
      xphase = normalize_phase (xphase);

      const uint16_t iphase = sm_bound<int> (0, sm_round_positive (xphase / 2 / M_PI * 65536), 65535);

      // corner frequencies are most likely not part of the sound, but analysis
      // errors; so we don't save the smallest/largest possible freq
      if (ifreq != 0 && ifreq != 65535)
        {
          ablock.freqs.push_back (ifreq);
          ablock.mags.push_back (imag);

          if (enc_params.enable_phases)
            ablock.phases.push_back (iphase);
        }
    }
}

static void
convert_noise (const vector<float>& noise, vector<uint16_t>& inoise)
{
  inoise.resize (noise.size());

  for (size_t i = 0; i < noise.size(); i++)
    inoise[i] = sm_factor2idb (noise[i]);
}

/**
 * This function saves the data produced by the encoder to a SpectMorph file.
 */
Error
Encoder::save (const string& filename)
{
  std::unique_ptr<Audio> audio (save_as_audio());

  return audio->save (filename);  // saving can fail
}

/**
 * This function saves the data produced by the encoder, returning a newly
 * allocated Audio object (caller must free this).
 */
Audio *
Encoder::save_as_audio()
{
  Audio *audio = new Audio();

  audio->fundamental_freq = enc_params.fundamental_freq;
  audio->mix_freq = enc_params.mix_freq;
  audio->frame_size_ms = enc_params.frame_size_ms;
  audio->frame_step_ms = enc_params.frame_step_ms;
  audio->attack_start_ms = optimal_attack.attack_start_ms;
  audio->attack_end_ms = optimal_attack.attack_end_ms;
  audio->zero_values_at_start = zero_values_at_start;
  audio->zeropad = enc_params.zeropad;

  for (vector<EncoderBlock>::iterator ai = audio_blocks.begin(); ai != audio_blocks.end(); ai++)
    {
      AudioBlock block;
      convert_freqs_mags_phases (*ai, block, enc_params);
      convert_noise (ai->noise, block.noise);
      block.original_fft = ai->original_fft;
      block.debug_samples = ai->debug_samples;
      audio->contents.push_back (block);
    }
  audio->sample_count = sample_count;
  audio->original_samples = original_samples;
  if (loop_start >= 0 && loop_end >= 0 && loop_type != Audio::LOOP_NONE)
    {
      audio->loop_type = loop_type;
      audio->loop_start = loop_start;
      audio->loop_end = loop_end;

      if (audio->loop_type == Audio::LOOP_TIME_FORWARD || audio->loop_type == Audio::LOOP_TIME_PING_PONG)
        {
          audio->loop_start += zero_values_at_start;
          audio->loop_end += zero_values_at_start;
        }
    }
  return audio;
}

void
Encoder::debug_decode (const string& filename, const vector<float>& enc_window)
{
  const double mix_freq   = enc_params.mix_freq;
  const size_t frame_step = enc_params.frame_step;
  const size_t frame_size = 4 * frame_step + 1;

  size_t pos = 0;
  vector<double> dec_signal;

  vector<double> dec_window (frame_size);
  for (size_t i = 0; i < dec_window.size(); i++)
    dec_window[i] = window_cos (2.0 * i / (frame_size - 1) - 1.0);

  assert (dec_window.size() >= frame_size);

  for (size_t i = 0; i < audio_blocks.size(); i++)
    {
      const EncoderBlock& block = audio_blocks[i];

      dec_signal.resize (pos + frame_size);
      for (size_t partial = 0; partial < block.freqs.size(); partial++)
        {
          const double SA = 0.5;
          const double mag   = block.mags[partial] * SA;
          const double f     = block.freqs[partial];
          double       phase = block.phases[partial];

          // do a phase optimal reconstruction of that partial
          for (size_t n = 0; n < frame_size; n++)
            {
              dec_signal[pos + n] += sin (phase) * mag * dec_window[n];
              phase += f / mix_freq * 2.0 * M_PI;
            }
        }
      pos += frame_step;
    }

  // strip zero values at start:
  std::copy (dec_signal.begin() + zero_values_at_start, dec_signal.end(), dec_signal.begin());
  dec_signal.resize (dec_signal.size() - zero_values_at_start);

  FILE *dec_file = fopen (filename.c_str(), "w");
  if (!dec_file)
    {
      fprintf (stderr, "error: can't open output file '%s'.\n", filename.c_str());
      exit (1);
    }
  for (size_t i = 0; i < dec_signal.size(); i++)
    {
      fprintf (dec_file, "%s", string_printf ("%.17g\n", dec_signal[i]).c_str());
    }
  fclose (dec_file);
}




/**
 * \mainpage SpectMorph Index Page
 *
 * \section intro_sec Introduction
 *
 * SpectMorph is a software which analyzes wav files and builds a frame based model of these wav files,
 * where each frame contains information about the spectrum. Each frame is represented as a sum of sine
 * waves, and a noise component. There are command line tools like smenc and smplay for encoding and
 * decoding SpectMorph models, which are documented in the manual pages. Technically, these tools are frontends
 * to the C++ classes in libspectmorph, which are documented here.
 *
 * \section enc_sec Encoding, loading and saving
 *
 * The encoder is implemented in SpectMorph::Encoder. It can be used to encode an audio file; the frames
 * (short snippets of the audio file, maybe 40 ms or so) are then available as vector containing
 * SpectMorph::AudioBlock objects.
 *
 * Many SpectMorph::AudioBlock objects are needed to represent a whole sound file, and the SpectMorph::Audio
 * class is used to store all parameters of an encoded file, along with the actual frames. Information like
 * the sampling rate (mix_freq) or the original note (fundamental freq) are stored in the SpectMorph::Audio
 * class. Functions for storing and loading SpectMorph::Audio objects exist in namespace SpectMorph::AudioFile.
 *
 * \section dec_sec Decoding
 *
 * The decoding process is frame oriented. For each SpectMorph::AudioBlock, a SpectMorph::Frame object needs
 * to be created. For each frame, a SpectMorph::SineDecoder and a SpectMorph::NoiseDecoder can be used to
 * reconstruct (something which sounds like) the original signal.
 */
