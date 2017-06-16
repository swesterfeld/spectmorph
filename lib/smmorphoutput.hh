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

  XParamProperty<MorphOutput>   portamento_glide;

  XParamProperty<MorphOutput>   vibrato_depth;
  LogParamProperty<MorphOutput> vibrato_frequency;
  XParamProperty<MorphOutput>   vibrato_attack;
};

class MorphOutput : public MorphOperator
{
  Q_OBJECT

  std::vector<std::string>     load_channel_op_names;
  std::vector<MorphOperator *> channel_ops;

  bool                         m_sines;
  bool                         m_noise;

  bool                         m_unison;
  int                          m_unison_voices;
  float                        m_unison_detune;

  bool                         m_portamento;
  float                        m_portamento_glide;

  bool                         m_vibrato;
  float                        m_vibrato_depth;
  float                        m_vibrato_frequency;
  float                        m_vibrato_attack;

public:
  MorphOutput (MorphPlan *morph_plan);
  ~MorphOutput();

  // inherited from MorphOperator
  const char        *type();
  int                insert_order();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  void               post_load (OpNameMap& op_name_map);
  OutputType         output_type();

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

  void           set_portamento (bool ep);
  bool           portamento() const;

  void           set_portamento_glide (float glide);
  float          portamento_glide() const;

  void           set_vibrato (bool ev);
  bool           vibrato() const;

  void           set_vibrato_depth (float depth);
  float          vibrato_depth() const;

  void           set_vibrato_frequency (float frequency);
  float          vibrato_frequency() const;

  void           set_vibrato_attack (float attack);
  float          vibrato_attack() const;

  void           set_channel_op (int ch, MorphOperator *op);
  MorphOperator *channel_op (int ch);

public slots:
  void on_operator_removed (MorphOperator *op);
};

}

#endif
