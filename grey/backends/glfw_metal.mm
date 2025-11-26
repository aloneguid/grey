// Metal + GLFW backend for macOS
// Based on ImGui's example_glfw_metal example

#ifdef __APPLE__
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <Cocoa/Cocoa.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "../app.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"
#include <chrono>
#include <thread>
#include <stdio.h>
#include <functional>

namespace grey::backends {
    using namespace std;

    static void glfw_error_callback(int error, const char* description) {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }

    class glfw_metal_app : public grey::app {

    public:
        glfw_metal_app(const std::string& title, int width, int height, float scale)
            : grey::app{scale}, title{title}, window_width{width}, window_height{height} {
            last_frame_time = std::chrono::high_resolution_clock::now();
        }

        void run(std::function<bool(const app& app)> render_frame) {
            @autoreleasepool {
                glfwSetErrorCallback(glfw_error_callback);
                if (!glfwInit())
                    return;

                // Create window with graphics context
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                window = glfwCreateWindow(window_width, window_height, title.c_str(), nullptr, nullptr);
                if (window == nullptr)
                    return;

                device = MTLCreateSystemDefaultDevice();
                commandQueue = [device newCommandQueue];

                // Setup Platform/Renderer backends
                ImGui_ImplGlfw_InitForOther(window, true);
                ImGui_ImplMetal_Init(device);

                NSWindow* nswin = glfwGetCocoaWindow(window);
                layer = [CAMetalLayer layer];
                layer.device = device;
                layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
                nswin.contentView.layer = layer;
                nswin.contentView.wantsLayer = YES;

                MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor new];

                // Setup Dear ImGui context
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
                //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
                //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
                // Note: ViewportsEnable is problematic with Metal on macOS, disable for now
                //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

                // Setup Dear ImGui style
                ImGui::StyleColorsDark();

                // one time initialisation
                on_after_initialised();

                float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

                // Main loop
                bool done = false;
                while (!glfwWindowShouldClose(window) && !done)
                {
                    @autoreleasepool
                    {
                        glfwPollEvents();

                        int width, height;
                        glfwGetFramebufferSize(window, &width, &height);
                        layer.drawableSize = CGSizeMake(width, height);
                        id<CAMetalDrawable> drawable = [layer nextDrawable];

                        id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
                        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0] * clear_color[3], clear_color[1] * clear_color[3], clear_color[2] * clear_color[3], clear_color[3]);
                        renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
                        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
                        renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
                        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

                        // Start the Dear ImGui frame
                        ImGui_ImplMetal_NewFrame(renderPassDescriptor);
                        ImGui_ImplGlfw_NewFrame();
                        ImGui::NewFrame();

                        bool has_more = render_frame(*this);
                        if (!has_more)
                            done = true;

                        // Rendering
                        ImGui::Render();
                        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

                        [renderEncoder endEncoding];

                        [commandBuffer presentDrawable:drawable];
                        [commandBuffer commit];

                        // Manual FPS control
                        auto b_now = std::chrono::high_resolution_clock::now();
                        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(b_now - last_frame_time).count();
                        if(duration_ms < max_frame_interval_ms) {
                            float sleep_time = max_frame_interval_ms - duration_ms;
                            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time)));
                        }
                        last_frame_time = b_now;
                    }
                }

                // Cleanup
                ImGui_ImplMetal_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();

                glfwDestroyWindow(window);
                glfwTerminate();
            }
        }

        void resize_main_viewport(int width, int height) {
            if (window) {
                glfwSetWindowSize(window, width, height);
            }
        }

        void move_main_viewport(int x, int y) {
            if (window) {
                glfwSetWindowPos(window, x, y);
            }
        }

        void* make_native_texture(grey::common::raw_img& img) {
            if (!device || !img) return nullptr;

            MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
            desc.pixelFormat = MTLPixelFormatRGBA8Unorm;
            desc.width = img.x;
            desc.height = img.y;
            desc.usage = MTLTextureUsageShaderRead;

            id<MTLTexture> texture = [device newTextureWithDescriptor:desc];
            if (!texture) return nullptr;

            MTLRegion region = MTLRegionMake2D(0, 0, img.x, img.y);
            [texture replaceRegion:region mipmapLevel:0 withBytes:img.get_data() bytesPerRow:img.x * 4];

            return (__bridge_retained void*)texture;
        }

        void set_dark_mode(bool enabled) {
            NSWindow* nswin = glfwGetCocoaWindow(window);
            if (nswin) {
                if (enabled) {
                    nswin.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
                } else {
                    nswin.appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
                }
            }
        }

    private:
        GLFWwindow* window{nullptr};
        id<MTLDevice> device{nil};
        id<MTLCommandQueue> commandQueue{nil};
        CAMetalLayer* layer{nil};
        string title;
        int window_width{-1};
        int window_height{-1};
        std::chrono::time_point<std::chrono::high_resolution_clock> last_frame_time;
    };
}

// Factory function to create the Metal app (C++ linkage)
std::unique_ptr<grey::app> create_glfw_metal_app(const std::string& title, int width, int height, float scale) {
    return std::make_unique<grey::backends::glfw_metal_app>(title, width, height, scale);
}

#endif // __APPLE__
