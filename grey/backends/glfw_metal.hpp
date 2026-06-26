#pragma once
#include "../app.h"

namespace grey::backends {
    using namespace std;

    class glfw_metal_app : public grey::app {
    public:
        glfw_metal_app(const std::string& title, int width, int height)
            : title{title}, window_width{width}, window_height{height} {


        }
        ~glfw_metal_app() override {
        }

        void run(std::function<bool(const app &)> render_frame) override {
        }

        void resize_main_viewport(int width, int height) override {
        }

        void move_main_viewport(int x, int y) override {
        }

        void foreground_main_viewport() override {

        }

    protected:
        std::shared_ptr<texture> make_native_texture(grey::common::raw_img &img) override {
            return nullptr;
        }

        void set_dark_mode(bool enabled) override {

        }

    private:
        string title;
        int window_width;
        int window_height;
    };
}