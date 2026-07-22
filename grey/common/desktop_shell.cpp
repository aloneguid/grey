#include "desktop_shell.h"
#include "platform.h"
#include "str.h"
#include <iostream>

#if PLATFORM_WINDOWS
#include <windows.h>
#include <ShlObj_core.h>
#include <shellapi.h>
#include "win32/CDialogEventHandler.hpp"
#elif PLATFORM_LINUX || PLATFORM_MACOS
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstdio>
#endif

using namespace std;

namespace grey::common {
#if PLATFORM_LINUX || PLATFORM_MACOS
    static std::string run_command(const std::string& cmd) {
        std::string out;
        if (FILE* p = popen(cmd.c_str(), "r")) {
            char buf[1024];
            while (fgets(buf, sizeof(buf), p)) out += buf;
            pclose(p);
        }
        // remove trailing newline
        if (!out.empty() && out.back() == '\n') {
            out.pop_back();
        }
        return out;
    }

    static bool has_command(const std::string& cmd) {
        std::string check = "which " + cmd + " > /dev/null 2>&1";
        return system(check.c_str()) == 0;
    }
#endif

    unsigned int desktop_shell::get_current_monitor_dpi() {
#if PLATFORM_WINDOWS
        return ::GetDpiForSystem();
#endif

        return 96;
    }

#if PLATFORM_WINDOWS
    void open_mssettings(const std::string name) {
        string url("ms-settings:");
        url += name;

        HINSTANCE hi = ::ShellExecute(
            nullptr,
            L"open",
            str::to_wstr(url).c_str(),
            nullptr,
            nullptr,
            SW_SHOWDEFAULT);
    }
#endif

