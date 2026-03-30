#ifndef _WIN32
#pragma once

// OpenGL3 + GLFW backend, which is cross-platform theoretically, but we will only use it for non-Windows platforms,
// because we will use DirectX 11 + Win32 for much better OS integration on Windows.

#include "../app.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <chrono>
#include <thread>
#include <vector>
#include <cstdlib>
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#if defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#include "macos_cgl.h"
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

namespace grey::backends {
    using namespace std;

    static void glfw_error_callback(int error, const char* description) {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }

    static char* g_glsl_version{nullptr};

    static bool gl_init() {
        glfwSetErrorCallback(glfw_error_callback);
        if(!glfwInit())
            return false;

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
        g_glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
        g_glsl_version = "#version 300 es";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
        g_glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
        g_glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

        return true;
    }

    /**
     * @brief https://github.com/ocornut/imgui/blob/docking/examples/example_glfw_opengl3/main.cpp
     */
    class glfw_gl3_app : public grey::app {

    public:
        glfw_gl3_app(const std::string& title, int width, int height)
            : title{title}, window_width{width}, window_height{height} {
            last_frame_time = std::chrono::high_resolution_clock::now();
            gl_init();
        }

        void run(std::function<bool(const app& app)> render_frame) {
            // Compute scale from primary monitor (works regardless of client API)
            GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
            float main_scale = primary_monitor
                ? ImGui_ImplGlfw_GetContentScaleForMonitor(primary_monitor)
                : 1.0f;
            int eff_w = (int)(window_width * main_scale);
            int eff_h = (int)(window_height * main_scale);

            // Attempt 1: GL 3.2 Core (normal macOS / Linux path)
            window = glfwCreateWindow(eff_w, eff_h, title.c_str(), nullptr, nullptr);

#if defined(__APPLE__)
            bool using_cgl_fallback = false;

            if(window == nullptr) {
                // Attempt 2: GL 2.1 legacy via NSGL — still uses NSOpenGLPFAAccelerated
                // internally, so may fail on CI, but worth a try.
                glfwDefaultWindowHints();
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
                g_glsl_version = "#version 120";
                window = glfwCreateWindow(eff_w, eff_h, title.c_str(), nullptr, nullptr);
            }

            if(window == nullptr) {
                // Attempt 3: CGL software-rendering fallback.
                // Create a bare GLFW window (no GL context) and attach a
                // manually-created NSOpenGLContext that does NOT request
                // NSOpenGLPFAAccelerated — the CI display session always has a
                // software renderer available.
                glfwDefaultWindowHints();
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                g_glsl_version = "#version 120";
                window = glfwCreateWindow(eff_w, eff_h, title.c_str(), nullptr, nullptr);
                if(window != nullptr) {
                    void* ns_win = (void*)glfwGetCocoaWindow(window);
                    if(grey_macos_create_software_gl_context(ns_win)) {
                        using_cgl_fallback = true;
                    } else {
                        glfwDestroyWindow(window);
                        window = nullptr;
                    }
                }
            }
#endif
            if(window == nullptr)
                return;

            // For the normal path GLFW manages the GL context; for the CGL
            // fallback the context is already current via macos_cgl.mm.
#if defined(__APPLE__)
            if(!using_cgl_fallback) {
                glfwMakeContextCurrent(window);
                glfwSwapInterval(1);
            }
#else
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1); // Enable vsync
#endif

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
            //io.ConfigViewportsNoAutoMerge = true;
            //io.ConfigViewportsNoTaskBarIcon = true;

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsLight();

            // Setup scaling
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(main_scale);
            style.FontScaleDpi = main_scale;
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 3
            io.ConfigDpiScaleFonts = true;
            io.ConfigDpiScaleViewports = true;
#endif

             // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
            if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            // Setup Platform/Renderer backends
#if defined(__APPLE__)
            if(using_cgl_fallback) {
                ImGui_ImplGlfw_InitForOther(window, true);
                // Disable multi-viewport: extra platform windows need their own
                // GL contexts, which is complex in the CGL fallback path.
                io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
            } else {
                ImGui_ImplGlfw_InitForOpenGL(window, true);
            }
#else
            ImGui_ImplGlfw_InitForOpenGL(window, true);
#endif
#ifdef __EMSCRIPTEN__
            ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
            ImGui_ImplOpenGL3_Init(g_glsl_version);


