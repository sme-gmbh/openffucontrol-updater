QT -= gui
QT += serialport testlib

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TEMPLATE = app

OBJECTS_DIR = .obj/
MOC_DIR = .moc/
UI_DIR = .ui/
RCC_DIR = .rcc/

# make install / make uninstall target
unix {
    target.path = /usr/bin/
    INSTALLS += target
}


SOURCES += \
        intelhexparser.cpp \
        main.cpp \
        maincontroller.cpp \
        modbus.cpp \
        modbustelegram.cpp \
        openffucontrolocuhandler.cpp

//LIBS     += -lmodbus

HEADERS += \
    intelhexparser.h \
    maincontroller.h \
    modbus.h \
    modbustelegram.h \
    openffucontrolocuhandler.h


linux-g++: QMAKE_TARGET.arch = $$QMAKE_HOST.arch
linux-g++-32: QMAKE_TARGET.arch = x86
linux-g++-64: QMAKE_TARGET.arch = x86_64
linux-cross: QMAKE_TARGET.arch = x86
win32-cross-32: QMAKE_TARGET.arch = x86
win32-cross: QMAKE_TARGET.arch = x86_64
win32-g++: QMAKE_TARGET.arch = $$QMAKE_HOST.arch
win32-msvc*: QMAKE_TARGET.arch = $$QMAKE_HOST.arch
linux-raspi: QMAKE_TARGET.arch = armv6l
linux-armv6l: QMAKE_TARGET.arch = armv6l
linux-armv7l: QMAKE_TARGET.arch = armv7l
linux-arm*: QMAKE_TARGET.arch = armv6l
linux-aarch64*: QMAKE_TARGET.arch = aarch64

unix {
    equals(QMAKE_TARGET.arch , x86_64): {
        message("Configured for x86_64")
        message("Using libftdi1")
        LIBS +=  -lftdi1
        DEFINES += USE_LIBFTDI1
    }

    equals(QMAKE_TARGET.arch , x86): {
        message("Configured for x86")
        message("Using libftdi1")
        LIBS +=  -lftdi1
        DEFINES += USE_LIBFTDI1
    }

    equals(QMAKE_TARGET.arch , armv6l): {
        message("Configured for armv6l")
        message("Using libftdi")
        LIBS +=  -lftdi
        DEFINES += USE_LIBFTDI
    }

    equals(QMAKE_TARGET.arch , armv7l): {
        message("Configured for armv7l")
        message("Using libftdi")
        LIBS +=  -lftdi
        DEFINES += USE_LIBFTDI
    }
}
