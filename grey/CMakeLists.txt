cmake_minimum_required (VERSION 3.20)

set (APP_NAME grey)

add_compile_definitions(X_EXPORT)

find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
#find_package(fmt CONFIG REQUIRED)

find_path(STB_INCLUDE_DIRS "stb.h")

file(GLOB core_src CONFIGURE_DEPENDS "*.cpp"
    "backends/*.cpp"
    "common/*.cpp"
    "common/*.cpp"
    "common/win32/*.cpp"
    "3rdparty/imgui-node-editor/*.cpp"
    "3rdparty/ImGuiColorTextEdit/*.cpp")
set(private_libs D3D11.lib implot::implot)
set(interface_libs imgui::imgui)

#add_library    (grey SHARED ${core_src} "api.cpp")
add_library    (libgrey STATIC ${core_src})

#set_property(TARGET grey PROPERTY
#  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#set_property(TARGET libgrey PROPERTY
#  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# dynamic lib should include both private and interface libs as private dependencies
#target_link_libraries(grey        
#   PRIVATE ${private_libs} ${interface_libs})

target_link_libraries(libgrey
   PRIVATE ${private_libs}
   INTERFACE ${interface_libs})

# target_include_directories(${APP_NAME} PRIVATE "../common")
