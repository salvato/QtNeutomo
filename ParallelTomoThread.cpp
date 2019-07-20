#include "float.h"
#include "mainwindow.h"
#include "Projection.h"
#include "ParallelTomoThread.h"
#include <QtMath>
#include <QObject>
#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOffscreenSurface>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


GLenum GL_TEXTURE_target             = GL_TEXTURE_RECTANGLE_ARB;
GLenum GL_TEXTURE_internalformat     = GL_LUMINANCE32F_ARB;
GLenum GL_TEXTURE_texFormat          = GL_LUMINANCE;
GLenum GL_FILTER_mode                = GL_LINEAR;


void
DoParallelTomo(tomoThreadParams params) {
    Q_UNUSED(params);
/*
  MainWindow* pDlg = (MainWindow *)params.pParent;
  int imgWidth = params.projWidth;
  int imgHeight = params.projHeight;
  int nProjections = params.nProjections;
  QString sTempPath = QString("/tmp/File_");
  QString sString, sFileOut;
  int newTex, newSumTex, oldSumTex;
  int nProjTex;
  bool bEnoughMemory = true;
  int nProjInMemory, lastAttachedTexture;

  GLuint glutWindowHandle = 0;// handle to offscreen "window"
  GLuint theList;// Display List
  GLfloat eyePlaneS[4], eyePlaneT[4], eyePlaneR[4];
  GLuint outTexID[3], *inTexID=nullptr;

  memset(eyePlaneS, 0, 4*sizeof(eyePlaneS[0]));
  memset(eyePlaneT, 0, 4*sizeof(eyePlaneT[0]));
  memset(eyePlaneR, 0, 4*sizeof(eyePlaneR[0]));
  eyePlaneS[0] = eyePlaneT[1] = eyePlaneR[2] = 1.0;

  GLenum attachmentpoints[3];
  attachmentpoints[0] = GL_COLOR_ATTACHMENT0;
  attachmentpoints[1] = GL_COLOR_ATTACHMENT1;
  attachmentpoints[2] = GL_COLOR_ATTACHMENT2;
  
  newSumTex = 0;
  oldSumTex = 1;
  newTex    = 2;

  CProjection Projection;

//  pDlg->EnableStopButton();// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

  clock_t start = clock();

  int m    = int(ceil(log(2.0*imgWidth)/log(2.0)));
  int nfft = 1 << m;
  float *wfilt  = new float[nfft];
  if(!buildFilter(wfilt, nfft, imgWidth, params.filterType)) {
    delete[] wfilt;
    return;
  }
  for(int i=0; i<nfft; i++) {
    wfilt[i] *= M_PI/float(nProjections-1);//Scale factor
  }

  QOpenGLContext* OpenGLContext = new QOpenGLContext(Q_NULLPTR);
  QOffscreenSurface* Surface = new QOffscreenSurface();
  QSurfaceFormat requestedFormat = QSurfaceFormat::defaultFormat();
  requestedFormat.setMajorVersion(5);
  requestedFormat.setMajorVersion(5);
  Surface->setFormat(requestedFormat);
  Surface->create();
  OpenGLContext->setFormat(requestedFormat);
  if(!OpenGLContext->create()) {
      qDebug() << "Failed to create OpenGL context";
      return;
  }
  OpenGLContext->makeCurrent(Surface);

  int texSizeX = qMin(pDlg->GetVolXSize(), imgWidth);
  int texSizeY = int(float(imgHeight)*float(texSizeX)/float(imgWidth)+0.5);
  int texSizeZ = texSizeX;

  CProjection Proj2Save(texSizeX, texSizeY);
  GLuint fbo;

  if(!initFBO(&fbo)) {
    qDebug() << "Unable to Initialize FBO !";
    return;
  }
  pDlg->ShowStatusMsg(QString("Creating Textures"));

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  // Create the 3 textures for the FBO
  glGenTextures(3, outTexID);
  if(!initFBOTextures(3, outTexID, texSizeX, texSizeY, attachmentpoints)) {
    pDlg->bDoTomoEnabled = false;
    return;
  }

  // Create the projective Textures
  nProjTex = nProjections-1;
  inTexID = new GLuint[nProjTex];
  glGenTextures(nProjTex, inTexID);
  if(!initProjTextures(nProjTex, inTexID)) {
    qDebug() << QString("Unable to initialize Projective Textures");
    pDlg->bDoTomoEnabled = false;
    return;
  }

  glTexGenfv(GL_S, GL_EYE_PLANE, eyePlaneS);
  glTexGenfv(GL_T, GL_EYE_PLANE, eyePlaneT);
  glTexGenfv(GL_R, GL_EYE_PLANE, eyePlaneR);
  if(!CheckGLErrors(_T("glTexGenfv"))){
    pDlg->bDoTomoEnabled = false;
    cgDestroyProgram(fragmentProgram);
    cgDestroyContext(cgContext);
    glutDestroyWindow(glutWindowHandle);
    if(wfilt   != NULL) delete[] wfilt; wfilt = NULL;
    if(inTexID != NULL) delete[] inTexID; 
    return;
  }

  // Projective Texture Matrices
  GLfloat* transformMats = new GLfloat[(nProjections-1)*16];
  GLfloat* pMat;
  glMatrixMode(GL_TEXTURE);
  for(int nProj=0; nProj<nProjections-1; nProj++) {
    pMat = transformMats+(nProj<<4);
    glLoadIdentity();
    glTranslatef(0.5*imgWidth-params.rotationCenter, 0.5*imgHeight, -1.0);
    glRotatef(pDlg->tiltAngle, 0.0, 0.0, 1.0);
    glRotatef(pDlg->GetAngles()[nProj], 0.0, 1.0, 0.0);
    glTranslatef(-0.5*imgWidth, -0.5*imgHeight, 0.0);
    glGetFloatv(GL_TEXTURE_MATRIX, pMat);
  }

  float *pImage, *pRow1, *pRow2;
  float *pr1    = new float[nfft];
  float *pr2    = new float[nfft];

  int zeroPad1 = (nfft-imgWidth)*sizeof(*pr1);
  int zeroPad2 = nfft*sizeof(*pr2);
  int rowSize  = imgWidth*sizeof(*pRow1);
  
  glEnable(GL_TEXTURE_target);

//typedef struct _MEMORYSTATUSEX {
//  DWORD     dwLength;
//  DWORD     dwMemoryLoad;
//  DWORDLONG ullTotalPhys;
//  DWORDLONG ullAvailPhys;
//  DWORDLONG ullTotalPageFile;
//  DWORDLONG ullAvailPageFile;
//  DWORDLONG ullTotalVirtual;
//  DWORDLONG ullAvailVirtual;
//  DWORDLONG ullAvailExtendedVirtual;
//} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;

  MEMORYSTATUSEX MemStat;
  memset(&MemStat, 0, sizeof(MemStat));// Zero structure
  MemStat.dwLength = sizeof(MemStat);
  ::GlobalMemoryStatusEx(&MemStat);// Get RAM snapshot
  int availableRam_kb = MemStat.ullAvailPhys/1024;
  availableRam_kb /= 3;//To Leave some space for others
  GLint cur_avail_mem_kb = 0;
  glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &cur_avail_mem_kb);
  cur_avail_mem_kb += availableRam_kb;

  int textureSize_kb = imgWidth*imgHeight*sizeof(*pImage)/1024;
  int MaxNumProjInMemory = int(cur_avail_mem_kb/textureSize_kb);// To allow some space
  MaxNumProjInMemory -= 3;// To allow some other space
  if(MaxNumProjInMemory < 2) {
    AfxMessageBox(_T("Too Big Textures !"));
    return;
  }

  lastAttachedTexture = 0;
  nProjInMemory = 0;
  for(int nProj=0; nProj<nProjections-1; nProj++) {// Exclude the 180� one...
    if(pDlg->GetEscape()) {
      cgDestroyProgram(fragmentProgram);
      cgDestroyContext(cgContext);
      glutDestroyWindow(glutWindowHandle);
      if(inTexID != NULL) delete[] inTexID; 
      if(wfilt   != NULL) delete[] wfilt; wfilt = NULL;
      if(pr1     != NULL) delete[] pr1;
      if(pr2     != NULL) delete[] pr2;
      for(int i=0; i<nProj; i++) {
        QString sFileTemp;
        sFileTemp.Format(_T("%s%004d.fft"), sTempPath, i);
        if(!DeleteFile(sFileTemp)) {
          DWORD Error = GetLastError();
          QString Value = _T("Unable to Delete ");
          Value += sFileTemp;
          AfxMessageBox(Value);
        }
      }
      if(transformMats != NULL) delete[] transformMats;
      return;
    }// if(pDlg->GetEscape())

    sString.Format(_T("Filtering Projection # %d at %.3f�"), nProj, pDlg->GetAngles()[nProj]);
    pDlg->ShowStatusMsg(sString);
    Projection.ReadFromFitsFile(pDlg->GetPathNames()[nProj]);
    pImage = Projection.pData;

    for(int nRow=0; nRow<imgHeight; nRow++) {
      pRow1 = pImage + nRow*imgWidth;
      memcpy(pr1, pRow1, rowSize);
      ZeroMemory(pr1+imgWidth, zeroPad1);//Zero padding
      ZeroMemory(pr2, zeroPad2);
      if(++nRow < imgHeight) {
        pRow2 = pImage + nRow*imgWidth;
        memcpy(pr2, pRow2, rowSize);
      } // if(++nRow < imgHeight)
      fft(pr1, pr2, m, 0);// One call two transforms !
		  for(int i=0; i<nfft; i++) {//Filtering...
			  pr1[i] *= wfilt[i];
			  pr2[i] *= wfilt[i];
		  }
		  fft(pr1, pr2, m, 1);//Inverse Transform
      memcpy(pRow1, pr1, rowSize);
      if(nRow < imgHeight)
        memcpy(pRow2, pr2, rowSize);
    } // for(int nRow=0; nRow<imgHeight; nRow++)
    
    if(pDlg->GetFilterSinograms()) {
      Projection.f_MedianThreshold = 0.0;
      Projection.Median(3);
    }

    if(bEnoughMemory) {
      // Try to transfer the Image to Texture Memory
      glBindTexture(GL_TEXTURE_target, inTexID[nProj]);
      glTexImage2D(GL_TEXTURE_target, 0, GL_TEXTURE_internalformat, imgWidth, imgHeight, 0, GL_TEXTURE_texFormat, GL_FLOAT, pImage);
      nProjInMemory++;
      //if(!CheckGLErrors(_T("Projective Texture Loading"))) {//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      if(nProjInMemory >= MaxNumProjInMemory) {
        bEnoughMemory = false;
        lastAttachedTexture = nProj;
        //sString.Format(_T("Memory for %d Projections\r\n"), nProjInMemory);
        //pMsg->AddText(sString);
      }
    }// if(bEnoughMemory)

    QString sFileTemp;
    sFileTemp.Format(_T("%s%004d.fft"), sTempPath, nProj);
    FILE* outStream = _tfopen(sFileTemp, _T("wb"));
    size_t nToWrite = imgHeight*imgWidth;
    size_t nWritten = fwrite(pImage, sizeof(*pImage), nToWrite, outStream);
    fclose(outStream);
    if(nWritten != nToWrite) {
      AfxMessageBox(_T("Error Writing Temp File: ") + sFileTemp);
      return;//<<<<<<<<<<<<<<<<<<<<<<< Gestire la condizione !!!!!!!!!!!!!!!!!!!!!!
    }
  }// for(int nProj=0; nProj<nProjections-1; nProj++)
  if(!CheckGLErrors(_T("Projective Texture Loading"))) {
      return;//<<<<<<<<<<<<<<<<<<<<<<< Gestire la condizione !!!!!!!!!!!!!!!!!!!!!!
  }
  clock_t fft_end = clock();
  sString.Format(_T("Filtered %d Slices With Dimensions:%dx%d in %f[sec]"), 
                 texSizeZ, texSizeX, texSizeY, (fft_end-start)/1000.0);
  pDlg->ShowStatusMsg(sString);

  if(pr1    != NULL) delete[] pr1; pr1 = NULL;
  if(pr2    != NULL) delete[] pr2; pr2 = NULL;
  if(wfilt  != NULL) delete[] wfilt; wfilt = NULL;
  
  float dZ = float(imgWidth)/float(texSizeZ);
  glClearColor(0.0, 0.0, 0.0, 1.0);

  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_target);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, imgWidth, 0.0, imgHeight, -imgWidth, imgWidth);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

	glPolygonMode(GL_FRONT, GL_FILL);
  GLfloat zPlane;
    
  float fMin = FLT_MAX, fMax =-FLT_MAX;

  for(int i=0; i<3; i++) {
    glDrawBuffer(attachmentpoints[i]);// Clear all the texture to be prepared
    glClear(GL_COLOR_BUFFER_BIT);     // Is that Needed ?
  }

  theList = glGenLists(1);
  glNewList(theList, GL_COMPILE);
    glLoadIdentity();
    glBegin(GL_QUADS);
      glVertex3f(0.0,      0.0,       0.0);
      glVertex3f(0.0,      texSizeY, 0.0);
      glVertex3f(texSizeX, texSizeY, 0.0);
      glVertex3f(texSizeX, 0.0,       0.0);
    glEnd();
  glEndList();

  // Calculation of the renderable cylindrical region
  // Must be corrected for the aspect ratio <<<<<<<<<<<<<<<<<<<<<<<<<<<
  float fLimit, r, rl, rr;
  r = 0.5*float(imgWidth-1.0);
  int iExclude = int(floor(fabs(params.rotationCenter)*dZ))+1;

  Proj2Save.ClearImage();

  cgGLEnableProfile(fragmentProfile); // The profile must be enabled for the binding to work
  cgGLBindProgram(fragmentProgram);   // Bind the program
  cgGLDisableProfile(fragmentProfile);// Disable profile at first

  for(int iPlane=0; iPlane<texSizeZ; iPlane++) {// Costruiamo i vari piani del volume.
    if(pDlg->GetEscape()) {// Diamo la possibilit� di bloccare il lento processo di ricostruzione
      cgDestroyProgram(fragmentProgram);
      cgDestroyContext(cgContext);
      glutDestroyWindow(glutWindowHandle);
      if(inTexID != NULL) delete[] inTexID; 
      if(transformMats != NULL) delete[] transformMats;
      for(int nProj=0; nProj<nProjections-1; nProj++) {
        QString sFileTemp;
        sFileTemp.Format(_T("%s%004d.fft"), sTempPath, nProj);
        if(!DeleteFile(sFileTemp)) {
          DWORD Error = GetLastError();
          QString Value = _T("Unable to Delete ");
          Value += sFileTemp;
          AfxMessageBox(Value);
        }
      }
      return;
    }// if(pDlg->GetEscape())
    sString.Format(_T("BackProjecting Plane # %d/%d"), iPlane+1, texSizeX);
    pDlg->ShowStatusMsg(sString);
    zPlane = dZ*float(iPlane) - r;// Volume plane Z coordinate 
    fLimit = ((r-iExclude)*(r-iExclude))-(zPlane*zPlane);// Region inside the cylinder
    if(fLimit > 0.0) {
      fLimit = sqrt(fLimit);// Cord Lenght
      rl = r - fLimit;
      rr = r + fLimit;

      for(int nProj=0; nProj<nProjections-1; nProj++) {
        if(nProj > lastAttachedTexture) {
          for(int i=0; i<nProjInMemory; i++) {
            lastAttachedTexture++;
            QString sFileTemp;
            sFileTemp.Format(_T("%s%004d.fft"), sTempPath, lastAttachedTexture);
            FILE* inStream = _tfopen(sFileTemp, _T("rb"));
            size_t nToRead = imgHeight*imgWidth;
            size_t nRead = fread(pImage, sizeof(*pImage), nToRead, inStream);
            fclose(inStream);
            if(nRead != nToRead)
              AfxMessageBox(_T("Error Reading Temp File: ") + sFileTemp);
            glBindTexture(GL_TEXTURE_target, inTexID[i]);
            glTexImage2D(GL_TEXTURE_target, 0, GL_TEXTURE_internalformat, imgWidth, imgHeight, 0, GL_TEXTURE_texFormat, GL_FLOAT, pImage);
            if(!CheckGLErrors(_T("Projective Texture Loading"))) {
              AfxMessageBox(_T("Gestire_Errore()"));
            }
            if(lastAttachedTexture == nProjections-2)
              break;
          }// for(int i=0; i<nProjInMemory; i++)
          //sString.Format(_T("Last Attached Texture= %d/%d\r\n"), lastAttachedTexture, nProjections-1);
          //pMsg->AddText(sString);
        }// if(nProj > lastAttachedTexture)
        glViewport(0, 0, texSizeX, texSizeY);  // Set the final dimension
        glBindTexture(GL_TEXTURE_target, inTexID[nProj%nProjInMemory]);// The texture to be rendered
        glDrawBuffer(attachmentpoints[newTex]);// Set the Render Destination
        glClear(GL_COLOR_BUFFER_BIT);
        pMat = transformMats+(nProj<<4);
        glMatrixMode(GL_TEXTURE);// Just to be sure...
        glLoadMatrixf(pMat);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_QUADS);//image rendering
          glVertex3f(rl, 0.0,       zPlane);
          glVertex3f(rl, imgHeight, zPlane);
          glVertex3f(rr, imgHeight, zPlane);
          glVertex3f(rr, 0.0,       zPlane);
        glEnd();
        glFinish();
        glDrawBuffer(attachmentpoints[newSumTex]);
        // Parallel projection for 1:1 pixel=texture mapping
        glViewport(0, 0, imgWidth, imgHeight);
        // Enable fragment program
        cgGLEnableProfile(fragmentProfile);
        cgGLSetTextureParameter(NewTexture, outTexID[newTex]);
        cgGLSetTextureParameter(OldTexture, outTexID[oldSumTex]);
        cgGLEnableTextureParameter(NewTexture);
        cgGLEnableTextureParameter(OldTexture);
        // Execute the progam
        glCallList(theList);
        // Exchange the textures
        newSumTex = 1-newSumTex;
        oldSumTex = 1-oldSumTex;
        //Ensure all commands executed
        glFinish();
        cgGLDisableProfile(fragmentProfile);
        #ifdef  _DEBUG
          CheckGLErrors(_T("Step"));
        #endif
      } // for(nProj=0; nProj<nProjections; nProj++)

      // Plane Done ! get GPU result
      glReadBuffer(attachmentpoints[newSumTex]);
      glReadPixels(0, 0, texSizeX, texSizeY, GL_TEXTURE_texFormat, GL_FLOAT, Proj2Save.pData);
      //Clear texture to prepare for the next Plane
      glDrawBuffer(attachmentpoints[oldSumTex]);
      glClear(GL_COLOR_BUFFER_BIT);

      sFileOut.Format(_T("Slice-%004d.fits"), iPlane);
      Proj2Save.i_SliceNum = iPlane;
      if(!Proj2Save.WriteToFitsFile(pDlg->GetSliceDir(), sFileOut)) {
        AfxMessageBox(Proj2Save.sErrorString);
        cgDestroyProgram(fragmentProgram);
        cgDestroyContext(cgContext);
        glutDestroyWindow(glutWindowHandle);
        if(inTexID != NULL) delete[] inTexID; 
        if(transformMats != NULL) delete[] transformMats;
        return;
      }
      glFinish();
    } else {// if(fLimit > 0.0)
      Proj2Save.ClearImage();
      sFileOut.Format(_T("Slice-%004d.fits"), iPlane);
      Proj2Save.i_SliceNum = iPlane;
      if(!Proj2Save.WriteToFitsFile(pDlg->GetSliceDir(), sFileOut)) {
        AfxMessageBox(Proj2Save.sErrorString);
        cgDestroyProgram(fragmentProgram);
        cgDestroyContext(cgContext);
        glutDestroyWindow(glutWindowHandle);
        if(inTexID != NULL) delete[] inTexID; 
        if(transformMats != NULL) delete[] transformMats;
        return;
      }
    }// if(fLimit > 0.0)
    if(!CheckGLErrors(_T("Plane Projection"))) {
      break;
    }
    if((iPlane < texSizeZ-1) &&(MaxNumProjInMemory < nProjections-1)) {
      for(int i=0; i<nProjInMemory; i++) {
        QString sFileTemp;
        sFileTemp.Format(_T("%s%004d.fft"), sTempPath, i);
        FILE* inStream = _tfopen(sFileTemp, _T("rb"));
        size_t nToRead = imgHeight*imgWidth;
        size_t nRead = fread(pImage, sizeof(*pImage), nToRead, inStream);
        fclose(inStream);
        if(nRead != nToRead)
          AfxMessageBox(_T("Error Reading Temp File: ") + sFileTemp);
        glBindTexture(GL_TEXTURE_target, inTexID[i]);
        glTexImage2D(GL_TEXTURE_target, 0, GL_TEXTURE_internalformat, imgWidth, imgHeight, 0, GL_TEXTURE_texFormat, GL_FLOAT, pImage);
        if(!CheckGLErrors(_T("Projective Texture Loading"))) {
          AfxMessageBox(_T("Gestire_Errore()"));
        }
      }// for(int i=0; i<nProjInMemory; i++)
      lastAttachedTexture = nProjInMemory;
      //sString.Format(_T("Last Attached Texture= %d/%d\r\n"), lastAttachedTexture, nProjections-1);
      //pMsg->AddText(sString);
    }
  }// for(int iPlane=0; iPlane<texSizeZ; iPlane++)
  
  for(int nProj=0; nProj<nProjections-1; nProj++) {
    QString sFileTemp;
    sFileTemp.Format(_T("%s%004d.fft"), sTempPath, nProj);
    if(!DeleteFile(sFileTemp)) {
      DWORD Error = GetLastError();
      QString Value = _T("Unable to Delete ");
      Value += sFileTemp;
      AfxMessageBox(Value);
    }
  }

  clock_t end = clock();
  sString.Format(_T("Saved %d Slices With Dimensions:%dx%d\nBackprojecting Time [sec]: %f"), 
                 texSizeZ, texSizeX, texSizeY, (end-start)/1000.0);
  AfxMessageBox(sString);

  glDisable(GL_TEXTURE_target);
  if(!CheckGLErrors(_T("glDisable"))) return;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteTextures(3, outTexID);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(nProjTex, inTexID);

  cgDestroyProgram(fragmentProgram);
  cgDestroyContext(cgContext);
  glDeleteLists(theList, 1);

  glutDestroyWindow(glutWindowHandle);
  if(transformMats != NULL) delete[] transformMats;
  if(inTexID != NULL) delete[] inTexID; 
  return;
*/
}


