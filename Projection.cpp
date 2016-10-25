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
#include "float.h"
#include <QtMath>
#include <QPoint>
#include <QString>
#include <QFileInfo>
#include <QDebug>
#include <QRect>
#include "Projection.h"


CProjection::CProjection(void) 
  : FLOAT_UNDEFINED(-FLT_MAX)
  , INT_UNDEFINED(INT_MIN)
  , n_rows(0)
  , n_columns(0)
  , pData(Q_NULLPTR)
  , f_angle(FLOAT_UNDEFINED)
  , f_BeamIntensity(FLOAT_UNDEFINED)
  , f_TimeOfFlight(FLOAT_UNDEFINED)
  , f_PosX(FLOAT_UNDEFINED)
  , f_PosY(FLOAT_UNDEFINED)
  , f_PosZ(FLOAT_UNDEFINED)
  , i_SliceNum(INT_UNDEFINED)
  , sErrorString("No Error")
{
}


CProjection::CProjection(int nColumns, int nRows)
  : FLOAT_UNDEFINED(-FLT_MAX)
  , INT_UNDEFINED(INT_MIN)
  , n_rows(nRows)
  , n_columns(nColumns)
  , pData(NULL)
  , f_angle(FLOAT_UNDEFINED)
  , f_BeamIntensity(FLOAT_UNDEFINED)
  , f_TimeOfFlight(FLOAT_UNDEFINED)
  , f_PosX(FLOAT_UNDEFINED)
  , f_PosY(FLOAT_UNDEFINED)
  , f_PosZ(FLOAT_UNDEFINED)
  , i_SliceNum(INT_UNDEFINED)
  , sErrorString("No Error")
{
  pData = new float[n_rows*n_columns];
}


CProjection::~CProjection(void) {
  if(pData != NULL) delete[] pData;
  pData = NULL;
}


bool
CProjection::Normalize(float fMin, float fMax) {
  if(!IsInitialized()) 
    return false;
  float myMin, myMax;
  GetMinMax(&myMin, &myMax);
  for(int i=0; i<n_rows*n_columns; i++)
    pData[i] = (fMax-fMin)*((pData[i]-myMin)/(myMax-myMin) + fMin);
  return pData != NULL;
}


bool
CProjection::IsInitialized(void) {
  return pData != NULL;
}


bool
CProjection::InitFromData(float* pDataIn, int nRows, int nColumns) {
  if(nRows < 1 || nColumns < 1) {
    sErrorString = "Illegal Input Values";
    return false;
  }
  n_rows = nRows;
  n_columns = nColumns;
  try {
    pData = new float[n_rows*n_columns];
  } catch(...) {
    sErrorString = "Not Enough Memory to Initialize Projection";
    return false;
  }
  try {
    memcpy(pData, pDataIn, sizeof(*pDataIn)*n_rows*n_columns );
  } catch(...) {
    sErrorString = "Error Copying Data\nImpossible to Initialize Projection";
    delete[] pData;
    pData = NULL;
    return false;
  }
  sErrorString = "No error";
  return true;
}


bool
CProjection::Crop(int xStart, int xEnd, int yStart, int yEnd) {
  if(!IsInitialized()) {
    sErrorString = "Error: Not Initialized";
    return false;
  }
  if((xStart < 0) || (xStart > n_columns-1) || (xEnd < 1) || (xEnd > n_columns) ||
     (yStart < 0) || (yStart > n_rows-1)    || (yEnd < 1) || (yEnd > n_rows)    ||
     (xStart >= xEnd) || (yStart >= yEnd)) {
    sErrorString = "Illegal Crop Region";
    return false;
  }

  int xSize = xEnd-xStart;
  int ySize = yEnd-yStart;
  float* pTemp = new float[xSize*ySize];
  int yOffi, yOffo;
  for(int y=0; y<ySize; y++) {
    yOffi = (y+yStart)*n_columns+xStart;
    yOffo = y * xSize;
    for(int x=0; x<xSize; x++) {
      *(pTemp+yOffo+x) = *(pData+yOffi+x);
    }
  }
  n_columns = xSize;
  n_rows = ySize;

  delete[] pData;
  pData = pTemp;

  sErrorString = "No error";
  return true;
}


