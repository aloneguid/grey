#include "os.h"
#include "reg.h"
#include <format>
#include "../str.h"
#include <Windows.h>
#include <VersionHelpers.h>
#include <combaseapi.h>

namespace grey::common::win32::os {
    using namespace std;

    std::string get_last_error_text() {
        wchar_t err[256];
        memset(err, 0, 256);
        ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
           NULL,
           GetLastError(),
           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 255, NULL);
        return str::to_str(wstring(err));
    }

    std::string get_computer_name() {
        TCHAR buffer[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

        ::GetComputerName(buffer, &size);

        return str::to_str(buffer);
    }

    std::string get_user_name() {
        TCHAR buffer[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

        ::GetUserName(buffer, &size);

        return str::to_str(buffer);
    }

    string pipe_read(HANDLE hReadPipe) {
        string result;
        vector<char> buffer;
        buffer.resize(1024);
        DWORD read{0};

        while(true) {

            bool read_ok = ::ReadFile(hReadPipe,
               (void*)&buffer[0], buffer.size(),
               &read, NULL);

            if(!read_ok || read == 0) {
                break;
            }

            std::copy(buffer.begin(), buffer.begin() + read, back_inserter(result));
        }

        return result;
    }

    int create_process(const std::string& cmdline,
       std::string& std_out,
       std::string& std_err,
       std::string& sys_error_text) {
        // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw

        STARTUPINFO si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{0};

        wchar_t rcmd[MAX_PATH];
        ::ZeroMemory(rcmd, sizeof(wchar_t) * MAX_PATH);
        wstring wcmdline = str::to_wstr(cmdline);
        memcpy(rcmd, &wcmdline[0], sizeof(wchar_t) * cmdline.size());

        // output redirection
        // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

        SECURITY_ATTRIBUTES saAttr{};
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        HANDLE hStdInRead, hStdInWrite;
        HANDLE hStdErrRead, hStdErrWrite;
        HANDLE hStdOutRead, hStdOutWrite;
        if(!::CreatePipe(&hStdInRead, &hStdInWrite, &saAttr, 0))
            return -1;
        if(!::CreatePipe(&hStdErrRead, &hStdErrWrite, &saAttr, 0))
            return -1;
        if(!::CreatePipe(&hStdOutRead, &hStdOutWrite, &saAttr, 0))
            return -1;

        if(!::SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0))
            return -2;
        if(!::SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0))
            return -2;
        if(!::SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0))
            return -2;

        si.hStdInput = hStdInRead;
        si.hStdError = hStdErrWrite;
        si.hStdOutput = hStdOutWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;

        // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
        BOOL created = ::CreateProcess(nullptr,
           rcmd,
           nullptr, nullptr,
           true,
           0,
           nullptr,
           nullptr,
           &si,
           &pi);

        if(created) {
            ::WaitForSingleObject(pi.hProcess, INFINITE);

            // not closing the handles before reading from out pipes causes read to hang at the end
            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);

            // close sides of the pipes we are not using
            ::CloseHandle(hStdErrWrite);
            ::CloseHandle(hStdOutWrite);
            ::CloseHandle(hStdInRead);

            std_out = pipe_read(hStdOutRead);
            std_err = pipe_read(hStdErrRead);

            DWORD exit_code{0};
            ::GetExitCodeProcess(pi.hProcess, &exit_code);
            return static_cast<int>(exit_code);
        } else {
            sys_error_text = get_last_error_text();
        }

        return -3;
    }

    double get_cpu_usage_perc() {
        FILETIME idleTime, kernelTime, userTime;
        if(GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
            ULARGE_INTEGER idleTimeU, kernelTimeU, userTimeU;
            idleTimeU.LowPart = idleTime.dwLowDateTime;
            idleTimeU.HighPart = idleTime.dwHighDateTime;
            kernelTimeU.LowPart = kernelTime.dwLowDateTime;
            kernelTimeU.HighPart = kernelTime.dwHighDateTime;
            userTimeU.LowPart = userTime.dwLowDateTime;
            userTimeU.HighPart = userTime.dwHighDateTime;

            ULONGLONG systemTime = kernelTimeU.QuadPart + userTimeU.QuadPart;
            ULONGLONG idleTime = idleTimeU.QuadPart;

            static ULONGLONG prevSystemTime = 0;
            static ULONGLONG prevIdleTime = 0;

            ULONGLONG systemTimeDiff = systemTime - prevSystemTime;
            ULONGLONG idleTimeDiff = idleTime - prevIdleTime;

            double cpuUsage = 100.0 - ((idleTimeDiff * 100.0) / systemTimeDiff);

            prevSystemTime = systemTime;
            prevIdleTime = idleTime;

            return cpuUsage;
        }
        return -1.0; // Failed to retrieve CPU usage
    }

    std::string machine_id() {
        // KLM\SOFTWARE\Microsoft\Cryptography\MachineGuid
        return win32::reg::get_value(win32::reg::hive::local_machine, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid");
    }

    bool is_windows11_or_greater() {
        if(!::IsWindows10OrGreater()) {
            return false;
        }

        // classic version APIs are manifest-dependent (without proper compatibility manifest, they can lie/degrade).
        const string build_number = win32::reg::get_value(
            win32::reg::hive::local_machine,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            "CurrentBuildNumber");

        if(build_number.empty()) {
            return false;
        }

        try {
            return stoi(build_number) >= 22000;
        } catch(...) {
            return false;
        }
    }

    string get_pressed_keys_text() {
        vector<string> pressed_keys;

        // get entire keyboard state
        BYTE keyboard_state[256];
        if(::GetKeyboardState(keyboard_state)) {
            for(int i = 0; i < 256; i++) {
                // "i" is VK itself
                if(keyboard_state[i] & 0x80) {
                    // key is pressed
                    
                    // modifier keys
                    if(i == VK_CONTROL) {
                        pressed_keys.push_back("Ctrl");
                    } else if(i == VK_MENU) {
                        pressed_keys.push_back("Alt");
                    } else if(i == VK_SHIFT) {
                        pressed_keys.push_back("Shift");
                    } else if(i >= '0' && i <= '9') {   // numbers
                        pressed_keys.push_back(string(1, static_cast<char>(i)));
                    } else if(i >= 'A' && i <= 'Z') {   // letters
                        pressed_keys.push_back(string(1, static_cast<char>(i)));
                    } else if(i >= VK_F1 && i <= VK_F12) { // F keys
                        pressed_keys.push_back(format("F{}", i - VK_F1 + 1));
                    } else {
                        // other keys
                    }
                }
            }
        }

        // return pressed_keys joined by "+" sign
        string r;
        for(string& s : pressed_keys) {
            if(!r.empty()) {
                r += "+";
            }
            r += s;
        }
        return r;
    }

    bool get_mouse_pos(long& x, long& y) {
        POINT pt;
        if(!::GetCursorPos(&pt)) return false;

        x = pt.x;
        y = pt.y;
        return true;
    }

    void set_clipboard_text(const std::string& text) {
        if(!::OpenClipboard(nullptr)) return;
        if(!::EmptyClipboard()) {
            ::CloseClipboard();
            return;
        }
        HGLOBAL gh = ::GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if(!gh) {
            ::CloseClipboard();
            return;
        }
        memcpy(::GlobalLock(gh), text.c_str(), text.size() + 1);
        ::GlobalUnlock(gh);
        HANDLE hResult = ::SetClipboardData(CF_TEXT, gh);
        ::CloseClipboard();
        if(!hResult) {
            ::GlobalFree(gh);
        }
    }

    std::string get_clipboard_text() {
        if(!::IsClipboardFormatAvailable(CF_UNICODETEXT)) return "";
        if(!OpenClipboard(nullptr)) return "";
        wstring r;

        HGLOBAL gh = ::GetClipboardData(CF_UNICODETEXT);
        if(gh != NULL) {
            LPWSTR lpstr = (LPWSTR)::GlobalLock(gh);
            if(lpstr != NULL) {

                r = lpstr;

                ::GlobalUnlock(lpstr);
            }
        }

        ::CloseClipboard();

        return str::to_str(r);
    }

    // RAII wrapper for screen capture GDI resources
    struct screen_capture_ctx {
        int w{0}, h{0};
        HDC hScreenDC{nullptr};
        HDC hMemDC{nullptr};
        HBITMAP hBitmap{nullptr};
        HGDIOBJ oldBmp{nullptr};
        bool valid{false};

        screen_capture_ctx(int src_x = 0, int src_y = 0, int src_w = 0, int src_h = 0) {
            // get monitor closest to mouse cursor
            POINT pt;
            ::GetCursorPos(&pt);
            HMONITOR hMon = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

            MONITORINFOEX mi{};
            mi.cbSize = sizeof(mi);
            ::GetMonitorInfo(hMon, &mi);

            // use sub-region if valid, otherwise full screen
            if (src_w > 0 && src_h > 0) {
                w = src_w;
                h = src_h;
            } else {
                src_x = mi.rcMonitor.left;
                src_y = mi.rcMonitor.top;
                w = mi.rcMonitor.right - mi.rcMonitor.left;
                h = mi.rcMonitor.bottom - mi.rcMonitor.top;
            }

            hScreenDC = ::GetDC(nullptr);
            if (!hScreenDC) return;

            hMemDC = ::CreateCompatibleDC(hScreenDC);
            if (!hMemDC) return;

            hBitmap = ::CreateCompatibleBitmap(hScreenDC, w, h);
            if (!hBitmap) return;

            oldBmp = ::SelectObject(hMemDC, hBitmap);

            if (::BitBlt(hMemDC, 0, 0, w, h, hScreenDC, src_x, src_y, SRCCOPY | CAPTUREBLT))
                valid = true;
        }

        ~screen_capture_ctx() {
            if (oldBmp && hMemDC) ::SelectObject(hMemDC, oldBmp);
            if (hBitmap) ::DeleteObject(hBitmap);
            if (hMemDC) ::DeleteDC(hMemDC);
            if (hScreenDC) ::ReleaseDC(nullptr, hScreenDC);
        }

        screen_capture_ctx(const screen_capture_ctx&) = delete;
        screen_capture_ctx& operator=(const screen_capture_ctx&) = delete;
    };

    bool capture_screen(int& width, int& height, std::vector<unsigned char>& out_pixels) {
        screen_capture_ctx ctx;
        if (!ctx.valid) return false;

        width = ctx.w;
        height = ctx.h;
        out_pixels.clear();

        // prepare BITMAPINFO for 24bpp
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = ctx.w;
        bmi.bmiHeader.biHeight = ctx.h; // positive -> bottom-up (BMP standard)
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 0; // can be 0 for BI_RGB

        // compute padded row size (each scanline padded to 4 bytes)
        int rowSize = ((ctx.w * 3 + 3) / 4) * 4;
        size_t pixelDataSize = static_cast<size_t>(rowSize) * static_cast<size_t>(ctx.h);

        std::vector<unsigned char> pixels;
        try {
            pixels.resize(pixelDataSize);
        } catch (...) {
            return false;
        }

        // retrieve bits into buffer
        int ret = ::GetDIBits(ctx.hMemDC, ctx.hBitmap, 0, ctx.h, pixels.data(), &bmi, DIB_RGB_COLORS);
        if (ret == 0) return false;

        // build BMP file header
        BITMAPFILEHEADER bfh{};
        bfh.bfType = 0x4D42; // 'BM'
        bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bfh.bfSize = static_cast<DWORD>(bfh.bfOffBits + pixelDataSize);
        bfh.bfReserved1 = 0;
        bfh.bfReserved2 = 0;

        // assemble output vector: file header + info header + pixel data
        out_pixels.resize(static_cast<size_t>(bfh.bfSize));
        unsigned char* p = out_pixels.data();

        // copy BITMAPFILEHEADER (14 bytes)
        memcpy(p, &bfh, sizeof(BITMAPFILEHEADER));
        p += sizeof(BITMAPFILEHEADER);

        // copy BITMAPINFOHEADER (40 bytes)
        memcpy(p, &bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
        p += sizeof(BITMAPINFOHEADER);

        // copy pixel data
        memcpy(p, pixels.data(), pixelDataSize);

        return true;
    }

    bool capture_screen_to_clipboard(int x, int y, int w, int h) {
        screen_capture_ctx ctx{x, y, w, h};
        if (!ctx.valid) return false;

        // prepare BITMAPINFO for 24bpp
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = ctx.w;
        bmi.bmiHeader.biHeight = ctx.h;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 0;

        int rowSize = ((ctx.w * 3 + 3) / 4) * 4;
        size_t pixelDataSize = static_cast<size_t>(rowSize) * static_cast<size_t>(ctx.h);

        // allocate global memory for CF_DIB: BITMAPINFOHEADER + pixel data
        size_t dibSize = sizeof(BITMAPINFOHEADER) + pixelDataSize;
        HGLOBAL hDib = ::GlobalAlloc(GMEM_MOVEABLE, dibSize);
        if (!hDib) return false;

        unsigned char* p = static_cast<unsigned char*>(::GlobalLock(hDib));
        if (!p) {
            ::GlobalFree(hDib);
            return false;
        }

        memcpy(p, &bmi.bmiHeader, sizeof(BITMAPINFOHEADER));

        int ret = ::GetDIBits(ctx.hMemDC, ctx.hBitmap, 0, ctx.h,
            p + sizeof(BITMAPINFOHEADER), &bmi, DIB_RGB_COLORS);
        ::GlobalUnlock(hDib);

        if (ret == 0) {
            ::GlobalFree(hDib);
            return false;
        }

        if (!::OpenClipboard(nullptr)) {
            ::GlobalFree(hDib);
            return false;
        }

        ::EmptyClipboard();
        HANDLE hResult = ::SetClipboardData(CF_DIB, hDib);
        ::CloseClipboard();

        if (!hResult) {
            ::GlobalFree(hDib);
            return false;
        }
        // clipboard owns hDib now

        return true;
    }
}