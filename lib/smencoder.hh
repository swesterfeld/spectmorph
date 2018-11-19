// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_ENCODER_HH
#define SPECTMORPH_ENCODER_HH

#include <sys/types.h>
#include <vector>
#include <string>
#include <map>

#include "smaudio.hh"
#include "smwavdata.hh"

namespace SpectMorph
{

/**
 * \brief Encoder parameters
 *
 * This struct contains parameters that configure the encoder algorithm.
 */
class EncoderParams
{
  /* config file parameters */
  std::vector<std::string>            param_name_d;   // names of all supported double parameters
  std::map<std::string, double>       param_value_d;  // values of double parameters from config file
  std::vector<std::string>            param_name_s;   // names of all supported string parameters
  std::map<std::string, std::string>  param_value_s;  // values of string parameters from config file

public:
  /** sample rate of the original audio file */
  float   mix_freq = 0;

  /** step size for analysis frames in milliseconds */
  float   frame_step_ms = 0;

  /** size of one analysis frame in milliseconds */
  float   frame_size_ms = 0;

  /** lower bound for the amount of zero padding used during analysis */
  int     zeropad = 0;

  /** size of the frame step in samples */
  size_t  frame_step = 0;

  /** size of the analysis frame in samples */
  size_t  frame_size = 0;

  /** analysis block size in samples, must be the smallest power of N greater than frame_size */
  size_t  block_size = 0;

  /** user defined fundamental freq */
  double  fundamental_freq = 0;

  /** whether to generate phases in output */
  bool    enable_phases = true;

  /** window to be used for analysis (needs to have block_size entries) */
  std::vector<float> window;

  bool add_config_entry (const std::string& param, const std::string& value);

  bool load_config (const std::string& filename);
  bool get_param (const std::string& param, double& value) const;
  bool get_param (const std::string& param, std::string& value) const;

  EncoderParams();

  /** use sane defaults for every parameter: */
  void setup_params (const WavData& wav_data, double fundamental_freq);
};

struct Tracksel {
  size_t   frame;
  size_t   d;         /* FFT position */
  double   freq;
  double   mag;
  double   mag2;      /* magnitude in dB */
  double   phase;     /* phase */

  Tracksel *prev, *next;
};

class EncoderBlock
{
public:
  std::vector<float> noise;          //!< noise envelope, representing the original signal minus sine components
  std::vector<float> freqs;          //!< frequencies of the sine components of this frame
  std::vector<float> mags;           //!< magnitudes of the sine components
  std::vector<float> phases;         //!< phases of the sine components
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

  std::vector< std::vector<Tracksel> > frame_tracksels; //!< Analog to Canny Algorithms edgels - only used internally

  struct Attack
  {
    double attack_start_ms;
    double attack_end_ms;
  };
  double attack_error (const std::vector< std::vector<double> >& unscaled_signal, const Attack& attack, std::vector<double>& out_scale);

  // single encoder steps:
  void compute_stft (const WavData& wav_data, int channel);
  void search_local_maxima();
  void link_partials();
  void validate_partials();
  void optimize_partials (int optimization_level);
  void spectral_subtract();
  void approx_noise();
  void compute_attack_params();
  void sort_freqs();

  Attack                               optimal_attack;
  size_t                               zero_values_at_start;
  size_t                               sample_count;

public:
  std::vector<EncoderBlock>            audio_blocks;    //!< current state, and end result of the encoding algorithm
  std::vector<float>                   original_samples;

  Encoder (const EncoderParams& enc_params);

  void debug_decode (const std::string& filename);

  // all-in-one encoding function:
  void encode (const WavData& wav_data, int channel, int optimization_level,
               bool attack, bool track_sines);

  void set_loop (Audio::LoopType loop_type, int loop_start, int loop_end);
  void set_loop_seconds (Audio::LoopType loop_type, double loop_start, double loop_end);

  Error save (const std::string& filename);
  Audio *save_as_audio();
};

}

#endif
