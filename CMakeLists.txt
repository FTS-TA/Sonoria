cmake_minimum_required(VERSION 3.16)

project(Essentia
    VERSION 2.1.0
    DESCRIPTION "Audio analysis library"
    LANGUAGES CXX C
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options (mirroring wscript options)
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_STATIC "Build static library" OFF)
option(BUILD_EXAMPLES "Build example programs" OFF)
option(BUILD_PYTHON "Build Python bindings" OFF)
option(BUILD_VAMP "Build Vamp plugin wrapper" OFF)
option(BUILD_TESTS "Build C++ tests" OFF)
option(STATIC_DEPENDENCIES "Build with static dependencies" OFF)
option(WITH_GAIA "Build with Gaia support" OFF)
option(WITH_TENSORFLOW "Build with TensorFlow support" OFF)
option(LIGHTWEIGHT "Build lightweight version" OFF)
option(NO_MSSE "Disable MSSE flags" OFF)
option(ONLY_PYTHON "Build only Python bindings (requires pre-built libsonoria)" OFF)

# FFT backend selection
set(FFT_BACKEND "FFTW" CACHE STRING "FFT backend to use (FFTW, KISS, ACCELERATE)")
set_property(CACHE FFT_BACKEND PROPERTY STRINGS "FFTW" "KISS" "ACCELERATE")

# Dependency paths (can be set via cmake-gui)
set(EIGEN3_DIR "" CACHE PATH "Path to Eigen3 installation")
set(FFTW_DIR "" CACHE PATH "Path to FFTW installation")
set(FFMPEG_DIR "" CACHE PATH "Path to FFmpeg installation")
set(SAMPLERATE_DIR "" CACHE PATH "Path to libsamplerate installation")
set(TAGLIB_DIR "" CACHE PATH "Path to TagLib installation")
set(YAML_DIR "" CACHE PATH "Path to libyaml installation")
set(CHROMAPRINT_DIR "" CACHE PATH "Path to Chromaprint installation")
set(GAIA_DIR "" CACHE PATH "Path to Gaia2 installation")
set(TENSORFLOW_DIR "" CACHE PATH "Path to TensorFlow installation")

# Cross-compilation options
option(CROSS_COMPILE_MINGW32 "Cross-compile for Windows with MinGW" OFF)
option(CROSS_COMPILE_ANDROID "Cross-compile for Android" OFF)
option(CROSS_COMPILE_IOS "Cross-compile for iOS (ARM)" OFF)
option(CROSS_COMPILE_IOS_SIM "Cross-compile for iOS Simulator" OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug, Release)" FORCE)
endif()

# Platform detection
if(WIN32)
    message(STATUS "Building on Windows")
elseif(APPLE)
    message(STATUS "Building on macOS")
elseif(UNIX)
    message(STATUS "Building on Linux/Unix")
endif()

# Position Independent Code (required for shared libraries and Python bindings)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compiler flags
if(MSVC)
    add_compile_options(/W2 /EHsc)
else()
    add_compile_options(-Wall -pipe)
    
    # SSE flags for x86/x64 platforms
    if(NOT NO_MSSE AND NOT EMSCRIPTEN AND NOT CROSS_COMPILE_ANDROID AND NOT CROSS_COMPILE_IOS)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "(i386|i686|x86|x86_64|AMD64)")
            add_compile_options(-msse -msse2 -mfpmath=sse)
        endif()
    endif()
    
    # pthread for Linux
    if(UNIX AND NOT APPLE)
        find_package(Threads REQUIRED)
    endif()
endif()

# Debug/Release flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building in Debug mode")
    if(NOT MSVC)
        add_compile_options(-g)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building in Release mode")
    if(NOT MSVC)
        add_compile_options(-O2)
    endif()
endif()

# Static library settings
if(BUILD_STATIC OR NOT BUILD_SHARED_LIBS)
    set(BUILD_STATIC TRUE)
    set(BUILD_SHARED_LIBS OFF)
    message(STATUS "Building static library")
