// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smsinedecoder.hh"
#include "smmath.hh"
#include "smfft.hh"
#include "smifftsynth.hh"
#include "smaudio.hh"
#include <birnet/birnetutils.hh>
#include <assert.h>
#include <stdio.h>
#include <math.h>

using SpectMorph::SineDecoder;
using SpectMorph::AudioBlock;
using Birnet::AlignedArray;
using std::vector;

/**
 * \brief Constructor setting up the various decoding parameters
 *
 * @param mix_freq    sample rate to be used for reconstruction
 * @param frame_size  frame size (in samples)
 * @param frame_step  frame step (in samples)
 * @param mode        selects decoding algorithm to be used
 */
SineDecoder::SineDecoder (double mix_freq, size_t frame_size, size_t frame_step, Mode mode)
  : mix_freq (mix_freq),
    frame_size (frame_size),
    frame_step (frame_step),
    mode (mode)
{
  ifft_synth = NULL;
}

SineDecoder::~SineDecoder()
{
  if (ifft_synth)
    {
      delete ifft_synth;
      ifft_synth = NULL;
    }
}

/**
 * \brief Function which decodes a part of the signal.
 *
 * This needs two adjecant frames as arguments.
 *
 * @param frame        the current frame (the frame to be decoded)
 * @param next_frame   the frame after the current frame
 * @param window       the reconstruction window used for MODE_PHASE_SYNC_OVERLAP
 */
