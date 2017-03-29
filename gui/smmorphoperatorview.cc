// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoperatorview.hh"
#include "smmorphoperator.hh"
#include "smrenameoperatordialog.hh"

#include <QGroupBox>
#include <QLabel>
#include <QMenu>
#include <QContextMenuEvent>
#include <QToolButton>

using namespace SpectMorph;

using std::string;

MorphOperatorView::MorphOperatorView (MorphOperator *op, MorphPlanWindow *morph_plan_window) :
  m_op (op),
  morph_plan_window (morph_plan_window),
  in_move (false)
{
  setObjectName ("operator-view");

  head_label = new QLabel();
  head_label->setAlignment (Qt::AlignCenter);
  head_label->setStyleSheet ("font-weight: bold;");

  QAction *action;
  context_menu = new QMenu ("Operator Context Menu", this);
  action = context_menu->addAction ("Rename");
  connect (action, SIGNAL (triggered()), this, SLOT (on_rename()));

  action = context_menu->addAction ("Remove");
  connect (action, SIGNAL (triggered()), this, SLOT (on_remove()));
  remove = false;

  on_operators_changed();
  connect (m_op->morph_plan(), SIGNAL (plan_changed()), this, SLOT (on_operators_changed()));
}

void
MorphOperatorView::set_body_layout (QLayout *body_layout)
{
  body_widget = new QWidget ();
  body_widget->setLayout (body_layout);
  body_layout->setContentsMargins (0, 0, 0, 0);

  QGridLayout *hb_layout = new QGridLayout();

  fold_button = new QToolButton();
  fold_button->setArrowType (Qt::RightArrow);
  fold_button->setStyleSheet ("QToolButton { border: none; }");
  fold_button->setToolButtonStyle (Qt::ToolButtonTextBesideIcon);
  connect (fold_button, SIGNAL (clicked()), this, SLOT (on_fold_clicked()));

  hb_layout->addWidget (head_label, 0, 0, 1, 2);
  hb_layout->addWidget (fold_button, 0, 0);
  hb_layout->addWidget (body_widget, 1, 0, 1, 2);
  setLayout (hb_layout);

  update_body_visible();
}

void
MorphOperatorView::on_fold_clicked()
{
  m_op->set_folded (!m_op->folded());

  update_body_visible();

  Q_EMIT need_resize();
}

void
MorphOperatorView::update_body_visible()
{
  body_widget->setVisible (!m_op->folded());
  fold_button->setArrowType (m_op->folded() ? Qt::RightArrow : Qt::DownArrow);
}

void
MorphOperatorView::on_operators_changed()
{
  string title = m_op->type_name() + ": " + m_op->name();

  head_label->setText (title.c_str());
}

void
MorphOperatorView::contextMenuEvent (QContextMenuEvent *event)
{
  event->accept();
  context_menu->exec (event->globalPos());
  if (remove)
    m_op->morph_plan()->remove (m_op);
}

void
MorphOperatorView::mousePressEvent (QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
      in_move = true;
      setCursor (Qt::SizeAllCursor);
    }
}

void
MorphOperatorView::mouseMoveEvent (QMouseEvent *event)
{
  if (in_move)
    {
      MorphOperator *op_next = morph_plan_window->where (m_op, event->globalPos());

      Q_EMIT move_indication (op_next);
    }
}

void
MorphOperatorView::mouseReleaseEvent (QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
      MorphOperator *op_next = morph_plan_window->where (m_op, event->globalPos());
      in_move = false;
      unsetCursor();

      // DELETION can occur here
      m_op->morph_plan()->move (m_op, op_next);
    }
}

void
MorphOperatorView::on_rename()
{
  RenameOperatorDialog dialog (this, m_op);
  if (dialog.exec())
    m_op->set_name (dialog.new_name());
}

void
MorphOperatorView::on_remove()
{
  // can't remove in the context menu callstack - so do it later
  remove = true;
}

MorphOperator *
MorphOperatorView::op()
{
  return m_op;
}

#include "smmorphsourceview.hh"
#include "smmorphoutputview.hh"
#include "smmorphlinearview.hh"
#include "smmorphgridview.hh"
#include "smmorphlfoview.hh"

MorphOperatorView *
MorphOperatorView::create (MorphOperator *op, MorphPlanWindow *window)
{
  string type = op->type();

  if (type == "SpectMorph::MorphSource") return new MorphSourceView (static_cast<MorphSource *> (op), window);
  if (type == "SpectMorph::MorphOutput") return new MorphOutputView (static_cast<MorphOutput *> (op), window);
  if (type == "SpectMorph::MorphLinear") return new MorphLinearView (static_cast<MorphLinear *> (op), window);
  if (type == "SpectMorph::MorphGrid")   return new MorphGridView   (static_cast<MorphGrid *> (op),   window);
  if (type == "SpectMorph::MorphLFO")    return new MorphLFOView    (static_cast<MorphLFO *> (op),    window);

  return NULL;
}
