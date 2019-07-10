// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoperatorview.hh"
#include "smrenameopwindow.hh"
#include "smoperatorrolemap.hh"

using namespace SpectMorph;

using std::string;

MorphOperatorView::MorphOperatorView (Widget *parent, MorphOperator *op, MorphPlanWindow *window) :
  Frame (parent),
  morph_plan_window (window),
  m_op (op)
{
  FixedGrid grid;

  title_label = new MorphOperatorTitle (this, "");
  title_label->set_align (TextAlign::CENTER);
  title_label->set_bold (true);
  grid.add_widget (title_label, 0, 0, 43, 4);

  connect (title_label->signal_move, this, &MorphOperatorView::on_move);
  connect (title_label->signal_end_move, this, &MorphOperatorView::on_end_move);
  connect (title_label->signal_rename, this, &MorphOperatorView::on_rename);

  fold_button = new ToolButton (this);
  grid.add_widget (fold_button, 2, 1, 2, 2);
  connect (fold_button->signal_clicked, this, &MorphOperatorView::on_fold_clicked);

  close_button = new ToolButton (this, 'x');
  grid.add_widget (close_button, 39, 1, 2, 2);
  connect (close_button->signal_clicked, [=]() { m_op->morph_plan()->remove (m_op); });

  body_widget = new Widget (this);

  update_body_visible();

  /* title update */
  connect (m_op->morph_plan()->signal_plan_changed, this, &MorphOperatorView::on_operators_changed);
  on_operators_changed();
}

void
MorphOperatorView::set_role (int role)
{
  if (m_role != role)
    {
      m_role = role;
      on_operators_changed();
    }
}

void
MorphOperatorView::set_role_colors()
{
  if (m_role == 2) /* directly connected to output */
    {
      title_label->set_color (Color (0.3, 0.9, 0.3));
      set_frame_color (ThemeColor::FRAME);
    }
  else if (m_role > 0) /* output or reachable */
    {
      title_label->set_color (ThemeColor::TEXT);
      set_frame_color (ThemeColor::FRAME);
    }
  else /* unreachable */
    {
      title_label->set_color (Color (0.7, 0.7, 0.7));
      set_frame_color (Color (0.7, 0.7, 0.7));
    }
}

void
MorphOperatorView::on_operators_changed()
{
  string title = m_op->type_name() + ": " + m_op->name();

  set_role_colors();

  title_label->set_text (title);
}

void
MorphOperatorView::on_move (double y)
{
  if (is_output()) // output operator: move not supported
    return;

  set_frame_color (ThemeColor::MENU_ITEM);

  MorphOperator *op_next = morph_plan_window->where (m_op, y);

  signal_move_indication (op_next, false);
}

void
MorphOperatorView::on_end_move (double y)
{
  if (is_output()) // output operator: move not supported
    return;

  set_role_colors();

  MorphOperator *op_next = morph_plan_window->where (m_op, y);

  signal_move_indication (op_next, true); // done

  if (op_next != m_op) // avoid redundant moves
    {
      // DELETION can occur here
      m_op->morph_plan()->move (m_op, op_next);
    }
}

void
MorphOperatorView::on_rename()
{
  RenameOpWindow::create (window(), m_op);
}
