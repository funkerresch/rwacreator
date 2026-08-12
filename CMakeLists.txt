cmake_minimum_required(VERSION 3.5)

project(rwacreator
    VERSION 1.4.5
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
    Svg
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
    QT_MESSAGELOGCONTEXT
)

add_compile_options(
    -Wno-unused-variable
    -Wno-unused-but-set-variable
)

# ------------------------------------------------------------------------------
# macOS specific settings
# ------------------------------------------------------------------------------

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

# ------------------------------------------------------------------------------
# Subprojects (libpd, portaudio, taglib)
# ------------------------------------------------------------------------------

# We never run the vendored libraries' own test suites, and taglib's break the
# build: it links them against a Homebrew CppUnit, which is arm64-only, so the
# x86_64 half of our universal build fails to link.
set(BUILD_TESTING OFF CACHE BOOL "Build the test suites of vendored libraries" FORCE)

add_subdirectory(libpd)
add_subdirectory(portaudio)
add_subdirectory(taglib)


# ------------------------------------------------------------------------------
# Explicit sources and headers from .pro file
# ------------------------------------------------------------------------------

set(RWACREATOR_HEADERS
    bluetooth/bluetoothbaseclass.h
    bluetooth/devicefinder.h
    bluetooth/devicehandler.h
    bluetooth/deviceinfo.h
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
    rwaapplication.h
    rwacreator.h
    rwastyles.h
    rwagraphicsview.h
    ofq/libofqf/qoscclient.h
    ofq/libofqf/qoscserver.h
    ofq/libofqf/qosctypes.h
    rwalistview.h
    rwalistbadgedelegate.h
    rwathemedicon.h
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
    rwastateview.h
    clipper/clipper.hpp
    vas_library/examples/PureData/rwa_binauralsimple~.h
    vas_library/examples/PureData/vas_reverb~.h
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
    bluetooth/bluetoothbaseclass.cpp
    bluetooth/devicefinder.cpp
    bluetooth/devicehandler.cpp
    bluetooth/deviceinfo.cpp
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
    rwalistbadgedelegate.cpp
    rwathemedicon.cpp
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
    rwastateview.cpp
    clipper/clipper.cpp
    vas_library/examples/PureData/rwa_binauralsimple~.c
    vas_library/examples/PureData/vas_reverb~.c
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

# Define variables for Info.plist substitution
set(EXECUTABLE_NAME "rwacreator")
set(BUNDLE_NAME "RWA Creator")
set(BUNDLE_DISPLAY_NAME "RWA Creator")

# the string BUNDLE_IDENTIFIER.plist should not overlap with the
# qt generated settings file-name (com.fhnw.rwacreator.plist, see main.cpp)
set(BUNDLE_IDENTIFIER "com.fhnw.rwa.creator")

set(BUNDLE_ICON_FILE "rwa-creator.icns")

# Document icon for .rwa files (see UTExportedTypeDeclarations /
# CFBundleDocumentTypes in Info.plist.in)
set(BUNDLE_DOCUMENT_ICON_FILE "rwa-document.icns")

# Get git commit hash
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Configure Info.plist from template
configure_file(
    ${CMAKE_SOURCE_DIR}/Info.plist.in
    ${CMAKE_BINARY_DIR}/Info.plist
    @ONLY
)

# qt_add_executable (NOT plain add_executable) so Qt's finalization links and
# imports the STATIC plugins this app needs, in particular the macOS
# permission plugin libqdarwinbluetoothpermission.a, without which
# QBluetoothPermission can't be queried and Bluetooth always reads as Denied.
# MANUAL_FINALIZATION: this target is configured across many later commands, so
# we finalize explicitly at the end of this file.
qt_add_executable(rwacreator MANUAL_FINALIZATION MACOSX_BUNDLE
    ${RWACREATOR_SOURCES}
    ${RWACREATOR_HEADERS}
)

set_target_properties(rwacreator PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/Info.plist
)

# Expose version and git hash to the code (About dialog)
target_compile_definitions(rwacreator PRIVATE
    RWA_VERSION="${PROJECT_VERSION}"
    RWA_GIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
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

# Qt6::Svg must be a direct dependency: macdeployqt only deploys the
# imageformats/libqsvg.dylib plugin (needed for QPixmap("*.svg")) when it
# sees QtSvg among the app's linked frameworks.
target_link_libraries(rwacreator PRIVATE
    Qt6::Widgets
    Qt6::Svg
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
        MACOSX_PACKAGE_LOCATION Resources/puredata
    )
endforeach()


file(GLOB IMAGE_FILES
    images/*.png
    images/*.jpg
    images/*.svg
)

foreach(file ${IMAGE_FILES})
    set_source_files_properties(${file} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources/images
    )
endforeach()

target_sources(rwacreator PRIVATE
    ${IMAGE_FILES}
)

target_sources(rwacreator PRIVATE
    ${PD_FILES}
)

set(APP_ICON ${CMAKE_SOURCE_DIR}/images/${BUNDLE_ICON_FILE})
set(DOCUMENT_ICON ${CMAKE_SOURCE_DIR}/images/${BUNDLE_DOCUMENT_ICON_FILE})
set_source_files_properties(${APP_ICON} ${DOCUMENT_ICON} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)
target_sources(rwacreator PRIVATE ${APP_ICON} ${DOCUMENT_ICON})

# ------------------------------------------------------------------------------
# Install
# ------------------------------------------------------------------------------

install(TARGETS rwacreator
    BUNDLE DESTINATION .
)

# ------------------------------------------------------------------------------
# Finalize the rwacreator Qt target (links/imports static plugins, incl. the
# macOS permission plugin). Must come after all of rwacreator's configuration.
# ------------------------------------------------------------------------------

qt_finalize_target(rwacreator)


# ------------------------------------------------------------------------------
# Deploy Qt frameworks & plugins into the bundle (macdeployqt)
# ------------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Locate macdeployqt next to the qmake of the Qt used for this build.
    get_target_property(_qmake_exe Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_exe}" DIRECTORY)
    # NO_DEFAULT_PATH: never fall back to PATH (which holds a Homebrew Qt here) —
    # macdeployqt must come from the same Qt that find_package() selected.

    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt
        HINTS "${_qt_bin_dir}" NO_DEFAULT_PATH)

    if(MACDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET rwacreator POST_BUILD
            COMMAND "${MACDEPLOYQT_EXECUTABLE}" "$<TARGET_BUNDLE_DIR:rwacreator>"
            MESSAGE(STATUS "MACDEPLOYQT_EXECUTABLE: ${MACDEPLOYQT_EXECUTABLE}")
            COMMENT "Deploying Qt frameworks into bundle (macdeployqt)"
            VERBATIM
        )
    else()
        message(WARNING "macdeployqt not found near ${_qt_bin_dir}; "
                        "Qt frameworks will not be bundled")
    endif()
endif()

# ------------------------------------------------------------------------------
# Rename the bundle wrapper to "RWA Creator.app". The target (and the binary
# at Contents/MacOS/rwacreator) must stay named "rwacreator" — CFBundleExecutable
# in Info.plist.in and macdeployqt above both depend on that — so the rename
# happens as a POST_BUILD step after macdeployqt, not via OUTPUT_NAME.
# ------------------------------------------------------------------------------

add_custom_command(TARGET rwacreator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}.app"
    COMMAND ${CMAKE_COMMAND} -E rename "$<TARGET_BUNDLE_DIR:rwacreator>" "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}.app"
    COMMENT "Renaming bundle to '${BUNDLE_NAME}.app'"
    VERBATIM
)

# ------------------------------------------------------------------------------
# rwatrace — headless engine trace harness (no audio, no GUI).
# Links the engine sources against tools/trace/fake_libpd.cpp instead of the
# real libpd/portaudio; used for golden-trace parity testing against the iOS
# Player. See docs/engine-runtime-investigation.md.
# ------------------------------------------------------------------------------

option(RWA_BUILD_TRACE_TOOLS "Build the headless rwatrace engine harness" ON)

if(RWA_BUILD_TRACE_TOOLS)
    qt_add_executable(rwatrace
        tools/trace/rwatracemain.cpp
        tools/trace/fake_libpd.cpp
        tools/trace/fake_libpd.h
        tools/trace/stub_backend.cpp
        rwaruntime.cpp
        rwaruntime.h
        rwaentity.cpp
        rwascene.cpp
        rwastate.cpp
        rwaasset1.cpp
        rwaarea.cpp
        rwalocation1.cpp
        rwautilities.cpp
        rwaattribute.cpp
        rwaimport.cpp
        clipper/clipper.cpp
    )

    target_include_directories(rwatrace PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/tools/trace
        ${CMAKE_SOURCE_DIR}/vas_library/source
        ${CMAKE_SOURCE_DIR}/vas_library/examples/PureData
        ${CMAKE_SOURCE_DIR}/libpd/libpd_wrapper
        ${CMAKE_SOURCE_DIR}/libpd/pure-data/src
        ${CMAKE_SOURCE_DIR}/portaudio/include
        ${CMAKE_SOURCE_DIR}/ofq/libofqf
        ${CMAKE_SOURCE_DIR}/cpp-httplib
        ${CMAKE_SOURCE_DIR}/taglib
        ${CMAKE_SOURCE_DIR}/taglib/taglib
        ${CMAKE_SOURCE_DIR}/taglib/taglib/ogg
        ${CMAKE_SOURCE_DIR}/taglib/taglib/toolkit
        ${CMAKE_SOURCE_DIR}/taglib/3rdparty
    )

    # Widgets/Bluetooth/SerialPort are header-only needs (rwautilities.h,
    # rwabackend.h include chain); the harness only instantiates QCoreApplication.
    target_link_libraries(rwatrace PRIVATE
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
        Qt6::Network
        Qt6::Bluetooth
        Qt6::Multimedia
        Qt6::SerialPort
        tag
        z
    )
endif()
