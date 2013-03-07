// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_AUDIO_HH
#define SPECTMORPH_AUDIO_HH

#include <bse/bsecxxplugin.hh>
#include <vector>

#include "smgenericin.hh"
#include "smgenericout.hh"

#define SPECTMORPH_BINARY_FILE_VERSION 11

namespace SpectMorph
{

/**
 * \brief Block of audio data, encoded in SpectMorph parametric format
 *
 * This represents a single analysis frame, usually containing sine waves in freqs
 * and phases, and a noise envelope for everything that remained after subtracting
 * the sine waves. The parameters original_fft and debug_samples are optional and
 * are used for debugging only.
 */
class AudioBlock
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

enum AudioLoadOptions
{
  AUDIO_LOAD_DEBUG,
  AUDIO_SKIP_DEBUG
};

/**
 * \brief Audio sample containing many blocks
 *
 * This class contains the information the SpectMorph::Encoder creates for a wav file. The
 * time dependant parameters are stored in contents, as a vector of audio frames; the
 * parameters that are the same for all frames are stored in this class.
 */
class Audio
{
  BIRNET_PRIVATE_CLASS_COPY (Audio);
public:
  Audio();
  ~Audio();

  enum LoopType {
    LOOP_NONE = 0,
    LOOP_FRAME_FORWARD,
    LOOP_FRAME_PING_PONG,
    LOOP_TIME_FORWARD,
    LOOP_TIME_PING_PONG,
  };

  float    fundamental_freq;            //!< fundamental frequency (note which was encoded), or 0 if not available
  float    mix_freq;                    //!< mix freq (sampling rate) of the original audio data
  float    frame_size_ms;               //!< length of each audio frame in milliseconds
  float    frame_step_ms;               //!< stepping of the audio frames in milliseconds
  float    attack_start_ms;             //!< start of attack in milliseconds
  float    attack_end_ms;               //!< end of attack in milliseconds
  float    start_ms;                    //!< instrument start marker to align multiple instruments for morphing
  int      zeropad;                     //!< FFT zeropadding used during analysis
  LoopType loop_type;                   //!< type of loop to be used during sustain phase of playback
  int      loop_start;                  //!< loop point to be used during sustain phase of playback
  int      loop_end;                    //!< loop point to be used during sustain phase of playback
  int      zero_values_at_start;        //!< number of zero values added by encoder (strip during decoding)
  int      sample_count;                //!< number of samples encoded (including zero_values_at_start)
  std::vector<float> original_samples;  //!< original time domain signal as samples (debugging only)
  std::vector<AudioBlock> contents;     //!< the actual frame data

  BseErrorType load (const std::string& filename, AudioLoadOptions load_options = AUDIO_LOAD_DEBUG);
  BseErrorType load (SpectMorph::GenericIn *file, AudioLoadOptions load_options = AUDIO_LOAD_DEBUG);
  BseErrorType save (const std::string& filename) const;
  BseErrorType save (SpectMorph::GenericOut *file) const;

  Audio *clone() const; // create a deep copy

  static bool loop_type_to_string (LoopType loop_type, std::string& s);
  static bool string_to_loop_type (const std::string& s, LoopType& loop_type);
};

}

#endif
