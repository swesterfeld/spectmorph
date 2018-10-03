// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTRUMENT_HH
#define SPECTMORPH_INSTRUMENT_HH

namespace SpectMorph
{

class Sample
{
SPECTMORPH_CLASS_NON_COPYABLE (Sample);
public:
  Sample()
  {
  }

  std::string filename;
  int         midi_note;
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
    /* new sample will be selected */
    m_selected = samples.size();

    Sample *sample = new Sample();
    samples.emplace_back (sample);
    sample->filename  = filename;
    sample->midi_note = 69;
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
