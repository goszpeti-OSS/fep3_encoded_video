set(CMAKE_BUILD_TYPE ${CONAN_SETTINGS_BUILD_TYPE})  #
#####################################################

# Execute (now hacked) find package script

# include(fep3-fast-plugin-macros.cmake)
file( GLOB SRCS *.cpp *.h )


set(PLUGIN_NAME fep3_encoded_video_plugin)
#IDLCXX_GENERATE(TARGET ${PLUGIN_NAME}_types FILES "types/stream_types.idl") # WARNINGS no-implicit-extensibility)
add_library(${PLUGIN_NAME} SHARED
${SRCS}
)

# target_compile_options(${PLUGIN_NAME} PRIVATE "/Od")
target_compile_definitions(${PLUGIN_NAME} PRIVATE BUILD_PLUGIN)

target_link_libraries(${PLUGIN_NAME} PRIVATE
 # ffmpeg::ffmpeg
        ffmpeg::avutil
        ffmpeg::avdevice
        ffmpeg::avcodec
        ffmpeg::avformat
        ffmpeg::postproc
        SDL2::SDL2main
        sdl_ttf::sdl_ttf
             fep3_participant_cpp_plugin
            dev_essential::pkg_rpc
            dev_essential::filesystem
            dev_essential::system
            )

if(MSVC)
    # warning coming from dds headers
    target_compile_options(${PLUGIN_NAME} PRIVATE /wd4245)
    target_link_libraries(${PLUGIN_NAME} PRIVATE "Iphlpapi.lib")
    install(FILES $<TARGET_PDB_FILE:${PLUGIN_NAME}> DESTINATION lib/fast_dds OPTIONAL)
endif()

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

# fast_deploy(${PLUGIN_NAME} $<TARGET_FILE_DIR:${PLUGIN_NAME}>)
# fast_install(lib/fast_dds)

install(TARGETS ${PLUGIN_NAME}
        EXPORT ${PLUGIN_NAME}_targets
        LIBRARY NAMELINK_SKIP DESTINATION lib/encoded_video
        RUNTIME DESTINATION lib/encoded_video)

install(EXPORT ${PLUGIN_NAME}_targets DESTINATION lib/cmake)

install(FILES 
        fep3_participant.fep_components
        #fep3-fast-plugin-macros.cmake
        #USER_QOS_PROFILES.xml
        #USER_QOS_PROFILES_FULL.xml
        DESTINATION lib
)
