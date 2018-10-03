// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTRUMENT_HH
#define SPECTMORPH_INSTRUMENT_HH

namespace SpectMorph
{
  /* OK in SpectMorph namespace */
  using pugi::xml_document;
  using pugi::xml_node;

enum MarkerType {
  MARKER_NONE = 0,
  MARKER_LOOP_START,
  MARKER_LOOP_END,
  MARKER_CLIP_START,
  MARKER_CLIP_END
};

class Sample
{
  SPECTMORPH_CLASS_NON_COPYABLE (Sample);

  std::map<MarkerType, double> marker_map;

public:
  Sample()
  {
  }
  void
  set_marker (MarkerType marker_type, double value)
  {
    marker_map[marker_type] = value;
  }
  double
  get_marker (MarkerType marker_type)
  {
    auto it = marker_map.find (marker_type);
    if (it != marker_map.end())
      return it->second;
    return -1;
  }

  std::string filename;
  int         midi_note;
  WavData     wav_data;
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

    sample->set_marker (MARKER_CLIP_START, 0.0 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
    sample->set_marker (MARKER_CLIP_END, 1.0 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
    sample->set_marker (MARKER_LOOP_START, 0.4 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
    sample->set_marker (MARKER_LOOP_END, 0.6 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());

    signal_samples_changed();

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
  void
  set_selected (int sel)
  {
    m_selected = sel;

    signal_samples_changed();
  }
  void
  load (const std::string& filename)
  {
    samples.clear();

    xml_document doc;
    doc.load_file ("/tmp/x.sminst");
    xml_node inst_node = doc.child ("instrument");
    for (xml_node sample_node : inst_node.children ("sample"))
      {
        std::string filename = sample_node.attribute ("filename").value();
        int midi_note = atoi (sample_node.attribute ("midi_note").value());
        if (midi_note == 0) /* default */
          midi_note = 69;

        /* try loading file */
        WavData wav_data;
        if (wav_data.load_mono (filename))
          {
            Sample *sample = new Sample();
            samples.emplace_back (sample);
            sample->filename  = filename;
            sample->midi_note = midi_note;
            sample->wav_data = wav_data;

            xml_node clip_node = sample_node.child ("clip");
            if (clip_node)
              {
                sample->set_marker (MARKER_CLIP_START, sm_atof (clip_node.attribute ("start").value()));
                sample->set_marker (MARKER_CLIP_END, sm_atof (clip_node.attribute ("end").value()));
              }
            xml_node loop_node = sample_node.child ("loop");
            if (loop_node)
              {
                sample->set_marker (MARKER_LOOP_START, sm_atof (loop_node.attribute ("start").value()));
                sample->set_marker (MARKER_LOOP_END, sm_atof (loop_node.attribute ("end").value()));
              }
          }
      }
    signal_samples_changed();
  }
  void
  save (const std::string& filename)
  {
    xml_document doc;
    xml_node inst_node = doc.append_child ("instrument");
    for (auto& sample : samples)
      {
        xml_node sample_node = inst_node.append_child ("sample");
        sample_node.append_attribute ("filename").set_value (sample->filename.c_str());
        sample_node.append_attribute ("midi_note").set_value (sample->midi_note);

        xml_node clip_node = sample_node.append_child ("clip");
        clip_node.append_attribute ("start") = string_printf ("%.3f", sample->get_marker (MARKER_CLIP_START)).c_str();
        clip_node.append_attribute ("end") = string_printf ("%.3f", sample->get_marker (MARKER_CLIP_END)).c_str();

        xml_node loop_node = sample_node.append_child ("loop");
        loop_node.append_attribute ("start") = string_printf ("%.3f", sample->get_marker (MARKER_LOOP_START)).c_str();
        loop_node.append_attribute ("end") = string_printf ("%.3f", sample->get_marker (MARKER_LOOP_END)).c_str();
      }
    doc.save_file (filename.c_str());
  }
  Signal<> signal_samples_changed;
};

}

#endif
