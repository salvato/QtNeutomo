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
#include "imagewindow.h"
#include <QMouseEvent>
#include <QSettings>

ImageWindow::ImageWindow(QWindow *pParent)
  : QOpenGLWindow(QOpenGLWindow::NoPartialUpdate, pParent)
  , selectionEnabled(false)
{

  vertices.append(QVector3D(-1.0, -1.0, 0.0));
  vertices.append(QVector3D( 1.0, -1.0, 0.0));
  vertices.append(QVector3D(-1.0,  1.0, 0.0));
  vertices.append(QVector3D(-1.0,  1.0, 0.0));
  vertices.append(QVector3D( 1.0,  1.0, 0.0));
  vertices.append(QVector3D( 1.0, -1.0, 0.0));

  selectionVertices.append(QVector3D(-1.0, -1.0, 0.0));
  selectionVertices.append(QVector3D( 1.0, -1.0, 0.0));
  selectionVertices.append(QVector3D( 1.0, -1.0, 0.0));
  selectionVertices.append(QVector3D( 1.0,  1.0, 0.0));
  selectionVertices.append(QVector3D( 1.0,  1.0, 0.0));
  selectionVertices.append(QVector3D(-1.0,  1.0, 0.0));
  selectionVertices.append(QVector3D(-1.0,  1.0, 0.0));
  selectionVertices.append(QVector3D(-1.0, -1.0, 0.0));

  // Restore previous section values
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  setPosition(QPoint(settings.value("ImageWindow/geometry", QPoint(100, 100)).toPoint()));
  // Remove the close button [X] from the title bar
  setFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
}


bool
ImageWindow::event(QEvent* event) {
  if(event->type()==QEvent::Close) {
    QSettings settings("Gabriele Salvato", "QtNeuTomo");
    settings.setValue("ImageWindow/geometry", position());
  }
  QOpenGLWindow::event(event);
  return true;
}


QSize
ImageWindow::minimumSizeHint() const {
  return QSize(256, 256);
}


QSize
ImageWindow::sizeHint() const {
  return QSize(512, 512);
}


void
ImageWindow::initializeGL() {
  // Set up the rendering context, load shaders and other resources, etc.:
  initializeOpenGLFunctions();
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  initShaders();
  // Transfer vertex data to VBO 0
  vertexbuffer.create();
  vertexbuffer.bind();
  vertexbuffer.allocate((void *)vertices.data(), vertices.size() * sizeof(QVector3D));
  selectionBuffer.create();
  selectionBuffer.bind();
  selectionBuffer.allocate((void *)selectionVertices.data(), selectionVertices.size() * sizeof(QVector3D));
}


void
ImageWindow::initShaders() {
  // Compile vertex shader
  if(!imageProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "/home/gabriele/qtprojects/QtNeuTomo/imageShader.vert"))
    close();
  // Compile fragment shader
  if(!imageProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "/home/gabriele/qtprojects/QtNeuTomo/imageShader.frag"))
    close();
  // Link shader pipeline
  if(!imageProgram.link())
    close();
  if(!selectionProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "/home/gabriele/qtprojects/QtNeuTomo/imageShader.vert"))
    close();
  if(!selectionProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "/home/gabriele/qtprojects/QtNeuTomo/selectionshader.frag"))
    close();
  if(!selectionProgram.link())
    close();
  // Bind shader pipeline for use
  if(!imageProgram.bind())
    close();
  // Bind shader pipeline for use
  if(!selectionProgram.bind())
    close();
}


void
ImageWindow::resizeGL(int width, int height) {
  myWidth  = width;
  myHeight = height;
}


void
ImageWindow::paintGL() {
  // Draw the scene:
  glClear(GL_COLOR_BUFFER_BIT);

  viewMatrix.setToIdentity();
  modelMatrix.setToIdentity();
  projectionMatrix.setToIdentity();
  mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

  if(glIsTexture(texture)) {
    glUseProgram(imageProgram.programId());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, texture);

    imageProgram.setUniformValue("mvp_Matrix",   mvpMatrix);

    // Tell OpenGL programmable pipeline how to locate vertex position data
    vertexbuffer.bind();
    int vertexLocation = imageProgram.attributeLocation("vertex_pos");
    imageProgram.enableAttributeArray(vertexLocation);
    imageProgram.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));
    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    uvbuffer.bind();
    int texcoordLocation = imageProgram.attributeLocation("vertex_tex");
    imageProgram.enableAttributeArray(texcoordLocation);
    imageProgram.setAttributeBuffer(texcoordLocation, GL_FLOAT, 0, 2, sizeof(QVector2D));

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
  }
  if(selectionEnabled) {
    glUseProgram(selectionProgram.programId());
    // Set modelview-projection matrix
    modelMatrix.setToIdentity();
    mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
    selectionProgram.setUniformValue("mvp_Matrix",   mvpMatrix);
    selectionBuffer.bind();
    int vertexLocation = selectionProgram.attributeLocation("vertex_pos");
    selectionProgram.enableAttributeArray(vertexLocation);
    selectionProgram.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));
    glDrawArrays(GL_LINES, 0, selectionVertices.size());
  }
  glFlush();
}


bool
ImageWindow::checkGLErrors(QString sModule) {
  GLenum error;
  if((error = glGetError()) == GL_NO_ERROR) return true;
  qDebug() << tr("Error %1 in ").arg(error) + sModule;
  while((error = glGetError()) != GL_NO_ERROR) {
    qDebug() << error + tr("\r\n");
  }
  return false;
}


