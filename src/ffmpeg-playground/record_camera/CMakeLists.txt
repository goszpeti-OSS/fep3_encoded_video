set(CMAKE_BUILD_TYPE ${CONAN_SETTINGS_BUILD_TYPE})  #
#####################################################

add_executable("record_camera"
    main.cpp
)
target_compile_options("record_camera" PRIVATE "/Od")

target_link_libraries("record_camera" PRIVATE
        dev_essential::filesystem
        dev_essential::system
        # ffmpeg::ffmpeg
        ffmpeg::avutil
        ffmpeg::avdevice
        ffmpeg::avcodec
        ffmpeg::avformat
        ffmpeg::postproc
)
