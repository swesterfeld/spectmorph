// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_HH
#define SPECTMORPH_MORPH_GRID_HH

#include "smmorphoperator.hh"

#include <map>

namespace SpectMorph
{

struct MorphGridNode
{
  MorphOperatorPtr  op;                     // a node has either an operator (op) as input,
  std::string       smset;                  // or an instrument (smset)
  std::string       path;
  double            delta_db;

  MorphGridNode();
};

class MorphGrid : public MorphOperator
{
public:
  struct Config : public MorphOperatorConfig
  {
    int               width;
    int               height;

    double            x_morphing;
    double            y_morphing;

    ControlType       x_control_type;
    ControlType       y_control_type;
    MorphOperatorPtr  x_control_op;
    MorphOperatorPtr  y_control_op;

    std::vector< std::vector<MorphGridNode> > input_node;
  };
protected:
  Config          m_config;
  int             m_zoom;
  int             m_selected_x;
  int             m_selected_y;

  std::map<std::string, std::string>        load_map;

  void update_size();
public:
  MorphGrid (MorphPlan *morph_plan);
  ~MorphGrid();

  // inherited from MorphOperator
  const char        *type() override;
  int                insert_order() override;
  bool               save (OutFile& out_file) override;
  bool               load (InFile&  in_file) override;
  void               post_load (OpNameMap& op_name_map) override;
  OutputType         output_type() override;
  MorphOperatorConfig *clone_config() override;

  std::vector<MorphOperator *> dependencies() override;

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
  void            set_x_control_type_and_op (ControlType control_type, MorphOperator *op);
  void            set_y_control_type_and_op (ControlType control_type, MorphOperator *op);

  void            set_input_node (int x, int y, const MorphGridNode& node);
  MorphGridNode   input_node (int x, int y);
  std::string     input_node_label (int x, int y);

/* slots: */
  void            on_operator_removed (MorphOperator *op);
};

}

#endif
