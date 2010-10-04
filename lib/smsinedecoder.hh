/* 
 * Copyright (C) 2009-2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SPECTMORPH_SINEDECODER_HH
#define SPECTMORPH_SINEDECODER_HH

#include "smframe.hh"
#include "smifftsynth.hh"
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
  size_t frame_size;
  size_t frame_step;
  std::vector<double> synth_fixed_phase, next_synth_fixed_phase;
  Mode mode;
  SpectMorph::IFFTSynth *ifft_synth;

public:
  SineDecoder (double mix_freq, size_t frame_size, size_t frame_step, Mode mode);
  ~SineDecoder();

  void process (const Frame& frame,
                const Frame& next_frame,
                const std::vector<double>& window,
                std::vector<float>& decoded_sines);
};

}
#endif
