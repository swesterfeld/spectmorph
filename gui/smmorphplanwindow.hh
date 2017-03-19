// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_WINDOW_HH
#define SPECTMORPH_MORPH_PLAN_WINDOW_HH

#include <assert.h>
#include <sys/time.h>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smmorphplan.hh"
#include "smmorphsource.hh"
#include "smmorphplanview.hh"
#include "smmorphoperator.hh"

#include <QWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QScrollArea>

namespace SpectMorph
{

class MorphPlanWindow : public QMainWindow
{
  Q_OBJECT

  std::string    m_filename;
  std::string    win_title;
  MorphPlanPtr   m_morph_plan;
  MorphPlanView *morph_plan_view;
  QScrollArea   *scroll_area;

  void add_op_action (QMenu *menu, const char *title, const char *type);
  void fill_template_menu (QMenu *menu);
  void update_window_title();
  bool load (const std::string& filename);

public:
  MorphPlanWindow (MorphPlanPtr morph_plan, const std::string& title);

  MorphOperator *where (MorphOperator *op, const QPoint& pos);
  void add_control_widget (QWidget *widget);
  void set_filename (const std::string& filename);

public slots:
  void on_file_import_clicked();
  void on_file_export_clicked();
  void on_load_index_clicked();
  void on_add_operator();
  void on_load_template();
  void on_need_resize();
  void on_update_window_size();

signals:
  void update_size();
};

}

#endif
