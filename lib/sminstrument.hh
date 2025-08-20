// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
class ZipWriter;
class ZipReader;
class Sample
{
public:
  enum class Loop { NONE, FORWARD, PING_PONG, SINGLE_FRAME };

  struct Shared
  {
    WavData     m_wav_data;
    std::string m_wav_data_hash;
  public:
    Shared (const WavData& wav_data);

    const WavData& wav_data() const;
    std::string    wav_data_hash() const;
  };
  typedef std::shared_ptr<Shared> SharedP;
private:

  SPECTMORPH_CLASS_NON_COPYABLE (Sample);

  std::map<MarkerType, double> marker_map;
  int m_midi_note = 69;
  Instrument *instrument = nullptr;
  Loop m_loop = Loop::NONE;
  double m_volume = 0;

  SharedP m_shared;

public:
  Sample (Instrument *inst, const WavData& wav_data);
  void    set_markers (const std::map<MarkerType, double>& markers);
  void    set_marker (MarkerType marker_type, double value);
  double  get_marker (MarkerType marker_type) const;

  int     midi_note() const;
  void    set_midi_note (int note);

  Loop    loop() const;
  void    set_loop (Loop loop);

  double  volume() const;
  void    set_volume (double volume);

  SharedP shared() const;

  const WavData&  wav_data() const;
  std::string     wav_data_hash() const;

  std::string filename;
  std::string short_name;

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
    bool enabled = false;

    std::vector<EncoderEntry> entries;
  };

  enum class LoadOptions
  {
    ALL,
    NAME_ONLY
  };

private:
  SPECTMORPH_CLASS_NON_COPYABLE (Instrument);

  std::vector<std::unique_ptr<Sample>> samples;
  int           m_selected = -1;
  std::string   m_name = "untitled";
  std::string   m_short_name;

  double        m_global_volume = 0;
  AutoVolume    m_auto_volume;
  AutoTune      m_auto_tune;
  EncoderConfig m_encoder_config;

  Error       load (const std::string& filename, ZipReader *zip_reader, LoadOptions load_options = LoadOptions::ALL);
  Error       save (const std::string& filename, ZipWriter *zip_writer) const;
public:
  Instrument();

  Sample     *add_sample (const WavData& wav_data, const std::string& filename);
  Sample     *sample (size_t n) const;
  void        remove_sample();
  std::string gen_short_name (std::vector<std::unique_ptr<Sample>>& samples, const std::string& filename);

  size_t      size() const;
  void        clear();
  std::string name() const;
  void        set_name (const std::string& name);
  std::string short_name() const;
  void        set_short_name (const std::string& short_name);

  std::string version();

  int         selected() const;
  void        set_selected (int sel);

  Error       load (const std::string& filename, LoadOptions load_options = LoadOptions::ALL);
  Error       load (ZipReader& zip_reader, LoadOptions load_options = LoadOptions::ALL);

  Error       save (const std::string& filename) const;
  Error       save (ZipWriter& zip_writer) const;

  Instrument *clone() const;
  void        update_order();
  void        marker_changed();
  void        volume_changed();

  std::map<int, int> used_count() const;

  double      global_volume() const;
  void        set_global_volume (double new_volume);

  AutoVolume  auto_volume() const;
  void        set_auto_volume (const AutoVolume& new_value);

  AutoTune    auto_tune() const;
  void        set_auto_tune (const AutoTune& new_value);

  EncoderConfig encoder_config() const;
  void          set_encoder_config (const EncoderConfig& new_value);

  Signal<> signal_volume_changed;
  Signal<> signal_samples_changed;
  Signal<> signal_selected_sample_changed;
  Signal<> signal_marker_changed;
  Signal<> signal_global_changed;  // global auto volume, auto tune or advanced params changed
};

}

#endif
