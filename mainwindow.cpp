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

#include "mainwindow.h"
#include <QtGui>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Projection.h"
#include "imagewindow.h"
#include "chooseroidlg.h"
#include "dotomodlg.h"
#include <float.h>


MainWindow::MainWindow(QWidget *parent)
  : QWidget(parent)
  , pImageWindow(Q_NULLPTR)
  , pProjSum(Q_NULLPTR)
  , pPreProcessDlg(Q_NULLPTR)
  , pChooseRoiDlg(Q_NULLPTR)
  , pDoTomoDlg(Q_NULLPTR)
{
  qDebug() << "Ideal thread number ="
           << QThread::idealThreadCount();
  initLayout();// Init Window Layout
  if(!checkOpenGL()) return;
  pPreProcessDlg = new PreProcessDlg();
  pChooseRoiDlg = new ChooseRoiDlg();
  pDoTomoDlg = new DoTomoDlg();
  pImageWindow = new ImageWindow();
  pImageWindow->showNormal();
  initEvents();
  // Restore previous section values
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  move(QPoint(settings.value("MainWindow/geometry", QPoint(10, 10)).toPoint()));

}


void
MainWindow::closeEvent(QCloseEvent *event) {
  pImageWindow->close();
  delete pImageWindow;
  pImageWindow = Q_NULLPTR;
  pChooseRoiDlg->close();
  delete pChooseRoiDlg;
  pChooseRoiDlg = Q_NULLPTR;
  if(pProjSum != Q_NULLPTR) delete pProjSum;
  pProjSum = Q_NULLPTR;
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  settings.setValue("MainWindow/geometry", pos());
  QWidget::closeEvent(event);
}


void
MainWindow::initLayout() {
  QVBoxLayout* pMainLayout  = new QVBoxLayout;
  QVBoxLayout* pLeftLayout  = new QVBoxLayout;
  QVBoxLayout* pRightLayout = new QVBoxLayout;

  pButtonPreprocess = new QPushButton("Preprocess", this);
  pButtonPreprocess->setToolTip("Pre Processing of the Projections:\nYou can subtract dark images and/or\nperform a flat-field normalization");
  pButtonDoTomo     = new QPushButton("Do Tomo", this);
  pButtonDoTomo->setToolTip("Perform Tomographic reconstruction");

  pStatusLine       = new QLineEdit("Initializing");
  pStatusLine->setReadOnly(true);
  pButtonDoTomo->setToolTip("This is the status bar:\nIt shows the current status of\n the App");

  pLeftLayout->addWidget(pButtonPreprocess);
  pRightLayout->addWidget(pButtonDoTomo);

  QHBoxLayout* pPanel = new QHBoxLayout();
  pPanel->addLayout(pLeftLayout);
  pPanel->addLayout(pRightLayout);

  pMainLayout->addLayout(pPanel);

  pMainLayout->addWidget(pStatusLine);

  setLayout(pMainLayout);
}


void
MainWindow::initEvents() {
  connect(pButtonPreprocess, SIGNAL(clicked()), this, SLOT(onPreProcessPushed()));
  connect(pButtonDoTomo,     SIGNAL(clicked()), this, SLOT(onDoTomoPushed()));
}


