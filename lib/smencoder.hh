// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_ENCODER_HH
#define SPECTMORPH_ENCODER_HH

#include <sys/types.h>
#include <vector>
#include <string>
#include <bse/gsldatautils.hh>

#include "smaudio.hh"

namespace SpectMorph
{

/**
 * \brief Encoder parameters
 *
 * This struct contains parameters that configure the encoder algorithm.
 */
struct EncoderParams
{
  /** sample rate of the original audio file */
  float   mix_freq;         

  /** step size for analysis frames in milliseconds */
  float   frame_step_ms;

  /** size of one analysis frame in milliseconds */
  float   frame_size_ms;

  /** lower bound for the amount of zero padding used during analysis */
  int     zeropad;

  /** size of the frame step in samples */
  size_t  frame_step;

  /** size of the analysis frame in samples */
  size_t  frame_size;

  /** analysis block size in samples, must be the smallest power of N greater than frame_size */
  size_t  block_size;

  /** user defined fundamental freq */
  double  fundamental_freq;

  /* config file parameters */
  std::vector<std::string>            param_name_d;   // names of all supported double parameters
  std::map<std::string, double>       param_value_d;  // values of double parameters from config file
  std::vector<std::string>            param_name_s;   // names of all supported string parameters
  std::map<std::string, std::string>  param_value_s;  // values of string parameters from config file

  bool load_config (const std::string& filename);
  bool get_param (const std::string& param, double& value);
  bool get_param (const std::string& param, std::string& value);

  EncoderParams();
};

struct Tracksel {
  size_t   frame;
  size_t   d;         /* FFT position */
  double   freq;
  double   mag;
  double   mag2;      /* magnitude in dB */
  double   phase;     /* phase */
  bool     is_harmonic;
  Tracksel *prev, *next;
};

class EncoderBlock
{
public:
  std::vector<float> noise;          //!< noise envelope, representing the original signal minus sine components
  std::vector<float> freqs;          //!< frequencies of the sine components of this frame
  std::vector<float> mags;           //!< magnitudes of the sine components
  std::vector<float> phases;         //!< phases of the sine components
  std::vector<float> lpc_lsf_p;      //!< LPC line spectrum frequencies, P(z) roots
  std::vector<float> lpc_lsf_q;      //!< LPC line spectrum frequencies, Q(z) roots
  std::vector<float> original_fft;   //!< original zeropadded FFT data - for debugging only
  std::vector<float> debug_samples;  //!< original audio samples for this frame - for debugging only
};

/**
 * \brief Encoder producing SpectMorph parametric data from sample data
 *
 * The encoder needs to perform a number of analysis steps to get from the input
 * signal to a parametric representation (which is built in audio_blocks). At the
 * moment, this process needs to be controlled by the caller, but a simpler
 * interface should be added.
 */
class Encoder
{
  EncoderParams                        enc_params;
  int                                  loop_start;
  int                                  loop_end;
  Audio::LoopType                      loop_type;

  bool check_harmonic (double freq, double& new_freq, double mix_freq);

  std::vector< std::vector<Tracksel> > frame_tracksels; //!< Analog to Canny Algorithms edgels - only used internally

  struct Attack
  {
    double attack_start_ms;
    double attack_end_ms;
  };
  double attack_error (const std::vector< std::vector<double> >& unscaled_signal, const std::vector<float>& window, const Attack& attack, std::vector<double>& out_scale);

public:
  std::vector<EncoderBlock>            audio_blocks;    //!< current state, and end result of the encoding algorithm
  Attack                               optimal_attack;
  size_t                               zero_values_at_start;
  size_t                               sample_count;
  std::vector<float>                   original_samples;

  Encoder (const EncoderParams& enc_params);

  // single encoder steps:
  void compute_stft (GslDataHandle *dhandle, int channel, const std::vector<float>& window);
  void search_local_maxima (const std::vector<float>& window);
  void link_partials();
  void validate_partials();
  void optimize_partials (const std::vector<float>& window, int optimization_level);
  void spectral_subtract (const std::vector<float>& window);
  void approx_noise (const std::vector<float>& window);
  void compute_attack_params (const std::vector<float>& window);
  void compute_lpc_lsf();
  void sort_freqs();
  void debug_decode (const std::string& filename, const std::vector<float>& window);

  // all-in-one encoding function:
  void encode (GslDataHandle *dhandle, int channel, const std::vector<float>& window, int optimization_level,
               bool attack, bool track_sines, bool do_lpc);

  void set_loop (Audio::LoopType loop_type, int loop_start, int loop_end);
  void set_loop_seconds (Audio::LoopType loop_type, double loop_start, double loop_end);
  void save (const std::string& filename);
};

}

#endif
