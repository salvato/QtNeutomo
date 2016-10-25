/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#ifndef DOTOMODLG_H
#define DOTOMODLG_H

#include <QDialog>
#include <QObject>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QPushButton)

class DoTomoDlg : public QDialog
{
  Q_OBJECT

public:
  DoTomoDlg(QWidget *parent = 0);
  QString getSlicesPath();
  QString getNormalizedPath();

public slots:
  void onNormalizedPathButtonPressed();
  void onSlicesPathButtonPressed();
  void onOkButtonPressed();
  void onCancelButtonPressed();

protected:
  void createWidgets();
  void buildLayout();
  void initStatus();
  void initSignals();

private:
  QLabel* normalizedPathLabel;
  QLabel* slicesPathLabel;

  QLineEdit* normalizedPathEdit;
  QLineEdit* slicesPathEdit;

  QPushButton* normalizedPathButton;
  QPushButton* slicesPathButton;
  QPushButton* okButton;
  QPushButton* cancelButton;
};

#endif // DOTOMODLG_H
