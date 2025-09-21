// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smnoisedecoder.hh"
#include "smmath.hh"
#include "smmain.hh"
#include "smfft.hh"
#include "smblockutils.hh"
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <map>
#include <mutex>

using std::vector;
using std::map;

using namespace SpectMorph;

namespace
{

class NoiseDecoderGlobal
{
public:
  static NoiseDecoderGlobal *
  the()
  {
    static Singleton<NoiseDecoderGlobal> singleton;
    return singleton.ptr();
  }
  std::mutex           mutex;
  map<size_t, float *> cos_window_for_block_size;

  ~NoiseDecoderGlobal()
  {
    for (auto map_it : cos_window_for_block_size)
      FFT::free_array_float (map_it.second);
  }
};

}

static size_t
next_power2 (size_t i)
{
  size_t p = 1;
  while (p < i)
    p *= 2;
  return p;
}

/**
 * Creates a noise decoder object.
 *
 * \param mix_freq        mix freq (sample rate) of the output sample data
 */
NoiseDecoder::NoiseDecoder (double mix_freq, size_t block_size) :
  mix_freq (mix_freq),
  block_size (block_size),
  noise_band_partition (Audio::N_NOISE_BANDS, block_size + 2, mix_freq)
{
  NoiseDecoderGlobal *g = NoiseDecoderGlobal::the();
  std::lock_guard lg (g->mutex);

  /* window initialization: protected by g->mutex */
  float*& win = g->cos_window_for_block_size[block_size];
  if (!win)
    {
      win = FFT::new_array_float (block_size);
      for (size_t i = 0; i < block_size; i++)
        win[i] = window_cos (2.0 * i / block_size - 1.0);
    }
  cos_window = win;

  /* k array initialization: protected by g->mutex */
  make_k_array();

  // 8 values before and after spectrum required by apply_window/SSE
  interpolated_spectrum = FFT::new_array_float (block_size + 18) + 8;

  assert (block_size == next_power2 (block_size));
}

NoiseDecoder::~NoiseDecoder()
{
  FFT::free_array_float (interpolated_spectrum - 8);
}

void
NoiseDecoder::set_seed (int seed)
{
  random_gen.set_seed (seed);
}

/**
 * This function decodes the noise contained in the frame and
 * fills the decoded_residue vector of the frame.
 *
 * \param audio_block   AudioBlock to be decoded
 */
void
NoiseDecoder::process (const uint16_t     *noise_envelope,
                       float              *samples,
                       OutputMode          output_mode,
                       float               portamento_stretch)
{
  assert (noise_band_partition.n_spectrum_bins() == block_size + 2);

  const double Eww = 0.375; // expected value of the energy of the window
  const double norm = mix_freq / (Eww * block_size);

  noise_band_partition.noise_envelope_to_spectrum (random_gen, noise_envelope, interpolated_spectrum, sqrt (norm) / 2);

  if (portamento_stretch > 1.01) // avoid aliasing during portamento
    {
      size_t boundary = 2 * size_t ((block_size / 2) / portamento_stretch);

      zero_float_block (block_size + 2 - boundary, interpolated_spectrum + boundary);
    }

  interpolated_spectrum[1] = interpolated_spectrum[block_size];
  if (output_mode == ADD_SPECTRUM_BH92)
    {
      apply_window_bh92 (interpolated_spectrum, samples);
    }
  else if (output_mode == SET_SPECTRUM_HANN)
    {
      apply_window_hann_overwrite (interpolated_spectrum, samples);
    }
  else if (output_mode == DEBUG_UNWINDOWED)
    {
      float *in = FFT::new_array_float (block_size);
      FFT::fftsr_float (block_size, interpolated_spectrum, in);
      memcpy (samples, in, block_size * sizeof (float));
      FFT::free_array_float (in);
    }
  else if (output_mode == DEBUG_NO_OUTPUT)
    {
    }
  else
    {
      float *in = FFT::new_array_float (block_size);
      FFT::fftsr_float (block_size, interpolated_spectrum, in);

      Block::mul (block_size, in, cos_window);

      if (output_mode == REPLACE)
        memcpy (samples, in, block_size * sizeof (float));
      else if (output_mode == ADD)
        Block::add (block_size, samples, in);
      else
        assert (false);

    #if 0 // DEBUG
      r_energy /= decoded_residue.size();
      double s_energy = 0;
      for (size_t i = 0; i < dinterpolated_spectrum.size(); i++)
        {
          double d = dinterpolated_spectrum[i];
          s_energy += d * d;
        }

      double xs_energy = 0;
      for (vector<double>::const_iterator fni = frame.noise_envelope.begin(); fni != frame.noise_envelope.end(); fni++)
        xs_energy += *fni;

      double i_energy = 0;
      for (size_t i = 0; i < block_size; i++)
        i_energy += interpolated_spectrum[i] * interpolated_spectrum[i] / norm;
      printf ("RE %f SE %f XE %f IE %f\n", r_energy, s_energy, xs_energy, i_energy);
    #endif

      FFT::free_array_float (in);
    }
}

