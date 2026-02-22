cmake_minimum_required(VERSION 3.5)

project(rwacreator
    VERSION 0.1
    LANGUAGES C CXX
)

# ------------------------------------------------------------------------------
# Qt
# ------------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Bluetooth
    Network
    Multimedia
    Concurrent
    SerialPort
)

# ------------------------------------------------------------------------------
# Global compile settings
# ------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(
    PUREDATA
    PD
    USINGQT
)

add_compile_options(
    -Wno-unused-variable
    -Wno-unused-but-set-variable
)

# ------------------------------------------------------------------------------
# macOS specific settings
# ------------------------------------------------------------------------------

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

# ------------------------------------------------------------------------------
# Subprojects (libpd, portaudio, taglib)
# ------------------------------------------------------------------------------

add_subdirectory(libpd)
add_subdirectory(portaudio)
add_subdirectory(taglib)


# ------------------------------------------------------------------------------
# Explicit sources and headers from .pro file
# ------------------------------------------------------------------------------

set(RWACREATOR_HEADERS
    bluetooth1/bluetoothbaseclass.h
    bluetooth1/connectionhandler.h
    bluetooth1/devicefinder.h
    bluetooth1/devicehandler.h
    bluetooth1/deviceinfo1.h
    libogg/include/ogg/config_types.h.in
    libogg/include/ogg/ogg.h
    libogg/include/ogg/os_types.h
    lo/lo.h
    rwaasset1.h
    rwaheadtrackerconnect.h
    rwahistory.h
    rwainputdialog.h
    rwalocation1.h
    rwapdextra~.h
    rwaruntime.h
    rwasearchdialog.h
    rwatoolbar.h
    rwaview.h
    rwascene.h
    rwastate.h
    rwabackend.h
    rwamapview.h
    rwasearchbox.h
    rwasuggestplaces.h
    rwaassetlist.h
    rwautilities.h
    rwaexport.h
    rwaimport.h
    rwaaudiooutput.h
    pawrapper.h
    rwaattribute.h
    rwaentity.h
    rwasimulator.h
    rwacreator.h
    rwastyles.h
    rwagraphicsview.h
    ofq/libofqf/qoscclient.h
    ofq/libofqf/qoscserver.h
    ofq/libofqf/qosctypes.h
    rwalistview.h
    rwastatelist.h
    rwasceneview.h
    rwaattributeview.h
    rwastateattributeview.h
    rwaassetattributeview.h
    rwalogview.h
    rwagameview.h
    rwascenelist.h
    rwasceneattributeview.h
    rwaarea.h
    bluetooth/characteristicinfo.h
    bluetooth/device.h
    bluetooth/deviceinfo.h
    bluetooth/serviceinfo.h
    rwastateview.h
    clipper/clipper.hpp
    vas_library/examples/PureData/m_pd.h
    vas_library/examples/PureData/rwa_binauralsimple~.h
    vas_library/source/vas_dynamicFirChannel.h
    vas_library/source/vas_fir.h
    vas_library/source/vas_fir_binaural.h
    vas_library/source/vas_fir_list.h
    vas_library/source/vas_fir_read.h
    vas_library/source/vas_firobject.h
    vas_library/source/vas_gps_util.h
    vas_library/source/vas_mem.h
    vas_library/source/vas_pdmaxobject.h
    vas_library/source/vas_util.h
    vorbis/include/vorbis/codec.h
    vorbis/include/vorbis/vorbisenc.h
    vorbis/include/vorbis/vorbisfile.h
    vorbis/lib/backends.h
    vorbis/lib/bitrate.h
    vorbis/lib/books/coupled/res_books_51.h
    vorbis/lib/books/coupled/res_books_stereo.h
    vorbis/lib/books/floor/floor_books.h
    vorbis/lib/books/uncoupled/res_books_uncoupled.h
    vorbis/lib/codebook.h
    vorbis/lib/codec_internal.h
    vorbis/lib/envelope.h
    vorbis/lib/highlevel.h
    vorbis/lib/lookup.h
    vorbis/lib/lookup_data.h
    vorbis/lib/lpc.h
    vorbis/lib/lsp.h
    vorbis/lib/masking.h
    vorbis/lib/mdct.h
    vorbis/lib/misc.h
    vorbis/lib/modes/floor_all.h
    vorbis/lib/modes/psych_11.h
    vorbis/lib/modes/psych_16.h
    vorbis/lib/modes/psych_44.h
    vorbis/lib/modes/psych_8.h
    vorbis/lib/modes/residue_16.h
    vorbis/lib/modes/residue_44.h
    vorbis/lib/modes/residue_44p51.h
    vorbis/lib/modes/residue_44u.h
    vorbis/lib/modes/residue_8.h
    vorbis/lib/modes/setup_11.h
    vorbis/lib/modes/setup_16.h
    vorbis/lib/modes/setup_22.h
    vorbis/lib/modes/setup_32.h
    vorbis/lib/modes/setup_44.h
    vorbis/lib/modes/setup_44p51.h
    vorbis/lib/modes/setup_44u.h
    vorbis/lib/modes/setup_8.h
    vorbis/lib/modes/setup_X.h
    vorbis/lib/os.h
    vorbis/lib/psy.h
    vorbis/lib/registry.h
    vorbis/lib/scales.h
    vorbis/lib/smallft.h
    vorbis/lib/window.h
)