bool
MainWindow::checkOpenGL() {
  // Create a native OpenGL context, enabling OpenGL rendering on a QSurface.
  QOpenGLContext* OpenGLContext = new QOpenGLContext(Q_NULLPTR);
  // Create an offscreen surface intended to be used with QOpenGLContext
  // to allow rendering with OpenGL in an arbitrary thread without the
  // need to create a QWindow.
  QOffscreenSurface* Surface = new QOffscreenSurface();
  QSurfaceFormat requestedFormat = QSurfaceFormat::defaultFormat();
  requestedFormat.setMajorVersion(5);
  requestedFormat.setMajorVersion(5);
  Surface->setFormat(requestedFormat);
  Surface->create();
  OpenGLContext->setFormat(requestedFormat);
  if(!OpenGLContext->create()) {
      qDebug() << "Failed to create OpenGL context";
      return false;
  }
  OpenGLContext->makeCurrent(Surface);

  bDoTomoEnabled = true;
  // Here is the place to check Graphic card capabilities !
  // After the GL context has been initialized
  char* cVersion1   = (char*)glGetString(GL_VERSION);
  qDebug() << QString("OpenGL Version: ")       + QString(cVersion1);
  char* cRenderer1  = (char*)glGetString(GL_RENDERER);
  qDebug() << QString("Graphic Adapter: ")      + QString(cRenderer1);
  char* cShader1 = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  qDebug() << QString("Shading Language Version: ")      + QString(cShader1);
  if(!OpenGLContext->hasExtension("GL_NV_texture_rectangle")) {
    qDebug() << "Sorry No ARB_texture_rectangle in OpenGL";
    return false;
  }
//  ARB_texture_rectangle
//  char* cExtensions = (char*)glGetString(GL_EXTENSIONS);
//  qDebug() << QString("Supported Extensions: ") + QString(cExtensions) + QString("\r\n");

/*
  if(!initCG()) bDoTomoEnabled = false;
  GLuint fbo;
  if(!initFBO(&fbo)) bDoTomoEnabled = false;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT,   1);
  // Create the 3 textures for the FBO
  GLuint outTexID[3];
  glGenTextures(3, outTexID);
  GLenum attachmentpoints[3];
  attachmentpoints[0] = GL_COLOR_ATTACHMENT0;
  attachmentpoints[1] = GL_COLOR_ATTACHMENT1;
  attachmentpoints[2] = GL_COLOR_ATTACHMENT2;
  glGenTextures(3, outTexID);
  if(!initFBOTextures(3, outTexID, 1, 2, attachmentpoints)) bDoTomoEnabled = false;


  if(!isExtensionSupported("GL_ARB_texture_rectangle") ||
     !isExtensionSupported("GL_ARB_texture_float")       // for GL_LUMINANCE32F_ARB support
    ) {
    bDoTomoEnabled = false;
  } else {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    GLuint TexID[3];
    glGenTextures(3, TexID);
    GLenum attachmentpoints[3];
    attachmentpoints[0] = GL_COLOR_ATTACHMENT0;
    attachmentpoints[1] = GL_COLOR_ATTACHMENT1;
    attachmentpoints[2] = GL_COLOR_ATTACHMENT2;
    for(int i=0; i<3; i++) {
      glBindTexture(GL_TEXTURE_RECTANGLE_ARB, TexID[i]);
      CheckGLErrors(_T("1"));
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,     GL_CLAMP);
      CheckGLErrors(_T("2"));
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,     GL_CLAMP);
      CheckGLErrors(_T("3"));
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      CheckGLErrors(_T("4"));
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      CheckGLErrors(_T("5"));
      glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE32F_ARB, 128, 128, 0, GL_LUMINANCE, GL_FLOAT, 0);
      if(!CheckGLErrors(_T("CheckGL()"))) {
      //if(glGetError() != GL_NO_ERROR) {
        bDoTomoEnabled =  false;
        break;
      } else {// Attach the texture to the FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentpoints[i], GL_TEXTURE_RECTANGLE_ARB, TexID[i], 0);
        if(!CheckFramebufferStatus()) {
          bDoTomoEnabled =  false;
          break;
        }
      }
    }
    glDeleteTextures(3, TexID);
      if(bDoTomoEnabled) {
      // Test for multiple GPU support
      if(wglGetExtensionsStringARB == NULL) {
        ::DestroyWindow(hwnd);
        return false;
      }
      const char *extensionsString = wglGetExtensionsStringARB(hdc);
     if(strstr(extensionsString, "WGL_NV_gpu_affinity")) {
        int gpuIndex = 0;
        HGPUNV hGPU[MAX_GPU];
        // Get a list of the first MAX_GPU GPUs in the system
        while((gpuIndex < MAX_GPU) && wglEnumGpusNV(gpuIndex, &hGPU[gpuIndex])) {
          gpuIndex++;
        }
        sString.Format(_T("Multiple GPU Support Enabled: %d GPUs Presents"),gpuIndex);
        ShowStatusMsg(sString);
        if(pMsg) pMsg->AddText(sString + _T("\r\n"));
      } else {
        sString = _T("Multiple GPU Support Not Present");
        ShowStatusMsg(sString);
        if(pMsg) pMsg->AddText(sString + _T("\r\n"));
      }
      // End Test forMultiple GPU Support
    }
  }
*/
/*
  glutDestroyWindow(glutWindowHandle);

  if(!bDoTomoEnabled) {
    sString = _T("Graphic card unable to perform Tomography !");
    ShowStatusMsg(sString);
    pMsg->AddText(sString + _T("\r\n"));
  }
*/
  OpenGLContext->doneCurrent();
  delete OpenGLContext;
  delete Surface;
  return true;
}