bool
initFBOTextures(int nTextures, GLuint* TexID, int texSizeX, int texSizeY, GLenum* attachmentpoints) {
    Q_UNUSED(nTextures)
    Q_UNUSED(TexID)
    Q_UNUSED(texSizeX)
    Q_UNUSED(texSizeY)
    Q_UNUSED(attachmentpoints)
/*
    for (int i=0; i< nTextures; i++) {
    glBindTexture(GL_TEXTURE_target, TexID[i]);
    glTexParameteri(GL_TEXTURE_target, GL_TEXTURE_WRAP_S,     GL_CLAMP);
    glTexParameteri(GL_TEXTURE_target, GL_TEXTURE_WRAP_T,     GL_CLAMP);
    glTexParameteri(GL_TEXTURE_target, GL_TEXTURE_MIN_FILTER, GL_FILTER_mode);
    glTexParameteri(GL_TEXTURE_target, GL_TEXTURE_MAG_FILTER, GL_FILTER_mode);
    glTexImage2D(GL_TEXTURE_target, 0, GL_TEXTURE_internalformat, texSizeX, texSizeY, 0, GL_TEXTURE_texFormat, GL_FLOAT, 0);
    if(!CheckGLErrors(_T("glGenTextures"))) return false;
    // Attach the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentpoints[i], GL_TEXTURE_target, TexID[i], 0);
    if(!CheckFramebufferStatus()) return false;
  } // for (int i=0; i< nTextures; i++)
*/
  return true;
}