            // Load Fonts
           // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
           // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
           // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
           // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
           // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
           // - Read 'docs/FONTS.md' for more instructions and details.
           // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
           // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
           //io.Fonts->AddFontDefault();
           //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
           //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
           //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
           //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
           //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
           //IM_ASSERT(font != nullptr);

           // Our state
            bool show_demo_window = true;
            bool show_another_window = false;
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

            // one time initialisation
            on_after_initialised();

            // Main loop
            bool done{false};
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
            io.IniFilename = nullptr;
            EMSCRIPTEN_MAINLOOP_BEGIN
#else
            while(!glfwWindowShouldClose(window) && !done)
#endif
            {
                // Poll and handle events (inputs, window resize, etc.)
                // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
                // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
                // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
                // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
                glfwPollEvents();
                if(glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
                    ImGui_ImplGlfw_Sleep(10);
                    continue;
                }

                // Start the Dear ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                bool has_more = render_frame(*this);
                if(!has_more)
                    done = true;

                // Rendering
                ImGui::Render();
                int display_w, display_h;
                glfwGetFramebufferSize(window, &display_w, &display_h);
                glViewport(0, 0, display_w, display_h);
                glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
                glClear(GL_COLOR_BUFFER_BIT);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                // In-app screenshot: if GREY_SCREENSHOT_PPM env var is set,
                // capture the framebuffer to a PPM file after the first N rendered frames.
                // sips (macOS) can convert PPM→PNG: sips -s format png file.ppm --out file.png
                {
                    static const int kScreenshotFrame = 200; // ~5 s at 40 FPS
                    static int s_frame_count = 0;
                    ++s_frame_count;
                    const char* ppm_path = std::getenv("GREY_SCREENSHOT_PPM");
                    if(ppm_path && s_frame_count == kScreenshotFrame && display_w > 0 && display_h > 0) {
                        std::vector<unsigned char> px(display_w * display_h * 3);
                        glPixelStorei(GL_PACK_ALIGNMENT, 1);
                        glReadPixels(0, 0, display_w, display_h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
                        // OpenGL reads bottom-to-top; flip vertically for PPM (top-to-bottom)
                        for(int row = 0; row < display_h / 2; row++) {
                            int opp = display_h - 1 - row;
                            std::swap_ranges(
                                px.begin() + row * display_w * 3,
                                px.begin() + (row + 1) * display_w * 3,
                                px.begin() + opp * display_w * 3);
                        }
                        FILE* f = fopen(ppm_path, "wb");
                        if(f) {
                            fprintf(f, "P6\n%d %d\n255\n", display_w, display_h);
                            fwrite(px.data(), 1, px.size(), f);
                            fclose(f);
                            fprintf(stderr, "Grey: framebuffer screenshot saved to %s (%dx%d)\n",
                                ppm_path, display_w, display_h);
                        } else {
                            fprintf(stderr, "Grey: failed to open screenshot file: %s\n", ppm_path);
                        }
                    }
                }

                // Update and Render additional Platform Windows
                // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
                //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
                if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    GLFWwindow* backup_current_context = glfwGetCurrentContext();
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                    glfwMakeContextCurrent(backup_current_context);
                }

                // Manual FPS control
                // see how much time has passed since last render
                auto b_now = std::chrono::high_resolution_clock::now();
                auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(b_now - last_frame_time).count();
                if(duration_ms < max_frame_interval_ms) {
                    float sleep_time = max_frame_interval_ms - duration_ms;
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time)));
                }

#if defined(__APPLE__)
                if(using_cgl_fallback) {
                    grey_macos_gl_flush();
                } else {
                    glfwSwapBuffers(window);
                }
#else
                glfwSwapBuffers(window);
#endif

                last_frame_time = b_now;
            }
#ifdef __EMSCRIPTEN__
            EMSCRIPTEN_MAINLOOP_END;
#endif

            // Cleanup
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

#if defined(__APPLE__)
            if(using_cgl_fallback) {
                grey_macos_destroy_gl_context();
            }
#endif
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        void resize_main_viewport(int width, int height) {}

        void move_main_viewport(int x, int y) {}

        std::shared_ptr<texture> make_native_texture(grey::common::raw_img& img) {
            return nullptr;
        }

        void set_dark_mode(bool enabled) {
        }

    private:
        GLFWwindow* window{nullptr};
        string title;
        int window_left{-1};
        int window_top{-1};
        int window_width{-1};
        int window_height{-1};
        std::chrono::time_point<std::chrono::high_resolution_clock> last_frame_time;
    };
}
#endif