bool
MainWindow::checkAndDeleteFiles(QString sPath) {
  QDir directory = QDir(sPath, QString("*.fits"), QDir::Name, QDir::Files);
  if(!directory.exists()) {
    qDebug() << sPath << " Directory does not exists";
    return false;
  }
  QStringList fileList = directory.entryList();
  if(!fileList.isEmpty()) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(QString("The Directory \n%1\nis Not Empty !").arg(sPath));
    msgBox.setInformativeText(QString("Would you like to delete Files ?\nWarning:\nNO WAY TO RECOVER FILES ONCE DELETED !"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int response = msgBox.exec();
    if(response == QMessageBox::No)
      return false;
    // Delete all files in the directory
    foreach (QString file, fileList) {
      qDebug() << "Removing" << file;
      if(!directory.remove(file)) {
        qDebug() << "unable to delete" << file;
        return false;
      }
    }
  }
  return true;
}


void
MainWindow::enableWidgets(bool bEnable) {
  pButtonPreprocess->setEnabled(bEnable);
  pButtonDoTomo->setEnabled(bEnable);
}


void
MainWindow::onPreProcessPushed() {
  pStatusLine->setText("onPreProcessPushed");

  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  pPreProcessDlg->restoreGeometry(settings.value("PreProcessDlg/geometry").toByteArray());
  if(pPreProcessDlg->exec() == QDialog::Rejected)
    return;
  qDebug() << "Image Normalization Started...";
  enableWidgets(false);
  if(!checkAndDeleteFiles(pPreProcessDlg->getOutputPath())) {
    enableWidgets(true);
    return;
  }
  if(pProjSum != Q_NULLPTR) delete pProjSum;
  pProjSum = new CProjection();
  if(!buildImgSum(pPreProcessDlg->getInputPath(), pProjSum)) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("Unable to read Projection files or File Error"));
    msgBox.setInformativeText(QString("Probably no .fits Files in Directory"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    enableWidgets(true);
    return;
  }
  pProjSum->Normalize(0.0, 1.0);// Just to maximize the image contrast...

  if(pProjSum->n_columns > pProjSum->n_rows)
    pImageWindow->setGeometry(QRect(0, 0, 512, (512*pProjSum->n_rows)/pProjSum->n_columns));
  else
    pImageWindow->setGeometry(QRect(0, 0, (512*pProjSum->n_columns)/pProjSum->n_rows, 512));
  pImageWindow->loadTexture(pProjSum->pData, pProjSum->n_columns, pProjSum->n_rows);
  connect(pImageWindow, SIGNAL(startPosEvent(QPoint)), pChooseRoiDlg, SLOT(onStartPosEvent(QPoint)));
  connect(pImageWindow, SIGNAL(endPosEvent(QPoint)),   pChooseRoiDlg, SLOT(onEndPosEvent(QPoint)));
  pImageWindow->enableSelection(true);
  pImageWindow->show();
  qApp->processEvents();

  pChooseRoiDlg->setWindowTitle(tr("Choose a Region Of Interest"));
  pChooseRoiDlg->restoreGeometry(settings.value("ChooseRoiDlg/geometry").toByteArray());
  connect(pChooseRoiDlg, SIGNAL(roiSelected(QRect)), this, SLOT(onRoiSelected(QRect)));
  connect(pChooseRoiDlg, SIGNAL(roiDiscarded()),     this, SLOT(onCancelRoiSelection()));
  pChooseRoiDlg->show();
  qApp->processEvents();
  // Now we are waiting for a selected Region of Interest...
}


void
MainWindow::onRoiSelected(QRect Selection) {
  cropRegion = Selection;
  pImageWindow->enableSelection(false);
  disconnect(pImageWindow, SIGNAL(startPosEvent(QPoint)), pChooseRoiDlg, 0);
  disconnect(pImageWindow, SIGNAL(endPosEvent(QPoint)), pChooseRoiDlg, 0);
  disconnect(pChooseRoiDlg, SIGNAL(roiSelected(QRect)), this, 0);
  disconnect(pChooseRoiDlg, SIGNAL(roiDiscarded()),     this, 0);
  pChooseRoiDlg->hide();
  pProjSum->Crop(cropRegion);
  pProjSum->Normalize(0.0, 1.0);// Just to maximize the image contrast...
  if(pProjSum->n_columns > pProjSum->n_rows)
    pImageWindow->setGeometry(QRect(0, 0, 512, (512*pProjSum->n_rows)/pProjSum->n_columns));
  else
    pImageWindow->setGeometry(QRect(0, 0, (512*pProjSum->n_columns)/pProjSum->n_rows, 512));
  pImageWindow->loadTexture(pProjSum->pData, pProjSum->n_columns, pProjSum->n_rows);
  connect(pImageWindow, SIGNAL(startPosEvent(QPoint)), pChooseRoiDlg, SLOT(onStartPosEvent(QPoint)));
  connect(pImageWindow, SIGNAL(endPosEvent(QPoint)),   pChooseRoiDlg, SLOT(onEndPosEvent(QPoint)));
  pImageWindow->enableSelection(true);
  pImageWindow->update();

  pChooseRoiDlg->setWindowTitle(tr("Choose a Pure Beam Region"));
  connect(pChooseRoiDlg, SIGNAL(roiSelected(QRect)), this, SLOT(onBeamSelected(QRect)));
  connect(pChooseRoiDlg, SIGNAL(roiDiscarded()),     this, SLOT(onCancelBeamSelection()));
  pChooseRoiDlg->show();
  // Now we are waiting for a selected Pure Beam Region...
}


void
MainWindow::onBeamSelected(QRect Selection) {
  // We got a pure beam region selected...
  beamRegion = Selection;
  bOpenBeamCorr = true;
  normalizeProjections();
  enableWidgets(true);
}


void
MainWindow::onCancelBeamSelection() {
  // No pure beam region selected...
  bOpenBeamCorr = false;
  normalizeProjections();
}


void
MainWindow::normalizeProjections() {
  pImageWindow->enableSelection(false);
  disconnect(pImageWindow, SIGNAL(startPosEvent(QPoint)), pChooseRoiDlg, 0);
  disconnect(pImageWindow, SIGNAL(startPosEvent(QPoint)), pChooseRoiDlg, 0);
  disconnect(pChooseRoiDlg, SIGNAL(roiSelected(QRect)), this, 0);
  disconnect(pChooseRoiDlg, SIGNAL(roiDiscarded()),     this, 0);
  pChooseRoiDlg->hide();
  pImageWindow->update();
  qApp->processEvents();

  CProjection Dark;
  if(pPreProcessDlg->getDarkUse()) {
    if(!buildImgSum(pPreProcessDlg->getDarkPath(), &Dark, cropRegion, pPreProcessDlg->getDarkMedianFilter())) {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setText(QString("Unable to read Dark files or File Error"));
      msgBox.setInformativeText(QString("Probably no .fits Files in Directory"));
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
      enableWidgets(true);
      return;
    }
  }

  CProjection Beam;
  if(pPreProcessDlg->getFlatFieldUse()) {
    if(!buildImgSum(pPreProcessDlg->getFlatFieldPath(), &Beam, cropRegion, pPreProcessDlg->getFlatFieldMedianFilter())) {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setText(QString("Unable to read Dark files or File Error"));
      msgBox.setInformativeText(QString("Probably no .fits Files in Directory"));
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
      enableWidgets(true);
      return;
    }
  }

  QDir directory = QDir(pPreProcessDlg->getInputPath(), QString("*.fits"), QDir::Name, QDir::Files);
  if(!directory.exists()) {
    qDebug() << pPreProcessDlg->getInputPath() << " Directory does no exists";
    return;
  }
  QStringList fileList = directory.entryList();
  if(fileList.isEmpty()) {
    qDebug() << pPreProcessDlg->getInputPath() << " No .fits files in Directory";
    return;
  }

  float angle=0.0;
  CProjection CurrProj;
  CurrProj.f_MedianThreshold = 0.0;

  foreach (QString file, fileList) {
    if(!CurrProj.ReadFromFitsFile(pPreProcessDlg->getInputPath() + tr("/") + file)) {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setText(QString("Error Reading Fits File:"));
      msgBox.setInformativeText(CurrProj.sErrorString);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
      return;
    }
    if(CurrProj.f_angle == CurrProj.FLOAT_UNDEFINED) {
      CurrProj.f_angle = angle;
      angle += pPreProcessDlg->getDegPerStep();
    }
    CurrProj.Crop(cropRegion);
    if(pPreProcessDlg->getInputMedianFilter())
      CurrProj.Median(3);
    if(pPreProcessDlg->getDarkUse() && pPreProcessDlg->getFlatFieldUse())
      CurrProj.FlatFieldCorr(Beam, Dark);
    else if(pPreProcessDlg->getFlatFieldUse())
      CurrProj.FlatFieldCorr(Beam);
    else if(pPreProcessDlg->getDarkUse())
      CurrProj.Subtract(Dark);
    // Do we have to normalize to the Open Beam Value ?
    if(bOpenBeamCorr)
      CurrProj.OpenBeamCorr(beamRegion);
    // Post Median Filtering ?
    if(pPreProcessDlg->getOutputdMedianFilter())
      CurrProj.Median(3);
    // Show the Normalized Projection
    if(CurrProj.n_columns > CurrProj.n_rows)
      pImageWindow->setGeometry(QRect(0, 0, 512, (512*CurrProj.n_rows)/CurrProj.n_columns));
    else
      pImageWindow->setGeometry(QRect(0, 0, (512*CurrProj.n_columns)/CurrProj.n_rows, 512));
    if(!CurrProj.WriteToFitsFile(pPreProcessDlg->getOutputPath(), file)) {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setText(QString("Error Writing Fits File:"));
      msgBox.setInformativeText(CurrProj.sErrorString);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
      return;
    }
    CurrProj.Normalize(0.0, 1.0);// Just to show in a normalized way...
    pImageWindow->loadTexture(CurrProj.pData, CurrProj.n_columns, CurrProj.n_rows);
    pImageWindow->setTitle(file);
    pImageWindow->update();
    pStatusLine->setText(tr("Writing File: %1").arg(file));
    // Both of these lines are needed
    pStatusLine->repaint();
    qApp->processEvents();
    CurrProj.Reset();
  }

}


void
MainWindow::onCancelRoiSelection() {
  pImageWindow->enableSelection(false);
  disconnect(pImageWindow, SIGNAL(startPosEvent(QPoint)), pChooseRoiDlg, 0);
  disconnect(pImageWindow, SIGNAL(startPosEvent(QPoint)), pChooseRoiDlg, 0);
  disconnect(pChooseRoiDlg, SIGNAL(roiSelected(QRect)), this, 0);
  disconnect(pChooseRoiDlg, SIGNAL(roiDiscarded()),     this, 0);
  pChooseRoiDlg->hide();
  enableWidgets(true);
}


// To build a new projection as a sum of all
// the projections in a Directory
bool
MainWindow::buildImgSum(QString sDirectory, CProjection* pImgSum) {
  QDir directory = QDir(sDirectory, QString("*.fits"), QDir::Name, QDir::Files);
  if(!directory.exists()) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("The Directory does not exists !"));
    msgBox.setInformativeText(sDirectory);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    return false;
  }
  QStringList fileList = directory.entryList();
  if(fileList.isEmpty()) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("The Directory does not contains any fits file !"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    return false;
  }
  int nFiles = 0;
  CProjection CurrProj;
  foreach (QString file, fileList) {
    if(!CurrProj.ReadFromFitsFile(sDirectory + tr("/") + file)) {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setText(QString("Error Reading Fits File:"));
      msgBox.setInformativeText(CurrProj.sErrorString);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
      return false;
    }
    nFiles++;
    if(nFiles == 1)
      pImgSum->Copy(CurrProj);
    else
      pImgSum->Sum(CurrProj);
    pStatusLine->setText(tr("Processed File: %1").arg(file));
    //! Both of these lines needed
    pStatusLine->repaint();
    qApp->processEvents();
  }
  pImgSum->LinTransform(1.0f/float(nFiles), 0.0f);
  pStatusLine->setText(tr("File Reading Done !"));
  return true;
}


