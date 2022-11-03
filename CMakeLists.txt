cmake_minimum_required (VERSION 3.8)

set (APP_NAME grey)

add_compile_definitions(X_EXPORT)

find_package(imgui CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
#find_package(implot CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_path(STB_INCLUDE_DIRS "stb.h")

#file(GLOB core_src CONFIGURE_DEPENDS "*.cpp" "backends/*.cpp" "3rdparty/imnodes/*.cpp" "common/*.cpp" "../common/*.cpp" "../common/win32/user.cpp")
file(GLOB core_src CONFIGURE_DEPENDS "*.cpp" "backends/*.cpp" "common/*.cpp" "../common/*.cpp" "../common/win32/user.cpp" "../common/win32/gdi.cpp")
set(private_libs fmt::fmt-header-only D3D11.lib nlohmann_json::nlohmann_json)
set(interface_libs imgui::imgui)

#add_executable (samples     ${core_src} "samples/launcher.cpp")
#add_executable (popup       ${core_src} "samples/popup.cpp")
#add_executable (cputracker  ${core_src} "samples/cputracker.cpp")
#add_executable (puredom     ${core_src} "samples/puredom.cpp")
#add_executable (mind        ${core_src} "samples/mind.cpp")
add_library    (grey SHARED ${core_src} "api.cpp")
add_library    (libgrey STATIC ${core_src})

set_property(TARGET grey PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET libgrey PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")



#target_link_libraries(samples     PRIVATE ${libs})
#target_link_libraries(popup       PRIVATE ${libs})
#target_link_libraries(cputracker  PRIVATE ${libs})
#target_link_libraries(mind        PRIVATE ${libs})
#target_link_libraries(puredom     PRIVATE ${libs})

# dynamic lib should include both private and interface libs as private dependencies
target_link_libraries(grey        
   PRIVATE ${private_libs} ${interface_libs})

target_link_libraries(libgrey
   PRIVATE ${private_libs}
   INTERFACE ${interface_libs})

