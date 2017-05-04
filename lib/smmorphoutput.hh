// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_HH
#define SPECTMORPH_MORPH_OUTPUT_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

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

  bool                         m_adsr;
  float                        m_adsr_skip;
  float                        m_adsr_attack;
  float                        m_adsr_decay;
  float                        m_adsr_sustain;
  float                        m_adsr_release;

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
  bool           sines();

  void           set_noise (bool en);
  bool           noise();

  void           set_unison (bool eu);
  bool           unison();

  void           set_unison_voices (int voices);
  int            unison_voices() const;

  void           set_unison_detune (float voices);
  float          unison_detune() const;

  void           set_adsr (bool eadsr);
  bool           adsr() const;

  void           set_adsr_skip (float skip);
  float          adsr_skip() const;

  void           set_adsr_attack (float attack);
  float          adsr_attack() const;

  void           set_adsr_decay (float decay);
  float          adsr_decay() const;

  void           set_adsr_sustain (float sustain);
  float          adsr_sustain() const;

  void           set_adsr_release (float release);
  float          adsr_release() const;

  void           set_channel_op (int ch, MorphOperator *op);
  MorphOperator *channel_op (int ch);

public slots:
  void on_operator_removed (MorphOperator *op);
};

}

#endif
