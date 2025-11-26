cmake_minimum_required (VERSION 3.28)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(APP_NAME pure)
set(APP_LONG_NAME "Pure ImGui without any wrappers")

find_package(imgui CONFIG REQUIRED)

# include next line only when building on Windows

if (WIN32)
    add_executable (${APP_NAME} "main_dx11.cpp")

    target_link_libraries(${APP_NAME} PRIVATE D3D11.lib imgui::imgui)
endif()

# Check if we are building on Linux
if (UNIX AND NOT APPLE)
    # sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config

    find_package(OpenGL REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)

    add_executable(${APP_NAME} "main_glfw_opengl3.cpp")
    target_link_libraries(${APP_NAME} PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
        )
endif()

# Check if we are building on macOS
if (APPLE)
    find_package(glfw3 CONFIG REQUIRED)
    find_library(METAL_FRAMEWORK Metal)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore)
    find_library(COCOA_FRAMEWORK Cocoa)

    # For now, we skip building the pure demo on macOS since it requires Objective-C++ 
    # and we don't have a pure Metal demo. The main grey library handles Metal properly.
    # add_executable(${APP_NAME} "main_glfw_metal.mm")
endif()