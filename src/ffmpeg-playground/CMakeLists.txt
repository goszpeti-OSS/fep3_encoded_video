set(CMAKE_BUILD_TYPE ${CONAN_SETTINGS_BUILD_TYPE})  #
#####################################################

add_executable("ffmpeg-playground"
    main.cpp
)
target_compile_options("ffmpeg-playground" PRIVATE "/Od")

target_link_libraries("ffmpeg-playground" PRIVATE
        dev_essential::filesystem
        dev_essential::system
        # ffmpeg::ffmpeg
        ffmpeg::avutil
        ffmpeg::avdevice
        ffmpeg::avcodec
        ffmpeg::avformat
        ffmpeg::postproc
)

# if(MSVC)
#     # warning coming from dds headers
#     target_compile_options(${PLUGIN_NAME} PRIVATE /wd4245)
#     target_link_libraries(${PLUGIN_NAME} PRIVATE "Iphlpapi.lib")
#     install(FILES $<TARGET_PDB_FILE:${PLUGIN_NAME}> DESTINATION lib/fast_dds OPTIONAL)
# endif()

# add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         ${CMAKE_CURRENT_SOURCE_DIR}/fep3_participant.fep_components
#         $<TARGET_FILE_DIR:${PLUGIN_NAME}>
# )
# add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#     ${CMAKE_CURRENT_SOURCE_DIR}/USER_QOS_PROFILES.xml
#     $<TARGET_FILE_DIR:${PLUGIN_NAME}>
# )
