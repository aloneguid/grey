#include <string>
#include <map>
#include "../backend.h"
#include <Windows.h>
#include "d3d11.h"
#include "../common/img.h"
#include "../common/win32/window.h"

namespace grey
{
    namespace backends
    {
        struct win32dx11_texture_tag {
            ID3D11ShaderResourceView* texture;
            size_t width;
            size_t height;
        };

        class win32dx11 : public backend {
        public:
            /// <summary>
            /// 
            /// </summary>
            /// <param name="title"></param>
            /// <param name="x"></param>
            /// <param name="y"></param>
            /// <param name="width"></param>
            /// <param name="height"></param>
            /// <param name="always_on_top"></param>
            /// <param name="dwStyle">https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles</param>
            win32dx11(const std::string& title);
            ~win32dx11();

            void run_one_frame();

            void run();

            virtual void* load_texture_from_file(
               const std::string& path, int& width, int& height);

            virtual void* load_texture_from_memory(
               const std::string& cache_name,
               unsigned char* buffer, unsigned int len, int& width, int& height);

            virtual void set_visibility(bool visible);

            virtual void resize(int width, int height);

            virtual void move(int x, int y);

            virtual void center_on_screen();

            virtual void set_title(const std::string& title);

            virtual void bring_to_top();

            virtual float get_system_scale();

            virtual void exit();

            virtual void set_theme(const std::string& theme_id);

            // instance specific

            // hwnd is initialised in the constructor and safe to use immediately after constructing the backend
            HWND get_hwnd() { return hwnd; }
            std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> on_unhandled_window_message;

            virtual void* get_native_window_handle() const override { return hwnd; }

            virtual void bring_native_window_to_top(void* raw_handle) override;

        private:
            const wchar_t* ClassName = L"GreyWin32";
            HINSTANCE h_module_inst;
            HWND hwnd{nullptr};
            ID3D11Device* dx_device{nullptr};
            ID3D11DeviceContext* dx_device_context{nullptr};
            IDXGISwapChain* dx_swap_chain{nullptr};
            bool dx_swap_chain_occluded{false};
            UINT resize_width{0}, resize_height{0};
            ID3D11RenderTargetView* dx_render_target_view{nullptr};
            static const int BorderSize{5};
            ImVec4 clear_color{0, 0, 0, 1.00f};
            bool owns_message_loop{false};
            size_t l{0}, t{0}, w{0}, h{0};
            float scale_cached{-1.0};

            // If you cache textures, they must be tied to the backend, not app-global. This is extremely
            // important. Reusing textures across backends results in them not shutting down properly.
            std::map<std::string, win32dx11_texture_tag> texture_cache;

            static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

            bool dx_create_device();
            void dx_cleanup_device();
            void dx_create_render_target();
            void dx_cleanup_render_target();

            ID3D11ShaderResourceView* image_data_to_texture(grey::common::raw_img& img) const;
        };
    }
}