bool
CProjection::Crop(QRect Selection) {
  if(!IsInitialized()) {
    sErrorString = "Error: Not Initialized";
    return false;
  }
  if(!QRect(0, 0, n_columns-1, n_rows-1).contains(Selection)) {
    sErrorString = "Illegal Crop Region";
    return false;
  }

  int xSize = abs(Selection.width());
  int ySize = abs(Selection.height());
  float* pTemp = new float[xSize*ySize];
  int xStart = Selection.width()>0  ? Selection.x() : Selection.width()  + Selection.x();
  int yStart = Selection.height()>0 ? Selection.y() : Selection.height() + Selection.y();
  int yOffi, yOffo;
  for(int y=0; y<ySize; y++) {
    yOffi = (y+yStart)*n_columns+xStart;
    yOffo = y * xSize;
    for(int x=0; x<xSize; x++) {
      *(pTemp+yOffo+x) = *(pData+yOffi+x);
    }
  }
  n_columns = xSize;
  n_rows = ySize;

  delete[] pData;
  pData = pTemp;

  sErrorString = "No error";
  return true;
}


bool
CProjection::Crop(QRectF _Selection) {
  if(!IsInitialized()) {
    sErrorString = "Error: Not Initialized";
    return false;
  }
  QRect Selection(_Selection.x(), _Selection.y(), _Selection.width(), _Selection.height());

  if(!QRect(0, 0, n_columns-1, n_rows-1).contains(Selection)) {
    sErrorString = "Illegal Crop Region";
    return false;
  }

  int xSize = abs(Selection.width());
  int ySize = abs(Selection.height());
  float* pTemp = new float[xSize*ySize];
  int yOffi, yOffo;
  for(int y=0; y<ySize; y++) {
    yOffi = (y+Selection.y())*n_columns+Selection.x();
    yOffo = y * xSize;
    for(int x=0; x<xSize; x++) {
      *(pTemp+yOffo+x) = *(pData+yOffi+x);
    }
  }
  n_columns = xSize;
  n_rows = ySize;

  delete[] pData;
  pData = pTemp;

  sErrorString = "No error";
  return true;
}


// A Median Filter
bool
CProjection::Median(int Ksize) {
  if(!IsInitialized()) {
      sErrorString = "Error: Not Initialized";
      return false;
  }
  int k2 = Ksize/2;
  int kmax= Ksize-k2;
  int i, j, k;
  int kk = Ksize*Ksize;
  float* kernel = new float[kk*sizeof(*pData)];
  float* tmp = new float[n_columns*n_rows*sizeof(*pData)];
  memcpy(tmp, pData, n_columns*n_rows*sizeof(*pData));

  int xmin, xmax, ymin, ymax;
  xmin = ymin = 0;
  xmax = n_columns; ymax = n_rows;
  float mean, fVal;

  for(int y=ymin; y<ymax; y++) {
      for(int x=xmin; x<xmax; x++) {
          mean = 0.0;
          for(j=-k2, i=0;j<kmax;j++) {
              for(k=-k2; k<kmax; k++) {
                  if((x+j>-1 && y+k>-1) &&(x+j<n_columns && y+k<n_rows)) {
                      fVal = *(pData+x+j+(y+k)*n_columns);
                      mean += fVal;
                      kernel[i++]=fVal;
                  }
              } // for(j=-k2, i=0;j<kmax;j++) {for(k=-k2; k<kmax; k++)
          } // for(long x=xmin; x<xmax; x++) {for(j=-k2, i=0;j<kmax;j++)
          mean /= float(kk);
          if(fabs(*(tmp+x+y*n_columns)-mean) > f_MedianThreshold*mean) {
              bSort(kernel, i);
              *(tmp+x+y*n_columns) = kernel[i/2];
          }
      } // for(long y=ymin; y<ymax; y++) {for(long x=xmin; x<xmax; x++)
  } // for(long y=ymin; y<ymax; y++)

  delete[] kernel;
  memcpy(pData, tmp, n_columns*n_rows*sizeof(*pData));
  delete[] tmp;
  sErrorString = QObject::tr("No error");
  return true;
}


void 
CProjection::bSort(float *a, int n) {
  int i,j;
  float tmp;
  for(i=0; i<n-1; i++) {
    for(j=0; j<n-1-i; j++)
      if(a[j+1] < a[j]) {  /* compare the two neighbors */
        tmp = a[j];         /* swap a[j] and a[j+1]      */
        a[j] = a[j+1];
        a[j+1] = tmp;
    }
  }
}