    void desktop_shell::open_default_apps_configuration() {
#if PLATFORM_WINDOWS
        open_mssettings("defaultapps");
#endif
    }

#if PLATFORM_WINDOWS
    std::string desktop_shell::file_open_dialog(const std::string &file_type_name, const std::string &extension) {
        // see https://learn.microsoft.com/en-us/windows/win32/shell/common-file-dialog#basic-usage

        string path;

        IFileDialog *pfd = NULL;
        HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
        if(SUCCEEDED(hr)) {
            // Create an event handling object, and hook it up to the dialog.
            IFileDialogEvents *pfde = NULL;
            hr = ::CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
            if(SUCCEEDED(hr)) {
                // Hook up the event handler.
                DWORD dwCookie;
                hr = pfd->Advise(pfde, &dwCookie);
                if(SUCCEEDED(hr)) {
                    // Set the options on the dialog.
                    DWORD dwFlags;

                    // Before setting, always get the options first in order
                    // not to override existing options.
                    hr = pfd->GetOptions(&dwFlags);
                    if(SUCCEEDED(hr)) {
                        // In this case, get shell items only for file system items.
                        hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
                        if(SUCCEEDED(hr)) {
                            // Set the file types to display only.
                            // Notice that this is a 1-based array.

                            wstring flt_n = str::to_wstr(file_type_name);
                            wstring flt_x = str::to_wstr(extension);
                            COMDLG_FILTERSPEC rgSpec[] = {
                                {flt_n.c_str(), flt_x.c_str()}
                            };

                            hr = pfd->SetFileTypes(1, rgSpec);
                            if(SUCCEEDED(hr)) {
                                // Set the selected file type index to Word Docs for this example.
                                hr = pfd->SetFileTypeIndex(1);
                                if(SUCCEEDED(hr)) {
                                    // Set the default extension to be ".doc" file.
                                    hr = pfd->SetDefaultExtension(flt_x.c_str());
                                    if(SUCCEEDED(hr)) {
                                        // Show the dialog
                                        hr = pfd->Show(NULL);
                                        if(SUCCEEDED(hr)) {
                                            // Obtain the result once the user clicks
                                            // the 'Open' button.
                                            // The result is an IShellItem object.
                                            IShellItem *psiResult;
                                            hr = pfd->GetResult(&psiResult);
                                            if(SUCCEEDED(hr)) {
                                                // We are just going to print out the
                                                // name of the file for sample sake.
                                                PWSTR pszFilePath = NULL;
                                                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH,
                                                                               &pszFilePath);
                                                if(SUCCEEDED(hr)) {
                                                    // user have made a positive selection here
                                                    path = str::to_str(pszFilePath);
                                                    CoTaskMemFree(pszFilePath);
                                                }
                                                psiResult->Release();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // Unhook the event handler.
                    pfd->Unadvise(dwCookie);
                }
            }
        }

        return path;
    }

    std::string desktop_shell::directory_open_dialog() {
        string path;

        IFileDialog *pfd = NULL;
        HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
        if(SUCCEEDED(hr)) {
            // Create an event handling object, and hook it up to the dialog.
            IFileDialogEvents *pfde = NULL;
            hr = ::CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
            if(SUCCEEDED(hr)) {
                // Hook up the event handler.
                DWORD dwCookie;
                hr = pfd->Advise(pfde, &dwCookie);
                if(SUCCEEDED(hr)) {
                    // Set the options on the dialog.
                    DWORD dwFlags;

                    // Before setting, always get the options first in order
                    // not to override existing options.
                    hr = pfd->GetOptions(&dwFlags);
                    if(SUCCEEDED(hr)) {
                        // In this case, get shell items only for file system items.
                        // And pick folders only.
                        hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS);
                        if(SUCCEEDED(hr)) {
                            // Show the dialog
                            hr = pfd->Show(NULL);
                            if(SUCCEEDED(hr)) {
                                // Obtain the result once the user clicks
                                // the 'Open' button.
                                // The result is an IShellItem object.
                                IShellItem *psiResult;
                                hr = pfd->GetResult(&psiResult);
                                if(SUCCEEDED(hr)) {
                                    PWSTR pszFilePath = NULL;
                                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH,
                                                                   &pszFilePath);
                                    if(SUCCEEDED(hr)) {
                                        // user have made a positive selection here
                                        path = str::to_str(pszFilePath);
                                        CoTaskMemFree(pszFilePath);
                                    }
                                    psiResult->Release();
                                }
                            }
                        }
                    }
                    // Unhook the event handler.
                    pfd->Unadvise(dwCookie);
                }
            }
        }

        return path;
    }

#elif PLATFORM_LINUX || PLATFORM_MACOS
    std::string desktop_shell::file_open_dialog(const std::string &file_type_name, const std::string &extension) {
        std::string extensions = extension;
        str::replace_all(extensions, ";", " ");

        if (has_command("zenity")) {
            std::string cmd = "zenity --file-selection --title=\"Open File\" --file-filter=\"" + file_type_name + " | " + extensions + "\" 2>/dev/null";
            return run_command(cmd);
        }

        if (has_command("kdialog")) {
            std::string cmd = "kdialog --getopenfilename . \"" + extensions + "\" 2>/dev/null";
            return run_command(cmd);
        }

#if PLATFORM_MACOS
        return run_command("osascript -e 'POSIX path of (choose file with prompt \"Open File\")' 2>/dev/null");
#endif

        return "";
    }

    std::string desktop_shell::directory_open_dialog() {
        if (has_command("zenity")) {
            return run_command("zenity --file-selection --directory --title=\"Select Folder\" 2>/dev/null");
        }

        if (has_command("kdialog")) {
            return run_command("kdialog --getexistingdirectory . \"Select Folder\" 2>/dev/null");
        }

#if PLATFORM_MACOS
        return run_command("osascript -e 'POSIX path of (choose folder with prompt \"Select Folder\")' 2>/dev/null");
#endif

        return "";
    }
#else
    std::string desktop_shell::file_open_dialog(const std::string &file_type_name, const std::string &extension) {
        return "";
    }

    std::string desktop_shell::directory_open_dialog() {
        return "";
    }
#endif

    bool desktop_shell::file_open_dialog_supported() {
#if PLATFORM_WINDOWS
        return true;
#elif PLATFORM_LINUX || PLATFORM_MACOS
        return has_command("zenity") || has_command("kdialog") || PLATFORM_MACOS;
#else
        return false;
#endif
    }

    bool desktop_shell::directory_open_dialog_supported() {
#if PLATFORM_WINDOWS
        return true;
#elif PLATFORM_LINUX || PLATFORM_MACOS
        return has_command("zenity") || has_command("kdialog") || PLATFORM_MACOS;
#else
        return false;
#endif
    }

    bool desktop_shell::open(const std::string& path, const std::string& args) {
#if PLATFORM_WINDOWS
        // ShellExecuteW explicitly — avoids ShellExecute macro's ANSI/Wide ambiguity.
        // nullptr for args when empty; some handlers mishandle L"".
        HINSTANCE hi = ::ShellExecuteW(
            nullptr,
            L"open",
            str::to_wstr(path).c_str(),
            args.empty() ? nullptr : str::to_wstr(args).c_str(),
            nullptr,
            SW_SHOWDEFAULT);
        // HINSTANCE is a pointer type; cast to INT_PTR before comparing to integer.
        return (INT_PTR)hi > 32;

#elif PLATFORM_LINUX || PLATFORM_MACOS
        pid_t pid = fork();

        if (pid < 0) {
            // Fork failed
            std::cerr << "Failed to fork process." << std::endl;
            return false;
        }

        if (pid == 0) {
            // Inside Child Process

            // Fork a second time to orphan the execution process.
            // This prevents "zombie" processes without making the parent call waitpid().
            pid_t grandchild_pid = fork();

            if (grandchild_pid == 0) {
                // Inside Grandchild Process
                // execlp replaces the current process image
#if PLATFORM_LINUX
                execlp("xdg-open", "xdg-open", path.c_str(), nullptr);
#elif PLATFORM_MACOS
                execlp("open", "open", path.c_str(), nullptr);
#endif

                // If execlp returns, it means it failed to execute
                std::cerr << "Failed to execute open utility" << std::endl;
                _exit(1);
            }

            // Child exits immediately, orphaning the grandchild.
            // init/systemd will automatically reap the grandchild.
            _exit(0);
        }

        // Inside Parent Process
        // Wait for the immediate child to exit (which happens instantly).
        int status;
        waitpid(pid, &status, 0);
        return true;

#else
        return false;
#endif
    }

}