bool
MainWindow::buildImgSum(QString sDirectory, CProjection* pImgSum, QRect cropRegion, bool bMedianFilter) {
  QDir directory = QDir(sDirectory, QString("*.fits"), QDir::Name, QDir::Files);
  if(!directory.exists()) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("The Directory does not exists !"));
    msgBox.setInformativeText(sDirectory);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    return false;
  }
  QStringList fileList = directory.entryList();
  if(fileList.isEmpty()) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("The Directory does not contains any fits file !"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    return false;
  }
  int nFiles = 0;
  CProjection CurrProj;
  CurrProj.f_MedianThreshold = 0.0;// Filter only if point differ from neighborood mean by f_MedianThreshold*mean
  foreach (QString file, fileList) {
    if(!CurrProj.ReadFromFitsFile(sDirectory + tr("/") + file)) {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setText(QString("Error Reading Fits File:"));
      msgBox.setInformativeText(CurrProj.sErrorString);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
      return false;
    }
    CurrProj.Crop(cropRegion);
    if(bMedianFilter)
      CurrProj.Median(3);
    nFiles++;
    if(nFiles == 1)
      pImgSum->Copy(CurrProj);
    else
      pImgSum->Sum(CurrProj);
    pStatusLine->setText(tr("Processed File: %1").arg(file));
    //! Both of these lines needed
    pStatusLine->repaint();
    qApp->processEvents();
  }
  pImgSum->LinTransform(1.0f/float(nFiles), 0.0f);
  pStatusLine->setText(tr("File Reading Done !"));
  return true;

}


