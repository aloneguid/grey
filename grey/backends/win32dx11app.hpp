#pragma once
#if _WIN32
#include "../app.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dwmapi.h>
#include "../common/str.h"
#if GREY_INCLUDE_IMPLOT
#include "implot.h"
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace grey::backends {

    using namespace std;

    // Helpers
    static LPCWSTR g_firstIconName{nullptr};
    BOOL CALLBACK EnumIconsProc(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam) {
        g_firstIconName = lpName;
        return FALSE;
    }

    void FindFirstIcon() {
        HMODULE hModule = ::GetModuleHandle(nullptr);
        ::EnumResourceNames(hModule, RT_GROUP_ICON, EnumIconsProc, 0);
    }

    // Data
    static ID3D11Device* g_pd3dDevice = nullptr;
    static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    static IDXGISwapChain* g_pSwapChain = nullptr;
    static bool                     g_SwapChainOccluded = false;
    static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
    static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
    static bool g_win32_close_on_focus_lost{false};

    // Forward declarations of helper functions
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Helper functions
    bool CreateDeviceD3D(HWND hWnd) {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0,};
        HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if(res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
            res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if(res != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    void CleanupDeviceD3D() {
        CleanupRenderTarget();
        if(g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
        if(g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
        if(g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    }

    void CreateRenderTarget() {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void CleanupRenderTarget() {
        if(g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    }

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

    // Win32 message handler
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch(msg) {
            case WM_SIZE:
                if(wParam == SIZE_MINIMIZED)
                    return 0;
                g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
                g_ResizeHeight = (UINT)HIWORD(lParam);
                return 0;
            case WM_SYSCOMMAND:
                if((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                    return 0;
                break;
            case WM_CREATE:
                // load main icon and set as window icon
                {
                    FindFirstIcon();
                    if(g_firstIconName) {
                        HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
                        HICON hMainIcon = ::LoadIcon(hInstance, g_firstIconName);
                        if(hMainIcon) {
                            ::SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hMainIcon);
                        }
                    }
                }
                return 0;
            case WM_DESTROY:
                ::PostQuitMessage(0);
                return 0;
            case WM_DPICHANGED:
                if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports) {
                    //const int dpi = HIWORD(wParam);
                    //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
                    const RECT* suggested_rect = (RECT*)lParam;
                    ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
                }
                break;
            case WM_KILLFOCUS:
                if(g_win32_close_on_focus_lost) {
                    ::PostQuitMessage(0);
                }
                break;
            case WM_COPYDATA:
            {
                // get "this" pointer
                grey::app* me = reinterpret_cast<grey::app*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
                if(me && me->on_user_message) {
                    COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;
                    string message{(char*)pcds->lpData};
                    me->on_user_message((int)pcds->dwData, message);
                }
            }
            break;
        }
        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    class win32dx11app : public grey::app {
    private:
        HWND hwnd{0};

        const float ClearColorF4[4] = {
            ClearColor[0] * ClearColor[3],
            ClearColor[1] * ClearColor[3],
            ClearColor[2] * ClearColor[3],
            ClearColor[3]};

    public:

        string title;
        int window_left{-1};
        int window_top{-1};
        int window_width{-1};
        int window_height{-1};

        win32dx11app(const std::string& title, int width, int height, float scale) :
            grey::app{scale}, title{title}, window_width{width}, window_height{height} {

            if(window_width == -1 || window_height == -1) {
                window_width = window_height = CW_USEDEFAULT;
            } else {
                // apply scaling factor
                window_width = (int)(window_width * this->scale);
                window_height = (int)(window_height * this->scale);
            }
        }

        void set_dark_mode(bool enabled) {
            BOOL win32_immersive_dark_mode = enabled;
            ::DwmSetWindowAttribute(
                hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
                &win32_immersive_dark_mode, sizeof(win32_immersive_dark_mode));
        }

        void get_screen_center(int width, int height, int& x, int& y) {
            int sw = ::GetSystemMetrics(SM_CXFULLSCREEN);
            int sh = ::GetSystemMetrics(SM_CYFULLSCREEN);

            x = sw / 2 - width / 2;
            y = sh / 2 - height / 2;
        }

        void resize_main_viewport(int width, int height) {
            // apply scaling factor
            window_width = (int)(width * scale);
            window_height = (int)(height * scale);

            if(hwnd) {
                UINT uFlags{0};

                if(win32_center_on_screen) {
                    get_screen_center(window_width, window_height, window_left, window_top);
                } else {
                    uFlags |= SWP_NOMOVE;
                }

                ::SetWindowPos(hwnd, HWND_TOP, window_left, window_top, window_width, window_height, uFlags);
            }
        }

        void run(std::function<bool(const app& app)> render_frame) {
            // Create application window
            //ImGui_ImplWin32_EnableDpiAwareness();

            // apply win32 customisations
            g_win32_close_on_focus_lost = win32_close_on_focus_lost;

            wstring class_name = grey::common::str::to_wstr(win32_window_class_name);
            WNDCLASSEXW wc = {
                sizeof(wc),
                CS_CLASSDC,
                WndProc, 0L, 0L,
                GetModuleHandle(nullptr),
                nullptr, nullptr, nullptr, nullptr,
                class_name.c_str(),
                nullptr};

            ::RegisterClassExW(&wc);

            wstring w_title = grey::common::str::to_wstr(title);

            DWORD dwStyle = WS_OVERLAPPEDWINDOW;
            //DWORD dwStyle = WS_POPUP;
            if(!win32_can_resize) {
                // remove resize frame and maximize button
                dwStyle &= ~WS_THICKFRAME;
                dwStyle &= ~WS_MAXIMIZEBOX;
            }
            //if(can_minimise) dwStyle |= WS_MINIMIZEBOX;
            //dwStyle &= ~WS_CAPTION;
            //dwStyle &= ~WS_SYSMENU; // remove system menu
            //dwStyle &= ~WS_BORDER;

            if(win32_center_on_screen) {
                get_screen_center(window_width, window_height, window_left, window_top);
            } else {
                window_left = window_top = CW_USEDEFAULT;
            }

            hwnd = ::CreateWindowW(wc.lpszClassName,
                w_title.c_str(), dwStyle,
                window_left, window_top,
                window_width, window_height,
                nullptr, nullptr,
                wc.hInstance, nullptr);

            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

            if(win32_transparent) {
                // remove all styles, so that window has no title, borders etc.
                ::SetWindowLong(hwnd, GWL_STYLE, 0);

                // Set the layered window extended style
                LONG_PTR exStyle = ::GetWindowLongPtr(hwnd, GWL_EXSTYLE);
                if(!(exStyle & WS_EX_LAYERED)) {
                    ::SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
                }

                // Set the transparency level
                COLORREF crKey = RGB(ClearColor[0] * 255, ClearColor[1] * 255, ClearColor[2] * 255);
                ::SetLayeredWindowAttributes(hwnd, crKey, 0, LWA_COLORKEY);
            }

            // Initialize Direct3D
            if(!CreateDeviceD3D(hwnd)) {
                CleanupDeviceD3D();
                ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
                return;
            }

            // We don't want to show the window immediately until at least one frame is rendered,
            // Because it may result in unwanted visual artifacts while the window is being created.
            //::ShowWindow(hwnd, SW_SHOWNORMAL);
            //::UpdateWindow(hwnd);
            //::SetForegroundWindow(hwnd);    // just as an extra precaution

            if(win32_always_on_top) {
                ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
#if GREY_INCLUDE_IMPLOT
            ImPlot::CreateContext();
#endif
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
            //io.ConfigViewportsNoAutoMerge = true;
            //io.ConfigViewportsNoTaskBarIcon = true;
            //io.ConfigViewportsNoDefaultParent = true;
            //io.ConfigDockingAlwaysTabBar = true;
            //io.ConfigDockingTransparentPayload = true;
            //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
            //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsLight();

            // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
            ImGuiStyle& style = ImGui::GetStyle();
            if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            // Setup Platform/Renderer backends
            ImGui_ImplWin32_Init(hwnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

            // Load Fonts
            // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
            // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
            // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
            // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
            // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
            // - Read 'docs/FONTS.md' for more instructions and details.
            // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
            //io.Fonts->AddFontDefault();
            //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
            //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
            //IM_ASSERT(font != nullptr);

            // Our state
            //ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            //ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

            // Main loop
            bool done{false};
            bool shown{false};

            // one time initialisation
            on_after_initialised();

            while(!done) {

                // Poll and handle messages (inputs, window resize, etc.)
                // See the WndProc() function below for our to dispatch events to the Win32 backend.
                MSG msg;
                while(::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                    if(msg.message == WM_QUIT)
                        done = true;
                }
                if(done)
                    break;

                // Handle window being minimized or screen locked
                if(g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
                    ::Sleep(10);
                    continue;
                }
                g_SwapChainOccluded = false;

                // Handle window resize (we don't resize directly in the WM_SIZE handler)
                if(g_ResizeWidth != 0 && g_ResizeHeight != 0) {
                    CleanupRenderTarget();
                    g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
                    g_ResizeWidth = g_ResizeHeight = 0;
                    CreateRenderTarget();
                }

                // Start the Dear ImGui frame
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                bool has_more = render_frame(*this);
                if(!has_more) done = true;

                // Rendering
                ImGui::Render();
                g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
                g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, ClearColorF4);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

                // Update and Render additional Platform Windows
                if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                }

                // Present
                HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
                //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
                g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

                // show after first frame is rendered
                if(!shown) {
                    ::ShowWindow(hwnd, SW_SHOWNORMAL);
                    shown = true;
                }
            }

            // Cleanup
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
#if GREY_INCLUDE_IMPLOT
            ImPlot::DestroyContext();
#endif
            ImGui::DestroyContext();

            CleanupDeviceD3D();
            ::DestroyWindow(hwnd);
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        }

        void* make_native_texture(grey::common::raw_img& img) {
            D3D11_TEXTURE2D_DESC desc{0};
            desc.Width = img.x;
            desc.Height = img.y;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = 0;

            ID3D11Texture2D* pTexture = nullptr;
            D3D11_SUBRESOURCE_DATA subResource;
            subResource.pSysMem = img.get_data();
            subResource.SysMemPitch = desc.Width * 4;
            subResource.SysMemSlicePitch = 0;
            g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
            ID3D11ShaderResourceView* out_srv{nullptr};

            // Create texture view
            if(pTexture) {
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
                ZeroMemory(&srvDesc, sizeof(srvDesc));
                srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = desc.MipLevels;
                srvDesc.Texture2D.MostDetailedMip = 0;
                g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &out_srv);
                pTexture->Release();
            }

            return out_srv;
        }
    };
}
#endif