bool
CProjection::Copy(CProjection& P2Copy) {
  if(!P2Copy.IsInitialized()) {
   sErrorString = QObject::tr("Error: Not Initialized");
   return false;
  }
  if(!IsInitialized()) {
    n_columns = P2Copy.n_columns;
    n_rows = P2Copy.n_rows;
    pData = new float[n_rows*n_columns];
  } else {
    float* pTmp = new float[P2Copy.n_columns*P2Copy.n_rows];
    if(pTmp == NULL) {
     sErrorString = QObject::tr("Not Enough Memory");
     return false;
    } else {
      delete[] pData;
      pData = pTmp;
      n_columns = P2Copy.n_columns;
      n_rows = P2Copy.n_rows;
    }
  }
  for(int i=0; i<n_rows*n_columns; i++) {
    *(pData+i) = *(P2Copy.pData+i);
  }
  sErrorString = QObject::tr("No error");
	return true;
}


bool
CProjection::Sum(CProjection& P2Add) {
  if(!P2Add.IsInitialized()) {
   sErrorString = QObject::tr("Error: Not Initialized");
   return false;
  }
  if(!IsInitialized()) {
    n_columns = P2Add.n_columns;
    n_rows = P2Add.n_rows;
    pData = new float[n_rows*n_columns];
  } else {
    if((n_columns != P2Add.n_columns) || (n_rows != P2Add.n_rows)) {
     sErrorString = QObject::tr("Different Size");
     return false;
    }
  }
  for(int i=0; i<n_rows*n_columns; i++) {
    *(pData+i) += *(P2Add.pData+i);
  }
  sErrorString = QObject::tr("No error");
	return true;
}


bool
CProjection::Subtract(CProjection& P2Subtract) {
  if(!P2Subtract.IsInitialized()) {
   sErrorString = QObject::tr("Error: Not Initialized");
   return false;
  }
  if(!IsInitialized()) {
    n_columns = P2Subtract.n_columns;
    n_rows = P2Subtract.n_rows;
    pData = new float[n_rows*n_columns];
  } else {
    if((n_columns != P2Subtract.n_columns) || (n_rows != P2Subtract.n_rows)) {
     sErrorString = QObject::tr("Different Size");
     return false;
    }
  }
  for(int i=0; i<n_rows*n_columns; i++) {
    *(pData+i) -= *(P2Subtract.pData+i);
  }
  sErrorString = QObject::tr("No error");
	return true;
}


bool
CProjection::LinTransform(float factor, float offset) {
  if(!IsInitialized()) {
   sErrorString = QObject::tr("Error: Not Initialized");
   return false;
  }
  for(int i=0; i<n_rows*n_columns; i++) {
    *(pData+i) = (*(pData+i) - offset) * factor;
  }
  sErrorString = QObject::tr("No error");
	return true;
}


bool
CProjection::GetMinMax(float* min, float* max) {
  if(!IsInitialized()) {
   sErrorString = QObject::tr("Error: Not Initialized");
   return false;
  }
  *min = FLT_MAX;
  *max =-FLT_MAX;
  for(int x=0; x<n_rows*n_columns; x++) {
    if(qIsInf(*(pData+x)))
      *min = *min;
    if(*(pData+x) < *min) *min = *(pData+x);
    if(*(pData+x) > *max) *max = *(pData+x);
  }
  sErrorString = QObject::tr("No error");
	return true;
}


bool
CProjection::GetPixelsSum(float* PixelsSum){

 if(!IsInitialized()) {
   sErrorString = QObject::tr("Error: Not Initialized");
   return false;
  }
  *PixelsSum = 0.0f;
  
  for(int x=0; x<n_rows*n_columns; x++) {
    if(!qIsInf(double(*(pData+x))))
      *PixelsSum += *(pData+x);
  }
  sErrorString = QObject::tr("No error");
	return true;
}


bool
CProjection::Reset(void) {
  if(IsInitialized()) {
    delete[] pData;
  }
  pData           = NULL;
  n_rows          = 0;
  n_columns       = 0;
  f_TimeOfFlight  = FLOAT_UNDEFINED;
  f_angle         = FLOAT_UNDEFINED;
  f_BeamIntensity = FLOAT_UNDEFINED;
  f_PosX          = FLOAT_UNDEFINED;
  f_PosY          = FLOAT_UNDEFINED;
  f_PosZ          = FLOAT_UNDEFINED;
  i_SliceNum      = INT_UNDEFINED;
  sErrorString    = QObject::tr("No error");
  return true;
}


bool
CProjection::FlatFieldCorr(CProjection& Beam, CProjection& Dark) {
  if(!IsInitialized() || !Beam.IsInitialized() || !Dark.IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitialized");
    return false;
  }
  //Flat Field Correction using both dark and open beam
  float den;
  for(int y=0; y<n_rows*n_columns; y++) {
    den = (*(Beam.pData+y)  - (*(Dark.pData+y)));
    if(den > FLT_MIN)
      *(pData+y) = (*(pData+y) - (*(Dark.pData+y))) / den;
    else if(*(Beam.pData+y) > FLT_MIN)
      *(pData+y) /= *(Beam.pData+y);
    if(*(pData+y) < 0.0) 
      *(pData+y) = FLT_MIN;
  }
  sErrorString = QObject::tr("No error");
  return true;
}