set(RWACREATOR_SOURCES
    main.cpp
    bluetooth1/bluetoothbaseclass.cpp
    bluetooth1/connectionhandler.cpp
    bluetooth1/devicefinder.cpp
    bluetooth1/devicehandler.cpp
    bluetooth1/deviceinfo1.cpp
    libogg/src/bitwise.c
    libogg/src/framing.c
    oggread~.c
    pd-extra/pd/externals/freeverb~/freeverb~.c
    rwaasset1.cpp
    rwaheadtrackerconnect.cpp
    rwahistory.cpp
    rwainputdialog.cpp
    rwalocation1.cpp
    rwaruntime.cpp
    rwasearchdialog.cpp
    rwatoolbar.cpp
    rwaview.cpp
    rwascene.cpp
    rwastate.cpp
    rwabackend.cpp
    rwamapview.cpp
    rwasearchbox.cpp
    rwasuggestplaces.cpp
    rwaassetlist.cpp
    rwautilities.cpp
    rwaexport.cpp
    rwaimport.cpp
    rwaaudiooutput.cpp
    pawrapper.cpp
    rwaattribute.cpp
    rwaentity.cpp
    rwasimulator.cpp
    rwacreator.cpp
    rwagraphicsview.cpp
    ofq/libofqf/qoscclient.cpp
    ofq/libofqf/qoscserver.cpp
    ofq/libofqf/qosctypes.cpp
    rwalistview.cpp
    rwastatelist.cpp
    rwasceneview.cpp
    rwaattributeview.cpp
    rwastateattributeview.cpp
    rwaassetattributeview.cpp
    rwalogview.cpp
    rwagameview.cpp
    rwascenelist.cpp
    rwasceneattributeview.cpp
    rwaarea.cpp
    bluetooth/characteristicinfo.cpp
    bluetooth/device.cpp
    bluetooth/deviceinfo.cpp
    bluetooth/serviceinfo.cpp
    rwastateview.cpp
    clipper/clipper.cpp
    vas_library/examples/PureData/rwa_binauralsimple~.c
    vas_library/source/vas_dynamicFirChannel.c
    vas_library/source/vas_fir.c
    vas_library/source/vas_fir_binaural.c
    vas_library/source/vas_fir_list.c
    vas_library/source/vas_fir_read.c
    vas_library/source/vas_firobject.c
    vas_library/source/vas_gps_util.c
    vas_library/source/vas_mem.c
    vas_library/source/vas_pdmaxobject.c
    vas_library/source/vas_util.c
    vorbis/lib/analysis.c
    vorbis/lib/bitrate.c
    vorbis/lib/block.c
    vorbis/lib/codebook.c
    vorbis/lib/envelope.c
    vorbis/lib/floor0.c
    vorbis/lib/floor1.c
    vorbis/lib/info.c
    vorbis/lib/lookup.c
    vorbis/lib/lpc.c
    vorbis/lib/lsp.c
    vorbis/lib/mapping0.c
    vorbis/lib/mdct.c
    vorbis/lib/misc.c
    vorbis/lib/psy.c
    vorbis/lib/registry.c
    vorbis/lib/res0.c
    vorbis/lib/sharedbook.c
    vorbis/lib/smallft.c
    vorbis/lib/synthesis.c
    vorbis/lib/vorbisenc.c
    vorbis/lib/vorbisfile.c
    vorbis/lib/window.c
    qmapcontrol/QMapControl/src/geometry.cpp 
    qmapcontrol/QMapControl/src/imagemanager.cpp 
    qmapcontrol/QMapControl/src/layer.cpp 
    qmapcontrol/QMapControl/src/layermanager.cpp 
    qmapcontrol/QMapControl/src/mapadapter.cpp 
    qmapcontrol/QMapControl/src/mapcontrol.cpp 
    qmapcontrol/QMapControl/src/mapnetwork.cpp 
    qmapcontrol/QMapControl/src/point.cpp 
    qmapcontrol/QMapControl/src/tilemapadapter.cpp 
    qmapcontrol/QMapControl/src/circlepoint.cpp 
    qmapcontrol/QMapControl/src/gps_position.cpp 
    qmapcontrol/QMapControl/src/osmmapadapter.cpp 
    qmapcontrol/QMapControl/src/maplayer.cpp 
    qmapcontrol/QMapControl/src/geometrylayer.cpp 
    qmapcontrol/QMapControl/src/emptymapadapter.cpp 
    qmapcontrol/QMapControl/src/rectpoint.cpp 
    qmapcontrol/QMapControl/src/polygonpoint.cpp 
    qmapcontrol/QMapControl/src/rwamapitem.cpp
)

