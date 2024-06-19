cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(APP_NAME demo)
set(APP_LONG_NAME "Desktop Demo")

file(GLOB core_src CONFIGURE_DEPENDS
    "*.cpp")
add_executable (${APP_NAME} WIN32 ${core_src} "main.cpp" main.rc)

target_compile_definitions(${APP_NAME} PRIVATE
    APP_SHORT_NAME="${APP_NAME}"
    APP_LONG_NAME="${APP_LONG_NAME}"
)

target_link_libraries(demo libgrey)
target_include_directories(${APP_NAME} PRIVATE "../../grey")

target_sources(${APP_NAME} PRIVATE dpi-aware.manifest)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:wmainCRTStartup")

# include .pdb symbol generation
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" AND CMAKE_BUILD_TYPE MATCHES "Release")
   target_compile_options(${APP_NAME} PRIVATE /Zi)

   # Tell linker to include symbol data
    set_target_properties(${APP_NAME} PROPERTIES 
        LINK_FLAGS "/INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF"
    )

    # Set file name & location
    set_target_properties(${APP_NAME} PROPERTIES 
        COMPILE_PDB_NAME ${APP_NAME} 
        COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR}
    )
endif()