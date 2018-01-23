#-------------------------------------------------
#
# Project created by QtCreator 2018-01-04T19:26:13
#
#-------------------------------------------------

QT += core gui websockets widgets

TARGET = StratuxAHRS
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
           QT_AUTO_SCREEN_SCALE_FACTOR

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    StreamReader.cpp \
    AHRSCanvas.cpp \
    AHRSMainWin.cpp \
    BugSelector.cpp \
    Keypad.cpp \
    PressButton.cpp \
    TrafficMath.cpp \
    Canvas.cpp

HEADERS += \
    StratuxStreams.h \
    StreamReader.h \
    AHRSCanvas.h \
    AHRSMainWin.h \
    BugSelector.h \
    Keypad.h \
    PressButton.h \
    TrafficMath.h \
    Canvas.h \
    AppDefs.h

FORMS += \
    AHRSMainWin.ui \
    BugSelector.ui \
    Keypad.ui

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    ahrsresources.qrc

DISTFILES += \
    AndroidManifest.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD
