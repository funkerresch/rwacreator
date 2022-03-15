TEMPLATE = app
QT += widgets network multimedia concurrent serialport bluetooth
DEFINES += "PUREDATA"
DEFINES += "PD"

extralib.target = extra
extralib.commands = echo "Precompiling libpd and portaudio.."; \
                            $$PWD/makeportaudioandlibpd.sh
extralib.depends =

QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = extra

QMAKE_CC=clang
QMAKE_CXX=clang++
CONFIG += static
CONFIG += c++11

QMAKE_RPATHDIR += @executable_path/../Frameworks

INCLUDEPATH += /usr/local/include
INCLUDEPATH += $$PWD/vas_library/source
INCLUDEPATH += $$PWD/vas_library/examples/PureData
INCLUDEPATH += $$PWD/libpd/libpd_wrapper
INCLUDEPATH += $$PWD/libpd/pure-data/src
INCLUDEPATH += portaudio/include
INCLUDEPATH += $$PWD/speech_tools/include
INCLUDEPATH += $$PWD/ofq/libofqf

macx
{
    LIBS += -framework AudioToolbox \
            -framework AudioUnit \
            -framework CoreAudio \
            -framework CoreServices \
            -framework Accelerate \
            -framework Carbon \
}

include($$PWD/qmapcontrol/QMapControl/QMapControl.pri)

HEADERS += \
    lo/lo.h \
    rwaasset1.h \
    rwaheadtrackerconnect.h \
    rwahistory.h \
    rwalocation1.h \
    rwapdextra~.h \
    rwaruntime.h \
    rwasearchdialog.h \
    rwatoolbar.h \
    rwaview.h \
    rwascene.h \
    rwastate.h \
    rwabackend.h \
    rwamapview.h \
    rwasearchbox.h \
    rwasuggestplaces.h \
    rwaassetlist.h \
    rwautilities.h \
    rwaexport.h \
    rwaimport.h \
    rwaaudiooutput.h \
    pawrapper.h \
    rwaattribute.h \
    rwaentity.h \
    rwasimulator.h \
    rwacreator.h \
    rwastyles.h \
    rwagraphicsview.h \
    $$PWD/ofq/libofqf/qoscclient.h \
    $$PWD/ofq/libofqf/qoscserver.h \
    $$PWD/ofq/libofqf/qosctypes.h \
    rwalistview.h \
    rwastatelist.h \
    rwasceneview.h \
    rwaattributeview.h \
    rwastateattributeview.h \
    rwaassetattributeview.h \
    rwalogview.h \
    rwasceneview.h \
    rwagameview.h \
    rwascenelist.h \
    rwasceneattributeview.h \
    rwaarea.h \
    bluetooth/characteristicinfo.h \
    bluetooth/device.h \
    bluetooth/deviceinfo.h \
    bluetooth/serviceinfo.h \
    rwastateview.h \
    clipper/clipper.hpp \
    vas_library/examples/PureData/m_pd.h \
    vas_library/examples/PureData/rwa_binauralsimple~.h \
    vas_library/source/vas_dynamicFirChannel.h \
    vas_library/source/vas_fir.h \
    vas_library/source/vas_fir_binaural.h \
    vas_library/source/vas_fir_list.h \
    vas_library/source/vas_fir_read.h \
    vas_library/source/vas_firobject.h \
    vas_library/source/vas_gps_util.h \
    vas_library/source/vas_mem.h \
    vas_library/source/vas_pdmaxobject.h \
    vas_library/source/vas_util.h



SOURCES += main.cpp \
    pd-extra/pd/externals/freeverb~/freeverb~.c \
    rwaasset1.cpp \
    rwaheadtrackerconnect.cpp \
    rwahistory.cpp \
    rwalocation1.cpp \
    rwaruntime.cpp \
    rwasearchdialog.cpp \
    rwatoolbar.cpp \
    rwaview.cpp \
    rwascene.cpp \
    rwastate.cpp \
    rwabackend.cpp \
    rwamapview.cpp \
    rwasearchbox.cpp \
    rwasuggestplaces.cpp \
    rwaassetlist.cpp \
    rwautilities.cpp \
    rwaexport.cpp \
    rwaimport.cpp \
    rwaaudiooutput.cpp \
    pawrapper.cpp \
    rwaattribute.cpp \
    rwaentity.cpp \
    rwasimulator.cpp \
    rwacreator.cpp \
    rwagraphicsview.cpp \
    $$PWD/ofq/libofqf/qoscclient.cpp \
    $$PWD/ofq/libofqf/qoscserver.cpp \
    $$PWD/ofq/libofqf/qosctypes.cpp \
    rwalistview.cpp \
    rwastatelist.cpp \
    rwasceneview.cpp \
    rwaattributeview.cpp \
    rwastateattributeview.cpp \
    rwaassetattributeview.cpp \
    rwalogview.cpp \
    rwagameview.cpp \
    rwascenelist.cpp \
    rwasceneattributeview.cpp \
    rwaarea.cpp \
    bluetooth/characteristicinfo.cpp \
    bluetooth/device.cpp \
    bluetooth/deviceinfo.cpp \
    bluetooth/serviceinfo.cpp \
    rwastateview.cpp \
    clipper/clipper.cpp \
    vas_library/examples/PureData/rwa_binauralsimple~.c \
    vas_library/source/vas_dynamicFirChannel.c \
    vas_library/source/vas_fir.c \
    vas_library/source/vas_fir_binaural.c \
    vas_library/source/vas_fir_list.c \
    vas_library/source/vas_fir_read.c \
    vas_library/source/vas_firobject.c \
    vas_library/source/vas_gps_util.c \
    vas_library/source/vas_mem.c \
    vas_library/source/vas_pdmaxobject.c \
    vas_library/source/vas_util.c


LIBS += -ltermcap
LIBS += -lcurses
LIBS += -lncurses
LIBS += -lz

installs.files += $$PWD/libpd/libs/libpd.dylib
installs.files += $$PWD/portaudio/lib/.libs/libportaudio.a
installs.path = $$OUT_PWD/rwacreator.app/Contents/MacOS
INSTALLS += installs

copydata.commands = $(COPY_DIR) $$PWD/images $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/rwainit.txt $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/fabian_dir256.txt $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaloopplayermono.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaloopplayerstereo.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaplayer5_1channelbinaural_fabian.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaplayer5_1channelbinaural.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaplayer7channelbinaural_fabian.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaplayermonobinaural_fabian.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaplayermonobrir1.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaplayermonobinaural.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaplayerstereobinaural_fabian.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwaplayerstereobinaural.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/stereoout.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/puredata/rwagetmetadata.pd $$OUT_PWD/rwacreator.app/Contents/MacOS \
&& $(COPY_FILE) $$PWD/portaudio/lib/.libs/libportaudio.2.dylib /usr/local/lib/ \
&& $(COPY_FILE) $$PWD/libpd/libs/libpd.dylib /Users/harveykeitel/libs \

first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)

QMAKE_EXTRA_TARGETS += first copydata

target.path = $$PWD/build
INSTALLS += target


macx: LIBS += -L$$PWD/portaudio/lib/.libs/ -lportaudio
macx: LIBS += -L$$PWD/libpd/libs/ -lpd
#PRE_TARGETDEPS += $$PWD/portaudio/lib/.libs/libportaudio.a
#PRE_TARGETDEPS += $$PWD/libpd/libs/libpd.a







