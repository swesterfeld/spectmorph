// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smwavdata.hh"

namespace SpectMorph
{
double detect_pitch (const WavData& wav_data, std::function<bool (double)> kill_progress_function = nullptr);
}
