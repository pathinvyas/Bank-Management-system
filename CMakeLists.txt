cmake_minimum_required(VERSION 3.16)

project(BlueBankProFull LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------
# ENSURE MOC, UIC, RCC ARE ALWAYS RUN
# -----------------------------------------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# -----------------------------------------------
# QT MODULES
# -----------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Sql
    PrintSupport
)

# -----------------------------------------------
# SOURCE FILES
# -----------------------------------------------
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/loginwindow.cpp
    src/dbmanager.cpp
)

set(HEADERS
    src/mainwindow.h
    src/loginwindow.h
    src/dbmanager.h
)

# -----------------------------------------------
# EXECUTABLE
# -----------------------------------------------
qt_add_executable(BlueBankProFull
    ${SOURCES}
    ${HEADERS}
)

# -----------------------------------------------
# QT LIBRARIES
# -----------------------------------------------
target_link_libraries(BlueBankProFull PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
)

target_include_directories(BlueBankProFull PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
