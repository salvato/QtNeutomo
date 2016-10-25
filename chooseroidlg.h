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
#ifndef CHOOSEROIDLG_H
#define CHOOSEROIDLG_H

#include <QDialog>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)

class ChooseRoiDlg : public QDialog
{
  Q_OBJECT

public:
  ChooseRoiDlg(QWidget *parent = 0);
  QRectF GetSelection();

public slots:
  void onDoneButtonPressed();
  void onCancelButtonPressed();
  void onStartPosEvent(QPoint);
  void onEndPosEvent(QPoint);

signals:
  void roiSelected(QRect Rect);
  void roiDiscarded();

protected:
  void createWidgets();
  void buildLayout();
  void initStatus();
  void initSignals();

private:
  QLabel* xLeftLabel;
  QLabel* yTopLabel;
  QLabel* xRightLabel;
  QLabel* yBottomLabel;

  QLineEdit* xLeftEdit;
  QLineEdit* yTopEdit;
  QLineEdit* xRightEdit;
  QLineEdit* yBottomEdit;

  QPushButton* doneButton;
  QPushButton* cancelButton;
  QPushButton* applyButton;
  QPushButton* selectAllButton;

  double xStart;
  double yStart;
  double xLeft;
  double xRight;
  double yTop;
  double yBottom;
};

#endif // CHOOSEROIDLG_H
