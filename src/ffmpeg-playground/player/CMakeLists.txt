set(CMAKE_BUILD_TYPE ${CONAN_SETTINGS_BUILD_TYPE})  #
#####################################################
file( GLOB SRCS *.cpp *.h )

add_executable("ffmpeg-player"
   ${SRCS}
)
target_compile_options("ffmpeg-player" PRIVATE "/Od")

target_link_libraries("ffmpeg-player" PRIVATE
        dev_essential::filesystem
        dev_essential::system
        # ffmpeg::ffmpeg
        ffmpeg::avutil
        ffmpeg::avdevice
        ffmpeg::avcodec
        ffmpeg::avformat
        ffmpeg::postproc
        SDL2::SDL2main
        sdl_ttf::sdl_ttf
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
