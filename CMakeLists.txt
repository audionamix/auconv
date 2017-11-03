cmake_minimum_required(VERSION 3.0)

# hunter for complex external dependencies (boost)
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.6.tar.gz"
    SHA1 "8c70cf364fe927dd72a8e38202fedbd1ac1b0d1a"
)

include(ExternalProject)
project(AuConv)

option(enable_tests "Build unit tests" ON)
option(enable_binary "Build auconverter binary" ON)

# enable c++11/14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11")
# ffmpeg
# -- external project
set(ffmpeg_install_dir ${CMAKE_BINARY_DIR}/ffmpeg-install)
set(ffmpeg_souce_dir ${CMAKE_BINARY_DIR}/ffmpeg-sources)
file(MAKE_DIRECTORY ${ffmpeg_souce_dir})
ExternalProject_Add(external_ffmpeg
  GIT_REPOSITORY "git@github.com:FFmpeg/FFmpeg.git"
  GIT_TAG "n3.3.1"
  # don't update so that we don't rebuild every time
  UPDATE_COMMAND ""
  SOURCE_DIR ${ffmpeg_souce_dir}
  CONFIGURE_COMMAND
    cd ${ffmpeg_souce_dir} && ./configure --enable-gpl --disable-yasm --disable-avdevice --disable-swscale --disable-postproc --prefix=${ffmpeg_install_dir}
  BUILD_COMMAND cd ${ffmpeg_souce_dir} && make
  INSTALL_DIR ${ffmpeg_install_dir}
  INSTALL_COMMAND cd ${ffmpeg_souce_dir} && make install
)
link_directories("${ffmpeg_install_dir}/lib")

# -- ffmpeg libraries and external dependencies
set(ffmpeg_libraries
  avcodec
  avfilter
  avformat
  avutil
  swresample
)
set(external_lib_names
  AudioToolbox
  QuartzCore
  AppKit
  OpenGL
  CoreFoundation
  VideoToolbox
  CoreMedia
  CoreVideo
  VideoDecodeAcceleration
  Security
  CoreServices
  iconv
  bz2
  z
)
set(external_libs "")
foreach(lib ${external_lib_names})
  find_library(lib_${lib} ${lib})
  set(external_libs
    ${external_libs}
    ${lib_${lib}}
  )
endforeach()

# libwave
set(wave_install_dir ${CMAKE_BINARY_DIR}/wave-install)
ExternalProject_Add(external_wave
  GIT_REPOSITORY "https://github.com/audionamix/wave"
  GIT_TAG "v0.6a"
  # don't update so that we don't rebuild every time
  UPDATE_COMMAND ""
  CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${wave_install_dir}"
)
link_directories("${wave_install_dir}/lib")

# boost
if (${enable_binary})
  hunter_add_package(Boost COMPONENTS program_options)
endif()

if (${enable_tests})
  enable_testing()

  set(gtest_install_dir ${CMAKE_BINARY_DIR}/googletest-install)
  ExternalProject_Add(external_googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "release-1.8.0"
    CMAKE_ARGS
      "-Dgtest_force_shared_crt=ON"
      "-DCMAKE_INSTALL_PREFIX=${gtest_install_dir}"
  )
  link_directories("${gtest_install_dir}/lib")

  # download formatted tests
  set(test_resource_path "${CMAKE_BINARY_DIR}/test_resource")
  file(MAKE_DIRECTORY "${test_resource_path}")

  # mp3
  message(STATUS "downloading test file https://ccrma.stanford.edu/~jos/mp3/gtr-jazz.mp3")
  file(DOWNLOAD
    "https://ccrma.stanford.edu/~jos/mp3/gtr-jazz.mp3"
    "${test_resource_path}/gtr-jazz.mp3"
  )
endif()

set(src "${CMAKE_SOURCE_DIR}/src")
include_directories(${src})
add_subdirectory("./src")
