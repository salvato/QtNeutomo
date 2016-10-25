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
#include "chooseroidlg.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QSettings>

ChooseRoiDlg::ChooseRoiDlg(QWidget *parent)
  : QDialog(parent)
  , xLeft(0)
  , xRight(0)
  , yTop(0)
  , yBottom(0)
{
  createWidgets();
  buildLayout();
  // Restore previous section values
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  restoreGeometry(settings.value("ChooseRoiDlg/geometry").toByteArray());
  initSignals();
}


void
ChooseRoiDlg::createWidgets() {
  xLeftLabel   = new QLabel(tr("X Left"));
  yTopLabel    = new QLabel(tr("Y Top"));
  xRightLabel  = new QLabel(tr("X Right"));
  yBottomLabel = new QLabel(tr("Y Bottom"));

  xLeftEdit   = new QLineEdit(tr("0"));
  yTopEdit    = new QLineEdit(tr("0"));
  xRightEdit  = new QLineEdit(tr("0"));
  yBottomEdit = new QLineEdit(tr("0"));

  doneButton      = new QPushButton("Done");
  cancelButton    = new QPushButton("Cancel");
  applyButton     = new QPushButton("Apply");
  selectAllButton = new QPushButton("Select\nAll");

  // Tooltips:
  xLeftEdit->setToolTip(tr("Set X Left coordinate"));
  yTopEdit->setToolTip(tr("Set Y Top coordinate"));
  xRightEdit->setToolTip(tr("Set X Right coordinate"));
  yBottomEdit->setToolTip(tr("Set Y Bottom coordinate"));
}


void
ChooseRoiDlg::buildLayout() {
  QGridLayout* mainLayout = new QGridLayout();

  mainLayout->addWidget(xLeftLabel,   1, 1);
  mainLayout->addWidget(yTopLabel,    2, 1);
  mainLayout->addWidget(xRightLabel,  3, 1);
  mainLayout->addWidget(yBottomLabel, 4, 1);

  mainLayout->addWidget(xLeftEdit,   1, 2);
  mainLayout->addWidget(yTopEdit,    2, 2);
  mainLayout->addWidget(xRightEdit,  3, 2);
  mainLayout->addWidget(yBottomEdit, 4, 2);

  mainLayout->addWidget(selectAllButton, 2, 3, 2, 1);

  mainLayout->addWidget(doneButton,   5, 1, 1, 2, Qt::AlignHCenter);
  mainLayout->addWidget(cancelButton, 5, 3, 1, 2, Qt::AlignHCenter);

  setLayout(mainLayout);

}


void
ChooseRoiDlg::initStatus() {
  xLeftEdit->setText(QString("%1").arg(xLeft));
  yTopEdit->setText(QString("%1").arg(yTop));
  xRightEdit->setText(QString("%1").arg(xRight));
  yBottomEdit->setText(QString("%1").arg(yBottom));
  update();
}

void
ChooseRoiDlg::initSignals() {
  connect(doneButton,   SIGNAL(clicked(bool)), this, SLOT(onDoneButtonPressed()));
  connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(onCancelButtonPressed()));
}


void
ChooseRoiDlg::onDoneButtonPressed() {
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  settings.setValue("ChooseRoiDlg/geometry",      saveGeometry());
  QRect Rect(QPoint(xLeft, yTop), QPoint(xRight, yBottom));
  emit roiSelected(Rect);
}

void
ChooseRoiDlg::onCancelButtonPressed() {
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  settings.setValue("ChooseRoiDlg/geometry", saveGeometry());
  emit roiDiscarded();
}


void
ChooseRoiDlg::onStartPosEvent(QPoint newPoint) {
  xStart  = newPoint.x();
  yStart  = newPoint.y();
  xLeft   = xStart;
  xRight  = xStart;
  yBottom = yStart;
  yBottom = yStart;
  xLeftEdit->setText(QString("%1").arg(xLeft));
  yTopEdit->setText(QString("%1").arg(yTop));
  xRightEdit->setText(QString("%1").arg(xRight));
  yBottomEdit->setText(QString("%1").arg(yBottom));
  update();
}


void
ChooseRoiDlg::onEndPosEvent(QPoint newPoint) {
  if(newPoint.x() > xStart) {
    xLeft   = xStart;
    xRight  = newPoint.x();
  } else {
    xRight = xStart;
    xLeft = newPoint.x();
  }

  if(newPoint.y() < yStart) {
    yTop    = yStart;
    yBottom = newPoint.y();
  } else {// New point in the second quadrant (origin in xLeft, yTop)
    yBottom    = yStart;
    yTop = newPoint.y();
  }
  xLeftEdit->setText(QString("%1").arg(xLeft));
  yTopEdit->setText(QString("%1").arg(yTop));
  xRightEdit->setText(QString("%1").arg(xRight));
  yBottomEdit->setText(QString("%1").arg(yBottom));
  update();
}


QRectF
ChooseRoiDlg::GetSelection() {
  return QRectF(QPoint(xLeft, yTop), QPoint(xRight, yBottom));
}
