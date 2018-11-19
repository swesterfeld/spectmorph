// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTRUMENT_HH
#define SPECTMORPH_INSTRUMENT_HH

#include "smutils.hh"
#include "smwavdata.hh"
#include "smsignal.hh"
#include "smmath.hh"
#include "smaudio.hh"

#include <map>
#include <memory>

namespace SpectMorph
{

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
  Sample (Instrument *inst);
  void    set_marker (MarkerType marker_type, double value);
  double  get_marker (MarkerType marker_type) const;

  int     midi_note() const;
  void    set_midi_note (int note);

  Loop    loop() const;
  void    set_loop (Loop loop);

  std::string filename;
  WavData     wav_data;

  std::unique_ptr<Audio> audio;
};

class Instrument
{
public:
  struct AutoVolume {
    enum { FROM_LOOP, GLOBAL } method = FROM_LOOP;

    bool    enabled = false;
    double  gain    = 0;     // used by: global
  };

  struct AutoTune {
    enum { SIMPLE, ALL_FRAMES, SMOOTH } method = SIMPLE;

    bool    enabled  = false;
    int     partials = 1;    // used by: all_frames, smooth
    double  time     = 100;  // used_by: smooth
    double  amount   = 25;   // used by: smooth
  };
  struct EncoderEntry
  {
    std::string param;
    std::string value;
  };
  struct EncoderConfig
  {
    std::vector<EncoderEntry> entries;
  };

private:
  SPECTMORPH_CLASS_NON_COPYABLE (Instrument);

  std::vector<std::unique_ptr<Sample>> samples;
  int m_selected = -1;
  std::string m_name = "untitled";

  AutoVolume    m_auto_volume;
  AutoTune      m_auto_tune;
  EncoderConfig m_encoder_config;
public:
  Instrument();

  Sample     *add_sample (const std::string& filename);
  Sample     *sample (size_t n) const;

  size_t      size() const;
  std::string name() const;

  int         selected() const;
  void        set_selected (int sel);

  void        load (const std::string& filename);
  void        save (const std::string& filename);

  void        update_order();
  void        marker_changed();

  AutoVolume  auto_volume() const;
  void        set_auto_volume (const AutoVolume& new_value);

  AutoTune    auto_tune() const;
  void        set_auto_tune (const AutoTune& new_value);

  EncoderConfig encoder_config() const;
  void          set_encoder_config (const EncoderConfig& new_value);

  Signal<> signal_samples_changed;
  Signal<> signal_marker_changed;
  Signal<> signal_global_changed;  // global auto volume, auto tune or advanced params changed
};

}

#endif
