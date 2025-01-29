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