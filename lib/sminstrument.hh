// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTRUMENT_HH
#define SPECTMORPH_INSTRUMENT_HH

#include "smpugixml.hh"
#include "smutils.hh"
#include "smwavdata.hh"
#include "smsignal.hh"
#include "smmath.hh"
#include "smaudio.hh"

#include <map>
#include <memory>

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

class Instrument;

class Sample
{
public:
  enum class Loop { NONE, FORWARD, PING_PONG, SINGLE_FRAME };

private:
  SPECTMORPH_CLASS_NON_COPYABLE (Sample);

  std::map<MarkerType, double> marker_map;
  int m_midi_note = 69;
  Instrument *instrument = nullptr;
  Loop m_loop = Loop::NONE;

public:
  Sample (Instrument *inst) :
    instrument (inst)
  {
  }
  void set_marker (MarkerType marker_type, double value);
  double
  get_marker (MarkerType marker_type) const
  {
    auto it = marker_map.find (marker_type);
    if (it != marker_map.end())
      return it->second;
    return -1;
  }
  int
  midi_note() const
  {
    return m_midi_note;
  }
  void set_midi_note (int note);
  Loop
  loop () const
  {
    return m_loop;
  }
  void set_loop (Loop loop);
  std::string filename;
  WavData     wav_data;

  std::unique_ptr<Audio> audio;
};

class Instrument
{
SPECTMORPH_CLASS_NON_COPYABLE (Instrument);

  std::vector<std::unique_ptr<Sample>> samples;
  int m_selected = -1;
  std::string m_name = "untitled";
  bool m_auto_volume = true;
  bool m_auto_tune = false;

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

    Sample *sample = new Sample (this);
    samples.emplace_back (sample);
    sample->filename  = filename;
    sample->wav_data = wav_data;

    sample->set_marker (MARKER_CLIP_START, 0.0 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
    sample->set_marker (MARKER_CLIP_END, 1.0 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
    sample->set_marker (MARKER_LOOP_START, 0.4 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
    sample->set_marker (MARKER_LOOP_END, 0.6 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());

    signal_samples_changed();

    return true; /* FIXME: fail if load fails */
  }
  size_t
  size() const
  {
    return samples.size();
  }
  std::string
  name() const
  {
    return m_name;
  }
  Sample *
  sample (size_t n) const
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

    char *basename = g_path_get_basename (filename.c_str());
    m_name = basename;
    g_free (basename);

    xml_document doc;
    doc.load_file (filename.c_str());
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
            Sample *sample = new Sample (this);
            samples.emplace_back (sample);
            sample->filename  = filename;
            sample->set_midi_note (midi_note);
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
                std::string loop_type = loop_node.attribute ("type").value();
                if (loop_type == "forward")
                  sample->set_loop (Sample::Loop::FORWARD);
                if (loop_type == "ping-pong")
                  sample->set_loop (Sample::Loop::PING_PONG);
                if (loop_type == "single-frame")
                  sample->set_loop (Sample::Loop::SINGLE_FRAME);

                sample->set_marker (MARKER_LOOP_START, sm_atof (loop_node.attribute ("start").value()));
                sample->set_marker (MARKER_LOOP_END, sm_atof (loop_node.attribute ("end").value()));
              }
          }
      }
    /* select first sample if possible */
    if (samples.empty())
      m_selected = -1;
    else
      m_selected = 0;
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
        sample_node.append_attribute ("midi_note").set_value (sample->midi_note());

        xml_node clip_node = sample_node.append_child ("clip");
        clip_node.append_attribute ("start") = string_printf ("%.3f", sample->get_marker (MARKER_CLIP_START)).c_str();
        clip_node.append_attribute ("end") = string_printf ("%.3f", sample->get_marker (MARKER_CLIP_END)).c_str();

        if (sample->loop() != Sample::Loop::NONE)
          {
            std::string loop_type;

            if (sample->loop() == Sample::Loop::FORWARD)
              loop_type = "forward";
            if (sample->loop() == Sample::Loop::PING_PONG)
              loop_type = "ping-pong";
            if (sample->loop() == Sample::Loop::SINGLE_FRAME)
              loop_type = "single-frame";

            xml_node loop_node = sample_node.append_child ("loop");
            loop_node.append_attribute ("type") = loop_type.c_str();
            loop_node.append_attribute ("start") = string_printf ("%.3f", sample->get_marker (MARKER_LOOP_START)).c_str();
            loop_node.append_attribute ("end") = string_printf ("%.3f", sample->get_marker (MARKER_LOOP_END)).c_str();
          }
      }
    doc.save_file (filename.c_str());
  }
  void
  update_order()
  {
    Sample *ssample = sample (selected());
    sort (samples.begin(), samples.end(),
      [](auto& s1, auto& s2) {
        if (s1->midi_note() > s2->midi_note())
          return true;
        if (s1->midi_note() < s2->midi_note())
          return false;
        return s1->filename < s2->filename;
      });

    for (size_t n = 0; n < samples.size(); n++)
      if (samples[n].get() == ssample)
        m_selected = n;

    signal_samples_changed();
  }
  void
  marker_changed()
  {
    signal_marker_changed();
  }
  bool
  auto_volume() const
  {
    return m_auto_volume;
  }
  void
  set_auto_volume (bool new_value)
  {
    m_auto_volume = new_value;

    signal_global_changed();
  }
  bool
  auto_tune() const
  {
    return m_auto_tune;
  }
  void
  set_auto_tune (bool new_value)
  {
    m_auto_tune = new_value;

    signal_global_changed();
  }
  Signal<> signal_samples_changed;
  Signal<> signal_marker_changed;
  Signal<> signal_global_changed;  // global auto volume, auto tune or advanced params changed
};

inline void
Sample::set_midi_note (int note)
{
  m_midi_note = note;

  instrument->update_order();
}

inline void
Sample::set_marker (MarkerType marker_type, double value)
{
  marker_map[marker_type] = value;
  instrument->marker_changed();
}

inline void
Sample::set_loop (Loop loop)
{
  m_loop = loop;
  instrument->marker_changed();
}
}

#endif
