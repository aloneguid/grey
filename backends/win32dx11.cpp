#include "win32dx11.h"
#include "../../common/str.h"
#include "../../common/win32/user.h"
#include "../../common/win32/shell.h"
#include <Windows.h>
#include <windowsx.h>

#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imconfig.h"

#include <iostream>

using namespace std;

grey::backends::win32dx11::win32dx11(const string& title, bool is_visible)
    : grey::backend(title, is_visible) {
    wc = {
          sizeof(WNDCLASSEX),
          CS_CLASSDC,
          WndProc,
          0L,
          0L,
          GetModuleHandle(nullptr),
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          L"GreyWin32",
          nullptr
    };

    if (!::RegisterClassEx(&wc))
        return;

    wstring wtitle = str::to_wstr(title);
    // 0  is WS_OVERLAPPED
    DWORD dwStyle = WS_POPUP;
    //DWORD dwExStyle = always_on_top ? WS_EX_TOPMOST : 0;
    DWORD dwExStyle = 0;
    hwnd = ::CreateWindowEx(dwExStyle,
        wc.lpszClassName,
        wtitle.c_str(),
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, NULL, wc.hInstance,

        // pass "this" pointer to WM_CREATE
        this);
    if (!hwnd) return;

    ::SetPropA(hwnd, "backend", this);

    // Initialize Direct3D
    if (!dx_create_device()) {
        dx_cleanup_device();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return;
    }

    // Show the window
    if (is_visible)
        ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // the below requires "docking" branch, see https://github.com/ocornut/imgui/wiki/Docking
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    //set_theme(theme);

    // apply scaling settings
    ImGuiStyle& style = ImGui::GetStyle();
    float scale = get_system_scale();
    style.ScaleAllSizes(scale);
    // don't apply font global scaling, they will look terrible on high DPI.
    // instead, when loading a font, apply scaling factor
    //io.FontGlobalScale = scale;
    io.DisplayFramebufferScale = {scale, scale};


    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(dx_device, dx_device_context);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Roboto-Regular.ttf", 16.0f);
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);
    /*io.Fonts->AddFontDefault();

    ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 16.0f, &icons_config, icon_ranges);*/

    // todo: can do freetype: https://github.com/ocornut/imgui/tree/master/misc/freetype
    //io.Fonts->AddFontFromMemoryCompressedTTF(
       //Roboto_compressed_data, Roboto_compressed_size, 15.0f);
    //io.Fonts->AddFontFromMemoryCompressedTTF(
       //NotoSans_compressed_data, NotoSans_compressed_size, 16.0f);

    //io.Fonts->AddFontFromFileTTF("fonts/fontawesome-webfont.ttf", 13.0f, &config, icon_ranges);

    // install custom settings handler
    //ImGui::ImGuiSettingsHandler handler;
    //ImGuiContext* ctx = ImGui::GetCurrentContext();
    //ctx->SettingsHandlers.push_back();
}

grey::backends::win32dx11::~win32dx11() {

    if (on_save_settings) {
        WINDOWPLACEMENT wp = { 0 };
        wp.length = sizeof(wp);
        ::GetWindowPlacement(hwnd, &wp);
        string data = str::base64_encode((const unsigned char*)&wp, sizeof(wp));
        on_save_settings("win32_wp", data);
    }

    // free textures
    for (auto& texture : texture_cache) {
        texture.second.texture->Release();
    }
    texture_cache.clear();

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    //ImPlot::DestroyContext();
    ImGui::DestroyContext();

    dx_cleanup_render_target();
    dx_cleanup_device();

    ::SendMessage(hwnd, WM_CLOSE, 0, 0);
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void grey::backends::win32dx11::run_one_frame() {
    // optimisation: don't do anything at all if there are no visible ImGui windows
    if (!any_window_visible()) return;

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // default renderer - render all the windows
    for (auto& wnd : windows) {
        wnd->render();
    }

    // Rendering
    ImGui::Render();
    const float clear_color_with_alpha[4] = {
        clear_color.x * clear_color.w,
        clear_color.y * clear_color.w,
        clear_color.z * clear_color.w,
        clear_color.w
    };

    dx_device_context->OMSetRenderTargets(1, &dx_render_target_view, nullptr);
    dx_device_context->ClearRenderTargetView(dx_render_target_view, clear_color_with_alpha);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    dx_swap_chain->Present(1, 0); // Present with vsync
    //dx_swap_chain->Present(0, 0); // Present without vsync
}

void grey::backends::win32dx11::run() {
    owns_message_loop = true;
    while (true) {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                return;
        }

        run_one_frame();
    }
}

