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
#pragma once
#include <QString>
#include <QObject>

#include "fitsio.h"// Fits image formats


class 
CProjection {

public:
  CProjection(void);
  CProjection(int nColumns, int nRows);
  virtual ~CProjection(void);

private:
  float GetPixelColorInterpolated(float x, float y, float rplColor);
  float GetPixelColorWithOverflow(int x, int y, float rplColor);

protected:
  void bSort(float *a, int n);
  void SetErrorString(const int status);
  virtual bool ReadKeys(fitsfile* fitsFilePtr);
  virtual bool WriteKeys(fitsfile* fitsFilePtr);

public:
  const  float FLOAT_UNDEFINED;
  const  int   INT_UNDEFINED;
  int    n_rows;
  int	 n_columns;
  float* pData;
  float  f_angle;
  float  f_BeamIntensity;
  float  f_MedianThreshold;
  float  f_TimeOfFlight;
  float  f_PosX, f_PosY, f_PosZ;
  int    i_SliceNum;
  QString sErrorString;

  virtual bool Reset(void);
  bool fileExists(QString path);
  bool ReadFromFitsFile(QString sPathName);
  bool WriteToFitsFile(QString sDirectory, QString sFileName);

  bool IsInitialized(void);
  bool InitFromData(float* pDataIn, int nRows, int nColumns);
  bool Normalize(float fMin, float fMax);
  bool Median(int Ksize);
  bool Copy(CProjection& P2Copy);
  bool Sum(CProjection& P2Add);
  bool Subtract(CProjection& P2Subtract);
  bool Crop(int xStart, int xEnd, int yStart, int yEnd);
  bool Crop(QRect Selection);
  bool Crop(QRectF Selection);
  bool LinTransform(float factor, float offset);
  bool GetMinMax(float* min, float* max);
  bool GetPixelsSum(float* PixelsSum);
  bool ClearImage(void);

  bool FlatFieldCorr(CProjection& Beam, CProjection& Dark);
  bool FlatFieldCorr(CProjection& Beam);
  bool OpenBeamCorr(int x0, int x1, int y0, int y1);
  bool OpenBeamCorr(QRect beamRegion);
  bool OpenBeamCorr(float fBeam);

  bool VerticalFlip(void);
  bool HorizontalFlip(void);
  bool Rotate(float angle);// In radians
};
