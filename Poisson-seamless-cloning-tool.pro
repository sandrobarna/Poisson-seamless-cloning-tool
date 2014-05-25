#-------------------------------------------------
#
# Project created by QtCreator 2014-05-20T11:11:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Poisson-seamless-cloning-tool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    scribblearea.cpp \
    poissonsolver.cpp

HEADERS  += mainwindow.h \
    scribblearea.h \
    poissonsolver.h \
    GpuPoissonSolver.h

FORMS    += mainwindow.ui

RESOURCES += \
    global.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-Poisson-seamless-cloning-tool-Desktop_Qt_5_2_1_MSVC2012_OpenGL_64bit-Debug/release/ -lgpu_poisson_solver
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-Poisson-seamless-cloning-tool-Desktop_Qt_5_2_1_MSVC2012_OpenGL_64bit-Debug/debug/ -lgpu_poisson_solver

INCLUDEPATH += $$PWD/../build-Poisson-seamless-cloning-tool-Desktop_Qt_5_2_1_MSVC2012_OpenGL_64bit-Debug/debug
DEPENDPATH += $$PWD/../build-Poisson-seamless-cloning-tool-Desktop_Qt_5_2_1_MSVC2012_OpenGL_64bit-Debug/debug
