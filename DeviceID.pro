#-------------------------------------------------
#
# Project created by QtCreator 2016-07-05T13:35:33
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DeviceID
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    displayitem.cpp \
    UserUartLink.cpp \
    firstinputdialog.cpp

HEADERS  += mainwindow.h \
    displayitem.h \
    UserUartLink.h \
    firstinputdialog.h

FORMS    += mainwindow.ui \
    displayitem.ui \
    firstinputdialog.ui

RESOURCES += \
    resorce.qrc