void
NoiseDecoder::precompute_tables()
{
  // trigger FFT planning which can be slow
  float *in = FFT::new_array_float (block_size);
  float *out = FFT::new_array_float (block_size);
  FFT::fftsr_float (block_size, in, out);
  FFT::free_array_float (in);
  FFT::free_array_float (out);
}

size_t
NoiseDecoder::preferred_block_size (double mix_freq)
{
  size_t bs = 1;

  while (bs * 2 / mix_freq < 0.040)    /* block size should not exceed 40ms (and be power of 2) */
    bs *= 2;

  return bs;
}

float *
NoiseDecoder::make_k_array()
{
  const float K0 = 0.35875;   // a0
  const float K1 = 0.244145;  // a1 / 2
  const float K2 = 0.07064;   // a2 / 2
  const float K3 = 0.00584;   // a3 / 2

  const size_t K_ARRAY_SIZE = 4 * 2 * 4;
  static float *k_array = NULL;
  if (!k_array)
    {
      k_array = FFT::new_array_float (K_ARRAY_SIZE);
      const float ks[] = { 0, K3, K2, K1, K0, K1, K2, K3, 0 }; // convolution coefficients for BH92 window
      size_t fi = 0, si = 0;
      for (size_t i = 0; i < K_ARRAY_SIZE; i++)
        {
          bool second = (i / 4) & 1;
          if (second)
            {
              k_array[i] = ks[1 + fi / 2];
              fi++;
            }
          else // second
            {
              k_array[i] = ks[si / 2];
              si++;
            }
        }
    }
  return k_array;
}