ID3D11ShaderResourceView* grey::backends::win32dx11::image_data_to_texture(grey::img::raw_img& img) const
{
   D3D11_TEXTURE2D_DESC desc{ 0 };
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
   dx_device->CreateTexture2D(&desc, &subResource, &pTexture);
   ID3D11ShaderResourceView* out_srv{ nullptr };

   // Create texture view
   if (pTexture)
   {
      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
      ZeroMemory(&srvDesc, sizeof(srvDesc));
      srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MipLevels = desc.MipLevels;
      srvDesc.Texture2D.MostDetailedMip = 0;
      dx_device->CreateShaderResourceView(pTexture, &srvDesc, &out_srv);
      pTexture->Release();
   }

   return out_srv;
}

// https://github.com/hoffstadt/DearPyGui/tree/0e51794aeef487dd7ca5dcb2550c631131374222/DearPyGui/src/platform
void* grey::backends::win32dx11::load_texture_from_file(const std::string& path, int& width, int& height)
{
   auto entry = texture_cache.find(path);
   if (entry == texture_cache.end())
   {
      grey::img::raw_img img_data = img::load_image_from_file(path);
      if (!img_data) return nullptr;

      ID3D11ShaderResourceView* texture = image_data_to_texture(img_data);
      if (texture)
      {
         win32dx11_texture_tag tag{ texture, img_data.x, img_data.y };
         texture_cache[path] = tag;

         width = img_data.x;
         height = img_data.y;
         return texture;
      }
   }
   else
   {
      width = entry->second.width;
      height = entry->second.height;
      return entry->second.texture;
   }

   return nullptr;
}

void* grey::backends::win32dx11::load_texture_from_memory(
   const string& cache_name,
   unsigned char* buffer, unsigned int len, int& width, int& height)
{
   auto entry = texture_cache.find(cache_name);
   if (entry == texture_cache.end())
   {
      grey::img::raw_img img_data = img::load_image_from_memory(buffer, len);
      if (!img_data) return nullptr;

      ID3D11ShaderResourceView* texture = image_data_to_texture(img_data);
      if (texture)
      {
         win32dx11_texture_tag tag{ texture, img_data.x, img_data.y };
         texture_cache[cache_name] = tag;

         width = img_data.x;
         height = img_data.y;
         return texture;
      }
   }
   else
   {
      width = entry->second.width;
      height = entry->second.height;
      return entry->second.texture;
   }

   return nullptr;
}

void grey::backends::win32dx11::set_visibility(bool visible)
{
   if (visible)
      ::ShowWindow(hwnd, SW_SHOW);
   else
      ::ShowWindow(hwnd, SW_HIDE);

   is_visible = visible;
}

void grey::backends::win32dx11::resize(int width, int height)
{
    float scale = get_system_scale();
    win32::user::set_window_pos(hwnd, -1, -1, width * scale, height * scale);
}

void grey::backends::win32dx11::move(int x, int y)
{
   win32::user::set_window_pos(hwnd, x, y, -1, -1);
}

void grey::backends::win32dx11::center_on_screen()
{
   RECT wr{};
   if (::GetWindowRect(hwnd, &wr))
   {
      int sw = ::GetSystemMetrics(SM_CXFULLSCREEN);
      int sh = ::GetSystemMetrics(SM_CYFULLSCREEN);

      move(
         sw / 2 - (wr.right - wr.left) / 2,
         sh / 2 - (wr.bottom - wr.top) / 2);
   }
}

void grey::backends::win32dx11::set_title(const std::string& title)
{
   ::SetWindowTextA(hwnd, title.c_str());
}

void grey::backends::win32dx11::bring_to_top()
{
   ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

   ::BringWindowToTop(hwnd);
}

float grey::backends::win32dx11::get_system_scale()
{
    int dpi = win32::shell::get_dpi();
    float scale = dpi / 96.f;
    return scale;
}

void grey::backends::win32dx11::exit()
{
   ::DestroyWindow(hwnd);
   hwnd = nullptr;
   //is_running = false;
}

void grey::backends::win32dx11::set_theme(colour_theme theme) {

    if(theme == colour_theme::follow_os) {
        bool is_light{false};
        win32::user::is_app_light_theme(is_light);

        theme = is_light ? colour_theme::light : colour_theme::dark;
    }

    backend::set_theme(theme);
}

// Helper functions

