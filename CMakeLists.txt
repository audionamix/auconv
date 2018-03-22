cmake_minimum_required(VERSION 3.0)

include(ExternalProject)
project(AuConv)

option(enable_tests "Build unit tests" ON)
option(enable_binary "Build auconverter binary" ON)

# enable c++11/14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11")

# libwave
set(wave_install_dir ${CMAKE_BINARY_DIR}/wave-install)
ExternalProject_Add(external_wave
  GIT_REPOSITORY "https://github.com/audionamix/wave"
  GIT_TAG "v0.8.1a"
  # don't update so that we don't rebuild every time
  UPDATE_COMMAND ""
  CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${wave_install_dir}"
    "-Dwave_enable_tests=OFF"
)
link_directories("${wave_install_dir}/lib")

if (${enable_tests})
  enable_testing()

  set(gtest_install_dir ${CMAKE_BINARY_DIR}/googletest-install)
  ExternalProject_Add(external_googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "718fd88d8f145c63b8cc134cf8fed92743cc112f"
    CMAKE_ARGS
      "-Dgtest_force_shared_crt=ON"
      "-DCMAKE_INSTALL_PREFIX=${gtest_install_dir}"
  )
  link_directories("${gtest_install_dir}/lib")

  # download formatted tests
  set(test_resource_path "${CMAKE_BINARY_DIR}/test_resource")
  file(MAKE_DIRECTORY "${test_resource_path}")

  # mp3
  message(STATUS "downloading test file")
  file(DOWNLOAD
    "https://ccrma.stanford.edu/~jos/mp3/gtr-jazz.mp3"
    "${test_resource_path}/gtr-jazz.mp3"
  )

  # other error files
  set(s3_files
    EverythingWasEverything.m4a
    MajorLazer-LeanOn.mp3
    The_Chainsmokers-Something_Just_Like_Thi-LLS.aif
    White-Noise-At-Beginning-After-Conversion.aif
    clicking.m4a
  )
  foreach(s3_file ${s3_files})
    set(s3_file_path ${test_resource_path}/${s3_file})
    if (NOT EXISTS ${s3_file_path})
      execute_process(COMMAND aws s3 cp s3://audionamix-public/auconv-testfiles/${s3_file} ${s3_file_path})
    endif()
  endforeach()
endif()

set(src "${CMAKE_SOURCE_DIR}/src")
include_directories(${src})
add_subdirectory("./src")