bool
initProjTextures(int nProjections, GLuint* TexID) {
    Q_UNUSED(nProjections)
    Q_UNUSED(TexID)
/*
  for(int i=0; i<nProjections; i++) {
    glBindTexture(GL_TEXTURE_target, TexID[i]);
    glTexParameteri(GL_TEXTURE_target, GL_TEXTURE_WRAP_S,     GL_CLAMP);
    glTexParameteri(GL_TEXTURE_target, GL_TEXTURE_WRAP_T,     GL_CLAMP);
    glTexParameteri(GL_TEXTURE_target, GL_TEXTURE_MIN_FILTER, GL_FILTER_mode);
    glTexParameteri(GL_TEXTURE_target, GL_TEXTURE_MAG_FILTER, GL_FILTER_mode);
    if(!CheckGLErrors(_T("glTexParameteri"))) return false;
  } // for(int i=0; i<nProjections; i++)

  // Texture coordinates in Eye Space
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  if(!CheckGLErrors(_T("glTexGeni"))) return false;
*/
  return true;
}


// Sets up the Cg runtime and creates shader.
bool
initCG() {
/*
  cgSetErrorCallback(cgErrorCallback);
  cgContext = cgCreateContext();
  #ifdef _DEBUG
    cgGLSetDebugMode(CG_TRUE);
  #else
    cgGLSetDebugMode(CG_FALSE);
  #endif

  _TCHAR  szAppPath[MAX_PATH] = _T("");
  QString strAppDirectory;

  ::GetModuleFileName(0, szAppPath, sizeof(szAppPath) - 1);

  // Extract directory
  strAppDirectory = szAppPath;
  strAppDirectory = strAppDirectory.Left(strAppDirectory.ReverseFind('\\'));
  strAppDirectory += _T("\\projTex.cg");
  char fragmentProgramFileName[MAX_PATH];
  size_t pConvertedChars;
  wcstombs_s(&pConvertedChars, fragmentProgramFileName, MAX_PATH-1, strAppDirectory, strAppDirectory.GetLength());

  char* fragmentProgramName = {"projTex"};
  fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  cgGLSetOptimalOptions(fragmentProfile);

  // create fragment programs
  fragmentProgram = 
    cgCreateProgramFromFile(cgContext,                // Cg runtime context
                            CG_SOURCE,                // Program in human-readable form
                            fragmentProgramFileName,  // Program Filename (in the same path is better...)
                            fragmentProfile,          // Profile: latest fragment profile
                            fragmentProgramName,      // Entry function name
                            NULL                      // No extra compiler options
                           );
  if(!fragmentProgram) {
    AfxMessageBox(CString(fragmentProgramName) + _T(".cg\nFragment Program not found in Directory\n")+strAppDirectory, MB_ICONSTOP);
    return false;
  }
  cgGLLoadProgram(fragmentProgram);// load program
  // and get parameter handles by name
  NewTexture = cgGetNamedParameter(fragmentProgram, "newTexture");
  OldTexture = cgGetNamedParameter(fragmentProgram, "oldTexture");
*/
  return true;
}


