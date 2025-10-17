cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable (demo WIN32 "main.cpp" main.rc)

if(WIN32)
    # "Transparent" demo for Windows
    add_executable (tdemo WIN32 "main_transparent.cpp" main.rc)

    target_compile_definitions(tdemo PRIVATE GREY_INCLUDE_IMNODES)
    target_link_libraries(tdemo libgrey)
    target_include_directories(tdemo PRIVATE "../../grey")

    target_sources(tdemo PRIVATE dpi-aware.manifest)

    # common settings for Windows
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:wmainCRTStartup")
endif()

target_compile_definitions(demo PRIVATE GREY_INCLUDE_IMNODES)
target_link_libraries(demo libgrey)
target_include_directories(demo PRIVATE "../../grey")

target_sources(demo PRIVATE dpi-aware.manifest)
