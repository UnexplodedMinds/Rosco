#-------------------------------------------------
#
# Project created by QtCreator 2018-01-04T19:26:13
#
#-------------------------------------------------

QT += core gui websockets widgets network

android {
    QT += androidextras
}

TARGET = Rosco
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS \
           QT_AUTO_SCREEN_SCALE_FACTOR

INCLUDEPATH += ./include

VPATH += ./include \
         ./ui

SOURCES += \
    main.cpp \
    StreamReader.cpp \
    AHRSCanvas.cpp \
    AHRSMainWin.cpp \
    BugSelector.cpp \
    Keypad.cpp \
    TrafficMath.cpp \
    Canvas.cpp \
    MenuDialog.cpp

HEADERS += \
    StratuxStreams.h \
    StreamReader.h \
    AHRSCanvas.h \
    AHRSMainWin.h \
    BugSelector.h \
    Keypad.h \
    TrafficMath.h \
    Canvas.h \
    AppDefs.h \
    MenuDialog.h

FORMS += \
    AHRSMainWin.ui \
    BugSelector.ui \
    Keypad.ui \
    MenuDialog.ui

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    AHRSResources.qrc

DISTFILES += \
    AndroidManifest.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD
