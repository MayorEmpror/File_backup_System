cmake_minimum_required(VERSION 3.10)

project(DFBMS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)

set(SOURCES
    main.cpp
    src/client/Client.cpp
    src/client/ClientSimulator.cpp
    src/core/backupmanager.cpp
    src/UI/LineChartWidget.cpp
    src/UI/MainWindow.cpp
)

set(HEADERS
    src/client/Client.h
    src/client/ClientSimulator.h
    src/client/IClient.h
    src/client/BackupRequest.h
    src/core/backupmanager.h
    src/core/Semaphore.h
    src/UI/LineChartWidget.h
    src/UI/MainWindow.h
)

add_executable(DFBMS ${SOURCES} ${HEADERS})

target_include_directories(DFBMS PRIVATE src)

target_link_libraries(DFBMS
    Qt6::Widgets
    Threads::Threads
)