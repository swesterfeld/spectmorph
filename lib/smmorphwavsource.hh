// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

class MorphWavSource : public MorphOperator
{
public:
  enum PlayMode {
    PLAY_MODE_STANDARD        = 1,
    PLAY_MODE_CUSTOM_POSITION = 2
  };
protected:
  int         m_object_id  = 0;
  int         m_instrument = 1;
  std::string m_lv2_filename;
  PlayMode    m_play_mode = PLAY_MODE_STANDARD;

public:
  MorphWavSource (MorphPlan *morph_plan);
  ~MorphWavSource();

  // inherited from MorphOperator
  const char *type();
  int         insert_order();
  bool        save (OutFile& out_file);
  bool        load (InFile&  in_file);
  OutputType  output_type();

  void        set_object_id (int id);
  int         object_id();

  void        set_instrument (int inst);
  int         instrument();

  void        set_lv2_filename (const std::string& filename);
  std::string lv2_filename();

  void        set_play_mode (PlayMode play_mode);
  PlayMode    play_mode() const;
};

}

#endif
