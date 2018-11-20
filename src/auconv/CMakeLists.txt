add_library(auconv SHARED
  file.cc
  file.h
)

add_dependencies(auconv
  external_ffmpeg
  external_wave
)

target_include_directories(auconv
  PRIVATE
    ${ffmpeg_include_dir}
    ${wave_install_dir}/include
)

target_link_libraries(auconv
  wave
  ${ffmpeg_libraries}
  ${ffmpeg_external_libs}
)

if (APPLE)
  # remove non desired rpaths
  add_custom_command(TARGET auconv
    POST_BUILD COMMAND
    ${CMAKE_INSTALL_NAME_TOOL} -delete_rpath "${wave_install_dir}/lib" -delete_rpath "${gtest_install_dir}/lib" -delete_rpath "${ffmpeg_lib_dir}" || true
    $<TARGET_FILE:auconv>
  )
  # and add the right one
  add_custom_command(TARGET auconv
    POST_BUILD COMMAND
    ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@loader_path/../Frameworks/auconv.framework/Resources" || true
    $<TARGET_FILE:auconv>
  )

  set_target_properties(auconv PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER com.audionamix.auconv
    PUBLIC_HEADER ${src}/auconv/file.h
  )

  # after build, we need to install dependencies into the framework
  add_custom_command(
    TARGET auconv
    POST_BUILD
    COMMAND bash ${src}/ffmpeg/install.sh $<TARGET_FILE:auconv> @rpath/auconv.framework/Versions/A/Resources
  )

  # install target is just about installing the framework
  install(DIRECTORY
    ${CMAKE_BINARY_DIR}/src/auconv/$<CONFIG>/auconv.framework
    DESTINATION Frameworks
  )
else ()
  install(TARGETS auconv
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
endif ()

# unit tests
if (${enable_tests})
  add_executable(auconv_test
    file_test.cc
  )

  add_dependencies(auconv_test
    external_googletest
  )

  target_link_libraries(auconv_test
    optimized gtest
    optimized gtest_main
    debug gtestd
    debug gtest_maind
    auconv
    wave
  )

  if (WIN32)
    # On windows we need the dlls to be copied next to the executable
    foreach(dll ${ffmpeg_shared_libraries})
      add_custom_command(TARGET auconv_test POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${dll}" "$<TARGET_FILE_DIR:auconv_test>")
    endforeach()
  endif()

  target_include_directories(auconv_test
    PRIVATE
      ${gtest_install_dir}/include
  )

  target_compile_definitions(auconv_test
    PUBLIC -DTEST_RESOURCES_PATH="${test_resource_path}"
  )
endif()