bool
ImageWindow::loadTexture(float* pImage, int Nx, int Ny) {
  glBindTexture(GL_TEXTURE_RECTANGLE, 0);
  if(glIsTexture(texture))
    glDeleteTextures(1, &texture);
  // Create the Texture: The generated textures
  // have no dimensionality; they assume the dimensionality
  // of the texture target to which they are first bound
  glGenTextures(1, &texture);
  // The ARB_texture_rectangle extension supports 2D
  // textures without requiring power-of-two dimensions.
  // Warning: Use [0..W]x[0..H] range for texture coordinates,
  // instead of normalized coordinates range [0..1]x[0..1].
  glBindTexture(GL_TEXTURE_RECTANGLE, texture);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S,     GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T,     GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texWidth  = Nx;
  texHeight = Ny;
  glBindTexture(GL_TEXTURE_RECTANGLE, texture);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, Nx, Ny, 0, GL_RED, GL_FLOAT, pImage);
  glBindTexture(GL_TEXTURE_RECTANGLE, 0);

  if(texWidth>=texHeight) {// Landscape type image
    scaleY = double(texHeight)/double(texWidth);
  } else {// Portrait type image
    scaleX = double(texWidth)/double(texHeight);
  }

  textureVertices.clear();
  textureVertices.append(QVector2D(0.0, 0.0));
  textureVertices.append(QVector2D(Nx,  0.0));
  textureVertices.append(QVector2D(0.0,  Ny));
  textureVertices.append(QVector2D(0.0,  Ny));
  textureVertices.append(QVector2D(Nx,   Ny));
  textureVertices.append(QVector2D(Nx,  0.0));
  uvbuffer.destroy();
  uvbuffer.create();
  uvbuffer.bind();
  uvbuffer.allocate((void *)textureVertices.data(), textureVertices.size() * sizeof(QVector2D));

  selectionBuffer.create();
  bool bError = checkGLErrors(tr("LoadTextures"));
  return bError;
}


bool
ImageWindow::initTexture() {
  glDeleteTextures(1, &texture);
  // Create the Texture: The generated textures
  // have no dimensionality; they assume the dimensionality
  // of the texture target to which they are first bound
  glGenTextures(1, &texture);
  // The ARB_texture_rectangle extension supports 2D
  // textures without requiring power-of-two dimensions.
  // Warning: Use [0..W]x[0..H] range for texture coordinates,
  // instead of normalized coordinates range [0..1]x[0..1].
  glBindTexture(GL_TEXTURE_RECTANGLE, texture);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S,     GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T,     GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_RECTANGLE, 0);
  return checkGLErrors(tr("LoadTextures"));
}


void
ImageWindow::enableSelection(bool bEnable) {
  selectionEnabled = bEnable;
}


bool
ImageWindow::isSelectionEnabled() {
  return selectionEnabled;
}


void
ImageWindow::mousePressEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::RightButton) {
    event->accept();
  } else if (event->buttons() & Qt::LeftButton) {
    QPointF p = event->pos();
    emit startPosEvent(QPoint(p.x()*texWidth/myWidth, (myHeight-p.y())*texHeight/myHeight));
    p.setY(float(myHeight) - p.y());
    startPos = 2.0*QPointF(p.x()/float(myWidth), p.y()/float(myHeight)) - QPointF(1.0, 1.0);
//    qDebug() << startPos.x() << startPos.y();
    event->accept();
  }
}


void
ImageWindow::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() & Qt::RightButton) {
    event->accept();
  } else if (event->button() & Qt::LeftButton) {
    event->accept();
  }
}


void
ImageWindow::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    QPointF p = event->pos();
    emit endPosEvent(QPoint(p.x()*texWidth/myWidth, (myHeight-p.y())*texHeight/myHeight));
    p.setY(float(myHeight) - p.y());
    endPos = 2.0*(QPointF(p.x()/float(myWidth), p.y()/float(myHeight))) - QPointF(1.0, 1.0);
    selectionVertices.clear();
    selectionVertices.append(QVector3D(QVector2D(startPos), 0.0));
    selectionVertices.append(QVector3D(startPos.x(), endPos.y(), 0.0));
    selectionVertices.append(QVector3D(startPos.x(), endPos.y(), 0.0));
    selectionVertices.append(QVector3D(QVector2D(endPos), 0.0));
    selectionVertices.append(QVector3D(QVector2D(endPos), 0.0));
    selectionVertices.append(QVector3D(endPos.x(), startPos.y(), 0.0));
    selectionVertices.append(QVector3D(endPos.x(), startPos.y(), 0.0));
    selectionVertices.append(QVector3D(startPos.x(), startPos.y(), 0.0));
    selectionBuffer.bind();
    selectionBuffer.allocate((void *)selectionVertices.data(), selectionVertices.size() * sizeof(QVector3D));
    update();
    event->accept();
  } else if (event->buttons() & Qt::RightButton) {
    event->accept();
//    emit windowUpdated();
  }
}


void
ImageWindow::wheelEvent(QWheelEvent* event) {
  QPoint numDegrees = event->angleDelta() / 120;
  if (!numDegrees.isNull()) {
    event->accept();
//    emit windowUpdated();
  }
}