void 
cgErrorCallback() {
/*
  CGerror lastError = cgGetError();
  if(lastError) {
    QString sError1, sError2, sError;
    char* QString = (char *)cgGetErrorString(lastError);
    size_t requiredSize = mbstowcs(NULL, cString, 0) + 1; 
    _TCHAR* tString = new _TCHAR[requiredSize];
    size_t count = mbstowcs(tString, cString, strlen(cString));
    if(count<requiredSize) tString[count] = 0;
    sError1.Format(_T("%s"), tString);
    delete[] tString;
    QString = (char *)cgGetLastListing(cgContext);
    if(QString != NULL) {
      requiredSize = mbstowcs(NULL, cString, 0) + 1;
      tString = new _TCHAR[requiredSize];
      count = mbstowcs(tString, cString, strlen(cString));
      if(count<requiredSize) tString[count] = 0;
      sError.Format(_T("%s%s%s"),sError1, _T("\n"), tString);
      delete[] tString;
    } else {
      sError.Format(_T("%s\nNULL"),sError1);
    }
  	AfxMessageBox(sError);
  	return;
  } // if(lastError)
*/
}


bool
CheckForCgError(QString Situation) {
    Q_UNUSED(Situation)
/*
  CGerror error;
  QString sError;
  const char* cString;
  _TCHAR* tString;
  size_t requiredSize, count;

  QString = cgGetLastErrorString(&error);
  requiredSize = mbstowcs(NULL, cString, 0) + 1; 
  tString = new _TCHAR[requiredSize];
  count = mbstowcs(tString, cString, strlen(cString));
  if(count<requiredSize) tString[count] = 0;
  sError.Format(_T("%s"), tString);
  delete[] tString;

  if(error != CG_NO_ERROR) {
    QString sMsg;
    if (error == CG_COMPILER_ERROR) {
      QString = cgGetLastListing(cgContext);
      requiredSize = mbstowcs(NULL, cString, 0) + 1; 
      tString = new _TCHAR[requiredSize];
      count = mbstowcs(tString, cString, strlen(cString));
      if(count<requiredSize) tString[count] = 0;
      sMsg.Format(_T("%s: %s\n%s"), Situation, sError, tString);
      delete[] tString;
    } else {
      sMsg.Format(_T("%s: %s"), Situation, sError);
    }
    AfxMessageBox(sMsg, MB_ICONSTOP);
    return false;
  } // if(error != CG_NO_ERROR)
*/
  return true;
}


