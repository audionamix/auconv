# Build ffmpeg and install informations requested here: https://www.ffmpeg.org/legal.html
if (APPLE)
  set(ffmpeg_install_dir ${CMAKE_BINARY_DIR}/ffmpeg-install)
  set(ffmpeg_clone_dir ${CMAKE_BINARY_DIR}/ffmpeg-clone)
  set(ffmpeg_souce_dir ${CMAKE_BINARY_DIR}/ffmpeg-sources)
  set(ffmpeg_version "n3.3.5")
  set(ffmpeg_configuration_file ${CMAKE_BINARY_DIR}/configuration.txt)
  set(ffmpeg_compressed_sources ${CMAKE_BINARY_DIR}/ffmpeg-sources.tar.gz)

  # Build target
  ExternalProject_Add(external_ffmpeg
    URL ${ffmpeg_souce_dir}
    # don't update so that we don't rebuild every time
    UPDATE_COMMAND ""
    SOURCE_DIR ${ffmpeg_souce_dir}
    CONFIGURE_COMMAND cd ${ffmpeg_souce_dir} && ./configure --extra-ldflags="-mmacosx-version-min=10.11" --extra-cflags="-mmacosx-version-min=10.11" --disable-static --enable-shared --disable-programs --disable-yasm --disable-avdevice --disable-swscale --disable-postproc  --prefix=${ffmpeg_install_dir}
    BUILD_COMMAND cd ${ffmpeg_souce_dir} && make
    INSTALL_DIR ${ffmpeg_install_dir}
    INSTALL_COMMAND cd ${ffmpeg_souce_dir} && make install
  )

  # We don't use the git in the external project so we can do the git diff and
  # tar the sources so we can redistribute them
  add_custom_command(
    TARGET external_ffmpeg
    PRE_BUILD
    COMMAND
      # if the clone hasn't been copied yet, do it !
      if [ ! -d "${ffmpeg_clone_dir}" ]; then
        git clone -n "https://github.com/FFmpeg/FFmpeg.git" ${ffmpeg_clone_dir} &&
        cd ${ffmpeg_clone_dir} && git checkout ${ffmpeg_version} &&
        cp -r ${ffmpeg_clone_dir}/* ${ffmpeg_souce_dir} \\;
      fi
  )
  file(WRITE
    ${ffmpeg_configuration_file}
    "Configuration flags: --extra-ldflags=\"-mmacosx-version-min=10.11\" --extra-cflags=\"-mmacosx-version-min=10.11\" --disable-static --enable-shared --disable-programs --disable-yasm --disable-avdevice --disable-swscale --disable-postproc"
  )
  add_custom_command(
    TARGET external_ffmpeg
    POST_BUILD
    COMMAND
      cd ${ffmpeg_clone_dir} &&
      git diff > changes.diff &&
      cp ${ffmpeg_configuration_file} . &&
      cd ${CMAKE_BINARY_DIR} &&
      # compress sources if not done already
      if [ ! -d "${ffmpeg_compressed_sources}" ]; then
        ${CMAKE_COMMAND} -E tar zcf ${ffmpeg_compressed_sources} ${CMAKE_BINARY_DIR}/ffmpeg-clone \\;
      fi
  )

  # link flags
  set(ffmpeg_libraries
    avcodec
    avfilter
    avformat
    avutil
    swresample
  )
  set(ffmpeg_external_lib_names
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

  set(ffmpeg_external_libs "")
  foreach(lib ${external_lib_names})
    find_library(lib_${lib} ${lib})
    set(ffmpeg_external_libs
      ${ffmpeg_external_libs}
      ${lib_${lib}}
    )
  endforeach()

  set(ffmpeg_include_dir ${ffmpeg_install_dir}/include PARENT_SCOPE)
  set(ffmpeg_lib_dir ${ffmpeg_install_dir}/lib PARENT_SCOPE)
  set(ffmpeg_libraries ${ffmpeg_libraries} PARENT_SCOPE)
  set(ffmpeg_external_libs ${ffmpeg_external_libs} PARENT_SCOPE)


elseif (WIN32)
  # TODO switch according to arch type here...
  set(ffmpeg_arch_prefix "win32")

  set(ffmpeg_shared_folder_name "ffmpeg-3.4.2-${ffmpeg_arch_prefix}-shared")
  set(ffmpeg_sources_folder_name "ffmpeg-3.4.2-${ffmpeg_arch_prefix}-dev")
  set(ffmpeg_shared_url "https://ffmpeg.zeranoe.com/builds/${ffmpeg_arch_prefix}/shared/${ffmpeg_shared_folder_name}.zip")
  set(ffmpeg_sources_url "https://ffmpeg.zeranoe.com/builds/${ffmpeg_arch_prefix}/dev/${ffmpeg_sources_folder_name}.zip")
  set(ffmpeg_shared_archive ${CMAKE_BINARY_DIR}/ffmpeg_shared.zip)
  set(ffmpeg_source_archive ${CMAKE_BINARY_DIR}/ffmpeg_sources.zip)
  set(ffmpeg_destination_dir "${CMAKE_CURRENT_BINARY_DIR}")
  set(ffmpeg_shared_dir "${ffmpeg_destination_dir}/ffmpeg-3.4.2-${ffmpeg_arch_prefix}-shared")
  set(ffmpeg_source_dir "${ffmpeg_destination_dir}/ffmpeg-3.4.2-${ffmpeg_arch_prefix}-dev")
  set(ffmpeg_download_script ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_download.cmake)

  set(download_commands
    "if (NOT EXISTS ${ffmpeg_shared_url} AND NOT EXISTS ${ffmpeg_sources_url})"
    "  file(DOWNLOAD ${ffmpeg_shared_url} ${ffmpeg_shared_archive})"
    "  file(DOWNLOAD ${ffmpeg_sources_url} ${ffmpeg_source_archive})"
    "endif()"
  )
  set(download_command "")
  foreach(command ${download_commands})
    set(download_command "${download_command}\n${command}")
  endforeach()
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_download.cmake ${download_command})

  add_custom_target(external_ffmpeg
    COMMAND
      ${CMAKE_COMMAND} -P ${ffmpeg_download_script} &&
      ${CMAKE_COMMAND} -E tar xfz ${ffmpeg_shared_archive} &&
      ${CMAKE_COMMAND} -E tar xfz ${ffmpeg_source_archive}
  )

  set(ffmpeg_libraries
    swresample.lib
    avutil.lib
    avcodec.lib
    avformat.lib
    avfilter.lib
  )
  set(ffmpeg_shared_libraries
    ${ffmpeg_shared_dir}/bin/swresample-2.dll
    ${ffmpeg_shared_dir}/bin/avutil-55.dll
    ${ffmpeg_shared_dir}/bin/avcodec-57.dll
    ${ffmpeg_shared_dir}/bin/avformat-57.dll
    ${ffmpeg_shared_dir}/bin/avfilter-6.dll
  )

  set(ffmpeg_compressed_sources ${ffmpeg_source_archive})

  set(ffmpeg_include_dir ${ffmpeg_source_dir}/include PARENT_SCOPE)
  set(ffmpeg_lib_dir ${ffmpeg_source_dir}/lib PARENT_SCOPE)
  set(ffmpeg_libraries ${ffmpeg_libraries} PARENT_SCOPE)
  set(ffmpeg_shared_libraries ${ffmpeg_shared_libraries} PARENT_SCOPE)
  set(ffmpeg_external_libs "" PARENT_SCOPE)

  # install the dlls in lib dir when installing
  install(FILES
    ${ffmpeg_shared_libraries}
    DESTINATION bin
  )
else()
  message(FATAL "Unsupported OS type")
endif()

# when installing, copy the compressed files in the install directory
install(FILES
  ${ffmpeg_compressed_sources}
  DESTINATION bin
)
