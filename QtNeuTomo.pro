#-------------------------------------------------
#
# Project created by QtCreator 2016-10-14T07:47:09
#
#-------------------------------------------------

QT       += core
QT       += gui
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
    dotomodlg.cpp \
    ParallelTomoThread.cpp

HEADERS  += mainwindow.h \
    preprocessdlg.h \
    Projection.h \
    imagewindow.h \
    chooseroidlg.h \
    dotomodlg.h \
    ParallelTomoThread.h

FORMS    +=

LIBS     += -L/home/rov/cfitsio-3.47/lib -lcfitsio
INCLUDEPATH += /home/rov/cfitsio-3.47/include

DISTFILES += \
    License.txt \
    README.md \
    imageShader.vert \
    imageShader.frag \
    selectionshader.frag
