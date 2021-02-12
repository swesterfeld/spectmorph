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

struct MorphOutputProperties
{
  MorphOutputProperties (MorphOutput *output);

  XParamProperty<MorphOutput>      portamento_glide;
};

class MorphOutput : public MorphOperator
{
public:
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

    bool                          portamento;
    float                         portamento_glide;

    bool                          vibrato;
    float                         vibrato_depth;
    float                         vibrato_frequency;
    float                         vibrato_attack;
  };
  Config                       m_config;

  static constexpr auto P_VELOCITY_SENSITIVITY = "velocity_sensitivity";

  static constexpr auto P_ADSR_SKIP    = "adsr_skip";
  static constexpr auto P_ADSR_ATTACK  = "adsr_attack";
  static constexpr auto P_ADSR_DECAY   = "adsr_decay";
  static constexpr auto P_ADSR_SUSTAIN = "adsr_sustain";
  static constexpr auto P_ADSR_RELEASE = "adsr_release";

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

  void           set_unison_voices (int voices);
  int            unison_voices() const;

  void           set_unison_detune (float voices);
  float          unison_detune() const;

  void           set_adsr (bool eadsr);
  bool           adsr() const;

  void           set_portamento (bool ep);
  bool           portamento() const;

  void           set_portamento_glide (float glide);
  float          portamento_glide() const;

  void           set_vibrato (bool ev);
  bool           vibrato() const;

  void           set_velocity_sensitivity (float vsense);
  float          velocity_sensitivity() const;

  void           set_channel_op (int ch, MorphOperator *op);
  MorphOperator *channel_op (int ch);

/* slots: */
  void on_operator_removed (MorphOperator *op);
};

}

#endif
