/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smmorphoperatorview.hh"
#include "smmorphoperator.hh"
#include "smrenameoperatordialog.hh"

#include <QGroupBox>
#include <QMenu>
#include <QContextMenuEvent>

using namespace SpectMorph;

using std::string;

MorphOperatorView::MorphOperatorView (MorphOperator *op, MorphPlanWindow *morph_plan_window) :
  m_op (op),
  morph_plan_window (morph_plan_window),
  in_move (false)
{
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
MorphOperatorView::on_operators_changed()
{
  string title = m_op->type_name() + ": " + m_op->name();

  setTitle (title.c_str());
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

      emit move_indication (op_next);
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
#include "smmorphlfoview.hh"

MorphOperatorView *
MorphOperatorView::create (MorphOperator *op, MorphPlanWindow *window)
{
  string type = op->type();

  if (type == "SpectMorph::MorphSource") return new MorphSourceView (static_cast<MorphSource *> (op), window);
  if (type == "SpectMorph::MorphOutput") return new MorphOutputView (static_cast<MorphOutput *> (op), window);
  if (type == "SpectMorph::MorphLinear") return new MorphLinearView (static_cast<MorphLinear *> (op), window);
  if (type == "SpectMorph::MorphLFO")    return new MorphLFOView    (static_cast<MorphLFO *> (op),    window);

  return NULL;
}
