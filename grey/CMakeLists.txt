cmake_minimum_required (VERSION 3.20)

set (STATIC_NAME libgrey)
set (DYNAMIC_NAME xgrey)

find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)

# all non-Windows build use OpenGL+GLFW3
if(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(Fontconfig REQUIRED) # since CMake 3.14
 endif()

find_path(STB_INCLUDE_DIRS "stb.h")

file(GLOB core_src
    "anim.cpp"
    "app.cpp"
    "themes.cpp"
    "widgets.cpp"
    "guid.cpp"
    "3rdparty/imgui-node-editor/*.cpp"
    "3rdparty/ImGuiColorTextEdit/*.cpp"
    "fonts/*.cpp"
    "common/*.cpp"
    "x/*.cpp")

if (WIN32)
    file(GLOB core_src ${core_src}
            "common/win32/gdi.cpp"
            "common/win32/window.cpp"
            "common/win32/reg.cpp")
    set(private_libs
        implot::implot
        D3D11.lib)
elseif (UNIX)
    # file(GLOB core_src ${core_src} "common/lin/*.cpp")
    set(private_libs
        implot::implot
        glfw
        OpenGL::GL
        Fontconfig::Fontconfig)
endif()

set(interface_libs imgui::imgui)

add_library(${DYNAMIC_NAME} SHARED ${core_src} "x.cpp")
add_library(${STATIC_NAME} STATIC ${core_src})

#set_property(TARGET grey PROPERTY
#  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#set_property(TARGET libgrey PROPERTY
#  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# dynamic library
# should include both private and interface libs as private dependencies
target_link_libraries(${DYNAMIC_NAME}        
   PRIVATE ${private_libs} ${interface_libs})

# static library
target_link_libraries(${STATIC_NAME}
   PRIVATE ${private_libs}
   INTERFACE ${interface_libs})

# Set GCC-specific flags for dynamic library
#if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_BUILD_TYPE MATCHES Release)
#    target_compile_options(${DYNAMIC_NAME} PRIVATE -Os)
#endif()

# dynamic library should be copied for the consumption to external bindings
# https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#generator-expression-reference

# dotnet binary - only execute when DOTNET_RID environment variable is set
if(DEFINED ENV{DOTNET_RID})
    add_custom_command(
        TARGET ${DYNAMIC_NAME} 
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${DYNAMIC_NAME}>
            ${CMAKE_SOURCE_DIR}/xbin/runtimes/$ENV{DOTNET_RID}/native/$<TARGET_FILE_NAME:${DYNAMIC_NAME}>
    )
endif()