bool
CProjection::FlatFieldCorr(CProjection& Beam) {
  if(!IsInitialized() || !Beam.IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitalized");
    return false;
  }
  //Flat Field Correction just using open beam
  for(int y=0; y<n_rows*n_columns; y++) {
    if(*(Beam.pData+y) > FLT_MIN)
      *(pData+y) /= *(Beam.pData+y);
    if(*(pData+y) < 0.0)
      *(pData+y) = FLT_MIN;
  }
  sErrorString = QObject::tr("No error");
  return true;
}


// Compute -log(I/I0) with I0 obtained from an
// unobstructed portion of the image
bool
CProjection::OpenBeamCorr(int x0, int x1, int y0, int y1) {
  if(!IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitalized");
    return false;
  }
  float fSum = 0.0;
  float nPix = float((y1-y0)*(x1-x0));
  int yOff;
  for(int y=y0; y<y1; y++) {
    yOff = y*n_columns;
    for(int x=x0; x<x1; x++) {
      fSum += *(pData+yOff+x);
    }
  }
  fSum /= float(nPix);
  if(fSum <= 0.0) fSum = 1.0;

  float fVal;
  for(int y=0; y<n_rows*n_columns; y++) {
    fVal = *(pData+y);
    if(fVal <= 0.0f) 
      fVal = 0.4f;
    fVal = log(fSum/fVal);
    *(pData+y) = fVal;
  }
  sErrorString = QObject::tr("No error");
  return true;
}


// Compute -log(I/I0) with I0 obtained from an
// unobstructed portion of the image
bool
CProjection::OpenBeamCorr(QRect beamRegion) {
  if(!IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitalized");
    return false;
  }
  int x0 = beamRegion.width()>0 ? beamRegion.x() : beamRegion.width()+beamRegion.x();
  int x1 = x0 + abs(beamRegion.width());
  int y0 = beamRegion.height()>0 ? beamRegion.y() : beamRegion.height()+beamRegion.y();
  int y1 = y0 + abs(beamRegion.height());
  float fSum = 0.0;
  float nPix = float((y1-y0)*(x1-x0));
  int yOff;
  for(int y=y0; y<y1; y++) {
    yOff = y*n_columns;
    for(int x=x0; x<x1; x++) {
      fSum += *(pData+yOff+x);
    }
  }
  fSum /= nPix;
  if(fSum <= 0.0) fSum = 1.0;

  float fVal;
  for(int y=0; y<n_rows*n_columns; y++) {
    fVal = *(pData+y);
    if(fVal <= 0.0f)
      fVal = 0.4f;
    fVal = log(fSum/fVal);
    *(pData+y) = fVal;
  }
  sErrorString = QObject::tr("No error");
  return true;
}


// Compute -log(I/I0) with I0 obtained from an
// external estimation of the beam intensity
bool
CProjection::OpenBeamCorr(float fBeam) {
  if(!IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitalized");
    return false;
  }
  if(fBeam <= 0.0) {
    sErrorString = QObject::tr("Illegal Beam Value");
    return false;
  }

  float fVal;
  for(int y=0; y<n_rows*n_columns; y++) {
    fVal = *(pData+y);
    if(fVal <= 0.0f) { 
      *(pData+y) = FLT_MAX;
      continue;
    } 
    fVal = log(fBeam/fVal);
    *(pData+y) = fVal;
  }
  sErrorString = QObject::tr("No error");
  return true;
}


bool
CProjection::fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if it is really a file and no a directory
    return (check_file.exists() && check_file.isFile());
}


