// Licensed GNU GPL v2 or later: http://www.gnu.org/licenses/gpl2.html

// some code copied from kdialog
//
//  Copyright (C) 1998 Matthias Hoelzer <hoelzer@kde.org>
//  Copyright (C) 2002 David Faure <faure@kde.org>
//  Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
//  Copyright (C) 2008 by Dmitry Suzdalev <dimsuz@gmail.com>
//  Copyright (C) 2011 Kai Uwe Broulik <kde@privat.broulik.de>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the7 implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include <QFileDialog>
#include <QApplication>
#include <QWindow>


// from kdialog:
// this class hooks into the eventloop and outputs the id
// of shown dialogs or makes the dialog transient for other winids.
// Will destroy itself on app exit.
class WinIdEmbedder: public QObject
{
public:
  WinIdEmbedder (WId winId) :
    QObject (qApp),
    id (winId)
  {
    if (qApp)
      qApp->installEventFilter (this);
  }
protected:
  bool eventFilter (QObject *o, QEvent *e);
private:
  WId id;
};

bool
WinIdEmbedder::eventFilter (QObject *o, QEvent *e)
{
  if (e->type() == QEvent::Show && o->isWidgetType() && o->inherits("QDialog") && id != 0)
    {
      QWidget *w = static_cast<QWidget*>(o);

      w->winId(); // ensure to create native window

      QWindow *dialog_win = w->windowHandle();
      QWindow *tparent = QWindow::fromWinId (id);

      if (dialog_win && tparent)
        dialog_win->setTransientParent (tparent);

      deleteLater(); // WinIdEmbedder is not needed anymore after the first dialog was shown
      return false;
    }
  return QObject::eventFilter(o, e);
}

int
show_dialog (bool open, QString dir, QString filter_spec, QString title, QString win_id_str)
{
  WId win_id = win_id_str.toLong();
  if (win_id)
    new WinIdEmbedder (win_id);

  QFileDialog dlg;
  if (open)
    {
      dlg.setAcceptMode (QFileDialog::AcceptOpen);
      dlg.setFileMode (QFileDialog::ExistingFile);
    }
  else
    {
      dlg.setAcceptMode (QFileDialog::AcceptSave);
      dlg.setFileMode (QFileDialog::AnyFile);
    }

  dlg.setDirectory (dir);
  dlg.setNameFilter (filter_spec.replace (QLatin1Char ('|'), QLatin1Char ('\n')));
  dlg.setWindowTitle (title);

  if (!dlg.exec())
    return 1;

  const QStringList result = dlg.selectedFiles();
  if (result.isEmpty())
    return 1;

  const QString file = result.at (0);
  printf ("%s\n", file.toLocal8Bit().data());
  return 0;
}

int
main (int argc, char **argv)
{
  QApplication app (argc, argv);

  if (argc != 6)
    {
      fprintf (stderr, "smfiledialog: bad number of args, expect (argc == 6), got %d\n", argc);
      return 1;
    }

  if (strcmp (argv[1], "open") == 0)
    {
      return show_dialog (true, argv[2], argv[3], argv[4], argv[5]);
    }
  else if (strcmp (argv[1], "save") == 0)
    {
      return show_dialog (false, argv[2], argv[3], argv[4], argv[5]);
    }
  else
    {
      fprintf (stderr, "smfiledialog: bad mode (expect: open or save)\n");
      return 1;
    }
}
