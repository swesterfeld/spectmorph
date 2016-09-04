// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_HH
#define SPECTMORPH_MORPH_GRID_HH

#include "smmorphoperator.hh"

#include <map>

namespace SpectMorph
{

struct MorphGridNode
{
  MorphOperator *op;                     // a node has either an operator (op) as input,
  std::string    smset;                  // or an instrument (smset)
  double         delta_db;

  MorphGridNode();
};

class MorphGrid : public MorphOperator
{
  Q_OBJECT

public:
  enum ControlType {
    CONTROL_GUI      = 1,
    CONTROL_SIGNAL_1 = 2,
    CONTROL_SIGNAL_2 = 3,
    CONTROL_OP       = 4
  };
protected:
  int             m_width;
  int             m_height;
  int             m_zoom;
  int             m_selected_x;
  int             m_selected_y;
  double          m_x_morphing;
  double          m_y_morphing;
  ControlType     m_x_control_type;
  ControlType     m_y_control_type;
  MorphOperator  *m_x_control_op;
  MorphOperator  *m_y_control_op;

  std::vector< std::vector<MorphGridNode> > m_input_node;
  std::map<std::string, std::string>        load_map;

  void update_size();
public:
  MorphGrid (MorphPlan *morph_plan);
  ~MorphGrid();

  // inherited from MorphOperator
  const char        *type();
  int                insert_order();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  void               post_load (OpNameMap& op_name_map);
  OutputType         output_type();

  void            set_width (int width);
  int             width();

  void            set_height (int height);
  int             height();

  void            set_zoom (int new_zoom);
  int             zoom();

  void            set_selected_x (int x);
  void            set_selected_y (int y);
  int             selected_x();
  int             selected_y();
  bool            has_selection();

  double          x_morphing();
  ControlType     x_control_type();
  MorphOperator  *x_control_op();
  double          y_morphing();
  ControlType     y_control_type();
  MorphOperator  *y_control_op();
  void            set_x_morphing (double new_value);
  void            set_y_morphing (double new_value);
  void            set_x_control_type (ControlType new_control_type);
  void            set_y_control_type (ControlType new_control_type);
  void            set_x_control_op (MorphOperator *op);
  void            set_y_control_op (MorphOperator *op);

  void            set_input_node (int x, int y, const MorphGridNode& node);
  MorphGridNode   input_node (int x, int y);
  std::string     input_node_label (int x, int y);

public slots:
  void            on_operator_removed (MorphOperator *op);
};

}

#endif
