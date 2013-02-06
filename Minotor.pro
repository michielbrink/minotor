#-------------------------------------------------
#
# Project created by QtCreator 2012-12-17T15:39:20
#
#-------------------------------------------------

QT       += core gui

TARGET = Minotor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ledmatrix.cpp \
    midi.cpp \
    minotor.cpp \
    minoanimation.cpp \
    configdialog.cpp \
    midimapping.cpp \
    midicontrol.cpp \
    uidial.cpp \
    uianimation.cpp \
    minomaster.cpp \
    uimaster.cpp \
    minoclocksource.cpp \
    uichannelview.cpp \
    uicue.cpp \
    minoanimationfactory.cpp \
    uianimationdescription.cpp \
    uianimationpicker.cpp \
    minabarsfromsides.cpp \
    minaexpandingobjects.cpp \
    minaplasma.cpp \
    minarandompixels.cpp \
    minawaveform.cpp \
    uianimationproperty.cpp \
    minoproperty.cpp \
    minoitemizedproperty.cpp \
    minoprogram.cpp \
    uiprogrameditor.cpp \
    mipromatrix.cpp \
    minaflash.cpp

HEADERS  += mainwindow.h \
    ledmatrix.h \
    midi.h \
    minotor.h \
    minoanimation.h \
    configdialog.h \
    midimapping.h \
    midicontrol.h \
    uidial.h \
    uianimation.h \
    minomaster.h \
    uimaster.h \
    minoclocksource.h \
    uicue.h \
    minoanimationfactory.h \
    uianimationdescription.h \
    uianimationpicker.h \
    minabarsfromsides.h \
    minaexpandingobjects.h \
    minaplasma.h \
    minarandompixels.h \
    minawaveform.h \
    uianimationproperty.h \
    minoproperty.h \
    minoitemizedproperty.h \
    minoprogram.h \
    uiprogrameditor.h \
    uiprogramview.h \
    mipromatrix.h \
    minaflash.h

FORMS    += mainwindow.ui \
    configdialog.ui \
    mainwindowng.ui

include(libraries/qextserialport/src/qextserialport.pri)

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += rtmidi
unix: CONFIG += extserialport

RESOURCES += \
    minotor.qrc