void
MainWindow::onDoTomoPushed() {
  enableWidgets(false);
  if(pDoTomoDlg->exec() == QDialog::Rejected) {
    enableWidgets(true);
    return;
  }
  if(!checkAndDeleteFiles(pDoTomoDlg->getSlicesPath())) {
    enableWidgets(true);
    return;
  }
  // Arrange projection files in an array with ascending angle
  if(!arrangeProjections(pDoTomoDlg->getNormalizedPath())) {
    enableWidgets(true);
    return;
  }
  // Try to extimate Rotation Center and Tilt
  rotationCenter = 0.0;
  tiltAngle      = 0.0;
  int i180       = 0;
  float fDist    = FLT_MAX;
  for(int i=angles.size()-1; i>-1; i--) {
    if(fabs(angles[i]-180.0f) < fDist) {
      fDist = fabs(angles[i]-180.0f);
      i180  = i;
    }
  }
  if(fDist > 1.0) {
    QString sString;
    sString =tr("Distance between specular images = %1°\nIt should be 180°").arg(angles[i180]-angles[0]);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("Warning !"));
    msgBox.setInformativeText(sString);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
  }

  CProjection* pP0 = new CProjection();
  CProjection* pP180 = new CProjection();
  pP0->ReadFromFitsFile(pDoTomoDlg->getNormalizedPath() + "/" + fileNames[0]);
  pP180->ReadFromFitsFile(pDoTomoDlg->getNormalizedPath() + "/" + fileNames[i180]);
  if(!FindCenter(pP0, pP180)) {
    enableWidgets(true);
    return;
  }
}


