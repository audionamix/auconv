add_library(auconv
  file.cc
  file.h
)

add_dependencies(auconv
  external_ffmpeg
  external_wave
)

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

target_include_directories(auconv
  PRIVATE
    ${ffmpeg_install_dir}/include
    ${wave_install_dir}/include
)

if (${enable_tests})
  add_executable(auconv_test
    file_test.cc
  )

  add_dependencies(auconv_test
    auconv
    external_googletest
  )

  target_link_libraries(auconv_test
    auconv
    gtest
    gtest_main
    wave
    ${ffmpeg_libraries}

    # needed by ffmpeg
    ${external_libs}
  )

  target_include_directories(auconv_test
    PRIVATE
      ${gtest_install_dir}/include
  )

  target_compile_definitions(auconv_test
    PUBLIC -DTEST_RESOURCES_PATH="${test_resource_path}"
  )
endif()
