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
#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QObject>

class ImageWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  ImageWindow(QWindow *pParent=Q_NULLPTR);
  QSize minimumSizeHint() const;
  QSize sizeHint() const;
  bool loadTexture(float* pImage, int Nx, int Ny);
  bool initTexture();
  void enableSelection(bool bEnable);
  bool isSelectionEnabled();

signals:
  void startPosEvent(QPoint startPos);
  void endPosEvent(QPoint endPos);

protected:
  void initializeGL();
  void resizeGL(int w, int height);
  void paintGL();
  bool checkGLErrors(QString sModule);
  void initShaders();
  bool event(QEvent* event);

  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent* event);

private:
  GLuint texture;
  int texWidth, texHeight;
  int myWidth, myHeight;
  double scaleX, scaleY;
  bool selectionEnabled;

  QOpenGLBuffer vertexbuffer;
  QOpenGLBuffer uvbuffer;
  QOpenGLBuffer selectionBuffer;

  QOpenGLShaderProgram imageProgram, selectionProgram;
  QMatrix4x4 projectionMatrix;
  QMatrix4x4 modelMatrix;
  QMatrix4x4 normalMatrix;
  QMatrix4x4 viewMatrix;
  QMatrix4x4 mvpMatrix;
  QVector<QVector3D> vertices;
  QVector<QVector2D> textureVertices;
  QVector<QVector3D> selectionVertices;
  QPointF startPos, endPos;
};

#endif // IMAGEWINDOW_H
