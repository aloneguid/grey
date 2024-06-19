cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(APP_NAME pure)
set(APP_LONG_NAME "Pure ImGui without any wrappers")

find_package(imgui CONFIG REQUIRED)

add_executable (${APP_NAME} "main.cpp")

target_link_libraries(${APP_NAME} PRIVATE D3D11.lib imgui::imgui)
