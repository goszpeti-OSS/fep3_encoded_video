set(CMAKE_BUILD_TYPE ${CONAN_SETTINGS_BUILD_TYPE}) #

# ####################################################

# Execute (now hacked) find package script

file(GLOB SRCS *.cpp *.h)

set(TARGET_NAME qt_playground)

add_executable(${TARGET_NAME}
    ${SRCS}
)

target_compile_options(${TARGET_NAME} PRIVATE "/Od")

target_link_libraries(${TARGET_NAME} PRIVATE
    qt::qt
    dev_essential::dev_essential

)

# Qt base libs for debug
add_custom_command( TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CONAN_QT_ROOT}/bin/Qt6Core${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${CONAN_QT_ROOT}/bin/Qt6Gui${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${CONAN_QT_ROOT}/bin/Qt6Multimedia${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${CONAN_QT_ROOT}/bin/Qt6MultimediaWidgets${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${CONAN_QT_ROOT}/bin/Qt6Network${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${CONAN_QT_ROOT}/bin/Qt6Widgets${CMAKE_SHARED_LIBRARY_SUFFIX}
        $<TARGET_FILE_DIR:${TARGET_NAME}>
)

# Qt platform for debug
if (MSVC)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${TARGET_NAME}>/platforms"
    )
    add_custom_command( TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CONAN_QT_ROOT}/res/archdatadir/plugins/platforms/qwindows.dll
            $<TARGET_FILE_DIR:${TARGET_NAME}>/platforms/
    )
endif()