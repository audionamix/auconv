add_subdirectory("./ffmpeg")
link_directories("${ffmpeg_lib_dir}")

add_subdirectory("./auconv")
if (${enable_binary})
  #add_subdirectory("./auconverter")
endif()