# ------------------------------------------------------------------------------
# Executable
# ------------------------------------------------------------------------------

add_executable(rwacreator MACOSX_BUNDLE
    ${RWACREATOR_SOURCES}
    ${RWACREATOR_HEADERS}
)

set_target_properties(rwacreator PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST
        ${CMAKE_SOURCE_DIR}/Info.qmake.macos.plist
)

set_target_properties(rwacreator PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.local.rwacreator
    MACOSX_BUNDLE_BUNDLE_NAME rwacreator
)

# ------------------------------------------------------------------------------
# Include paths
# ------------------------------------------------------------------------------

target_include_directories(rwacreator PRIVATE
    /usr/local/include
    ${CMAKE_SOURCE_DIR}/vas_library/source
    ${CMAKE_SOURCE_DIR}/vas_library/examples/PureData
    ${CMAKE_SOURCE_DIR}/libpd/libpd_wrapper
    ${CMAKE_SOURCE_DIR}/libpd/pure-data/src
    ${CMAKE_SOURCE_DIR}/portaudio/include
    ${CMAKE_SOURCE_DIR}/speech_tools/include
    ${CMAKE_SOURCE_DIR}/ofq/libofqf
    ${CMAKE_SOURCE_DIR}/libogg/include
    ${CMAKE_SOURCE_DIR}/vorbis/include
    ${CMAKE_SOURCE_DIR}/vorbis/lib   
    ${CMAKE_SOURCE_DIR}/qmapcontrol/QMapControl/src
    ${CMAKE_SOURCE_DIR}/cpp-httplib
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/taglib
    ${CMAKE_SOURCE_DIR}/taglib/taglib
    ${CMAKE_SOURCE_DIR}/taglib/taglib/ogg
    ${CMAKE_SOURCE_DIR}/taglib/taglib/toolkit
    ${CMAKE_SOURCE_DIR}/taglib/3rdparty
)

# ------------------------------------------------------------------------------
# Qt linkage
# ------------------------------------------------------------------------------

target_link_libraries(rwacreator PRIVATE
    Qt6::Widgets
    Qt6::Bluetooth
    Qt6::Network
    Qt6::Multimedia
    Qt6::Concurrent
    Qt6::SerialPort
)

# ------------------------------------------------------------------------------
# System libraries
# ------------------------------------------------------------------------------

target_link_libraries(rwacreator PRIVATE
    termcap
    curses
    ncurses
    z
)

# ------------------------------------------------------------------------------
# macOS Frameworks
# ------------------------------------------------------------------------------

if(APPLE)
    target_link_libraries(rwacreator PRIVATE
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreAudio"
        "-framework CoreServices"
        "-framework Accelerate"
        "-framework Carbon"
    )

    set_target_properties(rwacreator PROPERTIES
        INSTALL_RPATH "@executable_path/../Frameworks"
        BUILD_RPATH "@executable_path/../Frameworks"
    )
endif()

# ------------------------------------------------------------------------------
# Link libpd & portaudio
# ------------------------------------------------------------------------------

target_link_libraries(rwacreator PRIVATE
    libpd
    portaudio
    tag
)

# ------------------------------------------------------------------------------
# Bundle resources (PureData files, images)
# ------------------------------------------------------------------------------

set(PD_FILES
    puredata/fabian_dir256.txt
    puredata/rwaloopplayermono.pd
    puredata/rwaloopplayerstereo.pd
    puredata/rwaloopplayerstereoogg.pd
    puredata/rwaloopplayermonoogg.pd
    puredata/rwaplayer5_1channelbinaural_fabian.pd
    puredata/rwaplayer7channelbinaural_fabian.pd
    puredata/rwaplayermonobinaural_fabian.pd
    puredata/rwaplayermonobinauralogg_fabian.pd
    puredata/rwaplayerstereobinauralogg_fabian.pd
    puredata/rwaplayermonobrir1.pd
    puredata/rwaplayerstereobinaural_fabian.pd
    puredata/stereoout.pd
)

foreach(file ${PD_FILES})
    set_source_files_properties(${file} PROPERTIES
        MACOSX_PACKAGE_LOCATION MacOS
    )
endforeach()

set_source_files_properties(images PROPERTIES
    MACOSX_PACKAGE_LOCATION MacOS
)

target_sources(rwacreator PRIVATE
    ${PD_FILES}
    images
)

# ------------------------------------------------------------------------------
# Install
# ------------------------------------------------------------------------------

install(TARGETS rwacreator
    BUNDLE DESTINATION .
)
