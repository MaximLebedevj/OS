QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

unix: {
    LIBS += -L$$PWD/lib -lqwt
    INCLUDEPATH += $$PWD/include
    QMAKE_RPATHDIR += $$PWD/lib
}

win32: {
    INCLUDEPATH += $$PWD/include
    LIBS += -L$$PWD/lib -lqwt
}

TARGET = TempGUI
TEMPLATE = app

SOURCES += src/main.cpp \
           src/mainwindow.cpp

HEADERS += src/mainwindow.h

FORMS +=

DESTDIR = bin

CONFIG += outdir
OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = rcc
UI_DIR = ui

DESTDIR = bin

QMAKE_POST_LINK += mkdir -p bin obj moc rcc ui
win32:QMAKE_POST_LINK += mkdir -p $$DESTDIR && cp $$PWD/lib/qwt.dll $$DESTDIR/
unix:QMAKE_POST_LINK += mkdir -p $$DESTDIR && cp "$$PWD/lib/libqwt.so.6.2.0" "$$DESTDIR/"
