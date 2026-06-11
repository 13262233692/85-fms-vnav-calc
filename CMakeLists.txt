cmake_minimum_required(VERSION 3.20)
project(FMCSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(WIN32)
    set(CMAKE_PREFIX_PATH "D:/Qt/6.8.3/msvc2022_64" ${CMAKE_PREFIX_PATH})
endif()

find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    PrintSupport
    REQUIRED
)

set(PROJECT_SOURCES
    src/main.cpp
    src/core/arinc424_parser.cpp
    src/core/arinc424_parser.h
    src/core/arinc424_types.h
    src/core/navigation.cpp
    src/core/navigation.h
    src/core/vnav_solver.cpp
    src/core/vnav_solver.h
    src/gui/lnav_table.cpp
    src/gui/lnav_table.h
    src/gui/cdu_widget.cpp
    src/gui/cdu_widget.h
    src/gui/vnav_profile_widget.cpp
    src/gui/vnav_profile_widget.h
    src/gui/main_window.cpp
    src/gui/main_window.h
)

add_executable(FMCSimulator WIN32
    ${PROJECT_SOURCES}
)

target_include_directories(FMCSimulator PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/gui
)

target_link_libraries(FMCSimulator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
)

if(MSVC)
    target_compile_options(FMCSimulator PRIVATE /W4)
    set_property(TARGET FMCSimulator PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
else()
    target_compile_options(FMCSimulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    set_target_properties(FMCSimulator PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

install(TARGETS FMCSimulator
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(WIN32)
    get_target_property(QtCore_location Qt6::Core LOCATION)
    get_filename_component(QT_BIN_DIR ${QtCore_location} DIRECTORY)

    install(DIRECTORY "${QT_BIN_DIR}/" DESTINATION bin
        FILES_MATCHING PATTERN "Qt6*.dll"
        PATTERN "Qt6*d.dll" EXCLUDE
    )

    install(DIRECTORY "${QT_BIN_DIR}/../plugins/platforms" DESTINATION bin
        FILES_MATCHING PATTERN "qwindows*.dll"
        PATTERN "qwindowsd.dll" EXCLUDE
    )
endif()

enable_testing()
