// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_OUTPUT_HH
#define SPECTMORPH_MORPH_OUTPUT_HH

#include "smmorphoperator.hh"
#include "smutils.hh"
#include "smmath.hh"
#include "smproperty.hh"
#include "smmodulationlist.hh"

#include <string>

namespace SpectMorph
{

class MorphOutput;

class MorphOutput : public MorphOperator
{
public:
  enum FilterType {
    FILTER_TYPE_LADDER = 1,
    FILTER_TYPE_SALLEN_KEY = 2
  };
  enum FilterLadderMode {
    FILTER_LADDER_LP1 = 1,
    FILTER_LADDER_LP2 = 2,
    FILTER_LADDER_LP3 = 3,
    FILTER_LADDER_LP4 = 4,
  };
  enum FilterSKMode {
    FILTER_SK_LP1 = 1,
    FILTER_SK_LP2 = 2,
    FILTER_SK_LP3 = 3,
    FILTER_SK_LP4 = 4,
    FILTER_SK_LP6 = 5,
    FILTER_SK_LP8 = 6,

    FILTER_SK_BP2 = 7,
    FILTER_SK_BP4 = 8,
    FILTER_SK_BP6 = 9,
    FILTER_SK_BP8 = 10,

    FILTER_SK_HP1 = 11,
    FILTER_SK_HP2 = 12,
    FILTER_SK_HP3 = 13,
    FILTER_SK_HP4 = 14,
    FILTER_SK_HP6 = 15,
    FILTER_SK_HP8 = 16,
  };
  struct Config : public MorphOperatorConfig
  {
    std::vector<MorphOperatorPtr> channel_ops;

    float                         velocity_sensitivity;
    int                           pitch_bend_range;

    bool                          sines;
    bool                          noise;

    bool                          unison;
    int                           unison_voices;
    float                         unison_detune;

    bool                          adsr;
    float                         adsr_skip;
    float                         adsr_attack;
    float                         adsr_decay;
    float                         adsr_sustain;
    float                         adsr_release;

    bool                          filter;
    FilterType                    filter_type;
    FilterLadderMode              filter_ladder_mode;
    FilterSKMode                  filter_sk_mode;
    float                         filter_attack;
    float                         filter_decay;
    float                         filter_sustain;
    float                         filter_release;
    float                         filter_depth;
    float                         filter_key_tracking;
    ModulationData                filter_cutoff_mod;
    ModulationData                filter_resonance_mod;
    ModulationData                filter_drive_mod;

    bool                          portamento;
    float                         portamento_glide;

    bool                          vibrato;
    float                         vibrato_depth;
    float                         vibrato_frequency;
    float                         vibrato_attack;
  };
  Config                       m_config;

  static constexpr auto P_VELOCITY_SENSITIVITY = "velocity_sensitivity";
  static constexpr auto P_PITCH_BEND_RANGE     = "pitch_bend_range";

  static constexpr auto P_SINES  = "sines";
  static constexpr auto P_NOISE  = "noise";

  static constexpr auto P_UNISON        = "unison";
  static constexpr auto P_UNISON_VOICES = "unison_voices";
  static constexpr auto P_UNISON_DETUNE = "unison_detune";

  static constexpr auto P_ADSR         = "adsr";
  static constexpr auto P_ADSR_SKIP    = "adsr_skip";
  static constexpr auto P_ADSR_ATTACK  = "adsr_attack";
  static constexpr auto P_ADSR_DECAY   = "adsr_decay";
  static constexpr auto P_ADSR_SUSTAIN = "adsr_sustain";
  static constexpr auto P_ADSR_RELEASE = "adsr_release";

  static constexpr auto P_FILTER            = "filter";
  static constexpr auto P_FILTER_TYPE       = "filter_type";
  static constexpr auto P_FILTER_LADDER_MODE = "filter_ladder_mode";
  static constexpr auto P_FILTER_SK_MODE    = "filter_sk_mode";
  static constexpr auto P_FILTER_ATTACK     = "filter_attack";
  static constexpr auto P_FILTER_DECAY      = "filter_decay";
  static constexpr auto P_FILTER_SUSTAIN    = "filter_sustain";
  static constexpr auto P_FILTER_RELEASE    = "filter_release";
  static constexpr auto P_FILTER_DEPTH      = "filter_depth";
  static constexpr auto P_FILTER_KEY_TRACKING = "filter_key_tracking";
  static constexpr auto P_FILTER_CUTOFF     = "filter_cutoff";
  static constexpr auto P_FILTER_RESONANCE  = "filter_resonance";
  static constexpr auto P_FILTER_DRIVE      = "filter_drive";

  static constexpr auto P_PORTAMENTO        = "portamento";
  static constexpr auto P_PORTAMENTO_GLIDE  = "portamento_glide";

  static constexpr auto P_VIBRATO           = "vibrato";
  static constexpr auto P_VIBRATO_DEPTH     = "vibrato_depth";
  static constexpr auto P_VIBRATO_FREQUENCY = "vibrato_frequency";
  static constexpr auto P_VIBRATO_ATTACK    = "vibrato_attack";

protected:
  std::vector<std::string>     load_channel_op_names;

public:
  MorphOutput (MorphPlan *morph_plan);
  ~MorphOutput();

  // inherited from MorphOperator
  const char        *type() override;
  int                insert_order() override;
  bool               save (OutFile& out_file) override;
  bool               load (InFile&  in_file) override;
  void               post_load (OpNameMap& op_name_map) override;
  OutputType         output_type() override;

  std::vector<MorphOperator *> dependencies() override;
  MorphOperatorConfig *clone_config() override;

  void           set_channel_op (int ch, MorphOperator *op);
  MorphOperator *channel_op (int ch);

/* slots: */
  void on_operator_removed (MorphOperator *op);
};

}

#endif