// Create a FITS primary array containing a 2-D image
bool
CProjection::WriteToFitsFile(QString sDirectory, QString sFileName) {
  if(!IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitalized");
    return false;
  }

  fitsfile *fitsFilePtr;
  int iStatus = 0;

  QString sFilename = sDirectory + QObject::tr("/") + sFileName;// name for new FITS file
 
  int bitpix    =  FLOAT_IMG;// 32-bit float pixel values
  long naxis    =   2;       // 2-dimensional image
  long naxes[2] = { n_columns, n_rows };

  if(fileExists(sFilename)) {
    QFile::remove(sFilename);// Delete old file if it already exists
  }

  if(fits_create_file(&fitsFilePtr, sFilename.toLatin1(), &iStatus)) {// create new FITS file
    SetErrorString(iStatus);
    return false;
  }

  if(fits_create_img(fitsFilePtr, bitpix, naxis, naxes, &iStatus)) {
    SetErrorString(iStatus);          
    return false;
  }

  if(fits_write_img(fitsFilePtr, TFLOAT, 1, naxes[0]*naxes[1], pData, &iStatus)) {
    SetErrorString(iStatus);
    return false;
  }

  if(!WriteKeys(fitsFilePtr)) {
    SetErrorString(iStatus);
    fits_close_file(fitsFilePtr, &iStatus);
    return false;
  }

  if(fits_close_file(fitsFilePtr, &iStatus)) {
    SetErrorString(iStatus);
    return false;
  }

  return true;
}


bool
CProjection::WriteKeys(fitsfile* fitsFilePtr) {
  int iStatus = 0;
  int iRes;
  float fVal;

  if(f_TimeOfFlight == FLOAT_UNDEFINED) {
    iRes = fits_delete_key(fitsFilePtr, "GATEDELY", &iStatus);
    iRes = fits_delete_key(fitsFilePtr, "TOF", &iStatus);
    iStatus = 0;
  } else {
    fVal = f_TimeOfFlight * 1.0e-15f;
    iRes = fits_update_key(fitsFilePtr, TFLOAT, "GATEDELY", &fVal, "Gate delay", &iStatus);
  }

  if(f_angle == FLOAT_UNDEFINED) {
    iRes = fits_delete_key(fitsFilePtr, "ANGPOS", &iStatus);
    iStatus = 0;
  } else {
    iRes = fits_update_key(fitsFilePtr, TFLOAT, "ANGPOS", &f_angle, "Angular Position Degree", &iStatus);
  }

  if(f_BeamIntensity == FLOAT_UNDEFINED) {
    iRes = fits_delete_key(fitsFilePtr, "CURRENT", &iStatus);
    iStatus = 0;
  } else {
    iRes = fits_update_key(fitsFilePtr, TFLOAT, "CURRENT", &f_BeamIntensity, "Total Beam Current uA", &iStatus);
  }

  if(f_PosX == FLOAT_UNDEFINED) {
    iRes = fits_delete_key(fitsFilePtr, "X_POS", &iStatus);
    iStatus = 0;
  } else {
    iRes = fits_update_key(fitsFilePtr, TFLOAT, "X_POS", &f_PosX, "X Position mm", &iStatus);
  }

  if(f_PosY == FLOAT_UNDEFINED) {
    iRes = fits_delete_key(fitsFilePtr, "Y_POS", &iStatus);
    iStatus = 0;
  } else {
    iRes = fits_update_key(fitsFilePtr, TFLOAT, "Y_POS", &f_PosY, "Y Position mm", &iStatus);
  }

  if(f_PosZ == FLOAT_UNDEFINED) {
    iRes = fits_delete_key(fitsFilePtr, "Z_POS", &iStatus);
    iStatus = 0;
  } else {
    iRes = fits_update_key(fitsFilePtr, TFLOAT, "Z_POS", &f_PosZ, "Z Position mm", &iStatus);
  }

  if(i_SliceNum == FLOAT_UNDEFINED) {
    iRes = fits_delete_key(fitsFilePtr, "SLICENUM", &iStatus);
    iStatus = 0;
  } else {
    iRes = fits_update_key(fitsFilePtr, TINT, "SLICENUM", &i_SliceNum, "Slice Number", &iStatus);
  }
  if(iRes) qDebug() << "iRes =" << iRes;
  fits_clear_errmsg();
  return true;
}


void
CProjection::SetErrorString(const int status) {
  char status_str[FLEN_STATUS], errmsg[FLEN_ERRMSG];
  QString sString;
  if(status) {
    // get the error description
    fits_get_errstatus(status, status_str);
    sErrorString = QObject::tr("FITSIO status = %1: %2").arg(status).arg(QString(status_str));
    // get error stack messages
    while(fits_read_errmsg(errmsg))  {
      sString = QString(errmsg);
      sErrorString += sString;
    }
  }
}


