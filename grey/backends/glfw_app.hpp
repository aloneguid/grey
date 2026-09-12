#if PLATFORM_LINUX || PLATFORM_MACOS
#pragma once
#include "../app.h"
#include "imgui_impl_glfw.h"
#include "../common/ui_window.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>

namespace grey::backends {
    namespace w = widgets;

    static void glfw_error_callback(int error, const char* description) {
        std::fprintf(stderr, "GLFW Error %d: %s\n", error, description ? description : "");
    }

    class glfw_app : public app {
    public:
        glfw_app(const std::string& title, sz initial_size, GLFWerrorfun error_callback = glfw_error_callback)
            : app{title, initial_size}, title{title}, window_logical_size{initial_size} {
            ::glfwSetErrorCallback(error_callback);
            glfw_ready = glfwInit();
            if(!glfw_ready)
                return;

            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            widgets::scale = monitor ? ImGui_ImplGlfw_GetContentScaleForMonitor(monitor) : 1.0f;
        }

        ~glfw_app() override {
            terminate_glfw();
        }

        static point get_screen_center(const sz& physical_size) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            if(!monitor)
                return {};

            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            if(!mode)
                return {};

            int mx, my;
            glfwGetMonitorPos(monitor, &mx, &my);
            return point{
                (mode->width - physical_size.width) / 2 + mx,
                (mode->height - physical_size.height) / 2 + my
            };
        }

        void resize(const sz size) override {
            window_logical_size = size;
            if(!window)
                return;

            const sz physical_size = size * w::scale;
            if(center_on_screen) {
                window_pos = get_screen_center(physical_size);
                glfwSetWindowPos(window, static_cast<int>(window_pos.x), static_cast<int>(window_pos.y));
            }
            glfwSetWindowSize(window, static_cast<int>(physical_size.width), static_cast<int>(physical_size.height));
        }

        void move(point pos) override {
            window_pos = pos;
            if(window) {
                const point physical_pos = pos * w::scale;
                glfwSetWindowPos(window, static_cast<int>(physical_pos.x), static_cast<int>(physical_pos.y));
            }
        }

        void center() override {
            if(window) {
                const sz physical_size = window_logical_size * w::scale;
                window_pos = get_screen_center(physical_size);
                glfwSetWindowPos(window, static_cast<int>(window_pos.x), static_cast<int>(window_pos.y));
            }
        }

        void foreground() override {
            if(window)
                glfwFocusWindow(window);
        }

    protected:
        bool create_window() {
            if(!glfw_ready)
                return false;

            // GLFW delegates the native title bar, frame, shadows, and rounded corners to the
            // active window manager/compositor (including the different Ubuntu X11/Wayland paths).
            // Every non-native chrome mode remains explicitly undecorated.
            const int decorated = chrome == system_chrome::native ? GLFW_TRUE : GLFW_FALSE;
            glfwWindowHint(GLFW_DECORATED, decorated);
            glfwWindowHint(GLFW_FLOATING, always_on_top ? GLFW_TRUE : GLFW_FALSE);
            if(use_transparency_colour_key_value)
                glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

            const sz physical_size = window_logical_size * w::scale;
            if(center_on_screen)
                window_pos = get_screen_center(physical_size);

            window = glfwCreateWindow(
                static_cast<int>(physical_size.width),
                static_cast<int>(physical_size.height),
                title.c_str(), nullptr, nullptr);
            if(!window)
                return false;

            if(center_on_screen)
                glfwSetWindowPos(window, static_cast<int>(window_pos.x), static_cast<int>(window_pos.y));

            apply_transparency();
            return true;
        }

        void apply_transparency() {
            if(!window)
                return;

            const int alpha = std::clamp(transparency_window_alpha, 0, 255);
            if(alpha == last_transparency_window_alpha)
                return;

            last_transparency_window_alpha = alpha;
            const common::ui_window w{window};
            w.opacity(static_cast<float>(alpha) / 255.0f);
        }

        void terminate_glfw() {
            if(!glfw_ready)
                return;

            glfwTerminate();
            glfw_ready = false;
        }

        GLFWwindow* window{nullptr};
        bool glfw_ready{false};
        std::string title;
        point window_pos{-1, -1};
        sz window_logical_size;
        int last_transparency_window_alpha{255};
    };
}

#endif