bool
MainWindow::FindCenter(CProjection* pProjection0, CProjection* pProjection180) {
  bool bFindOk = true;
  pProjection180->HorizontalFlip();
  int imgWidth = pProjection0->n_columns;
  int imgHeight = pProjection0->n_rows;
  float* rm = new float[imgHeight];
  float* m  = new float[imgHeight];
  float* _x = new float[imgHeight];
  float* r  = new float[imgWidth];
  memset(m, 0, sizeof(*m)*imgHeight);
  float *p1, *p2;
  float fMin, fMax, rMin, rMax;
  float rc;

  for(int row=0; row<imgHeight; row++) {
    _x[row] = row-0.5*imgHeight;
    p1 = pProjection0->pData+row*imgWidth;
    p2 = pProjection180->pData+row*imgWidth;
    crossCorr(p1, p2, r, imgWidth, imgWidth);
    rMax = -FLT_MAX;
    for(int i=0; i<imgWidth; i++) {
      if(*(r+i) > rMax) {
        rMax    = *(r+i);
        rm[row] = rMax;
        m[row]  = double(i)-0.5*imgWidth;
      } // if(*(r+i) > rMax)
    } // for(i=0; i<imgWidth; i++)
  } // for(row=0; row<height; row++)
  lineRegress(&_x[imgHeight/4], &m[imgHeight/4], imgHeight/2, &rotationCenter,  &tiltAngle, &rc);

  int n = 0;
  float* xx = new float[imgHeight];
  float* mm = new float[imgHeight];
  for(int i=0; i<imgHeight; i++) {
    if(fabs(rotationCenter+(tiltAngle)*_x[i]-m[i]) < 2) {
      xx[n] = _x[i];
      mm[n] = m[i];
      n++;
    }
  }
  if(n > imgHeight/8) {
    lineRegress(xx, mm, n, &rotationCenter,  &tiltAngle, &rc);
  }

  if(xx != NULL) delete[] xx;
  if(mm != NULL) delete[] mm;
  if(_x != NULL) delete[] _x; _x = NULL;
  if(r  != NULL) delete[] r;
  if(m  != NULL) delete[] m; m= NULL;
  if(rm != NULL) delete[] rm;

  tiltAngle   = atan(tiltAngle)*90.0/M_PI;
  rotationCenter = 0.5*(rotationCenter);
  CProjection p0, p180;
  p0.Copy(*pProjection0);
  p180.Copy(*pProjection180);
  p0.GetMinMax(&fMin, &fMax);
  p180.GetMinMax(&rMin, &rMax);
  if((fMax-fMin == 0.0) ||(rMax-rMin==0.0))
    return false;
  float f_in;
  for(int i=0; i<p0.n_columns*p0.n_rows; i++) {
    f_in = (*(p0.pData+i)-fMin)/(fMax-fMin);
    //f_in = 0.5*sin(M_PI*f_in-(M_PI*.5))+1.0;
    *(p0.pData+i) = f_in;
    f_in = (*(p180.pData+i)-fMin)/(fMax-fMin);
    ///f_in = 0.5*sin(M_PI*f_in-(M_PI*.5))+1.0;
    *(p180.pData+i) = f_in;
  }
  QString sString;
  sString = tr("Rotation center %1").arg(-rotationCenter);
  qDebug() << sString;
  sString = tr("Tilt Angle %1").arg(-0.5*tiltAngle);
  qDebug() << sString;

//  if(pAlignWnd) delete pAlignWnd;
//  pAlignWnd = NULL;
//  pAlignWnd = new CAlignWnd(this, _T("Aligning Window"));

//  if(pCenterDlg) delete pCenterDlg;
//  pCenterDlg = NULL;
//  pCenterDlg = new CCenterDlg(this);

//  RECT rDeskTop;
//  ::GetWindowRect(::GetDesktopWindow(), &rDeskTop);
//  int ratio = 1;
//  if((rDeskTop.right < p0.n_columns) ||
//     (rDeskTop.bottom < p0.n_rows)) {
//    double ratio1 = double(p0.n_columns)/double(rDeskTop.right-100);
//    double ratio2 = double(p0.n_rows)/double(rDeskTop.bottom-100);
//    ratio1 = ceil(ratio1);
//    ratio2 = ceil(ratio2);
//    ratio = int(max(ratio1, ratio2) + 0.5);
//  }
//  pAlignWnd->Initialize(p0.n_columns/ratio, p0.n_rows/ratio);
//  pAlignWnd->LoadTextures(p0.pData, p180.pData, p0.n_columns, p0.n_rows);
//  pAlignWnd->SetTilt(tiltAngle);
//  pAlignWnd->SetPos(rotationCenter);
//  pAlignWnd->SetEndMessage(MSG_ALIGN_DONE);
//  pAlignWnd->SetAbortMessage(MSG_ALIGN_ABORTED);
//  pAlignWnd->SetAlignmentChangedMessage(MSG_ALIGN_CHANGED);
//  pAlignWnd->ShowWindow(SW_SHOW);
//  pAlignWnd->UpdateWindow();

//  CString sString;
//  sString.Format(_T("%.3f"), -rotationCenter);
//  pCenterDlg->editCenter.SetWindowText(sString);
//  sString.Format(_T("%.3f"), -0.5*tiltAngle);
//  pCenterDlg->editTilt.SetWindowText(sString);
//  pCenterDlg->ShowWindow(SW_SHOW);
//  pCenterDlg->UpdateWindow();

  enableWidgets(true);
  return bFindOk;
}


