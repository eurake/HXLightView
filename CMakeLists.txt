cmake_minimum_required(VERSION 3.10)

project(HXLightView VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

# MSVC 默认按系统代码页(936/GBK)解析源文件，UTF-8 中文/emoji 会触发 C4819/C2001
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 查找Qt5模块
find_package(Qt5 REQUIRED COMPONENTS Widgets Sql Network)

add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/Database.h
    src/Database.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/HomeTab.h
    src/HomeTab.cpp
    src/InputTab.h
    src/InputTab.cpp
    src/QueryTab.h
    src/QueryTab.cpp
    src/TcpServer.h
    src/TcpServer.cpp
    src/TcpServerTab.h
    src/TcpServerTab.cpp
    src/SettingsTab.h
    src/SettingsTab.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network)
