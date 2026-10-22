cmake_minimum_required(VERSION 3.10)

project(LaserMark VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt5 REQUIRED COMPONENTS Widgets Sql Network)

add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/AppConfig.h
    src/AppConfig.cpp
    src/Database.h
    src/Database.cpp
    src/TcpServer.h
    src/TcpServer.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    resources/LaserMark.qrc
    resources/LaserMark.rc
)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network)