bool
CProjection::ReadFromFitsFile(QString sPathName) {
  fitsfile *fitsFilePtr;
  int iStatus=0;
  int iScaling = 1;

  if(fits_open_file(&fitsFilePtr, sPathName.toLatin1(), READONLY, &iStatus)) {
    SetErrorString(iStatus);
    return false;
  }

  int nHdu;
  if(fits_get_num_hdus(fitsFilePtr, &nHdu, &iStatus)) {
    SetErrorString(iStatus);
    return false;
  }
  if(nHdu != 1) {
    sErrorString = QObject::tr("Numero di HDU > 1");
    return false;
  }

  int HduType;
  if(fits_get_hdu_type(fitsFilePtr, &HduType, &iStatus)) {
    SetErrorString(iStatus);
    return false;
  }
  if(HduType != IMAGE_HDU) {
    sErrorString = QObject::tr("File: %1 is NOT an Image !").arg(sPathName);
    return false;
  }

  int nAxes;
  if(fits_get_img_dim(fitsFilePtr, &nAxes, &iStatus)) {
    SetErrorString(iStatus);
    return false;
  }

  int imgType;
  if(fits_get_img_type(fitsFilePtr, &imgType, &iStatus)) {
    SetErrorString(iStatus);
    return false;
  }
  
  long* axisDim = new long[nAxes]; 
  if(fits_get_img_size(fitsFilePtr, 2, axisDim, &iStatus)) {
    SetErrorString(iStatus);
    return false;
  }

  n_columns = axisDim[0]/iScaling;
  n_rows    = axisDim[1]/iScaling;

  long firstPixel[] = {1, 1, 1};
  int dataType;
  if(pData) delete[] pData;
  pData = new float[axisDim[0]*axisDim[1]];

  void* pImage;
  switch(imgType) {
    case BYTE_IMG:
      dataType = TBYTE;
      pImage = new uchar[axisDim[0]*axisDim[1]];
      if(fits_read_pix(fitsFilePtr, dataType, firstPixel, axisDim[0]*axisDim[1], NULL, pImage, NULL, &iStatus)) {
        SetErrorString(iStatus);
        return false;
      }
      for(int y=0; y<n_rows; y++) {
        for(int x=0; x<n_columns; x++) {
          *(pData+y*n_columns+x) = float(*((uchar *)pImage+iScaling*(y*axisDim[0]+x)));
        }
      }
      delete[] (uchar *)pImage;
      break;
    case SHORT_IMG: 
      dataType = TUSHORT;
      pImage = new ushort[axisDim[0]*axisDim[1]];
      if(fits_read_pix(fitsFilePtr, dataType, firstPixel, axisDim[0]*axisDim[1], NULL, pImage, NULL, &iStatus)) {
        SetErrorString(iStatus);
        return false;
      }
      for(int y=0; y<n_rows; y++) {
        for(int x=0; x<n_columns; x++) {
          *(pData+y*n_columns+x) = float(*((unsigned short *)pImage+iScaling*(y*axisDim[0]+x)));
        }
      }
      delete[] (ushort *)pImage;
      break;
    case LONG_IMG: 
      dataType = TUINT;
      pImage = new uint[axisDim[0]*axisDim[1]];
      if(fits_read_pix(fitsFilePtr, dataType, firstPixel, axisDim[0]*axisDim[1], NULL, pImage, NULL, &iStatus)) {
        SetErrorString(iStatus);
        return false;
      }
      for(int y=0; y<n_rows; y++) {
        for(int x=0; x<n_columns; x++) {
          *(pData+y*n_columns+x) = float(*((unsigned int *)pImage+iScaling*(y*axisDim[0]+x)));
        }
      }
      delete[] (uint *)pImage;
      break;
    case FLOAT_IMG: 
      dataType = TFLOAT;
      if(fits_read_pix(fitsFilePtr, dataType, firstPixel, axisDim[0]*axisDim[1], NULL, pData, NULL, &iStatus)) {
        SetErrorString(iStatus);
        return false;
      }
      break;
    case DOUBLE_IMG: 
      dataType = TDOUBLE;
      pImage = new double[axisDim[0]*axisDim[1]];
      if(fits_read_pix(fitsFilePtr, dataType, firstPixel, axisDim[0]*axisDim[1], NULL, pImage, NULL, &iStatus)) {
        SetErrorString(iStatus);
        return false;
      }
      for(int y=0; y<n_rows; y++) {
        for(int x=0; x<n_columns; x++) {
          *(pData+y*n_columns+x) = float(*((double *)pImage+iScaling*(y*axisDim[0]+x)));
        }
      }
      delete[] (double *)pImage;
      break;
    default:
      sErrorString = QObject::tr("File: %1 has an unknown format").arg(sPathName);
      return false;
  }
  delete[] axisDim;  

  if(!ReadKeys(fitsFilePtr)) {
    fits_close_file(fitsFilePtr, &iStatus);
    return false;
  }

  if(fits_close_file(fitsFilePtr, &iStatus)) {
    SetErrorString(iStatus);
    return false;
  }
  sErrorString = QObject::tr("No error");
  return true;
}


