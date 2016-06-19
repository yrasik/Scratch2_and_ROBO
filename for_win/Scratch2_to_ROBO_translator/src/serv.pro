QT       += core gui

#QT       += core
QT       += network
#QT       -= gui
QT += serialport
QT += widgets


TARGET = serv
#CONFIG   += console
#CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    parser.cpp \
    serial_thread.cpp \
    dialog.cpp \
    robo_tcp_server.cpp \
    tcp_sockets.cpp

HEADERS += \
    parser.h \
    serial_thread.h \
    dialog.h \
    robo_tcp_server.h \
    tcp_sockets.h \
    support.h
