// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperator.hh"
#include "smproperty.hh"
#include "smcurve.hh"

#include <string>

namespace SpectMorph
{

class MorphLFO;

class MorphLFO : public MorphOperator
{
  LeakDebugger2 leak_debugger2 { "SpectMorph::MorphLFO" };
public:
  enum WaveType {
    WAVE_SINE           = 1,
    WAVE_TRIANGLE       = 2,
    WAVE_SAW_UP         = 3,
    WAVE_SAW_DOWN       = 4,
    WAVE_SQUARE         = 5,
    WAVE_RANDOM_SH      = 6,
    WAVE_RANDOM_LINEAR  = 7,
    WAVE_CUSTOM         = 8
  };
  enum Note {
    NOTE_32_1 = 1,
    NOTE_16_1 = 2,
    NOTE_8_1  = 3,
    NOTE_4_1  = 4,
    NOTE_2_1  = 5,
    NOTE_1_1  = 6,
    NOTE_1_2  = 7,
    NOTE_1_4  = 8,
    NOTE_1_8  = 9,
    NOTE_1_16 = 10,
    NOTE_1_32 = 11,
    NOTE_1_64 = 12
  };
  enum NoteMode {
    NOTE_MODE_STRAIGHT = 1,
    NOTE_MODE_TRIPLET  = 2,
    NOTE_MODE_DOTTED   = 3
  };
  static constexpr auto P_WAVE_TYPE    = "wave_type";
  static constexpr auto P_FREQUENCY    = "frequency";
  static constexpr auto P_DEPTH        = "depth";
  static constexpr auto P_CENTER       = "center";
  static constexpr auto P_START_PHASE  = "start_phase";
  static constexpr auto P_NOTE         = "note";
  static constexpr auto P_NOTE_MODE    = "note_mode";

  struct Config : public MorphOperatorConfig
  {
    WaveType       wave_type;
    float          frequency;
    float          depth;
    float          center;
    float          start_phase;
    bool           sync_voices;
    bool           beat_sync;
    Note           note;
    NoteMode       note_mode;
    Curve          curve; // for wave type custom
  };
protected:
  Config      m_config;
public:
  MorphLFO (MorphPlan *morph_plan);

  // inherited from MorphOperator
  const char        *type() override;
  int                insert_order() override;
  bool               save (OutFile& out_file) override;
  bool               load (InFile&  in_file) override;
  OutputType         output_type() override;
  MorphOperatorConfig *clone_config() override;

  bool sync_voices() const;
  void set_sync_voices (float new_sync_voices);

  bool beat_sync() const;
  void set_beat_sync (bool beat_sync);

  const Curve& curve() const;
  void set_curve (const Curve& curve);
  bool custom_shape() const;
};

}
