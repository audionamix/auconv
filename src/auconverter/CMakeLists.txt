add_executable(auconverter
  main.cc
)

set(cxxopts_install_dir ${CMAKE_BINARY_DIR}/cxxopts-install)
ExternalProject_Add(external_cxxopts
  GIT_REPOSITORY "https://github.com/jarro2783/cxxopts.git"
  GIT_TAG "v2.0.0"
  SOURCE_DIR ${cxxopts_install_dir}
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  TEST_COMMAND ""
  INSTALL_COMMAND ""
)

add_dependencies(auconverter
  external_cxxopts
)

target_link_libraries(auconverter
  auconv
  wave
)

target_include_directories(auconverter
  PUBLIC
    ${cxxopts_install_dir}/include
)

install(TARGETS auconverter
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
