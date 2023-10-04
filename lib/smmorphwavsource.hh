// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_HH

#include "smmorphoperator.hh"
#include "smmodulationlist.hh"
#include "smproperty.hh"

#include <string>

namespace SpectMorph
{

class MorphWavSource;
class Project;
class Instrument;

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
    ModulationData    position_mod;
  };
  static constexpr auto P_PLAY_MODE = "play_mode";
  static constexpr auto P_POSITION  = "position";

  static constexpr auto USER_BANK = "User";
protected:
  Config      m_config;

  int         m_instrument = 1;
  std::string m_bank = USER_BANK;
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
  std::vector<MorphOperator *> dependencies() override;
  MorphOperatorConfig *clone_config() override;

  void        set_object_id (int id);
  int         object_id();

  void        set_instrument (int inst);
  int         instrument();

  void        set_bank (const std::string& bank);
  std::string bank();

  void        set_lv2_filename (const std::string& filename);
  std::string lv2_filename();

  void        on_instrument_updated (const std::string& bank, int number, const Instrument *new_instrument);
};

}

#endif