void
NoiseDecoder::apply_window_bh92 (float *spectrum, float *fft_buffer)
{
  float *expand_in = spectrum - 8;

  // BS
  expand_in[8 + block_size] = spectrum[1];
  expand_in[9 + block_size] = 0;
  // BS+1
  expand_in[10 + block_size] = spectrum[block_size - 2];
  expand_in[11 + block_size] = -spectrum[block_size - 1];
  // BS+2
  expand_in[12 + block_size] = spectrum[block_size - 4];
  expand_in[13 + block_size] = -spectrum[block_size - 3];
  // BS+3
  expand_in[14 + block_size] = spectrum[block_size - 6];
  expand_in[15 + block_size] = -spectrum[block_size - 5];

  // 0
  expand_in[8] = spectrum[0];
  expand_in[9] = 0;
  // -1
  expand_in[6] = spectrum[2];
  expand_in[7] = -spectrum[3];
  // -2
  expand_in[4] = spectrum[4];
  expand_in[5] = -spectrum[5];
  // -3
  expand_in[2] = spectrum[6];
  expand_in[3] = -spectrum[7];
  // -4 (expand_in[0] and expand_in[1] will be multiplied with 0 and added to the fft_buffer, and therefore should not be NaN)
  expand_in[0] = 0.0;
  expand_in[1] = 0.0;

  const float K0 = 0.35875;   // a0
  const float K1 = 0.244145;  // a1 / 2
  const float K2 = 0.07064;   // a2 / 2
  const float K3 = 0.00584;   // a3 / 2

#if defined(__SSE__) || defined(SM_ARM_SSE) /* fast SSEified convolution */
  if (sm_sse())
    {
#if 0
  for (size_t i = 0; i < K_ARRAY_SIZE; i++)
    {
      printf ("%.8f ", k_array[i]);
      if ((i & 7) == 7)
        printf ("\n");
    }
  printf ("================\n");
#endif
      const __m128 *in = reinterpret_cast<__m128 *> (expand_in);
      const __m128 *k = reinterpret_cast<__m128 *> (make_k_array());
      const __m128 k0 = k[0];
      const __m128 k1 = k[1];
      const __m128 k2 = k[2];
      const __m128 k3 = k[3];
      const __m128 k4 = k[4];
      const __m128 k5 = k[5];
      const __m128 k6 = k[6];
      const __m128 k7 = k[7];

#define CONV(I0,I1,I2,I3,I4,OUT) \
      { \
        __m128 f = _mm_add_ps (_mm_mul_ps (I0, k0), _mm_mul_ps (I1, k2)); \
       __m128 s = _mm_add_ps (_mm_mul_ps (I1, k1), _mm_mul_ps (I2, k3)); \
        f = _mm_add_ps (f, _mm_mul_ps (I2, k4)); \
        s = _mm_add_ps (s, _mm_mul_ps (I3, k5)); \
        f = _mm_add_ps (f, _mm_mul_ps (I3, k6)); \
        s = _mm_add_ps (s, _mm_mul_ps (I4, k7)); \
        const __m128 hi = _mm_shuffle_ps (f, s, _MM_SHUFFLE (3,2,3,2)); \
        const __m128 lo = _mm_shuffle_ps (f, s, _MM_SHUFFLE (1,0,1,0)); \
        OUT = _mm_add_ps (OUT, _mm_add_ps (hi, lo)); \
      }
      size_t i = 0;
      __m128 i0 = in[0];
      __m128 i1 = in[1];
      __m128 i2 = in[2];
      __m128 i3 = in[3];
      __m128 i4;
      while (i < block_size - 20)
        {
          i4 = in[4];
          CONV(i0,i1,i2,i3,i4,*(__m128 *)(fft_buffer + i));

          i0 = in[5];
          CONV(i1,i2,i3,i4,i0,*(__m128 *)(fft_buffer + i + 4));

          i1 = in[6];
          CONV(i2,i3,i4,i0,i1,*(__m128 *)(fft_buffer + i + 8));

          i2 = in[7];
          CONV(i3,i4,i0,i1,i2,*(__m128 *)(fft_buffer + i + 12));

          i3 = in[8];
          CONV(i4,i0,i1,i2,i3,*(__m128 *)(fft_buffer + i + 16));

          in += 5;
          i += 20;
        }
      while (i < block_size)
        {
          const __m128 i0 = in[0];
          const __m128 i1 = in[1];
          const __m128 i2 = in[2];
          const __m128 i3 = in[3];
          const __m128 i4 = in[4];

          CONV(i0,i1,i2,i3,i4,*(__m128 *)(fft_buffer + i));

          in++;
          i += 4;
        }
        {
          const __m128 i0 = in[0];
          const __m128 i1 = in[1];
          const __m128 i2 = in[2];
          const __m128 i3 = in[3];
          const __m128 i4 = in[4];

          // last value (heighest frequency -> store in bin 1)
          F4Vector fft_buffer_last = { { 0.0, 0.0, 0.0, 0.0 } };
          CONV(i0,i1,i2,i3,i4,fft_buffer_last.v);
          fft_buffer[1] += fft_buffer_last.f[0];
        }
    }
  else
#endif
    {
      alignas (16) float spectrum[block_size + 2]; // SSE alignment
      for (size_t i = 8; i < block_size + 2 + 8; i += 2)
        {
          float out_re = K0 * expand_in[i];
          float out_im = K0 * expand_in[i + 1];

          out_re += K1 * (expand_in[i - 2] + expand_in[i + 2]);
          out_im += K1 * (expand_in[i - 1] + expand_in[i + 3]);
          out_re += K2 * (expand_in[i - 4] + expand_in[i + 4]);
          out_im += K2 * (expand_in[i - 3] + expand_in[i + 5]);
          out_re += K3 * (expand_in[i - 6] + expand_in[i + 6]);
          out_im += K3 * (expand_in[i - 5] + expand_in[i + 7]);
          spectrum[i-8] = out_re;
          spectrum[i-7] = out_im;
        }
      spectrum[1] = spectrum[block_size];
      Block::add (block_size, fft_buffer, spectrum);
    }
}

void
NoiseDecoder::apply_window_hann_overwrite (float *spectrum, float *fft_buffer)
{
  float *expand_in = spectrum - 8;

  // BS
  expand_in[8 + block_size] = spectrum[1];
  expand_in[9 + block_size] = 0;
  // BS+1
  expand_in[10 + block_size] = spectrum[block_size - 2];
  expand_in[11 + block_size] = -spectrum[block_size - 1];

  // 0
  expand_in[8] = spectrum[0];
  expand_in[9] = 0;
  // -1
  expand_in[6] = spectrum[2];
  expand_in[7] = -spectrum[3];

  const float K0 = 0.5;   // a0
  const float K1 = 0.25;  // a1 / 2

  for (size_t i = 8; i < block_size + 2 + 8; i += 2)
    {
      float out_re = K0 * expand_in[i];
      float out_im = K0 * expand_in[i + 1];

      out_re += K1 * (expand_in[i - 2] + expand_in[i + 2]);
      out_im += K1 * (expand_in[i - 1] + expand_in[i + 3]);
      fft_buffer[i-8] = out_re;
      fft_buffer[i-7] = out_im;
    }
  fft_buffer[1] = fft_buffer[block_size];
}
