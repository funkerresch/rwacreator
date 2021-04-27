DEPENDPATH += $$PWD/src/
INCLUDEPATH += $$PWD/src/
MOC_DIR = tmp
OBJECTS_DIR = obj

# Input
HEADERS += \
           $$PWD/src/geometry.h \
           $$PWD/src/imagemanager.h \
           $$PWD/src/layer.h \
           $$PWD/src/layermanager.h \
           $$PWD/src/mapadapter.h \
           $$PWD/src/mapcontrol.h \
           $$PWD/src/mapnetwork.h \
           $$PWD/src/point.h \
           $$PWD/src/tilemapadapter.h \
           $$PWD/src/circlepoint.h \
           $$PWD/src/gps_position.h \
           $$PWD/src/osmmapadapter.h \
           $$PWD/src/maplayer.h \
           $$PWD/src/geometrylayer.h \
           $$PWD/src/emptymapadapter.h \
            $$PWD/src/rectpoint.h \
    $$PWD/src/polygonpoint.h \
    $$PWD/src/rwamapitem.h

SOURCES += \
           $$PWD/src/geometry.cpp \
           $$PWD/src/imagemanager.cpp \
           $$PWD/src/layer.cpp \
           $$PWD/src/layermanager.cpp \
           $$PWD/src/mapadapter.cpp \
           $$PWD/src/mapcontrol.cpp \
           $$PWD/src/mapnetwork.cpp \
           $$PWD/src/point.cpp \
           $$PWD/src/tilemapadapter.cpp \
           $$PWD/src/circlepoint.cpp \
           $$PWD/src/gps_position.cpp \
           $$PWD/src/osmmapadapter.cpp \
           $$PWD/src/maplayer.cpp \
           $$PWD/src/geometrylayer.cpp \
           $$PWD/src/emptymapadapter.cpp \
    $$PWD/src/rectpoint.cpp \
    $$PWD/src/polygonpoint.cpp \
    $$PWD/src/rwamapitem.cpp

QT += network

SUBDIRS += \
    $$PWD/../../qt-qthttp/src/qhttp/qhttp.pro

