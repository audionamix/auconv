add_executable(auconverter
  main.cc
)

find_package(Boost CONFIG REQUIRED program_options)

target_link_libraries(auconverter
  auconv
  wave
  ${ffmpeg_libraries}
  ${external_libs}
  Boost::program_options
)