bool
MainWindow::arrangeProjections(QString sPath) {
  angles.clear();
  fileNames.clear();
  QDir directory = QDir(sPath, QString("*.fits"), QDir::Name, QDir::Files);
  if(!directory.exists()) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("The Directory does not exists !"));
    msgBox.setInformativeText(sPath);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    return false;
  }
  QStringList fileList = directory.entryList();
  if(fileList.isEmpty()) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("The Directory %1 does not contains any fits file !").arg(sPath));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    return false;
  }
  CProjection CurrProj;
  foreach (QString file, fileList) {
    if(!CurrProj.ReadFromFitsFile(sPath + tr("/") + file)) {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setText(QString("Error Reading Fits File:"));
      msgBox.setInformativeText(CurrProj.sErrorString);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
      return false;
    }
    pStatusLine->setText(tr("Read File: %1").arg(file));
    pStatusLine->repaint();
    qApp->processEvents();
    fileNames.append(file);
    angles.append(CurrProj.f_angle);
  }
  // Sort Files in Ascending Angle order
  float tmp;
  QString sTmp;
  for(int i=0; i<angles.size()-1; i++) {
    for(int j=0; j<angles.size()-1-i; j++)
      if(angles[j+1] < angles[j]) {
        tmp = angles[j];
        angles[j] = angles[j+1];
        angles[j+1] = tmp;
        sTmp = fileNames[j];
        fileNames[j] = fileNames[j+1];
        fileNames[j+1] = sTmp;
    }
  }
  for(int i=0; i<angles.size(); i++) {
    qDebug() << fileNames[i] << angles[i];
  }
  return true;
}

