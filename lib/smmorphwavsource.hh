// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_HH

#include "smmorphoperator.hh"
#include "smproperty.hh"

#include <string>

namespace SpectMorph
{

class MorphWavSource;
class Project;

class MorphWavSource : public MorphOperator
{
public:
  enum PlayMode {
    PLAY_MODE_STANDARD        = 1,
    PLAY_MODE_CUSTOM_POSITION = 2
  };
  struct Config : public MorphOperatorConfig
  {
    Project          *project = nullptr;

    int               object_id = 0;
    PlayMode          play_mode             = PLAY_MODE_STANDARD;
    ControlType       position_control_type = CONTROL_GUI;
    float             position;
    MorphOperatorPtr  position_op;
  };
  static constexpr auto P_POSITION = "position";
protected:
  Config      m_config;
  std::string load_position_op;

  int         m_instrument = 1;
  std::string m_lv2_filename;

public:
  MorphWavSource (MorphPlan *morph_plan);
  ~MorphWavSource();

  // inherited from MorphOperator
  const char *type() override;
  int         insert_order() override;
  bool        save (OutFile& out_file) override;
  bool        load (InFile&  in_file) override;
  OutputType  output_type() override;
  void        post_load (OpNameMap& op_name_map) override;
  std::vector<MorphOperator *> dependencies() override;
  MorphOperatorConfig *clone_config() override;

  void        set_object_id (int id);
  int         object_id();

  void        set_instrument (int inst);
  int         instrument();

  void        set_lv2_filename (const std::string& filename);
  std::string lv2_filename();

  void        set_play_mode (PlayMode play_mode);
  PlayMode    play_mode() const;

  void        set_position_control_type (ControlType new_control_type);
  ControlType position_control_type() const;

  void        set_position_op (MorphOperator *op);
  MorphOperator *position_op() const;

  void        set_position_control_type_and_op (ControlType new_control_type, MorphOperator *op);

  void        on_operator_removed (MorphOperator *op);
};

}

#endif