void
SineDecoder::process (const AudioBlock& block,
                      const AudioBlock& next_block,
		      const vector<double>& window,
                      vector<float>& decoded_sines)
{
  /* phase synchronous reconstruction (no loops) */
  if (mode == MODE_PHASE_SYNC_OVERLAP)
    {
      AlignedArray<float, 16> aligned_decoded_sines (frame_size);
      for (size_t i = 0; i < block.freqs.size(); i++)
        {
          const double SA = double (frame_step) / double (frame_size) * 2.0;
          const double mag_epsilon = 1e-8;

          VectorSinParams params;
          params.mag = sm_idb2factor (block.mags[i]) * SA;
          if (params.mag > mag_epsilon)
            {
              params.mix_freq = mix_freq;
              params.freq = block.freqs[i];
              params.phase = block.phases[i];
              params.mode = VectorSinParams::ADD;

              fast_vector_sinf (params, &aligned_decoded_sines[0], &aligned_decoded_sines[frame_size]);
            }
        }
      for (size_t t = 0; t < frame_size; t++)
        decoded_sines[t] = aligned_decoded_sines[t] * window[t];
      return;
    }
  else if (mode == MODE_PHASE_SYNC_OVERLAP_IFFT)
    {
      const size_t block_size = frame_size;

      if (!ifft_synth)
        ifft_synth = new IFFTSynth (block_size, mix_freq, IFFTSynth::WIN_HANNING);

      ifft_synth->clear_partials();
      for (size_t i = 0; i < block.freqs.size(); i++)
        {
          const double SA = double (frame_step) / double (frame_size) * 2.0;
          const double mag_epsilon = 1e-8;

          const double mag = block.mags[i] * SA;
          if (mag > mag_epsilon)
            ifft_synth->render_partial (block.freqs[i], mag, fmod (block.phases[i], 2 * M_PI));
        }
      ifft_synth->get_samples (&decoded_sines[0]);
      return;
    }

  zero_float_block (decoded_sines.size(), &decoded_sines[0]);

  /* phase distorted reconstruction */
  vector<float> freqs = block.freqs;
  vector<float> nfreqs = next_block.freqs;

  int todo = freqs.size() + nfreqs.size();

  synth_fixed_phase = next_synth_fixed_phase;
  synth_fixed_phase.resize (freqs.size());
  next_synth_fixed_phase.resize (nfreqs.size());

  const double SIN_AMP = 1.0;
  const bool TRACKING_SYNTH = true;
  while (todo)
    {
      double best_delta = 1e10;
      int best_i = 0, best_j = 0; /* init to get rid of gcc warning */
      for (size_t i = 0; i < freqs.size(); i++)
	{
	  for (size_t j = 0; j < nfreqs.size(); j++)
	    {
	      double delta = fabs (freqs[i] - nfreqs[j]) / freqs[i];
	      if (freqs[i] >= 0 && nfreqs[j] >= 0 && delta < best_delta && delta < 0.1)
		{
		  best_delta = delta;
		  best_i = i;
		  best_j = j;
		}
	    }
	}
      if (best_delta < 0.1)
	{
	  double freq = freqs[best_i];
	  freqs[best_i] = -1;
	  double nfreq = nfreqs[best_j];
	  nfreqs[best_j] = -1;
	  double mag = block.mags[best_i];
	  double nmag = block.mags[best_j];

	  // fprintf (stderr, "%f | %f ==> %f | %f\n", freq, mag, nfreq, nmag);
	  assert (fabs (nfreq - freq) / freq < 0.1);

	  double phase_delta = 2 * M_PI * freq / mix_freq;
	  double nphase_delta = 2 * M_PI * nfreq / mix_freq;
	  double phase = synth_fixed_phase[best_i];
	  if (TRACKING_SYNTH)
	    {
	      for (size_t i = 0; i < frame_step; i++)
		{
		  double inter = i / double (frame_step);

		  decoded_sines [i] += sin (phase) * ((1 - inter) * mag + inter * nmag) * SIN_AMP;
		  phase += (1 - inter) * phase_delta + inter * nphase_delta;
		  while (phase > 2 * M_PI)
		    phase -= 2 * M_PI;
		}
	      next_synth_fixed_phase[best_j] = phase;
	    }
	  else
	    {
	      for (size_t i = 0; i < frame_size; i++)
		{
		  decoded_sines [i] += sin (phase) * window[i] * mag * SIN_AMP;
		  phase += phase_delta;
		  while (phase > 2 * M_PI)
		    phase -= 2 * M_PI;
		  // nfreq phase required -> ramp
		  if (i == frame_step - 1)
		    next_synth_fixed_phase[best_j] = phase;
		}
	    }
	  todo -= 2;
	}
      else
	{
	  for (size_t from = 0; from < freqs.size(); from++)
	    {
	      if (freqs[from] > -1)
		{
		  double freq = freqs[from];
		  freqs[from] = -1;
		  double mag = block.mags[from];

		  // fprintf (stderr, "%f | %f   >>> \n", freq, mag);

		  double phase_delta = 2 * M_PI * freq / mix_freq;
		  double phase = synth_fixed_phase[from];
		  if (TRACKING_SYNTH)
		    {
		      for (size_t i = 0; i < frame_step; i++)
			{
			  double inter = i / double (frame_step);

			  decoded_sines [i] += sin (phase) * (1 - inter) * mag * SIN_AMP;
			  phase += phase_delta;
			  while (phase > 2 * M_PI)
			    phase -= 2 * M_PI;
			}
		    }
		  else
		    {
		      for (size_t i = 0; i < frame_size; i++)
			{
			  decoded_sines [i] += sin (phase) * window[i] * mag * SIN_AMP;
			  phase += phase_delta;
			  while (phase > 2 * M_PI)
			    phase -= 2 * M_PI;
			}
		    }
		  todo--;
		}
	    }
	  for (size_t to = 0; to < nfreqs.size(); to++)
	    {
	      if (nfreqs[to] > -1)
		{
		  double freq = nfreqs[to];
		  nfreqs[to] = -1;
		  double mag = next_block.mags[to];

		  // fprintf (stderr, "%f | %f   <<< \n", freq, mag);

		  double phase_delta = 2 * M_PI * freq / mix_freq;
		  double phase = 0;
		  if (TRACKING_SYNTH)
		    {
		      for (size_t i = 0; i < frame_step; i++)
			{
			  double inter = i / double (frame_step);

			  decoded_sines[i] += sin (phase) * inter * mag * SIN_AMP; /* XXX */
			  phase += phase_delta;
			  while (phase > 2 * M_PI)
			    phase -= 2 * M_PI;
			}
		      next_synth_fixed_phase[to] = phase;
		    }
		  todo--;
		}
	    }
	}
    }
}


