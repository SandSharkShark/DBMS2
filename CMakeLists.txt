cmake_minimum_required(VERSION 3.10)

project(dbms_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Qt设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置Qt路径
if(APPLE)
    set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@5")
endif()

# 查找Qt包
find_package(Qt5 COMPONENTS Widgets Core REQUIRED)
find_package(OpenSSL REQUIRED)

# 明确列出所有源文件
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/DatabaseManager.cpp
    src/Table.cpp
    src/SQLParser.cpp
    src/SQLHighlighter.cpp
    src/TableViewDialog.cpp
    src/UserManager.cpp
    src/LoginDialog.cpp
    src/RegisterDialog.cpp
    src/UserManagerDialog.cpp
    src/SettingsDialog.cpp
    src/BatchProcessDialog.cpp
)

# 在设置源文件之前添加资源
qt_add_resources(RESOURCES resources.qrc)

# 设置源文件时包含资源
set(SOURCES
    ${SOURCES}
    ${RESOURCES}
)

# 明确列出所有头文件
set(HEADERS
    include/mainwindow.h
    include/DatabaseManager.h
    include/Table.h
    include/SQLParser.h
    include/SQLHighlighter.h
    include/forward_declarations.h
    include/TableViewDialog.h
    include/User.h
    include/UserManager.h
    include/LoginDialog.h
    include/RegisterDialog.h
    include/UserManagerDialog.h
    include/SettingsDialog.h
    include/BatchProcessDialog.h
)

# 添加包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 使用target_link_libraries设置包含路径和链接库
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    Qt5::Widgets
    Qt5::Core
    OpenSSL::SSL
    OpenSSL::Crypto
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 添加调试信息
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Header files: ${HEADERS}")
message(STATUS "Qt5_DIR: ${Qt5_DIR}")
message(STATUS "OpenSSL_DIR: ${OPENSSL_ROOT_DIR}")
