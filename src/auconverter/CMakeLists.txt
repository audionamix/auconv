add_executable(auconverter
  main.cc
)

target_link_libraries(auconverter
  auconv
  wave
  ${ffmpeg_libraries}
  ${external_libs}
)