bool
CProjection::ReadKeys(fitsfile* fitsFilePtr) {
  int iStatus=0;
  int iRes;

  f_TimeOfFlight  = FLOAT_UNDEFINED;
  f_angle         = FLOAT_UNDEFINED;
  f_BeamIntensity = FLOAT_UNDEFINED;
  f_PosX          = FLOAT_UNDEFINED;
  f_PosY          = FLOAT_UNDEFINED;
  f_PosZ          = FLOAT_UNDEFINED;
  i_SliceNum      = INT_UNDEFINED;

  iRes = fits_read_key(fitsFilePtr, TFLOAT, "GATEDELY", &f_TimeOfFlight, NULL, &iStatus);
  iStatus = 0;
  if(!iRes) f_TimeOfFlight *= 1.0e15f;
  iRes = fits_read_key(fitsFilePtr, TFLOAT, "TOF", &f_TimeOfFlight, NULL, &iStatus);
  iStatus = 0;
  if(!iRes) f_TimeOfFlight *= 1.0e6f;
  iRes = fits_read_key(fitsFilePtr, TFLOAT, "ANGPOS", &f_angle, NULL, &iStatus);
  iStatus = 0;
  iRes = fits_read_key(fitsFilePtr, TFLOAT, "CURRENT", &f_BeamIntensity, NULL, &iStatus);
  iStatus = 0;
  iRes = fits_read_key(fitsFilePtr, TFLOAT, "X_POS", &f_PosX, NULL, &iStatus);
  iStatus = 0;
  iRes = fits_read_key(fitsFilePtr, TFLOAT, "Y_POS", &f_PosY, NULL, &iStatus);
  iStatus = 0;
  iRes = fits_read_key(fitsFilePtr, TFLOAT, "Z_POS", &f_PosZ, NULL, &iStatus);
  iStatus = 0;
  iRes = fits_read_key(fitsFilePtr, TINT, "SLICENUM", &i_SliceNum, NULL, &iStatus);
  iStatus = 0;

  return true;
}


bool
CProjection::VerticalFlip(void){
  if(!IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitalized");
    return false;
  }
  float* pTemp = new float[n_columns*n_rows];
  if(pTemp == NULL) {
    sErrorString = QObject::tr("Not Enough Memory to Flip Image");
    return false;
  }
  int iOff = n_columns*n_rows-1;
  for(int i=0; i<n_columns*n_rows; i++)
    *(pTemp+i) = *(pData+iOff-i);
  delete[] pData;
  pData = pTemp;
  sErrorString = QObject::tr("No error");
  return true;
}


bool
CProjection::HorizontalFlip(void) {
  if(!IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitalized");
    return false;
  }
  float* pTemp = new float[n_columns*n_rows];
  if(pTemp == NULL) {
    sErrorString = QObject::tr("Not Enough Memory to Create Mirror Image");
    return false;
  }
  int Off;
  for(int y=0; y<n_rows; y++) {
    Off = y*n_columns;
    for(int x=0; x<n_columns; x++) {
      *(pTemp+Off+n_columns-x-1) = *(pData+Off+x);
    }
  }
  delete[] pData;
  pData = pTemp;
  sErrorString = QObject::tr("No error");
  return true;
}


bool
CProjection::ClearImage(void) {
  if(!IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitalized");
    return false;
  }
  memset(pData, '0', n_columns*n_rows*sizeof(*pData));
  return true;
}


