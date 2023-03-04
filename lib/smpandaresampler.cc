// This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0
#include "smpandaresampler.hh"
#include "hiir/Downsampler2xFpu.h"
#include "hiir/Upsampler2xFpu.h"
#ifdef __SSE__
#include "hiir/Downsampler2xSse.h"
#include "hiir/Upsampler2xSse.h"
#endif
#ifdef __SSE__
#include <xmmintrin.h>
#endif
#include <math.h>
#include <string.h>

#ifdef PANDA_RESAMPLER_HEADER_ONLY
#  define PANDA_RESAMPLER_FN inline
#else
#  define PANDA_RESAMPLER_FN
#endif

#define PANDA_RESAMPLER_FN_ALWAYS_INLINE inline __attribute__((always_inline))

#if defined (__ARM_NEON) || defined(__arm64__) || defined(__aarch64__)
#include <arm_neon.h>
#define PANDA_RESAMPLER_NEON
#endif

namespace PandaResampler
{

#ifdef PANDA_RESAMPLER_NEON
/* use NEON instructions for FIR resampler code written for SSE */
typedef float32x4_t __m128;

static PANDA_RESAMPLER_FN_ALWAYS_INLINE
__m128 _mm_mul_ps(__m128 a, __m128 b)
{
  return vmulq_f32(a, b);
}

static PANDA_RESAMPLER_FN_ALWAYS_INLINE
__m128 _mm_add_ps(__m128 a, __m128 b)
{
  return vaddq_f32(a, b);
}
#endif

/* see: http://ds9a.nl/gcc-simd/ */
union F4Vector
{
  float f[4];
#if defined (__SSE__) || defined (PANDA_RESAMPLER_NEON)
  __m128 v;   // vector of four single floats
#endif
};

using std::min;
using std::max;
using std::copy;
using std::vector;

/* --- Resampler2 methods --- */
PANDA_RESAMPLER_FN
Resampler2::Resampler2 (Mode      mode,
                        uint      ratio,
                        Precision precision,
                        bool      use_sse_if_available,
                        Filter    filter)
{
  mode_ = mode;
  ratio_ = ratio;
  precision_ = precision;
  use_sse_if_available_ = use_sse_if_available;
  filter_ = filter;

  init_stage (impl_x2, 2);
  init_stage (impl_x4, 4);
  init_stage (impl_x8, 8);
}

PANDA_RESAMPLER_FN
void
Resampler2::init_stage (std::unique_ptr<Impl>& impl,
                        uint                   stage_ratio)
{
  /* only allocate/initialize stage if necessary */
  if (stage_ratio > ratio_ || impl)
    return;

  if (sse_available() && use_sse_if_available_)
    {
      switch (filter_)
        {
          case FILTER_FIR: impl.reset (create_impl<true> (stage_ratio));
                           break;
          case FILTER_IIR: impl.reset (create_impl_iir<true> (stage_ratio));
                           break;
        }
    }
  else
    {
      switch (filter_)
        {
          case FILTER_FIR: impl.reset (create_impl<false> (stage_ratio));
                           break;
          case FILTER_IIR: impl.reset (create_impl_iir<false> (stage_ratio));
                           break;
        }
    }
  // should have created an implementation at this point
  PANDA_RESAMPLER_CHECK (impl.get());
}

PANDA_RESAMPLER_FN
bool
Resampler2::sse_available()
{
#if defined (__SSE__) || defined (PANDA_RESAMPLER_NEON)
  return true;
#else
  return false;
#endif
}

PANDA_RESAMPLER_FN
Resampler2::Precision
Resampler2::find_precision_for_bits (uint bits)
{
  if (bits <= 1)
    return PREC_LINEAR;
  if (bits <= 8)
    return PREC_48DB;
  if (bits <= 12)
    return PREC_72DB;
  if (bits <= 16)
    return PREC_96DB;
  if (bits <= 20)
    return PREC_120DB;

  /* thats the best precision we can deliver (and by the way also close to
   * the best precision possible with floats anyway)
   */
  return PREC_144DB;
}

PANDA_RESAMPLER_FN
const char *
Resampler2::precision_name (Precision precision)
{
  switch (precision)
  {
  case PREC_LINEAR:  return "linear interpolation";
  case PREC_48DB:    return "8 bit (48dB)";
  case PREC_72DB:    return "12 bit (72dB)";
  case PREC_96DB:    return "16 bit (96dB)";
  case PREC_120DB:   return "20 bit (120dB)";
  case PREC_144DB:   return "24 bit (144dB)";
  default:			    return "unknown precision enum value";
  }
}

namespace Aux {

/*
 * FIR filter routine
 *
 * A FIR filter has the characteristic that it has a finite impulse response,
 * and can be computed by convolution of the input signal with that finite
 * impulse response.
 *
 * Thus, we use this for computing the output of the FIR filter
 *
 * output = input[0] * taps[0] + input[1] * taps[1] + ... + input[N-1] * taps[N-1]
 *
 * where input is the input signal, taps are the filter coefficients, in
 * other texts sometimes called h[0]..h[N-1] (impulse response) or a[0]..a[N-1]
 * (non recursive part of a digital filter), and N is the filter order.
 */
template<class Accumulator> static PANDA_RESAMPLER_FN_ALWAYS_INLINE
Accumulator
fir_process_one_sample (const float *input,
                        const float *taps, /* [0..order-1] */
			const uint   order)
{
  Accumulator out = 0;
  for (uint i = 0; i < order; i++)
    out += input[i] * taps[i];
  return out;
}

/*
 * FIR filter routine for 4 samples simultaneously
 *
 * This routine produces (approximately) the same result as fir_process_one_sample
 * but computes four consecutive output values at once using vectorized SSE
 * instructions. Note that input and sse_taps need to be 16-byte aligned here.
 *
 * Also note that sse_taps is not a plain impulse response here, but a special
 * version that needs to be computed with fir_compute_sse_taps.
 */
static PANDA_RESAMPLER_FN_ALWAYS_INLINE
void
fir_process_4samples_sse (const float *input,
                          const float *sse_taps,
			  const uint   order,
			  float       *out0,
			  float       *out1,
			  float       *out2,
			  float       *out3)
{
#if defined (__SSE__) || defined (PANDA_RESAMPLER_NEON)
  /* input and taps must be 16-byte aligned */
  const F4Vector *input_v = reinterpret_cast<const F4Vector *> (input);
  const F4Vector *sse_taps_v = reinterpret_cast<const F4Vector *> (sse_taps);
  F4Vector out0_v, out1_v, out2_v, out3_v;

  out0_v.v = _mm_mul_ps (input_v[0].v, sse_taps_v[0].v);
  out1_v.v = _mm_mul_ps (input_v[0].v, sse_taps_v[1].v);
  out2_v.v = _mm_mul_ps (input_v[0].v, sse_taps_v[2].v);
  out3_v.v = _mm_mul_ps (input_v[0].v, sse_taps_v[3].v);

  for (uint i = 1; i < (order + 6) / 4; i++)
    {
      out0_v.v = _mm_add_ps (out0_v.v, _mm_mul_ps (input_v[i].v, sse_taps_v[i * 4 + 0].v));
      out1_v.v = _mm_add_ps (out1_v.v, _mm_mul_ps (input_v[i].v, sse_taps_v[i * 4 + 1].v));
      out2_v.v = _mm_add_ps (out2_v.v, _mm_mul_ps (input_v[i].v, sse_taps_v[i * 4 + 2].v));
      out3_v.v = _mm_add_ps (out3_v.v, _mm_mul_ps (input_v[i].v, sse_taps_v[i * 4 + 3].v));
    }

  *out0 = out0_v.f[0] + out0_v.f[1] + out0_v.f[2] + out0_v.f[3];
  *out1 = out1_v.f[0] + out1_v.f[1] + out1_v.f[2] + out1_v.f[3];
  *out2 = out2_v.f[0] + out2_v.f[1] + out2_v.f[2] + out2_v.f[3];
  *out3 = out3_v.f[0] + out3_v.f[1] + out3_v.f[2] + out3_v.f[3];
#else
  PANDA_RESAMPLER_CHECK(false); // should not be reached
#endif
}


/*
 * fir_compute_sse_taps takes a normal vector of FIR taps as argument and
 * computes a specially scrambled version of these taps, ready to be used
 * for SSE operations (by fir_process_4samples_sse).
 *
 * we require a special ordering of the FIR taps, to get maximum benefit of the SSE operations
 *
 * example: suppose the FIR taps are [ x1 x2 x3 x4 x5 x6 x7 x8 x9 ], then the SSE taps become
 *
 * [ x1 x2 x3 x4   0 x1 x2 x3   0  0 x1 x2   0  0  0 x1      <- for input[0]
 *   x5 x6 x7 x8  x4 x5 x6 x7  x3 x4 x5 x6  x2 x3 x4 x5      <- for input[1]
 *   x9  0  0  0  x8 x9  0  0  x7 x8 x9  0  x6 x7 x8 x9 ]    <- for input[2]
 * \------------/\-----------/\-----------/\-----------/
 *    for out0     for out1      for out2     for out3
 *
 * so that we can compute out0, out1, out2 and out3 simultaneously
 * from input[0]..input[2]
 */
static inline vector<float>
fir_compute_sse_taps (const vector<float>& taps)
{
  const int order = taps.size();
  vector<float> sse_taps ((order + 6) / 4 * 16);

  for (int j = 0; j < 4; j++)
    for (int i = 0; i < order; i++)
      {
	int k = i + j;
	sse_taps[(k / 4) * 16 + (k % 4) + j * 4] = taps[i];
      }

  return sse_taps;
}

/*
 * This function tests the SSEified FIR filter code (that is, the reordering
 * done by fir_compute_sse_taps and the actual computation implemented in
 * fir_process_4samples_sse).
 *
 * It prints diagnostic information, and returns true if the filter
 * implementation works correctly, and false otherwise. The maximum filter
 * order to be tested can be optionally specified as argument.
 */
static inline bool
fir_test_filter_sse (bool       verbose,
                     const uint max_order = 64)
{
  int errors = 0;
  if (verbose)
    printf ("testing SSE filter implementation:\n\n");

  for (uint order = 0; order < max_order; order++)
    {
      vector<float> taps (order);
      for (uint i = 0; i < order; i++)
	taps[i] = i + 1;

      AlignedArray<float> sse_taps (fir_compute_sse_taps (taps));
      if (verbose)
	{
	  for (uint i = 0; i < sse_taps.size(); i++)
	    {
	      printf ("%3d", (int) (sse_taps[i] + 0.5));
	      if (i % 4 == 3)
		printf ("  |");
	      if (i % 16 == 15)
		printf ("   ||| upper bound = %d\n", (order + 6) / 4);
	    }
	  printf ("\n\n");
	}

      AlignedArray<float> random_mem (order + 6);
      for (uint i = 0; i < order + 6; i++)
	random_mem[i] = 1.0 - rand() / (0.5 * RAND_MAX);

      /* FIXME: the problem with this test is that we explicitely test SSE code
       * here, but the test case is not compiled with -msse within the BEAST tree
       */
      float out[4];
      fir_process_4samples_sse (&random_mem[0], &sse_taps[0], order,
	                        &out[0], &out[1], &out[2], &out[3]);

      double avg_diff = 0.0;
      for (int i = 0; i < 4; i++)
	{
	  double diff = fir_process_one_sample<double> (&random_mem[i], &taps[0], order) - out[i];
	  avg_diff += fabs (diff);
	}
      avg_diff /= (order + 1);
      bool is_error = (avg_diff > 0.00001);
      if (is_error || verbose)
	printf ("*** order = %d, avg_diff = %g\n", order, avg_diff);
      if (is_error)
	errors++;
    }
  if (errors)
    printf ("*** %d errors detected\n", errors);

  return (errors == 0);
}

} // Aux

using namespace Aux; // avoid anon namespace

/*
 * Factor 2 upsampling of a data stream
 *
 * Template arguments:
 *   ORDER     number of resampling filter coefficients
 *   USE_SSE   whether to use SSE (vectorized) instructions or not
 */
template<uint ORDER, bool USE_SSE>
class Resampler2::Upsampler2 final : public Resampler2::Impl {
  vector<float>       taps;
  AlignedArray<float> history;
  AlignedArray<float> sse_taps;
protected:
  /* fast SSE optimized convolution */
  PANDA_RESAMPLER_FN_ALWAYS_INLINE
  void
  process_4samples_aligned (const float *input /* aligned */,
                            float       *output)
  {
    const uint H = (ORDER / 2); /* half the filter length */

    output[1] = input[H];
    output[3] = input[H + 1];
    output[5] = input[H + 2];
    output[7] = input[H + 3];

    fir_process_4samples_sse (input, &sse_taps[0], ORDER, &output[0], &output[2], &output[4], &output[6]);
  }
  /* slow convolution */
  PANDA_RESAMPLER_FN_ALWAYS_INLINE
  void
  process_sample_unaligned (const float *input,
                            float       *output)
  {
    const uint H = (ORDER / 2); /* half the filter length */
    output[0] = fir_process_one_sample<float> (&input[0], &taps[0], ORDER);
    output[1] = input[H];
  }
  PANDA_RESAMPLER_FN_ALWAYS_INLINE
  void
  process_block_aligned (const float *input,
                         uint         n_input_samples,
			 float       *output)
  {
    uint i = 0;
    if (USE_SSE)
      {
	while (i + 3 < n_input_samples)
	  {
	    process_4samples_aligned (&input[i], &output[i*2]);
	    i += 4;
	  }
      }
    while (i < n_input_samples)
      {
	process_sample_unaligned (&input[i], &output[2*i]);
	i++;
      }
  }
  PANDA_RESAMPLER_FN_ALWAYS_INLINE
  void
  process_block_unaligned (const float *input,
                           uint         n_input_samples,
			   float       *output)
  {
    uint i = 0;
    if (USE_SSE)
      {
	while ((reinterpret_cast<ptrdiff_t> (&input[i]) & 15) && i < n_input_samples)
	  {
	    process_sample_unaligned (&input[i], &output[2 * i]);
	    i++;
	  }
      }
    process_block_aligned (&input[i], n_input_samples - i, &output[2 * i]);
  }
public:
  /*
   * Constructs an Upsampler2 object with a given set of filter coefficients.
   *
   * init_taps: coefficients for the upsampling FIR halfband filter
   */
  Upsampler2 (float *init_taps) :
    taps (init_taps, init_taps + ORDER),
    history (2 * ORDER),
    sse_taps (fir_compute_sse_taps (taps))
  {
    PANDA_RESAMPLER_CHECK ((ORDER & 1) == 0);    /* even order filter */
  }
  /*
   * The function process_block() takes a block of input samples and produces a
   * block with twice the length, containing interpolated output samples.
   */
  void
  process_block (const float *input,
                 uint         n_input_samples,
		 float       *output) override
  {
    const uint history_todo = min (n_input_samples, ORDER - 1);

    copy (input, input + history_todo, &history[ORDER - 1]);
    process_block_aligned (&history[0], history_todo, output);
    if (n_input_samples > history_todo)
      {
	process_block_unaligned (input, n_input_samples - history_todo, &output [2 * history_todo]);

	// build new history from new input
	copy (input + n_input_samples - history_todo, input + n_input_samples, &history[0]);
      }
    else
      {
	// build new history from end of old history
	// (very expensive if n_input_samples tends to be a lot smaller than ORDER often)
	memmove (&history[0], &history[n_input_samples], sizeof (history[0]) * (ORDER - 1));
      }
  }
  /*
   * Returns the FIR filter order.
   */
  uint
  order() const override
  {
    return ORDER;
  }
  double
  delay() const override
  {
    return order() - 1;
  }
  void
  reset() override
  {
    std::fill (history.begin(), history.end(), 0.0);
  }
  bool
  sse_enabled() const override
  {
    return USE_SSE;
  }
};

/*
 * Factor 2 downsampling of a data stream
 *
 * Template arguments:
 *   ORDER    number of resampling filter coefficients
 *   USE_SSE  whether to use SSE (vectorized) instructions or not
 */
template<uint ORDER, bool USE_SSE>
class Resampler2::Downsampler2 final : public Resampler2::Impl {
  vector<float>        taps;
  AlignedArray<float> history_even;
  AlignedArray<float> history_odd;
  AlignedArray<float> sse_taps;
  /* fast SSE optimized convolution */
  template<int ODD_STEPPING> PANDA_RESAMPLER_FN_ALWAYS_INLINE
  void
  process_4samples_aligned (const float *input_even /* aligned */,
                            const float *input_odd,
			    float       *output)
  {
    const uint H = (ORDER / 2) - 1; /* half the filter length */

    fir_process_4samples_sse (input_even, &sse_taps[0], ORDER, &output[0], &output[1], &output[2], &output[3]);

    output[0] += 0.5f * input_odd[H * ODD_STEPPING];
    output[1] += 0.5f * input_odd[(H + 1) * ODD_STEPPING];
    output[2] += 0.5f * input_odd[(H + 2) * ODD_STEPPING];
    output[3] += 0.5f * input_odd[(H + 3) * ODD_STEPPING];
  }
  /* slow convolution */
  template<int ODD_STEPPING> PANDA_RESAMPLER_FN_ALWAYS_INLINE
  float
  process_sample_unaligned (const float *input_even,
                            const float *input_odd)
  {
    const uint H = (ORDER / 2) - 1; /* half the filter length */

    return fir_process_one_sample<float> (&input_even[0], &taps[0], ORDER) + 0.5f * input_odd[H * ODD_STEPPING];
  }
  template<int ODD_STEPPING> PANDA_RESAMPLER_FN_ALWAYS_INLINE
  void
  process_block_aligned (const float *input_even,
                         const float *input_odd,
			 float       *output,
			 uint         n_output_samples)
  {
    uint i = 0;
    if (USE_SSE)
      {
	while (i + 3 < n_output_samples)
	  {
	    process_4samples_aligned<ODD_STEPPING> (&input_even[i], &input_odd[i * ODD_STEPPING], &output[i]);
	    i += 4;
	  }
      }
    while (i < n_output_samples)
      {
	output[i] = process_sample_unaligned<ODD_STEPPING> (&input_even[i], &input_odd[i * ODD_STEPPING]);
	i++;
      }
  }
  template<int ODD_STEPPING> PANDA_RESAMPLER_FN_ALWAYS_INLINE
  void
  process_block_unaligned (const float *input_even,
                           const float *input_odd,
			   float       *output,
			   uint         n_output_samples)
  {
    uint i = 0;
    if (USE_SSE)
      {
	while ((reinterpret_cast<ptrdiff_t> (&input_even[i]) & 15) && i < n_output_samples)
	  {
	    output[i] = process_sample_unaligned<ODD_STEPPING> (&input_even[i], &input_odd[i * ODD_STEPPING]);
	    i++;
	  }
      }
    process_block_aligned<ODD_STEPPING> (&input_even[i], &input_odd[i * ODD_STEPPING], &output[i], n_output_samples);
  }
  void
  deinterleave2 (const float *data,
                 uint         n_data_values,
		 float       *output)
  {
    for (uint i = 0; i < n_data_values; i += 2)
      output[i / 2] = data[i];
  }
public:
  /*
   * Constructs a Downsampler2 class using a given set of filter coefficients.
   *
   * init_taps: coefficients for the downsampling FIR halfband filter
   */
  Downsampler2 (float *init_taps) :
    taps (init_taps, init_taps + ORDER),
    history_even (2 * ORDER),
    history_odd (2 * ORDER),
    sse_taps (fir_compute_sse_taps (taps))
  {
    PANDA_RESAMPLER_CHECK ((ORDER & 1) == 0);    /* even order filter */
  }
  /*
   * The function process_block() takes a block of input samples and produces
   * a block with half the length, containing downsampled output samples.
   */
  void
  process_block (const float *input,
                 uint         n_input_samples,
		 float       *output) override
  {
    if (!PANDA_RESAMPLER_CHECK ((n_input_samples & 1) == 0))
      return;

    const uint BLOCKSIZE = 1024;

    F4Vector  block[BLOCKSIZE / 4]; /* using F4Vector ensures 16-byte alignment */
    float    *input_even = &block[0].f[0];

    while (n_input_samples)
      {
	uint n_input_todo = min (n_input_samples, BLOCKSIZE * 2);

        /* since the halfband filter contains zeros every other sample
	 * and since we're using SSE instructions, which expect the
	 * data to be consecutively represented in memory, we prepare
	 * a block of samples containing only even-indexed samples
	 *
	 * we keep the deinterleaved data on the stack (instead of per-class
	 * allocated memory), to ensure that even running a lot of these
	 * downsampler streams will not result in cache trashing
	 *
         * FIXME: this implementation is suboptimal for non-SSE, because it
	 * performs an extra deinterleaving step in any case, but deinterleaving
	 * is only required for SSE instructions
	 */
	deinterleave2 (input, n_input_todo, input_even);

	const float       *input_odd = input + 1; /* we process this one with a stepping of 2 */

	const uint n_output_todo = n_input_todo / 2;
	const uint history_todo = min (n_output_todo, ORDER - 1);

	copy (input_even, input_even + history_todo, &history_even[ORDER - 1]);
	deinterleave2 (input_odd, history_todo * 2, &history_odd[ORDER - 1]);

	process_block_aligned <1> (&history_even[0], &history_odd[0], output, history_todo);
	if (n_output_todo > history_todo)
	  {
	    process_block_unaligned<2> (input_even, input_odd, &output[history_todo], n_output_todo - history_todo);

	    // build new history from new input (here: history_todo == ORDER - 1)
	    copy (input_even + n_output_todo - history_todo, input_even + n_output_todo, &history_even[0]);
	    deinterleave2 (input_odd + n_input_todo - history_todo * 2, history_todo * 2, &history_odd[0]); /* FIXME: can be optimized */
	  }
	else
	  {
	    // build new history from end of old history
	    // (very expensive if n_output_todo tends to be a lot smaller than ORDER often)
	    memmove (&history_even[0], &history_even[n_output_todo], sizeof (history_even[0]) * (ORDER - 1));
	    memmove (&history_odd[0], &history_odd[n_output_todo], sizeof (history_odd[0]) * (ORDER - 1));
	  }

	n_input_samples -= n_input_todo;
	input += n_input_todo;
	output += n_output_todo;
      }
  }
  /*
   * Returns the filter order.
   */
  uint
  order() const override
  {
    return ORDER;
  }
  double
  delay() const override
  {
    return order() / 2 - 0.5;
  }
  void
  reset() override
  {
    std::fill (history_even.begin(), history_even.end(), 0.0);
    std::fill (history_odd.begin(), history_odd.end(), 0.0);
  }
  bool
  sse_enabled() const override
  {
    return USE_SSE;
  }
};

template<bool USE_SSE> Resampler2::Impl*
Resampler2::create_impl (uint stage_ratio)
{
  // START generated code
  static constexpr double coeffs2_24[52] =
  {
    -1.896649020687189e-07,
    8.9375397078734594e-07,
    -2.9108958281584469e-06,
    7.7609698680954118e-06,
    -1.8119956302920787e-05,
    3.8375105842230682e-05,
    -7.5309071659470099e-05,
    0.00013889694095199994,
    -0.00024318689587188649,
    0.00040722732084584021,
    -0.00065600665137277646,
    0.001021395018417595,
    -0.0015431295374089733,
    0.0022699842719303941,
    -0.0032614400850958444,
    0.0045904754831414861,
    -0.0063486601639801904,
    0.0086558345711950594,
    -0.011679015470590512,
    0.015670714074410223,
    -0.021051408534455165,
    0.028604536062639324,
    -0.040008178410666853,
    0.059644444619411124,
    -0.10364569487012831,
    0.31748269557078473,
    0.31748269557078473,
    -0.10364569487012831,
    0.059644444619411124,
    -0.040008178410666853,
    0.028604536062639324,
    -0.021051408534455165,
    0.015670714074410223,
    -0.011679015470590512,
    0.0086558345711950594,
    -0.0063486601639801904,
    0.0045904754831414861,
    -0.0032614400850958444,
    0.0022699842719303941,
    -0.0015431295374089733,
    0.001021395018417595,
    -0.00065600665137277646,
    0.00040722732084584021,
    -0.00024318689587188649,
    0.00013889694095199994,
    -7.5309071659470099e-05,
    3.8375105842230682e-05,
    -1.8119956302920787e-05,
    7.7609698680954118e-06,
    -2.9108958281584469e-06,
    8.9375397078734594e-07,
    -1.896649020687189e-07,
  };
  if (stage_ratio == 2 && precision_ == 24 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<52, USE_SSE> > (coeffs2_24, 52, 2.0);
  if (stage_ratio == 2 && precision_ == 24 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<52, USE_SSE> > (coeffs2_24, 52, 1.0);
  static constexpr double coeffs4_24[16] =
  {
    -7.8113862062895476e-06,
    0.00011777177340253331,
    -0.00080979027642968178,
    0.0035914299596685019,
    -0.011907675232141868,
    0.032618531462663004,
    -0.083680449562963555,
    0.31007801153486508,
    0.31007801153486508,
    -0.083680449562963555,
    0.032618531462663004,
    -0.011907675232141868,
    0.0035914299596685019,
    -0.00080979027642968178,
    0.00011777177340253331,
    -7.8113862062895476e-06,
  };
  if (stage_ratio == 4 && precision_ == 24 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<16, USE_SSE> > (coeffs4_24, 16, 2.0);
  if (stage_ratio == 4 && precision_ == 24 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<16, USE_SSE> > (coeffs4_24, 16, 1.0);
  static constexpr double coeffs8_24[12] =
  {
    -3.0345557546583312e-05,
    0.00057384655742621254,
    -0.0043830674681261195,
    0.020226878808907018,
    -0.070915964811982507,
    0.30452864347609143,
    0.30452864347609143,
    -0.070915964811982507,
    0.020226878808907018,
    -0.0043830674681261195,
    0.00057384655742621254,
    -3.0345557546583312e-05,
  };
  if (stage_ratio == 8 && precision_ == 24 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<12, USE_SSE> > (coeffs8_24, 12, 2.0);
  if (stage_ratio == 8 && precision_ == 24 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<12, USE_SSE> > (coeffs8_24, 12, 1.0);
  static constexpr double coeffs2_20[42] =
  {
    2.4629216796772203e-06,
    -9.7990456966301657e-06,
    2.8137964376908064e-05,
    -6.7464881821884139e-05,
    0.00014336862073315604,
    -0.000278702763263149,
    0.00050528391212770792,
    -0.00086555167177917124,
    0.001414202687011217,
    -0.0022199926486379607,
    0.003368236285452741,
    -0.0049651473206043309,
    0.0071463283371987034,
    -0.010094148846828722,
    0.014074361299068285,
    -0.019516906264665349,
    0.0272094990821482,
    -0.038828382182376338,
    0.058747406804456566,
    -0.10308466896169781,
    0.31729177522077334,
    0.31729177522077334,
    -0.10308466896169781,
    0.058747406804456566,
    -0.038828382182376338,
    0.0272094990821482,
    -0.019516906264665349,
    0.014074361299068285,
    -0.010094148846828722,
    0.0071463283371987034,
    -0.0049651473206043309,
    0.003368236285452741,
    -0.0022199926486379607,
    0.001414202687011217,
    -0.00086555167177917124,
    0.00050528391212770792,
    -0.000278702763263149,
    0.00014336862073315604,
    -6.7464881821884139e-05,
    2.8137964376908064e-05,
    -9.7990456966301657e-06,
    2.4629216796772203e-06,
  };
  if (stage_ratio == 2 && precision_ == 20 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<42, USE_SSE> > (coeffs2_20, 42, 2.0);
  if (stage_ratio == 2 && precision_ == 20 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<42, USE_SSE> > (coeffs2_20, 42, 1.0);
  static constexpr double coeffs4_20[14] =
  {
    4.3979674631863943e-05,
    -0.00050306469192140939,
    0.0027962504087410051,
    -0.010470114594085408,
    0.030755143193163859,
    -0.082041866707154076,
    0.30941949170527272,
    0.30941949170527272,
    -0.082041866707154076,
    0.030755143193163859,
    -0.010470114594085408,
    0.0027962504087410051,
    -0.00050306469192140939,
    4.3979674631863943e-05,
  };
  if (stage_ratio == 4 && precision_ == 20 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<14, USE_SSE> > (coeffs4_20, 14, 2.0);
  if (stage_ratio == 4 && precision_ == 20 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<14, USE_SSE> > (coeffs4_20, 14, 1.0);
  static constexpr double coeffs8_20[10] =
  {
    0.00017230594713343064,
    -0.002551731819446271,
    0.015994679393099207,
    -0.06565066677090392,
    0.30203532650661641,
    0.30203532650661641,
    -0.06565066677090392,
    0.015994679393099207,
    -0.002551731819446271,
    0.00017230594713343064,
  };
  if (stage_ratio == 8 && precision_ == 20 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<10, USE_SSE> > (coeffs8_20, 10, 2.0);
  if (stage_ratio == 8 && precision_ == 20 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<10, USE_SSE> > (coeffs8_20, 10, 1.0);
  static constexpr double coeffs2_16[32] =
  {
    -3.5142734993474452e-05,
    0.00011358789579768951,
    -0.00028037958674221312,
    0.00059293788160310029,
    -0.0011294540908177168,
    0.0019917372726928405,
    -0.0033089217809884803,
    0.0052441962508097874,
    -0.0080093910223938553,
    0.011898075091115748,
    -0.017362581771322445,
    0.025204120984314127,
    -0.037100932061169559,
    0.057416112765805175,
    -0.10224463341225128,
    0.31700464719515836,
    0.31700464719515836,
    -0.10224463341225128,
    0.057416112765805175,
    -0.037100932061169559,
    0.025204120984314127,
    -0.017362581771322445,
    0.011898075091115748,
    -0.0080093910223938553,
    0.0052441962508097874,
    -0.0033089217809884803,
    0.0019917372726928405,
    -0.0011294540908177168,
    0.00059293788160310029,
    -0.00028037958674221312,
    0.00011358789579768951,
    -3.5142734993474452e-05,
  };
  if (stage_ratio == 2 && precision_ == 16 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<32, USE_SSE> > (coeffs2_16, 32, 2.0);
  if (stage_ratio == 2 && precision_ == 16 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<32, USE_SSE> > (coeffs2_16, 32, 1.0);
  static constexpr double coeffs4_16[10] =
  {
    0.00055713256761683592,
    -0.0048543666906354314,
    0.021809335002826412,
    -0.073141515220609826,
    0.30562814686107148,
    0.30562814686107148,
    -0.073141515220609826,
    0.021809335002826412,
    -0.0048543666906354314,
    0.00055713256761683592,
  };
  if (stage_ratio == 4 && precision_ == 16 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<10, USE_SSE> > (coeffs4_16, 10, 2.0);
  if (stage_ratio == 4 && precision_ == 16 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<10, USE_SSE> > (coeffs4_16, 10, 1.0);
  static constexpr double coeffs8_16[8] =
  {
    -0.0010885239331601664,
    0.011649378449320126,
    -0.059557473859641011,
    0.29900137394294291,
    0.29900137394294291,
    -0.059557473859641011,
    0.011649378449320126,
    -0.0010885239331601664,
  };
  if (stage_ratio == 8 && precision_ == 16 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<8, USE_SSE> > (coeffs8_16, 8, 2.0);
  if (stage_ratio == 8 && precision_ == 16 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<8, USE_SSE> > (coeffs8_16, 8, 1.0);
  static constexpr double coeffs2_12[24] =
  {
    -0.00031919473602139891,
    0.00083854004488146685,
    -0.0017905303270299995,
    0.0033731130894814397,
    -0.0058408892523820347,
    0.0095314079754320758,
    -0.014935625615431904,
    0.022881655727240446,
    -0.035057791064865292,
    0.055817399757994692,
    -0.10122584733772397,
    0.31665472430452679,
    0.31665472430452679,
    -0.10122584733772397,
    0.055817399757994692,
    -0.035057791064865292,
    0.022881655727240446,
    -0.014935625615431904,
    0.0095314079754320758,
    -0.0058408892523820347,
    0.0033731130894814397,
    -0.0017905303270299995,
    0.00083854004488146685,
    -0.00031919473602139891,
  };
  if (stage_ratio == 2 && precision_ == 12 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<24, USE_SSE> > (coeffs2_12, 24, 2.0);
  if (stage_ratio == 2 && precision_ == 12 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<24, USE_SSE> > (coeffs2_12, 24, 1.0);
  static constexpr double coeffs4_12[8] =
  {
    -0.0025910542040449157,
    0.017312836258421893,
    -0.068161446853832547,
    0.30340841235941457,
    0.30340841235941457,
    -0.068161446853832547,
    0.017312836258421893,
    -0.0025910542040449157,
  };
  if (stage_ratio == 4 && precision_ == 12 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<8, USE_SSE> > (coeffs4_12, 8, 2.0);
  if (stage_ratio == 4 && precision_ == 12 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<8, USE_SSE> > (coeffs4_12, 8, 1.0);
  static constexpr double coeffs8_12[6] =
  {
    0.005872148420194066,
    -0.049275035331134497,
    0.29336813890765762,
    0.29336813890765762,
    -0.049275035331134497,
    0.005872148420194066,
  };
  if (stage_ratio == 8 && precision_ == 12 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<6, USE_SSE> > (coeffs8_12, 6, 2.0);
  if (stage_ratio == 8 && precision_ == 12 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<6, USE_SSE> > (coeffs8_12, 6, 1.0);
  static constexpr double coeffs2_8[16] =
  {
    -0.0026367453410967019,
    0.0056954995988467462,
    -0.010750637983096105,
    0.018689598049002096,
    -0.031243628674597745,
    0.052760932803671098,
    -0.099248122111822379,
    0.31597034387100925,
    0.31597034387100925,
    -0.099248122111822379,
    0.052760932803671098,
    -0.031243628674597745,
    0.018689598049002096,
    -0.010750637983096105,
    0.0056954995988467462,
    -0.0026367453410967019,
  };
  if (stage_ratio == 2 && precision_ == 8 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<16, USE_SSE> > (coeffs2_8, 16, 2.0);
  if (stage_ratio == 2 && precision_ == 8 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<16, USE_SSE> > (coeffs2_8, 16, 1.0);
  static constexpr double coeffs4_8[6] =
  {
    0.013331613494158878,
    -0.063984766541747701,
    0.30161384865948221,
    0.30161384865948221,
    -0.063984766541747701,
    0.013331613494158878,
  };
  if (stage_ratio == 4 && precision_ == 8 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<6, USE_SSE> > (coeffs4_8, 6, 2.0);
  if (stage_ratio == 4 && precision_ == 8 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<6, USE_SSE> > (coeffs4_8, 6, 1.0);
  static constexpr double coeffs8_8[4] =
  {
    -0.037276258261764332,
    0.28635090020526976,
    0.28635090020526976,
    -0.037276258261764332,
  };
  if (stage_ratio == 8 && precision_ == 8 && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<4, USE_SSE> > (coeffs8_8, 4, 2.0);
  if (stage_ratio == 8 && precision_ == 8 && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<4, USE_SSE> > (coeffs8_8, 4, 1.0);
  // END generated code

  /* linear interpolation coefficients; barely useful for actual audio use,
   * but useful for testing
   */
  static constexpr double coeffs_linear[2] = {
    0.25,
    /* here, a 0.5 coefficient will be used */
    0.25,
  };

  if (precision_ == PREC_LINEAR && mode_ == UP)
    return create_impl_with_coeffs <Upsampler2<2, USE_SSE> > (coeffs_linear, 2, 2.0);
  if (precision_ == PREC_LINEAR && mode_ == DOWN)
    return create_impl_with_coeffs <Downsampler2<2, USE_SSE> > (coeffs_linear, 2, 1.0);
  return 0;
}

template<uint ORDER>
class Resampler2::IIRDownsampler2 final : public Resampler2::Impl {
  hiir::Downsampler2xFpu<ORDER> downs;
  double delay_;
public:
  IIRDownsampler2 (const double *coeffs, double group_delay) :
    delay_ ((group_delay - 1) / 2)
  {
    downs.set_coefs (coeffs);
  }
  void
  process_block (const float *input, uint n_input_samples, float *output) override
  {
    const uint n_output_samples = n_input_samples / 2;

    downs.process_block (output, input, n_output_samples);
  }
  uint
  order() const override
  {
    return ORDER;
  }
  double
  delay() const override
  {
    return delay_;
  }
  void
  reset() override
  {
    downs.clear_buffers();
  }
  bool
  sse_enabled() const override
  {
    return false;
  }
};

template<uint ORDER>
class Resampler2::IIRUpsampler2 final : public Resampler2::Impl {
  hiir::Upsampler2xFpu<ORDER> ups;
  double delay_;
public:
  IIRUpsampler2 (const double *coeffs, double group_delay) :
    delay_ (group_delay)
  {
    ups.set_coefs (coeffs);
  }
  void
  process_block (const float *input, uint n_input_samples, float *output) override
  {
    ups.process_block (output, input, n_input_samples);
  }
  uint
  order() const override
  {
    return ORDER;
  }
  double
  delay() const override
  {
    return delay_;
  }
  void
  reset() override
  {
    ups.clear_buffers();
  }
  bool
  sse_enabled() const override
  {
    return false;
  }
};

#ifdef __SSE__
template<uint ORDER>
class Resampler2::IIRDownsampler2SSE final : public Resampler2::Impl {
  hiir::Downsampler2xSse<ORDER> downs;
  double delay_;
public:
  IIRDownsampler2SSE (const double *coeffs, double group_delay) :
    delay_ ((group_delay - 1) / 2)
  {
    downs.set_coefs (coeffs);
  }
  void
  process_block (const float *input, uint n_input_samples, float *output) override
  {
    const uint n_output_samples = n_input_samples / 2;

    downs.process_block (output, input, n_output_samples);
  }
  uint
  order() const override
  {
    return ORDER;
  }
  double
  delay() const override
  {
    return delay_;
  }
  void
  reset() override
  {
    downs.clear_buffers();
  }
  bool
  sse_enabled() const override
  {
    return true;
  }
};

template<uint ORDER>
class Resampler2::IIRUpsampler2SSE final : public Resampler2::Impl {
  hiir::Upsampler2xSse<ORDER> ups;
  double delay_;
public:
  IIRUpsampler2SSE (const double *coeffs, double group_delay) :
    delay_ (group_delay)
  {
    ups.set_coefs (coeffs);
  }
  void
  process_block (const float *input, uint n_input_samples, float *output) override
  {
    ups.process_block (output, input, n_input_samples);
  }
  uint
  order() const override
  {
    return ORDER;
  }
  double
  delay() const override
  {
    return delay_;
  }
  void
  reset() override
  {
    ups.clear_buffers();
  }
  bool
  sse_enabled() const override
  {
    return true;
  }
};
#endif /* __SSE__ */

template<bool USE_SSE> Resampler2::Impl*
Resampler2::create_impl_iir (uint stage_ratio)
{
  // START generated code
  constexpr std::array<double,3> coeffs2_8 = {
    0.13533476491166646,
    0.44459059808236606,
    0.80006724816152464,
  };
  if (stage_ratio == 2 && precision_ == PREC_48DB)
    return create_impl_iir_with_coeffs (coeffs2_8, 1.749694);

  constexpr std::array<double,2> coeffs4_8 = {
    0.12564829488677751,
    0.56413565549879052,
  };
  if (stage_ratio == 4 && precision_ == PREC_48DB)
    return create_impl_iir_with_coeffs (coeffs4_8, 1.554290);

  constexpr std::array<double,1> coeffs8_8 = {
    0.34210403268814876,
  };
  if (stage_ratio == 8 && precision_ == PREC_48DB)
    return create_impl_iir_with_coeffs (coeffs8_8, 0.980631);

  constexpr std::array<double,5> coeffs2_12 = {
    0.057369561854075074,
    0.2095436081316879,
    0.41352768544651608,
    0.6351349011042412,
    0.86943780167618079,
  };
  if (stage_ratio == 2 && precision_ == PREC_72DB)
    return create_impl_iir_with_coeffs (coeffs2_12, 2.758511);

  constexpr std::array<double,3> coeffs4_12 = {
    0.063242386110162196,
    0.26520245698601469,
    0.66713594063634607,
  };
  if (stage_ratio == 4 && precision_ == PREC_72DB)
    return create_impl_iir_with_coeffs (coeffs4_12, 2.162389);

  constexpr std::array<double,2> coeffs8_12 = {
    0.11010398332301433,
    0.53640535169046299,
  };
  if (stage_ratio == 8 && precision_ == PREC_72DB)
    return create_impl_iir_with_coeffs (coeffs8_12, 1.603448);

  constexpr std::array<double,6> coeffs2_16 = {
    0.041451595119442179,
    0.15510356876083609,
    0.31565680487417447,
    0.49770230748789734,
    0.68754139898746236,
    0.88864894857989574,
  };
  if (stage_ratio == 2 && precision_ == PREC_96DB)
    return create_impl_iir_with_coeffs (coeffs2_16, 3.258518);

  constexpr std::array<double,3> coeffs4_16 = {
    0.063242386110162196,
    0.26520245698601469,
    0.66713594063634607,
  };
  if (stage_ratio == 4 && precision_ == PREC_96DB)
    return create_impl_iir_with_coeffs (coeffs4_16, 2.162389);

  constexpr std::array<double,2> coeffs8_16 = {
    0.11010398332301433,
    0.53640535169046299,
  };
  if (stage_ratio == 8 && precision_ == PREC_96DB)
    return create_impl_iir_with_coeffs (coeffs8_16, 1.603448);

  constexpr std::array<double,8> coeffs2_20 = {
    0.024474822059978408,
    0.094054346501929856,
    0.19872162695194262,
    0.32597599445882591,
    0.46482603848881743,
    0.60862663328164524,
    0.75647898374965283,
    0.91392075106875681,
  };
  if (stage_ratio == 2 && precision_ == PREC_120DB)
    return create_impl_iir_with_coeffs (coeffs2_20, 4.258575);

  constexpr std::array<double,4> coeffs4_20 = {
    0.03806054747623356,
    0.15621232623983866,
    0.37118048037198415,
    0.73087698794653666,
  };
  if (stage_ratio == 4 && precision_ == PREC_120DB)
    return create_impl_iir_with_coeffs (coeffs4_20, 2.771786);

  constexpr std::array<double,3> coeffs8_20 = {
    0.054591151801747943,
    0.23954799102035762,
    0.64335720472138391,
  };
  if (stage_ratio == 8 && precision_ == PREC_120DB)
    return create_impl_iir_with_coeffs (coeffs8_20, 2.227224);

  constexpr std::array<double,9> coeffs2_24 = {
    0.01964694276744065,
    0.076088803821783499,
    0.16263241326637887,
    0.2704225137521028,
    0.39083229614395837,
    0.51740920918216626,
    0.6470358330763375,
    0.7804624622392915,
    0.92268241849452293,
  };
  if (stage_ratio == 2 && precision_ == PREC_144DB)
    return create_impl_iir_with_coeffs (coeffs2_24, 4.758752);

  constexpr std::array<double,5> coeffs4_24 = {
    0.025414320818611134,
    0.10332655701804375,
    0.24011358015046655,
    0.45161232259950512,
    0.77416388521132473,
  };
  if (stage_ratio == 4 && precision_ == PREC_144DB)
    return create_impl_iir_with_coeffs (coeffs4_24, 3.382479);

  constexpr std::array<double,3> coeffs8_24 = {
    0.054591151801747943,
    0.23954799102035762,
    0.64335720472138391,
  };
  if (stage_ratio == 8 && precision_ == PREC_144DB)
    return create_impl_iir_with_coeffs (coeffs8_24, 2.227224);
  // END generated code
  return 0;
}

template<class CArray>
inline Resampler2::Impl*
Resampler2::create_impl_iir_with_coeffs (const CArray& carray, double group_delay)
{
  constexpr uint n_coeffs = std::tuple_size<CArray>::value;

#ifdef __SSE__
  if (use_sse_if_available_)
    {
      if (mode_ == UP)
        return new IIRUpsampler2SSE<n_coeffs> (carray.data(), group_delay);
      else
        return new IIRDownsampler2SSE<n_coeffs> (carray.data(), group_delay);
    }
  else
#endif
    {
      if (mode_ == UP)
        return new IIRUpsampler2<n_coeffs> (carray.data(), group_delay);
      else
        return new IIRDownsampler2<n_coeffs> (carray.data(), group_delay);
    }
}

PANDA_RESAMPLER_FN
bool
Resampler2::test_filter_impl (bool verbose)
{
  if (sse_available())
    {
      return fir_test_filter_sse (verbose);
    }
  else
    {
      if (verbose)
        printf ("SSE filter implementation not tested: no SSE support available\n");
      return true;
    }
}

} // namespace PandaResampler
