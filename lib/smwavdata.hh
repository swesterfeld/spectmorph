// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_WAVE_DATA_HH
#define SPECTMORPH_WAVE_DATA_HH

#include "smutils.hh"

#include <vector>
#include <functional>

#include <sndfile.h>

namespace SpectMorph
{

class WavData
{
public:
  enum class OutFormat { WAV, FLAC };

private:
  std::vector<float> m_samples;
  float              m_mix_freq;
  int                m_n_channels;
  int                m_bit_depth;
  std::string        m_error_blurb;

  bool save (std::function<SNDFILE* (SF_INFO *)> open_func, OutFormat out_format);
  bool load (std::function<SNDFILE* (SF_INFO *)> open_func);

public:
  WavData();
  WavData (const std::vector<float>& samples, int n_channels, float mix_freq, int bit_depth);

  static std::vector<std::string> supported_extensions();

  bool load (const std::vector<unsigned char>& in);
  bool load (const std::string& filename);
  bool load_mono (const std::string& filename);

  void load (const std::vector<float>& samples, int n_channels, float mix_freq, int bit_depth);

  bool save (const std::string& filename, OutFormat out_format = OutFormat::WAV);
  bool save (std::vector<unsigned char>& out, OutFormat out_format = OutFormat::WAV);

  void clear();
  void prepend (const std::vector<float>& samples);

  float                       mix_freq() const;
  int                         n_channels() const;
  size_t                      n_values() const;
  int                         bit_depth() const;
  const std::vector<float>&   samples() const;
  const char                 *error_blurb() const;

  float operator[] (size_t pos) const;
};

}

#endif /* SPECTMORPH_WAVE_DATA_HH */
