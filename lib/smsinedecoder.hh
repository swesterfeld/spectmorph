// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SINEDECODER_HH
#define SPECTMORPH_SINEDECODER_HH

#include "smifftsynth.hh"
#include "smaudio.hh"
#include <vector>

namespace SpectMorph {

/**
 * \brief Decoder for the sine component (deterministic component) of the signal
 *
 * To decode a signal, create a SineDecoder and always call process with two
 * frames, the current frame and the next frame.
 */
class SineDecoder
{
public:
  /**
   * \brief Different supported modes for decoding.
   */
  enum Mode {
    /**
     * This mode preserves the phases of the original signal, but signal modifications like
     * looping a frame are not possible (without recomputing the phases in the frames at least).
     */
    MODE_PHASE_SYNC_OVERLAP,
    /**
     * Like MODE_PHASE_SYNC_OVERLAP, however IFFTSynth is used to render the partials, which
     * makes this mode faster than MODE_PHASE_SYNC_OVERLAP.
     */
    MODE_PHASE_SYNC_OVERLAP_IFFT,
    /**
     * This mode uses sine waves with phases derived from their frequency for reconstruction.
     * in general this does not preserve the original phase information, but should still sound
     * the same - also the frames can be reordered (for instance loops are possible).
     */
    MODE_TRACKING
  };
private:
  double mix_freq;
  double fundamental_freq;
  size_t frame_size;
  size_t frame_step;
  std::vector<double> synth_fixed_phase, next_synth_fixed_phase;
  Mode mode;
  SpectMorph::IFFTSynth *ifft_synth;

public:
  SineDecoder (double fundamental_freq, double mix_freq, size_t frame_size, size_t frame_step, Mode mode);
  ~SineDecoder();

  void process (const AudioBlock& block,
                const AudioBlock& next_block,
                const std::vector<double>& window,
                std::vector<float>& decoded_sines);
};

}
#endif
