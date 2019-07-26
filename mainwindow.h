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

#pragma once
#include "preprocessdlg.h"


// File Formats
enum
ENUM_PROJECTION_FORMATS {
  PROJECTION_FORMAT_BMP             = 0,
  PROJECTION_FORMAT_FITS            = 1,
  PROJECTION_FORMAT_BINARY          = 2,
  PROJECTION_FORMAT_MODIFIED_BINARY = 3,
  PROJECTION_FORMAT_JPEG            = 4
};


enum
FILTER_TYPES {
  SHEPP_LOGAN = 0,
  RAM_LAK     = 1
};


enum
BEAM_GEOMETRY {
  PARALLEL_BEAM = 0,
  CONE_BEAM     = 1
};

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
  explicit MainWindow(QWidget *parent = nullptr);

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
  bool lineRegress(float *x, float *y, int n, float *a, float *b, float *r);
  void crossCorr(float *y1, float *y2, float *r, int nSamples, int nChan);
  bool FindCenter(CProjection* pProjection0, CProjection* pProjection180);

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

  QVector<float> angles;
  QVector<QString> fileNames;

  bool bDoTomoEnabled;
  bool bOpenBeamCorr;

  float rotationCenter;
  float tiltAngle;
};