bool grey::backends::win32dx11::dx_create_device()
{
   // Setup swap chain
   DXGI_SWAP_CHAIN_DESC sd;
   ::ZeroMemory(&sd, sizeof(sd));
   sd.BufferCount = 2;
   sd.BufferDesc.Width = 0;
   sd.BufferDesc.Height = 0;
   sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   sd.BufferDesc.RefreshRate.Numerator = 60;
   sd.BufferDesc.RefreshRate.Denominator = 1;
   sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
   sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   sd.OutputWindow = hwnd;
   sd.SampleDesc.Count = 1;
   sd.SampleDesc.Quality = 0;
   sd.Windowed = TRUE;
   sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

   UINT createDeviceFlags = 0;
   //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
   D3D_FEATURE_LEVEL featureLevel;
   const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
   if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &dx_swap_chain, &dx_device, &featureLevel, &dx_device_context) != S_OK)
      return false;

   dx_create_render_target();
   return true;
}

void grey::backends::win32dx11::dx_cleanup_device()
{
   dx_cleanup_render_target();
   if (dx_swap_chain) { dx_swap_chain->Release(); dx_swap_chain = nullptr; }
   if (dx_device_context) { dx_device_context->Release(); dx_device_context = nullptr; }

   if (dx_device)
   {
      dx_device->Release();
      dx_device = nullptr;
   }
}

void grey::backends::win32dx11::dx_create_render_target()
{
   ID3D11Texture2D* pBackBuffer{ nullptr };
   dx_swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
   if (pBackBuffer)
   {
      dx_device->CreateRenderTargetView(pBackBuffer, nullptr, &dx_render_target_view);
      pBackBuffer->Release();
   }
}

void grey::backends::win32dx11::dx_cleanup_render_target()
{
   if (dx_render_target_view)
   {
      dx_render_target_view->Release();
      dx_render_target_view = nullptr;
   }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI grey::backends::win32dx11::WndProc(
    HWND hWnd, UINT msg,
    WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    win32dx11* backend{ nullptr };

    switch (msg) {
        case WM_CREATE: {
            {
                // get a pointer to this backend
                CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
                backend = (win32dx11*)cs->lpCreateParams;
            }

            // load main icon and set as window icon
            {
                HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
                //HANDLE hMainIcon = ::LoadImage(hInstance, MAKEINTRESOURCE(1033), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
                HICON hMainIcon = ::LoadIcon(hInstance, L"IDI_ICON1");
                if (hMainIcon)
                    ::SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hMainIcon);
            }
            return 0;
        }

        case WM_SIZE: {
            if (!backend)
                backend = (win32dx11*)::GetPropA(hWnd, "backend");

            if (backend && backend->dx_device != nullptr && wParam != SIZE_MINIMIZED) {
                backend->dx_cleanup_render_target();
                backend->dx_swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                backend->dx_create_render_target();
            }
            return 0;
        }
        
        // https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-nchittest
        case WM_NCHITTEST: {
            if (!backend)
                backend = (win32dx11*)::GetPropA(hWnd, "backend");

            POINT p;
            if (::GetCursorPos(&p)) {
                if (::ScreenToClient(hWnd, &p)) {
                    //cout << p.x << "x" << p.y << endl;

                    // get window width and height
                    RECT window;
                    ::GetWindowRect(hWnd, &window);
                    int width = window.right - window.left;
                    int height = window.bottom - window.top;

                    if (backend->is_resizeable) {
                        bool at_left = p.x < BorderSize;
                        bool at_right = p.x > width - BorderSize;
                        bool at_top = p.y < BorderSize;
                        bool at_bottom = p.y > height - BorderSize;

                        if (at_left && at_top)
                            return HTTOPLEFT;
                        else if (at_top && at_right)
                            return HTTOPRIGHT;
                        else if (at_right && at_bottom)
                            return HTBOTTOMRIGHT;
                        else if (at_bottom && at_left)
                            return HTBOTTOMLEFT;
                        else if (at_left)
                            return HTLEFT;
                        else if (at_right)
                            return HTRIGHT;
                        else if (at_top)
                            return HTTOP;
                        else if (at_bottom)
                            return HTBOTTOM;
                    }

                    // might conflict with ImGui's "close" button
                    if (p.y <= (20 * backend->get_system_scale()) && p.x < width - 40) {
                        //cout << "cap!" << endl;
                        return HTCAPTION;
                    }

                }
            }

            return ::DefWindowProc(hWnd, msg, wParam, lParam);
        }

        case WM_SYSCOMMAND: {
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        }

        case WM_DESTROY: {
            if (!backend)
                backend = (win32dx11*)::GetPropA(hWnd, "backend");

            if (backend->owns_message_loop) {
                ::PostQuitMessage(0);
            }

            return 0;
        }

        default: {
            if (backend && backend->on_unhandled_window_message) {
                return backend->on_unhandled_window_message(hWnd, msg, wParam, lParam);
            }
        }
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}