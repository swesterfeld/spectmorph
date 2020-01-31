// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LFO_HH
#define SPECTMORPH_MORPH_LFO_HH

#include "smmorphoperator.hh"
#include "smproperty.hh"

#include <string>

namespace SpectMorph
{

class MorphLFO;

struct MorphLFOProperties
{
  MorphLFOProperties (MorphLFO *lfo);

  LogParamProperty<MorphLFO>    frequency;
  LinearParamProperty<MorphLFO> depth;
  LinearParamProperty<MorphLFO> center;
  LinearParamProperty<MorphLFO> start_phase;
};

class MorphLFO : public MorphOperator
{
public:
  enum WaveType {
    WAVE_SINE           = 1,
    WAVE_TRIANGLE       = 2,
    WAVE_SAW_UP         = 3,
    WAVE_SAW_DOWN       = 4,
    WAVE_SQUARE         = 5,
    WAVE_RANDOM_SH      = 6,
    WAVE_RANDOM_LINEAR  = 7
  };
protected:
  WaveType       m_wave_type;
  float          m_frequency;
  float          m_depth;
  float          m_center;
  float          m_start_phase;
  bool           m_sync_voices;

public:
  MorphLFO (MorphPlan *morph_plan);
  ~MorphLFO();

  // inherited from MorphOperator
  const char        *type();
  int                insert_order();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  OutputType         output_type();

  WaveType wave_type();
  void set_wave_type (WaveType new_wave_type);

  float frequency() const;
  void set_frequency (float new_frequency);

  float depth() const;
  void set_depth (float new_depth);

  float center() const;
  void set_center (float new_center);

  float start_phase() const;
  void set_start_phase (float new_start_phase);

  bool sync_voices() const;
  void set_sync_voices (float new_sync_voices);
};

}

#endif
