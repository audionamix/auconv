add_library(auconv
  file.cc
  file.h
)

add_dependencies(auconv
  external_ffmpeg
  external_wave
)

target_include_directories(auconv
  PRIVATE
    ${ffmpeg_install_dir}/include
    ${wave_install_dir}/include
)

# unit tests
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

# install rules
install(TARGETS auconv
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
# includes
install(FILES
  ${src}/auconv/file.h
  DESTINATION include/auconv
)
# external dependencies
set(dependencies "")
foreach (ffmpeg_lib ${ffmpeg_libraries})
  set(dependencies
    ${dependencies}
    ${ffmpeg_install_dir}/lib/lib${ffmpeg_lib}.a
  )
endforeach()
install(FILES
  ${dependencies}
  DESTINATION lib
)