// Creates framebuffer object, binds it to reroute rendering operations 
// from the traditional framebuffer to the offscreen buffer
bool
initFBO(GLuint* pFbo) {
    Q_UNUSED(pFbo)
/*
  glGenFramebuffersEXT(1, pFbo);// Create a FBO
  glBindFramebufferEXT(GL_FRAMEBUFFER, *pFbo);// bind offscreen framebuffer
  if(!CheckGLErrors(_T("initFBO"))) {
    return false;
  }
*/
  return true;
}


bool
buildFilter(float* wfilt, int nfft, int rowWidth, FILTER_TYPES filterType) {
/*
  int m = int(log(double(nfft))/log(2.0)+0.5);
  float *pr1    = NULL;
  float *pr2    = NULL;
  if(filterType == RAM_LAK) {
    pr1    = new float[nfft];
    pr2    = new float[nfft];
    ZeroMemory(pr1, nfft*sizeof(*pr1));
    ZeroMemory(pr2, nfft*sizeof(*pr2));

    // Ram-Lak Filter
    // See:
    // J.Banhart ed. "Advanced Tomographyc Methods in Materials
    // Research and Engineering - Oxford University (2008)
    float tau = 2.0f/float(rowWidth-1);
          *pr1 = 0.25/tau;
          for(int i=1; i<rowWidth; i+=2) {
                  *(pr1+nfft-i) = *(pr1+i) = -1.0/(i*i*M_PI*M_PI*tau);
          }

    fft(pr1, pr2, m, 0);
    memcpy(wfilt, pr1, nfft*sizeof(*pr1));
  } else {//Shepp-Logan
    pr1    = new float[nfft];
    pr2    = new float[nfft];
    ZeroMemory(pr1, nfft*sizeof(*pr1));
    ZeroMemory(pr2, nfft*sizeof(*pr2));
    // Shepp-Logan Filter
    // See:
    // Manuel Dierick
    // Tomographic Imaging Techniques using Cold and Thermal Neutron Beams
    // Thesis (2005)
    // Department of Subatomic and Radiation Physics - Gent University
    float tau = 2.0f/float(rowWidth-1);
    *pr1 = 2.0/(M_PI*M_PI*tau);
          for(int i=1; i<rowWidth; i++) {
                  *(pr1+nfft-i) = *(pr1+i) = 2.0/(M_PI*M_PI*tau*(1.0-4.0*i*i));
          }

    fft(pr1, pr2, m, 0);
    memcpy(wfilt, pr1, nfft*sizeof(*pr1));
  }
  if(pr1    != NULL) delete[] pr1; pr1 = NULL;
  if(pr2    != NULL) delete[] pr2; pr2 = NULL;
*/
  return true;
}


