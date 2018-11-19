// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "sminstrument.hh"
#include "smpugixml.hh"

#include <map>
#include <memory>

using namespace SpectMorph;

using pugi::xml_document;
using pugi::xml_node;
using std::string;

Sample::Sample (Instrument *inst) :
  instrument (inst)
{
}

void
Sample::set_marker (MarkerType marker_type, double value)
{
  marker_map[marker_type] = value;
  instrument->marker_changed();
}

double
Sample::get_marker (MarkerType marker_type) const
{
  auto it = marker_map.find (marker_type);
  if (it != marker_map.end())
    return it->second;
  return -1;
}

int
Sample::midi_note() const
{
  return m_midi_note;
}

void
Sample::set_midi_note (int note)
{
  m_midi_note = note;

  instrument->update_order();
}

Sample::Loop
Sample::loop () const
{
  return m_loop;
}

void
Sample::set_loop (Loop loop)
{
  m_loop = loop;
  instrument->marker_changed();
}

Instrument::Instrument()
{
}

Sample *
Instrument::add_sample (const string& filename)
{
  /* try loading file */
  WavData wav_data;
  if (!wav_data.load_mono (filename))
    return nullptr;

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

  return sample;
}

size_t
Instrument::size() const
{
  return samples.size();
}

string
Instrument::name() const
{
  return m_name;
}

Sample *
Instrument::sample (size_t n) const
{
  if (n < samples.size())
    return samples[n].get();
  else
    return nullptr;
}

int
Instrument::selected() const
{
  return m_selected;
}

void
Instrument::set_selected (int sel)
{
  m_selected = sel;

  signal_samples_changed();
}

void
Instrument::load (const string& filename)
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
      string filename = sample_node.attribute ("filename").value();
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
              string loop_type = loop_node.attribute ("type").value();
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
  // auto tune
  m_auto_tune.enabled = false; // default
  xml_node auto_tune_node = inst_node.child ("auto_tune");
  if (auto_tune_node)
    {
      string method = auto_tune_node.attribute ("method").value();

      if (method == "simple")
        {
          m_auto_tune.method = AutoTune::SIMPLE;
          m_auto_tune.enabled = true;
        }
      else if (method == "all_frames")
        {
          m_auto_tune.method   = AutoTune::ALL_FRAMES;
          m_auto_tune.partials = atoi (auto_tune_node.attribute ("partials").value());
          m_auto_tune.enabled  = true;
        }
      else if (method == "smooth")
        {
          m_auto_tune.method   = AutoTune::SMOOTH;
          m_auto_tune.partials = atoi (auto_tune_node.attribute ("partials").value());
          m_auto_tune.time     = sm_atof (auto_tune_node.attribute ("time").value());
          m_auto_tune.amount   = sm_atof (auto_tune_node.attribute ("amount").value());
          m_auto_tune.enabled  = true;
        }
    }

  // auto volume
  m_auto_volume.enabled = false;
  xml_node auto_volume_node = inst_node.child ("auto_volume");
  if (auto_volume_node)
    {
      string method = auto_volume_node.attribute ("method").value();

      if (method == "from_loop")
        {
          m_auto_volume.method  = AutoVolume::FROM_LOOP;
          m_auto_volume.enabled = true;
        }
      else if (method == "global")
        {
          m_auto_volume.method  = AutoVolume::GLOBAL;
          m_auto_volume.gain    = sm_atof (auto_volume_node.attribute ("gain").value());
          m_auto_volume.enabled = true;
        }
      else
        {
          fprintf (stderr, "unknown auto volume method: %s\n", method.c_str());
        }
    }
  m_encoder_config = EncoderConfig(); // reset
  for (xml_node enc_node : inst_node.children ("encoder_config"))
    {
      EncoderEntry entry;
      entry.param = enc_node.attribute ("param").value();

      if (entry.param != "")
        {
          entry.value = enc_node.attribute ("value").value();
          m_encoder_config.entries.push_back (entry);
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
Instrument::save (const string& filename)
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
          string loop_type;

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
  if (m_auto_tune.enabled)
    {
      xml_node auto_tune_node = inst_node.append_child ("auto_tune");
      if (m_auto_tune.method == AutoTune::SIMPLE)
        {
          auto_tune_node.append_attribute ("method").set_value ("simple");
        }
      else if (m_auto_tune.method == AutoTune::ALL_FRAMES)
        {
          auto_tune_node.append_attribute ("method").set_value ("all_frames");
          auto_tune_node.append_attribute ("partials") = m_auto_tune.partials;
        }
      else if (m_auto_tune.method == AutoTune::SMOOTH)
        {
          auto_tune_node.append_attribute ("method").set_value ("smooth");
          auto_tune_node.append_attribute ("partials") = m_auto_tune.partials;
          auto_tune_node.append_attribute ("time")     = string_printf ("%.1f", m_auto_tune.time).c_str();
          auto_tune_node.append_attribute ("amount")   = string_printf ("%.1f", m_auto_tune.amount).c_str();
        }
    }
  if (m_auto_volume.enabled)
    {
      xml_node auto_volume_node = inst_node.append_child ("auto_volume");
      if (m_auto_volume.method == AutoVolume::FROM_LOOP)
        {
          auto_volume_node.append_attribute ("method").set_value ("from_loop");
        }
      if (m_auto_volume.method == AutoVolume::GLOBAL)
        {
          auto_volume_node.append_attribute ("method").set_value ("global");
          auto_volume_node.append_attribute ("gain") = string_printf ("%.1f", m_auto_volume.gain).c_str();
        }
    }
  for (auto entry : m_encoder_config.entries)
    {
      xml_node conf_node = inst_node.append_child ("encoder_config");
      conf_node.append_attribute ("param").set_value (entry.param.c_str());
      conf_node.append_attribute ("value").set_value (entry.value.c_str());
    }
  doc.save_file (filename.c_str());
}

void
Instrument::update_order()
{
  Sample *ssample = sample (selected());
  using SUPtr = std::unique_ptr<Sample>;
  sort (samples.begin(), samples.end(),
    [](const SUPtr& s1, const SUPtr& s2) {
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
Instrument::marker_changed()
{
  signal_marker_changed();
}

Instrument::AutoVolume
Instrument::auto_volume() const
{
  return m_auto_volume;
}

void
Instrument::set_auto_volume (const AutoVolume& new_value)
{
  m_auto_volume = new_value;

  signal_global_changed();
}

Instrument::AutoTune
Instrument::auto_tune() const
{
  return m_auto_tune;
}

void
Instrument::set_auto_tune (const AutoTune& new_value)
{
  m_auto_tune = new_value;

  signal_global_changed();
}

Instrument::EncoderConfig
Instrument::encoder_config() const
{
  return m_encoder_config;
}

void
Instrument::set_encoder_config (const EncoderConfig& new_value)
{
  m_encoder_config = new_value;

  signal_global_changed();
}