else()
    message(STATUS "Building shared library")
endif()

# Set up dependency search paths
if(EIGEN3_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${EIGEN3_DIR}")
endif()
if(FFTW_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${FFTW_DIR}")
endif()
if(FFMPEG_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${FFMPEG_DIR}")
endif()
if(SAMPLERATE_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${SAMPLERATE_DIR}")
endif()
if(TAGLIB_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${TAGLIB_DIR}")
endif()
if(YAML_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${YAML_DIR}")
endif()
if(CHROMAPRINT_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${CHROMAPRINT_DIR}")
endif()
if(GAIA_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${GAIA_DIR}")
endif()
if(TENSORFLOW_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${TENSORFLOW_DIR}")
endif()

# Find dependencies
set(HAVE_FFTW FALSE)
set(HAVE_AVFORMAT FALSE)
set(HAVE_AVCODEC FALSE)
set(HAVE_AVUTIL FALSE)
set(HAVE_SWRESAMPLE FALSE)
set(HAVE_SAMPLERATE FALSE)
set(HAVE_TAGLIB FALSE)
set(HAVE_YAML FALSE)
set(HAVE_CHROMAPRINT FALSE)
set(HAVE_GAIA FALSE)
set(HAVE_TENSORFLOW FALSE)

# Eigen3 (mandatory)
find_package(Eigen3 3.3.4 REQUIRED)
message(STATUS "Found Eigen3")

# FFTW
if(FFT_BACKEND STREQUAL "FFTW")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FFTW fftw3f)
    endif()
    if(NOT FFTW_FOUND)
        find_library(FFTW_LIB NAMES fftw3f libfftw3f-3)
        find_path(FFTW_INCLUDE_DIR NAMES fftw3.h)
        if(FFTW_LIB AND FFTW_INCLUDE_DIR)
            set(FFTW_FOUND TRUE)
            set(FFTW_LIBRARIES ${FFTW_LIB})
            set(FFTW_INCLUDE_DIRS ${FFTW_INCLUDE_DIR})
        endif()
    endif()
    
    if(FFTW_FOUND)
        set(HAVE_FFTW TRUE)
        message(STATUS "Found FFTW")
        include_directories(${FFTW_INCLUDE_DIRS})
    else()
        message(WARNING "FFTW not found - FFT algorithms will be disabled")
    endif()
endif()

# Kiss FFT (alternative FFT backend)
if(FFT_BACKEND STREQUAL "KISS")
    message(STATUS "Using KISS FFT backend")
    set(KISSFFT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/kiss_fft130")
    include_directories(${KISSFFT_DIR} ${KISSFFT_DIR}/tools)
    set(KISSFFT_SOURCES
        ${KISSFFT_DIR}/kiss_fft.c
        ${KISSFFT_DIR}/tools/kiss_fftr.c
    )
endif()

# Accelerate framework (macOS only)
if(FFT_BACKEND STREQUAL "ACCELERATE" AND APPLE)
    message(STATUS "Using Accelerate Framework for FFT")
    find_library(ACCELERATE_FRAMEWORK Accelerate)
    if(ACCELERATE_FRAMEWORK)
        set(HAVE_ACCELERATE TRUE)
    endif()
endif()

# FFmpeg/libav
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(AVCODEC libavcodec)
    pkg_check_modules(AVFORMAT libavformat)
    pkg_check_modules(AVUTIL libavutil)
    pkg_check_modules(SWRESAMPLE libswresample)
endif()

if(NOT AVCODEC_FOUND)
    find_library(AVCODEC_LIB NAMES avcodec)
    find_path(AVCODEC_INCLUDE_DIR NAMES libavcodec/avcodec.h)
    if(AVCODEC_LIB AND AVCODEC_INCLUDE_DIR)
        set(AVCODEC_FOUND TRUE)
        set(AVCODEC_LIBRARIES ${AVCODEC_LIB})
        set(AVCODEC_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIR})
    endif()
endif()

if(NOT AVFORMAT_FOUND)
    find_library(AVFORMAT_LIB NAMES avformat)
    find_path(AVFORMAT_INCLUDE_DIR NAMES libavformat/avformat.h)
    if(AVFORMAT_LIB AND AVFORMAT_INCLUDE_DIR)
        set(AVFORMAT_FOUND TRUE)
        set(AVFORMAT_LIBRARIES ${AVFORMAT_LIB})
        set(AVFORMAT_INCLUDE_DIRS ${AVFORMAT_INCLUDE_DIR})
    endif()
endif()

if(NOT AVUTIL_FOUND)
    find_library(AVUTIL_LIB NAMES avutil)
    find_path(AVUTIL_INCLUDE_DIR NAMES libavutil/avutil.h)
    if(AVUTIL_LIB AND AVUTIL_INCLUDE_DIR)
        set(AVUTIL_FOUND TRUE)
        set(AVUTIL_LIBRARIES ${AVUTIL_LIB})
        set(AVUTIL_INCLUDE_DIRS ${AVUTIL_INCLUDE_DIR})
    endif()
endif()

if(NOT SWRESAMPLE_FOUND)
    find_library(SWRESAMPLE_LIB NAMES swresample)
    find_path(SWRESAMPLE_INCLUDE_DIR NAMES libswresample/swresample.h)
    if(SWRESAMPLE_LIB AND SWRESAMPLE_INCLUDE_DIR)
        set(SWRESAMPLE_FOUND TRUE)
        set(SWRESAMPLE_LIBRARIES ${SWRESAMPLE_LIB})
        set(SWRESAMPLE_INCLUDE_DIRS ${SWRESAMPLE_INCLUDE_DIR})
    endif()
endif()

if(AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND AND SWRESAMPLE_FOUND)
    set(HAVE_AVFORMAT TRUE)
    set(HAVE_AVCODEC TRUE)
    set(HAVE_AVUTIL TRUE)
    set(HAVE_SWRESAMPLE TRUE)
    message(STATUS "Found FFmpeg/libav")
endif()

# libsamplerate
if(PkgConfig_FOUND)
    pkg_check_modules(SAMPLERATE samplerate)
endif()
if(NOT SAMPLERATE_FOUND)
    find_library(SAMPLERATE_LIB NAMES samplerate)
    find_path(SAMPLERATE_INCLUDE_DIR NAMES samplerate.h)
    if(SAMPLERATE_LIB AND SAMPLERATE_INCLUDE_DIR)
        set(SAMPLERATE_FOUND TRUE)
        set(SAMPLERATE_LIBRARIES ${SAMPLERATE_LIB})
        set(SAMPLERATE_INCLUDE_DIRS ${SAMPLERATE_INCLUDE_DIR})
    endif()
endif()
if(SAMPLERATE_FOUND)
    set(HAVE_SAMPLERATE TRUE)
    message(STATUS "Found libsamplerate")
endif()

# TagLib
if(PkgConfig_FOUND)
    pkg_check_modules(TAGLIB taglib)
endif()
if(NOT TAGLIB_FOUND)
    find_package(TagLib 1.9 QUIET)
endif()
if(NOT TAGLIB_FOUND)
    find_library(TAGLIB_LIB NAMES tag)
    find_path(TAGLIB_INCLUDE_DIR NAMES taglib/tag.h)
    if(TAGLIB_LIB AND TAGLIB_INCLUDE_DIR)
        set(TAGLIB_FOUND TRUE)
        set(TAGLIB_LIBRARIES ${TAGLIB_LIB})
        set(TAGLIB_INCLUDE_DIRS ${TAGLIB_INCLUDE_DIR})
    endif()
endif()
if(TAGLIB_FOUND)
    set(HAVE_TAGLIB TRUE)
    message(STATUS "Found TagLib")
endif()

# libyaml
if(PkgConfig_FOUND)
    pkg_check_modules(YAML yaml-0.1)
endif()
if(NOT YAML_FOUND)
    find_library(YAML_LIB NAMES yaml)
    find_path(YAML_INCLUDE_DIR NAMES yaml.h)
    if(YAML_LIB AND YAML_INCLUDE_DIR)
        set(YAML_FOUND TRUE)
        set(YAML_LIBRARIES ${YAML_LIB})
        set(YAML_INCLUDE_DIRS ${YAML_INCLUDE_DIR})
    endif()
endif()
if(YAML_FOUND)
    set(HAVE_YAML TRUE)
    message(STATUS "Found libyaml")
endif()

# Chromaprint
if(PkgConfig_FOUND)
    pkg_check_modules(CHROMAPRINT libchromaprint)
endif()
if(NOT CHROMAPRINT_FOUND)
    find_library(CHROMAPRINT_LIB NAMES chromaprint)
    find_path(CHROMAPRINT_INCLUDE_DIR NAMES chromaprint.h)
    if(CHROMAPRINT_LIB AND CHROMAPRINT_INCLUDE_DIR)
        set(CHROMAPRINT_FOUND TRUE)
        set(CHROMAPRINT_LIBRARIES ${CHROMAPRINT_LIB})
        set(CHROMAPRINT_INCLUDE_DIRS ${CHROMAPRINT_INCLUDE_DIR})
    endif()
endif()
if(CHROMAPRINT_FOUND)
    set(HAVE_CHROMAPRINT TRUE)
    message(STATUS "Found Chromaprint")
endif()

# Gaia2 (optional)
if(WITH_GAIA)
    if(PkgConfig_FOUND)
        pkg_check_modules(GAIA gaia2)
    endif()
    if(NOT GAIA_FOUND)
        find_library(GAIA_LIB NAMES gaia2)
        find_path(GAIA_INCLUDE_DIR NAMES gaia/gaia.h)
        if(GAIA_LIB AND GAIA_INCLUDE_DIR)
            set(GAIA_FOUND TRUE)
            set(GAIA_LIBRARIES ${GAIA_LIB})
            set(GAIA_INCLUDE_DIRS ${GAIA_INCLUDE_DIR})
        endif()
    endif()
    if(GAIA_FOUND)
        set(HAVE_GAIA TRUE)
        message(STATUS "Found Gaia2")
    else()
        message(FATAL_ERROR "Gaia2 requested but not found")
    endif()
endif()

# TensorFlow (optional)
if(WITH_TENSORFLOW)
    if(TENSORFLOW_DIR)
        set(TENSORFLOW_INCLUDE_DIRS "${TENSORFLOW_DIR}/include")
        set(TENSORFLOW_LIBRARIES "${TENSORFLOW_DIR}/lib/libtensorflow_cc.so")
        set(HAVE_TENSORFLOW TRUE)
        message(STATUS "Using TensorFlow from ${TENSORFLOW_DIR}")
    else()
        find_path(TENSORFLOW_INCLUDE_DIR NAMES tensorflow/core/public/session.h)
        if(TENSORFLOW_INCLUDE_DIR)
            set(TENSORFLOW_FOUND TRUE)
            set(HAVE_TENSORFLOW TRUE)
            set(TENSORFLOW_INCLUDE_DIRS ${TENSORFLOW_INCLUDE_DIR})
            message(STATUS "Found TensorFlow")
        else()
            message(FATAL_ERROR "TensorFlow requested but not found. Please set TENSORFLOW_DIR")
        endif()
    endif()
endif()

# Collect source files
file(GLOB_RECURSE ESSENTIA_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sonoria/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sonoria/**/*.cpp"
)

# Algorithm sources will be generated by the registration script
set(ALGORITHM_SOURCES "")
set(ALGORITHM_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sonoria"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sonoria/scheduler"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sonoria/streaming"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sonoria/streaming/algorithms"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sonoria/utils"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty"
)

# Add version.h directory to includes
list(APPEND ALGORITHM_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Third-party sources
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/spline")
    list(APPEND ALGORITHM_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/spline")
    list(APPEND ESSENTIA_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/spline/splineutil.cpp")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/nnls")
    list(APPEND ALGORITHM_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/nnls")
    list(APPEND ESSENTIA_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/nnls/nnls.c")
endif()

# Add Kiss FFT sources if using KISS backend
if(FFT_BACKEND STREQUAL "KISS")
    list(APPEND ESSENTIA_CORE_SOURCES ${KISSFFT_SOURCES})
endif()

# Collect algorithm source files
file(GLOB_RECURSE ALGORITHM_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/algorithms/**/*.cpp"
)

# Create library
if(BUILD_STATIC)
    add_library(sonoria STATIC
        ${ESSENTIA_CORE_SOURCES}
        ${ALGORITHM_SOURCE_FILES}
    )
else()
    add_library(sonoria SHARED
        ${ESSENTIA_CORE_SOURCES}
        ${ALGORITHM_SOURCE_FILES}
    )
endif()

target_include_directories(sonoria PUBLIC
    $<BUILD_INTERFACE:${ALGORITHM_INCLUDES}>
    $<INSTALL_INTERFACE:include/sonoria>
)

# Add compile definitions
target_compile_definitions(sonoria PRIVATE
    EIGEN_PERMANENTLY_DISABLE_STUPID_WARNINGS
    EIGEN_MPL2_ONLY
    __STDC_CONSTANT_MACROS
)

# Link dependencies
if(FFTW_FOUND AND FFT_BACKEND STREQUAL "FFTW")
    target_link_libraries(sonoria PRIVATE ${FFTW_LIBRARIES})
endif()

if(HAVE_AVCODEC)
    target_link_libraries(sonoria PRIVATE ${AVCODEC_LIBRARIES})
    target_include_directories(sonoria PRIVATE ${AVCODEC_INCLUDE_DIRS})
endif()

if(HAVE_AVFORMAT)
    target_link_libraries(sonoria PRIVATE ${AVFORMAT_LIBRARIES})
    target_include_directories(sonoria PRIVATE ${AVFORMAT_INCLUDE_DIRS})
endif()

if(HAVE_AVUTIL)
    target_link_libraries(sonoria PRIVATE ${AVUTIL_LIBRARIES})
    target_include_directories(sonoria PRIVATE ${AVUTIL_INCLUDE_DIRS})
endif()

if(HAVE_SWRESAMPLE)
    target_link_libraries(sonoria PRIVATE ${SWRESAMPLE_LIBRARIES})
    target_include_directories(sonoria PRIVATE ${SWRESAMPLE_INCLUDE_DIRS})
endif()

if(HAVE_SAMPLERATE)
    target_link_libraries(sonoria PRIVATE ${SAMPLERATE_LIBRARIES})
    target_include_directories(sonoria PRIVATE ${SAMPLERATE_INCLUDE_DIRS})
endif()

if(HAVE_TAGLIB)
    if(TARGET TagLib::tag)
        target_link_libraries(sonoria PRIVATE TagLib::tag)
    else()
        target_link_libraries(sonoria PRIVATE ${TAGLIB_LIBRARIES})
        target_include_directories(sonoria PRIVATE ${TAGLIB_INCLUDE_DIRS})
    endif()
endif()

if(HAVE_YAML)
    target_link_libraries(sonoria PRIVATE ${YAML_LIBRARIES})
    target_include_directories(sonoria PRIVATE ${YAML_INCLUDE_DIRS})
endif()

if(HAVE_CHROMAPRINT)
    target_link_libraries(sonoria PRIVATE ${CHROMAPRINT_LIBRARIES})
    target_include_directories(sonoria PRIVATE ${CHROMAPRINT_INCLUDE_DIRS})
endif()

if(HAVE_GAIA)
    target_link_libraries(sonoria PRIVATE ${GAIA_LIBRARIES})
    target_include_directories(sonoria PRIVATE ${GAIA_INCLUDE_DIRS})
endif()

if(HAVE_TENSORFLOW)
    target_include_directories(sonoria PRIVATE ${TENSORFLOW_INCLUDE_DIRS})
    target_link_libraries(sonoria PRIVATE ${TENSORFLOW_LIBRARIES})
endif()

if(FFT_BACKEND STREQUAL "ACCELERATE" AND HAVE_ACCELERATE)
    target_link_libraries(sonoria PRIVATE ${ACCELERATE_FRAMEWORK})
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(sonoria PRIVATE Threads::Threads)
endif()

# Set library properties
set_target_properties(sonoria PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 2
    PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/sonoria/sonoria.h"
)

# Installation
include(GNUInstallDirs)

install(TARGETS sonoria
    EXPORT EssentiaTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sonoria
)

# Install headers
install(DIRECTORY src/sonoria/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sonoria
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY src/algorithms/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sonoria/algorithms
    FILES_MATCHING PATTERN "*.h"
)

# Generate and install pkg-config file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/sonoria.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/sonoria.pc"
    @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sonoria.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Export targets
install(EXPORT EssentiaTargets
    FILE EssentiaTargets.cmake
    NAMESPACE Essentia::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Essentia
)

# Create config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EssentiaConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/EssentiaConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Essentia
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/EssentiaConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/EssentiaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/EssentiaConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Essentia
)

# Examples
if(BUILD_EXAMPLES)
    message(STATUS "Building examples")
    file(GLOB EXAMPLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/examples/*.cpp")
    
    foreach(example_source ${EXAMPLE_SOURCES})
        get_filename_component(example_name ${example_source} NAME_WE)
        add_executable(${example_name} ${example_source})
        target_link_libraries(${example_name} PRIVATE sonoria)
        
        if(BUILD_STATIC AND STATIC_DEPENDENCIES)
            set_target_properties(${example_name} PROPERTIES
                LINK_SEARCH_START_STATIC ON
                LINK_SEARCH_END_STATIC ON
            )
        endif()
    endforeach()
endif()

# Python bindings
if(BUILD_PYTHON)
    message(STATUS "Building Python bindings")
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    
    add_subdirectory(src/python)
endif()

# Tests
if(BUILD_TESTS)
    message(STATUS "Building tests")
    enable_testing()
    
    # Google Test
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        # Download and build Google Test
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/release-1.10.0.zip
        )
        FetchContent_MakeAvailable(googletest)
    endif()
    
    # Add test executable
    file(GLOB TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/test/src/basetest/*.cpp"
    )
    
    add_executable(basetest
        ${TEST_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/test/3rdparty/gtest-1.6.0/src/gtest-all.cc"
    )
    
    target_include_directories(basetest PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/test/3rdparty/gtest-1.6.0/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/3rdparty/gtest-1.6.0"
        ${ALGORITHM_INCLUDES}
    )
    
    target_link_libraries(basetest PRIVATE sonoria GTest::gtest_main)
    
    add_test(NAME basetest COMMAND basetest)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "================================ CONFIGURATION SUMMARY ================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build static: ${BUILD_STATIC}")
message(STATUS "Build shared: ${BUILD_SHARED_LIBS}")
message(STATUS "Build examples: ${BUILD_EXAMPLES}")
message(STATUS "Build Python: ${BUILD_PYTHON}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "FFT backend: ${FFT_BACKEND}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  Eigen3: Found")
message(STATUS "  FFTW: ${HAVE_FFTW}")
message(STATUS "  FFmpeg: ${HAVE_AVCODEC}")
message(STATUS "  libsamplerate: ${HAVE_SAMPLERATE}")
message(STATUS "  TagLib: ${HAVE_TAGLIB}")
message(STATUS "  libyaml: ${HAVE_YAML}")
message(STATUS "  Chromaprint: ${HAVE_CHROMAPRINT}")
message(STATUS "  Gaia2: ${HAVE_GAIA}")
message(STATUS "  TensorFlow: ${HAVE_TENSORFLOW}")
message(STATUS "======================================================================================")
