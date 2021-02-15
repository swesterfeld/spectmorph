// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_HH
#define SPECTMORPH_MORPH_OUTPUT_HH

#include "smmorphoperator.hh"
#include "smutils.hh"
#include "smmath.hh"
#include "smproperty.hh"

#include <string>

namespace SpectMorph
{

class MorphOutput;

class MorphOutput : public MorphOperator
{
public:
  enum FilterType {
    FILTER_LP1 = 1,
    FILTER_LP2 = 2,
    FILTER_LP3 = 3,
    FILTER_LP4 = 4
  };
  struct Config : public MorphOperatorConfig
  {
    std::vector<MorphOperatorPtr> channel_ops;

    float                         velocity_sensitivity;

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
    float                         filter_attack;
    float                         filter_decay;
    float                         filter_sustain;
    float                         filter_release;
    float                         filter_depth;
    float                         filter_cutoff;
    float                         filter_resonance;

    bool                          portamento;
    float                         portamento_glide;

    bool                          vibrato;
    float                         vibrato_depth;
    float                         vibrato_frequency;
    float                         vibrato_attack;
  };
  Config                       m_config;

  static constexpr auto P_VELOCITY_SENSITIVITY = "velocity_sensitivity";

  static constexpr auto P_UNISON_VOICES = "unison_voices";
  static constexpr auto P_UNISON_DETUNE = "unison_detune";

  static constexpr auto P_ADSR_SKIP    = "adsr_skip";
  static constexpr auto P_ADSR_ATTACK  = "adsr_attack";
  static constexpr auto P_ADSR_DECAY   = "adsr_decay";
  static constexpr auto P_ADSR_SUSTAIN = "adsr_sustain";
  static constexpr auto P_ADSR_RELEASE = "adsr_release";

  static constexpr auto P_FILTER_ATTACK     = "filter_attack";
  static constexpr auto P_FILTER_DECAY      = "filter_decay";
  static constexpr auto P_FILTER_SUSTAIN    = "filter_sustain";
  static constexpr auto P_FILTER_RELEASE    = "filter_release";
  static constexpr auto P_FILTER_DEPTH      = "filter_depth";
  static constexpr auto P_FILTER_CUTOFF     = "filter_cutoff";
  static constexpr auto P_FILTER_RESONANCE  = "filter_resonance";

  static constexpr auto P_PORTAMENTO_GLIDE  = "portamento_glide";

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

  void           set_sines (bool es);
  bool           sines() const;

  void           set_noise (bool en);
  bool           noise() const;

  void           set_unison (bool eu);
  bool           unison() const;

  void           set_adsr (bool eadsr);
  bool           adsr() const;

  void           set_filter (bool efilter);
  bool           filter() const;

  void           set_portamento (bool ep);
  bool           portamento() const;

  void           set_vibrato (bool ev);
  bool           vibrato() const;

  void           set_channel_op (int ch, MorphOperator *op);
  MorphOperator *channel_op (int ch);

/* slots: */
  void on_operator_removed (MorphOperator *op);
};

}

#endif
