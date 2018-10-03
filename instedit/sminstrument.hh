// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTRUMENT_HH
#define SPECTMORPH_INSTRUMENT_HH

namespace SpectMorph
{

enum MarkerType {
  MARKER_NONE = 0,
  MARKER_LOOP_START,
  MARKER_LOOP_END,
  MARKER_CLIP_START,
  MARKER_CLIP_END
};

class Markers
{
  std::map<MarkerType, double> pos_map;
public:
  void
  clear()
  {
    pos_map.clear();
  }
  void
  set (MarkerType marker_type, double value)
  {
    pos_map[marker_type] = value;
  }
  double
  get (MarkerType marker_type)
  {
    auto it = pos_map.find (marker_type);
    if (it != pos_map.end())
      return it->second;
    return -1;
  }
};

class Sample
{
SPECTMORPH_CLASS_NON_COPYABLE (Sample);
public:
  Sample()
  {
  }

  std::string filename;
  int         midi_note;
  WavData     wav_data;
  Markers     markers;
};

class Instrument
{
SPECTMORPH_CLASS_NON_COPYABLE (Instrument);

  std::vector<std::unique_ptr<Sample>> samples;
  int m_selected = -1;

public:
  Instrument()
  {
  }
  bool
  add_sample (const std::string& filename)
  {
    /* try loading file */
    WavData wav_data;
    if (!wav_data.load_mono (filename))
      return false;

    /* new sample will be selected */
    m_selected = samples.size();

    Sample *sample = new Sample();
    samples.emplace_back (sample);
    sample->filename  = filename;
    sample->midi_note = 69;
    sample->wav_data = wav_data;

    sample->markers.clear();
    sample->markers.set (MARKER_CLIP_START, 0.0 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
    sample->markers.set (MARKER_CLIP_END, 1.0 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
    sample->markers.set (MARKER_LOOP_START, 0.4 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
    sample->markers.set (MARKER_LOOP_END, 0.6 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());

    return true; /* FIXME: fail if load fails */
  }
  size_t
  size()
  {
    return samples.size();
  }
  Sample *
  sample (size_t n)
  {
    if (n < samples.size())
      return samples[n].get();
    else
      return nullptr;
  }
  int
  selected()
  {
    return m_selected;
  }
};

}

#endif