/*
 *
afx_msg LRESULT
CNeuTomoDlg::OnAlignDone(WPARAM wParam, LPARAM lParam) {
  rotationCenter = pAlignWnd->GetPos();
  tiltAngle      = pAlignWnd->GetTilt();
  if(pAlignWnd)  delete pAlignWnd; pAlignWnd = NULL;
  if(pCenterDlg) delete pCenterDlg; pCenterDlg = NULL;
  //Start the Show Thread
  hThread=(HANDLE)_beginthread(RunTomoThread, 0, this);
  return LRESULT(0);
}


afx_msg LRESULT
CNeuTomoDlg::OnAlignChanged(WPARAM wParam, LPARAM lParam) {
  rotationCenter = pAlignWnd->GetPos();
  tiltAngle      = pAlignWnd->GetTilt();
  CString sString;
  sString.Format(_T("%.3f"), -rotationCenter);
  pCenterDlg->editCenter.SetWindowText(sString);
  sString.Format(_T("%.3f"), -0.5*tiltAngle);
  pCenterDlg->editTilt.SetWindowText(sString);
  return LRESULT(0);
}


afx_msg LRESULT
CNeuTomoDlg::OnAlignAborted(WPARAM wParam, LPARAM lParam) {
  if(pAlignWnd)  delete pAlignWnd; pAlignWnd = NULL;
  if(pCenterDlg) delete pCenterDlg; pCenterDlg = NULL;
        EnableButtons();
  return LRESULT(0);
}


*/

// Linear Regression
// y(x) = a + b*x, for n samples
// The following assumes the standard deviations are unknown for x and y
// Returns a, b and r the regression coefficient
bool
MainWindow::lineRegress(float *x, float *y, int n, float *a, float *b, float *r) {
  int i;
  float sumx=0,sumy=0,sumx2=0,sumy2=0,sumxy=0;
  float sxx,syy,sxy;

  *a = *b = *r = 0;
  if(n < 2) return false;
  // Compute some things we need
  for(i=0;i<n;i++) {
    sumx += x[i];
    sumy += y[i];
    sumx2 += (x[i] * x[i]);
    sumy2 += (y[i] * y[i]);
    sumxy += (x[i] * y[i]);
  }
  sxx = sumx2 - sumx * sumx / n;
  syy = sumy2 - sumy * sumy / n;
  sxy = sumxy - sumx * sumy / n;
  // Infinite slope (b), non existant intercept (a)
  if(fabs(sxx) == 0.0) return false;
  // Calculate the slope (b) and intercept (a)
  *b = sxy / sxx;
  *a = sumy / n - (*b) * sumx / n;
  // Compute the regression coefficient
  if(fabs(syy) == 0)
    *r = 1;
  else
    *r = sxy / sqrt(sxx * syy);
  return true;
}


void
MainWindow::crossCorr(float *y1, float *y2, float *r, int nSamples, int nChan) {
  int i, j, k, n;
  float r1, r2, mean1=0.0, mean2=0.0;
  memset(r, 0, sizeof(*r)*nChan);
  for(j=0; j<nSamples; j++) {// t
    mean1 += *(y1+j);
    mean2 += *(y2+j);
  } // for(j=0; j<nSamples; j++)
  mean1 /= double(nSamples);
  mean2 /= double(nSamples);

  for(i=-nChan/2; i<nChan/2; i++) {// tau
    n = 0;
    for(j=0; j<nSamples; j++) {// t
      k = j + i;// t + tau
      if(k<0 || k>=nSamples) continue;
      r1 = *(y1+j);
      r2 = *(y2+k);
      *(r+i+nChan/2) += (r1-mean1)*(r2-mean2);
      n++;
    }// for(i=-nChan/2; i<nChan/2; i++) {for(j=0; j<nSamples; j++)
    *(r+i+nChan/2) /= double(n);// Normalize to N_Points
  } // for(i=-nChan/2; i<nChan/2; i++)
}

