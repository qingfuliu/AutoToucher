#判断qt路径
if (NOT CMAKE_PREFIX_PATH)
    message(warning "CMAKE_PREFIX_PATH is not defined, you may need to set it "
            "(-DCMAKE_PREFIX_PATH=\"path/to/Qt/lib/cmake\" or -DCMAKE_PREFIX_PATH=/usr/include/{host}/qt{version}/ on Ubuntu)")
    #    set(CMAKE_PREFIX_PATH "H:/qt/5.15.2/mingw81_64")
endif ()

#Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets Svg)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg)
find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)

set(TARGET_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/include)
set(TARGET_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/gui)

file(GLOB_RECURSE Source_Code LIST_DIRECTORIES False ${TARGET_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE Ui_Code LIST_DIRECTORIES False ${TARGET_SOURCE_DIR}/*.ui)
file(GLOB_RECURSE Header_Code LIST_DIRECTORIES False ${TARGET_INCLUDE_DIR}/*.h)

# add qrc resources
set(QRC_FILES
        ${TARGET_SOURCE_DIR}/resource/resource.qrc
)
qt5_add_resources(QRC_RESOURCES ${QRC_FILES})

include_directories(${TARGET_INCLUDE_DIR}/gui)
##库文件
add_executable(${PROJECT_NAME}  ${Source_Code} ${Header_Code} ${Ui_Code} ${QRC_RESOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${TARGET_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})
if (WIN32)
    set(DEBUG_SUFFIX)
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB ${REQUIRED_LIBS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt${QT_VERSION}${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()