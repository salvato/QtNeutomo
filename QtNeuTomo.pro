#-------------------------------------------------
#
# Project created by QtCreator 2016-10-14T07:47:09
#
#-------------------------------------------------

QT       += core
QT       += gui
QT       += multimedia
QT       += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = QtNeuTomo
TEMPLATE = app
CONFIG 	+= c++11


SOURCES += main.cpp\
    mainwindow.cpp \
    preprocessdlg.cpp \
    Projection.cpp \
    imagewindow.cpp \
    chooseroidlg.cpp \
    dotomodlg.cpp

HEADERS  += mainwindow.h \
    preprocessdlg.h \
    Projection.h \
    imagewindow.h \
    chooseroidlg.h \
    dotomodlg.h

FORMS    +=

LIBS     += -L/home/gabriele/cfitsio/lib -lcfitsio
INCLUDEPATH += /home/gabriele/cfitsio

DISTFILES += \
    License.txt \
    README.md \
    imageShader.vert \
    imageShader.frag \
    selectionshader.frag
