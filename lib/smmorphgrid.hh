// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_HH
#define SPECTMORPH_MORPH_GRID_HH

#include "smmorphoperator.hh"

#include <map>

namespace SpectMorph
{

class MorphGrid : public MorphOperator
{
  Q_OBJECT

  int m_width;
  int m_height;
  int m_selected_x;
  int m_selected_y;

  std::vector< std::vector<MorphOperator *> > m_input_op;
  std::map<std::string, std::string>          load_map;

  void update_size();
public:
  MorphGrid (MorphPlan *morph_plan);
  ~MorphGrid();

  // inherited from MorphOperator
  const char        *type();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  void               post_load (OpNameMap& op_name_map);
  OutputType         output_type();

  void            set_width (int width);
  int             width();

  void            set_height (int height);
  int             height();

  void            set_selected_x (int x);
  void            set_selected_y (int y);
  int             selected_x();
  int             selected_y();
  bool            has_selection();

  void            set_input_op (int x, int y, MorphOperator *op);
  MorphOperator  *input_op (int x, int y);
};

}

#endif
