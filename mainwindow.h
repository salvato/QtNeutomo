/*
 *

QtNeuTomo performs tomographic reconstructions from parallel
beam projection data.


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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "preprocessdlg.h"

QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QVBoxLayout)
QT_FORWARD_DECLARE_CLASS(CProjection)
QT_FORWARD_DECLARE_CLASS(ImageWindow)
QT_FORWARD_DECLARE_CLASS(ChooseRoiDlg)
QT_FORWARD_DECLARE_CLASS(DoTomoDlg)
QT_FORWARD_DECLARE_CLASS(CProjection)

class MainWindow : public QWidget
{
  Q_OBJECT

public:// Functions
  explicit MainWindow(QWidget *parent = 0);

public slots:
  void onPreProcessPushed();
  void onDoTomoPushed();
  void onRoiSelected(QRect Selection);
  void onCancelRoiSelection();
  void onBeamSelected(QRect Selection);
  void onCancelBeamSelection();

protected:
  void closeEvent(QCloseEvent *event);

private:// Functions
  void initLayout();
  bool checkOpenGL();
  void initEvents();
  bool checkAndDeleteFiles(QString sPath);
  bool buildImgSum(QString sDirectory, CProjection* pImgSum);
  bool buildImgSum(QString sDirectory, CProjection* pImgSum, QRect cropRegion, bool bMedianFilter);
  void normalizeProjections();
  void enableWidgets(bool bEnable);
  bool arrangeProjections(QString sPath);

private:// Objects
  QPushButton* pButtonPreprocess;
  QPushButton* pButtonDoTomo;
  QLineEdit*   pStatusLine;
  ImageWindow* pImageWindow;
  CProjection* pProjSum;

  PreProcessDlg* pPreProcessDlg;
  ChooseRoiDlg*  pChooseRoiDlg;
  DoTomoDlg*     pDoTomoDlg;

  QRect cropRegion;
  QRect beamRegion;

  QVector<float> angle;
  QVector<QString> filename;

  bool bDoTomoEnabled;
  bool bOpenBeamCorr;
};
#endif // MAINWINDOW_H