bool
CProjection::Rotate(float angle){// In radians
  if(!IsInitialized()) {
    sErrorString = QObject::tr("Projection Uninitalized");
    return false;
  }
  float fmin, fmax;
  GetMinMax(&fmin, &fmax);
  float cos_angle = (float) cos(angle);	//these two are needed later (to rotate)
  float sin_angle = (float) sin(angle);
	
  //Calculate the size of the new bitmap (rotate corners of image)
  QPointF p[4];                              //original corners of the image
  p[0] = QPointF(-0.5f, -0.5f);
  p[1] = QPointF(n_columns-0.5f, -0.5f);
  p[2] = QPointF(-0.5f, n_rows-0.5f);
  p[3] = QPointF(n_columns-0.5f, n_rows-0.5f);

  QPointF newp[4];								//rotated positions of corners
  //(rotate corners)
  for(int i=0; i<4; i++) {
    newp[i].setX(p[i].x()*cos_angle - p[i].y()*sin_angle);
    newp[i].setY(p[i].x()*sin_angle + p[i].y()*cos_angle);
  }//for i

  //(read new dimensions from location of corners)
  float minx = (float) qMin(qMin(newp[0].x(),newp[1].x()), qMin(newp[2].x(),newp[3].x()));
  float miny = (float) qMin(qMin(newp[0].y(),newp[1].y()), qMin(newp[2].y(),newp[3].y()));
  float maxx = (float) qMax(qMax(newp[0].x(),newp[1].x()), qMax(newp[2].x(),newp[3].x()));
  float maxy = (float) qMax(qMax(newp[0].y(),newp[1].y()), qMax(newp[2].y(),newp[3].y()));

  int newWidth = (int) qFloor(maxx-minx+0.5f);
  int newHeight= (int) qFloor(maxy-miny+0.5f);

  //float newxcenteroffset = 0.5f * newWidth;
  //float newycenteroffset = 0.5f * newHeight;

  float ssx=((maxx+minx)- ((float) newWidth-1))/2.0f;   //start for x
  float ssy=((maxy+miny)- ((float) newHeight-1))/2.0f;  //start for y

  //create destination image
  CProjection* p_imgDest;
  p_imgDest = new CProjection(newWidth, newHeight);

  float x, y;             //destination location (float, with proper offset)
  float origx, origy;     //origin location
  int destx, desty;       //destination location
  float *pxptr;

  y = ssy;                //initialize y
  for(desty=0; desty<newHeight; desty++) {
    x = ssx;//initialize x
    pxptr = p_imgDest->pData+desty*newWidth;//pointer to first byte in row
    for(destx=0; destx<newWidth; destx++) {
      //get source pixel coordinate for current destination point
      origx = cos_angle*x + sin_angle*y;
      origy = cos_angle*y - sin_angle*x;
      *pxptr++ = GetPixelColorInterpolated(origx, origy, fmin);
      x++;
    }//for destx
    y++;
  }//for desty

  //select the destination
  int x0, y0;
  x0 = int((newWidth-n_columns)*0.5);
  y0 = int((newHeight-n_rows)*0.5);
  p_imgDest->Crop(x0, x0+n_columns, y0, y0+n_rows);
  p_imgDest->GetMinMax(&fmin, &fmax);
  memcpy(pData, p_imgDest->pData, p_imgDest->n_rows*p_imgDest->n_columns*sizeof(*p_imgDest->pData));

  delete p_imgDest;

  return true;
}


float
CProjection::GetPixelColorInterpolated(float x, float y, float rplColor) {

  int xi = (int)(x); if(x<0) xi--;   //these replace (incredibly slow) floor 
  int yi = (int)(y); if(y<0) yi--;   //(Visual c++ 2003, AMD Athlon)
  float color;

  //bilinear interpolation
  if(xi<-1 || xi>=n_columns || yi<-1 || yi>=n_rows) {// all 4 points are outside bounds
    //we don't need to interpolate anything with all points outside in this case
    return rplColor;
  }//if

  float t1 = x - xi;
  float t2 = y - yi;
  float d  = t1 * t2;
  float b  = t1 - d;
  float c  = t2 - d;
  float a  = 1.0f - t1 - c;
  //get four neighbouring pixels
  if((xi+1)<n_columns && xi>=0 && (yi+1)<n_rows && yi>=0) {
    //all pixels are inside the image
    float *pxptr = pData + yi*n_columns + xi;
    color  = a * (*pxptr++);
    color += b * (*pxptr);
    pxptr += n_columns - 1; //move to next row
    color += c * (*pxptr++); 
    color += d * (*pxptr); 
  } else {
    //default (slower) way to get pixels (some pixels out of borders)
    float rgb11, rgb21, rgb12, rgb22;
    rgb11 = GetPixelColorWithOverflow(xi,   yi,   rplColor);
    rgb21 = GetPixelColorWithOverflow(xi+1, yi,   rplColor);
    rgb12 = GetPixelColorWithOverflow(xi,   yi+1, rplColor);
    rgb22 = GetPixelColorWithOverflow(xi+1, yi+1, rplColor);
    //calculate linear interpolation
    color = a*rgb11 + b*rgb21 + c*rgb12 + d*rgb22;
  }//if
  return color;

}


float
CProjection::GetPixelColorWithOverflow(int x, int y, float rplColor) {
  float fval;
  if((x > -1) && (x < n_columns) && (y > -1) && (y < n_rows)) {
    fval = *(pData+y*n_columns+x);
  } else {
    fval = rplColor;
  }
  return fval;
}