/*
  fft - calculate FFT

  Carl Crawford
  Purdue University
  W. Lafayette, IN. 47907

  Calling Sequence....fft(real, im, m, iopt)

  Where real and im are the real and imaginary
  parts of the input data.  The result is
  returned in place.  M is the log base 2
  of the number of elements in the array.

  iopt is equal to 0 for the forward
  transform and 1 for the inverse transform.
*/
void
fft(float *a, float *b, int m, int iopt) {
  // a[]  real part of data
  // b[]  imaginary part of data
  // m    size of data = 2**m
  // iopt: 0=dft, 1=inverse dft
  int	nv2, nm1, n, le, le1, ip;
  float pile1, tmp;
  float ua, ub, wa, wb, ta, tb, *ap, *bp;
  int i, j, l;
  n = 1 << m;
  // For the inverse transform
  // scale the result by N
  if(iopt) {
    for(i=0, ap=a, bp=b; i<n; i++) {
      *ap++ /= n;
      *bp++ /=-n;
    }
  }
  nv2 = n/2;
  nm1 = n - 1;
  j = 0;
  for(i=0;i<nm1;i++) {
    if(i<j) {
      ta   = a[j]; tb   = b[j];
      a[j] = a[i]; b[j] = b[i];
      a[i] = ta;   b[i] = tb;
    }
    l = nv2;
    while(l < (j+1) ) {
      j = j - l;
      l = l / 2;
    }
    j = j + l;
  }
  for(l=1; l<=m; l++) {
    le = 1 << l;
    le1 = le >> 1;
    ua = 1.0;	ub = 0.0;
    pile1 = float(M_PI) / le1;
    wa = cosf(pile1);	wb = -sinf(pile1);
    for(j=0; j<le1; j++) {
      for(i=j; i<n; i+=le) {
        ip = i + le1;
        ta = a[ip] * ua - b[ip] * ub;
        tb = a[ip] * ub + b[ip] * ua;
        a[ip] = a[i] - ta;
        b[ip] = b[i] - tb;
        a[i] += ta;
        b[i] += tb;
      }
      ua = (tmp = ua) * wa - ub * wb;
      ub = tmp * wb + ub * wa;
    }
  }
  if(iopt != 0) {
    for(i=0; i<n; i++)
      b[i] = -b[i];
  }
}

