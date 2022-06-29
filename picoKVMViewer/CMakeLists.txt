cmake_minimum_required(VERSION 3.1.0)

project(picoKVMViewer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets Multimedia MultimediaWidgets SerialPort REQUIRED)

add_executable(picoKVMViewer
    viewer.ui
    viewer.cpp
    ExtendedVideoWidget.cpp
    main.cpp
)


include_directories(${CMAKE_SOURCE_DIR})

target_link_libraries(picoKVMViewer Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets Qt5::SerialPort)
