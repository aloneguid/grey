#pragma once

#include "../app.h"

#if PLATFORM_MACOS

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"
#include "../common/ui_window.h"

#include <Cocoa/Cocoa.h>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace grey::backends {
    namespace w = widgets;

    struct metal_texture final : texture {
        id<MTLTexture> texture_id;

        explicit metal_texture(id<MTLTexture> texture_id)
            : texture{(void*) texture_id}, texture_id{texture_id} {
        }

        ~metal_texture() override {
            [texture_id release];
            texture_id = nil;
            data = nullptr;
        }
    };

    static bool glfw_macos_monitor_available{false};

    static void glfw_metal_error_callback(int error, const char* description) {
        if(error == GLFW_PLATFORM_ERROR && !glfw_macos_monitor_available && description &&
            std::strcmp(description, "Cocoa: Cannot query workarea without screen") == 0)
            return;

        std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }

    class glfw_metal_app final : public app {
    public:
        glfw_metal_app(const std::string& title, sz size)
            : app{title}, title{title}, window_logical_size{size} {
            ::glfwSetErrorCallback(glfw_metal_error_callback);
            if(!glfwInit())
                return;

            glfw_ready = true;
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            NSArray* screens = [NSScreen screens];
            int monitor_count = 0;
            GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
            monitor_available = monitor && monitors && monitor_count > 0 && screens.count > 0;
            for(int i = 0; monitor_available && i < monitor_count; ++i) {
                const CGDirectDisplayID display_id = glfwGetCocoaMonitor(monitors[i]);
                bool screen_found = false;
                for(NSScreen* screen in screens) {
                    NSNumber* screen_number = [screen.deviceDescription objectForKey:@"NSScreenNumber"];
                    if(screen_number && screen_number.unsignedIntValue == display_id) {
                        screen_found = true;
                        break;
                    }
                }
                monitor_available = screen_found;
            }
            widgets::scale = monitor_available ? ImGui_ImplGlfw_GetContentScaleForMonitor(monitor) : 1.0f;
            glfw_macos_monitor_available = monitor_available;
        }

        ~glfw_metal_app() override = default;

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

        void run(std::function<bool()> render_frame) override {
            if(!glfw_ready)
                return;

            glfwWindowHint(GLFW_DECORATED, show_title_bar ? GLFW_TRUE : GLFW_FALSE);
            glfwWindowHint(GLFW_FLOATING, always_on_top ? GLFW_TRUE : GLFW_FALSE);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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
                return;

            if(center_on_screen)
                glfwSetWindowPos(window, static_cast<int>(window_pos.x), static_cast<int>(window_pos.y));

            apply_transparency();

            device = MTLCreateSystemDefaultDevice();
            if(!device) {
                glfwDestroyWindow(window);
                window = nullptr;
                glfwTerminate();
                glfw_ready = false;
                return;
            }

            command_queue = [device newCommandQueue];
            if(!command_queue) {
                [device release];
                device = nil;
                glfwDestroyWindow(window);
                window = nullptr;
                glfwTerminate();
                glfw_ready = false;
                return;
            }

            NSWindow* native_window = glfwGetCocoaWindow(window);
            monitor_available = monitor_available && native_window.screen != nil;
            glfw_macos_monitor_available = monitor_available;
            NSView* content_view = native_window.contentView;
            [content_view setWantsLayer:YES];
            content_view.autoresizingMask |= NSViewWidthSizable | NSViewHeightSizable;
            metal_layer = [CAMetalLayer layer];
            metal_layer.device = device;
            metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
            metal_layer.frame = content_view.bounds;
            metal_layer.displaySyncEnabled = YES;
            content_view.layer = metal_layer;
            update_drawable_size();

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
            if(monitor_available) {
                io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
                io.ConfigViewportsNoAutoMerge = true;
                io.ConfigViewportsNoTaskBarIcon = true;
                io.ConfigViewportsNoDefaultParent = false;
            }

            ImGui::StyleColorsDark();
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(w::scale);
            style.FontScaleDpi = w::scale;
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 3
            io.ConfigDpiScaleFonts = true;
            io.ConfigDpiScaleViewports = true;
#endif
            if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                style.WindowRounding = 0.0f;
                if(!use_transparency_colour_key_value)
                    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            ImGui_ImplGlfw_InitForOther(window, true);
            ImGui_ImplMetal_Init(device);
            on_after_initialised();

            bool done = false;
            while(!glfwWindowShouldClose(window) && !done) {
                apply_transparency();
                glfwPollEvents();
                update_drawable_size();
                if(glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
                    ImGui_ImplGlfw_Sleep(10);
                    continue;
                }

                id<CAMetalDrawable> drawable = [metal_layer nextDrawable];
                if(!drawable)
                    continue;

                MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
                const auto clear_color = get_clear_color();
                MTLRenderPassColorAttachmentDescriptor* color = pass.colorAttachments[0];
                color.texture = drawable.texture;
                color.loadAction = MTLLoadActionClear;
                color.storeAction = MTLStoreActionStore;
                color.clearColor = MTLClearColorMake(
                    clear_color[0], clear_color[1], clear_color[2], clear_color[3]);

                ImGui_ImplGlfw_NewFrame();
                ImGui_ImplMetal_NewFrame(pass);
                ImGui::NewFrame();
                if(!render_main_window(render_frame))
                    done = true;

                ImGui::Render();
                id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
                id<MTLRenderCommandEncoder> command_encoder = [command_buffer
                    renderCommandEncoderWithDescriptor:pass];
                ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), command_buffer, command_encoder);
                [command_encoder endEncoding];

                if(monitor_available && (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) {
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                }

                [command_buffer presentDrawable:drawable];
                [command_buffer commit];

                fps_pause();
            }

            ImGui_ImplMetal_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            metal_layer = nil;
            [command_queue release];
            command_queue = nil;
            [device release];
            device = nil;
            glfwDestroyWindow(window);
            window = nullptr;
            glfwTerminate();
            glfw_ready = false;
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
            update_drawable_size();
        }

        void move(point pos) override {
            window_pos = pos;
            if(window)
                glfwSetWindowPos(window, static_cast<int>(pos.x * w::scale), static_cast<int>(pos.y * w::scale));
        }

        void foreground() override {
            if(window)
                glfwFocusWindow(window);
        }

        std::shared_ptr<texture> make_native_texture(grey::common::raw_img& img) override {
            if(!device)
                return nullptr;

            MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
                texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                width:img.x height:img.y mipmapped:NO];
            descriptor.usage = MTLTextureUsageShaderRead;
            id<MTLTexture> image_texture = [device newTextureWithDescriptor:descriptor];
            if(!image_texture)
                return nullptr;

            MTLRegion region = MTLRegionMake2D(0, 0, img.x, img.y);
            [image_texture replaceRegion:region mipmapLevel:0 withBytes:img.get_data() bytesPerRow:img.x * 4];
            return std::make_shared<metal_texture>(image_texture);
        }

        void set_dark_mode(bool) override {
        }

    private:
        void update_drawable_size() {
            if(!window || !metal_layer)
                return;

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            metal_layer.drawableSize = CGSizeMake(width, height);
        }

        GLFWwindow* window{nullptr};
        bool glfw_ready{false};
        bool monitor_available{false};
        std::string title;
        point window_pos{-1, -1};
        sz window_logical_size;
        int last_transparency_window_alpha{255};
        id<MTLDevice> device{nil};
        id<MTLCommandQueue> command_queue{nil};
        CAMetalLayer* metal_layer{nil};
    };
}

#endif