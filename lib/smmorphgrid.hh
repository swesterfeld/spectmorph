// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_HH
#define SPECTMORPH_MORPH_GRID_HH

#include "smmorphoperator.hh"

namespace SpectMorph
{

class MorphGrid : public MorphOperator
{
  Q_OBJECT

  int m_width;
  int m_height;

public:
  MorphGrid (MorphPlan *morph_plan);
  ~MorphGrid();

  // inherited from MorphOperator
  const char        *type();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  OutputType         output_type();

  void        set_width (int width);
  int         width();

  void        set_height (int height);
  int         height();
};

}

#endif
