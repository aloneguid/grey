#pragma once

#include "glfw_app.hpp"

#if PLATFORM_MACOS

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "imgui.h"
#include "imgui_impl_metal.h"

#include <Cocoa/Cocoa.h>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

#include <cstdio>
#include <cstring>

namespace grey::backends {
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

    static void glfw_metal_error_callback(int error, const char* description) {
        if(error == GLFW_PLATFORM_ERROR && description &&
            std::strcmp(description, "Cocoa: Cannot query workarea without screen") == 0)
            return;

        std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }

    class glfw_metal_app final : public glfw_app {
    public:
        glfw_metal_app(const std::string& title, sz size)
            : glfw_app{title, size, glfw_metal_error_callback} {
            if(!glfw_ready)
                return;

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        ~glfw_metal_app() override = default;

        void run(std::function<bool()> render_frame) override {
            if(!create_window())
                return;

            device = MTLCreateSystemDefaultDevice();
            if(!device) {
                glfwDestroyWindow(window);
                window = nullptr;
                terminate_glfw();
                return;
            }

            command_queue = [device newCommandQueue];
            if(!command_queue) {
                [device release];
                device = nil;
                glfwDestroyWindow(window);
                window = nullptr;
                terminate_glfw();
                return;
            }

            NSWindow* native_window = glfwGetCocoaWindow(window);
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
            MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
            io.ConfigViewportsNoAutoMerge = true;
            io.ConfigViewportsNoTaskBarIcon = true;
            io.ConfigViewportsNoDefaultParent = false;

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

                const auto clear_color = get_clear_color();
                MTLRenderPassColorAttachmentDescriptor* color = pass.colorAttachments[0];
                color.texture = drawable.texture;
                color.loadAction = MTLLoadActionClear;
                color.storeAction = MTLStoreActionStore;
                color.clearColor = MTLClearColorMake(
                    clear_color[0], clear_color[1], clear_color[2], clear_color[3]);

                id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
                id<MTLRenderCommandEncoder> command_encoder = [command_buffer
                    renderCommandEncoderWithDescriptor:pass];
                ImGui_ImplMetal_NewFrame(pass);
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                if(!render_main_window(render_frame))
                    done = true;

                ImGui::Render();
                ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), command_buffer, command_encoder);

                if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                }

                [command_encoder endEncoding];
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
            terminate_glfw();
        }

        void resize(const sz size) override {
            glfw_app::resize(size);
            update_drawable_size();
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

        void update_drawable_size() {
            if(!window || !metal_layer)
                return;

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            metal_layer.drawableSize = CGSizeMake(width, height);
        }

        id<MTLDevice> device{nil};
        id<MTLCommandQueue> command_queue{nil};
        CAMetalLayer* metal_layer{nil};
    };
}

#endif