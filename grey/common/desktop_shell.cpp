#include "desktop_shell.h"
#include "platform.h"
#include "str.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#include <ShlObj_core.h>
#include <shellapi.h>
#include "win32/CDialogEventHandler.hpp"
#endif

using namespace std;

namespace grey::common {
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

#else
    std::string desktop_shell::file_open_dialog(const std::string &file_type_name, const std::string &extension) {
        return "";
    }

#endif

    bool desktop_shell::file_open_dialog_supported() {
#if PLATFORM_WINDOWS
        return true;
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

#elif PLATFORM_LINUX
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
                // execlp replaces the current process image with xdg-open
                execlp("xdg-open", "xdg-open", path.c_str(), nullptr);

                // If execlp returns, it means it failed to execute
                std::cerr << "Failed to execute xdg-open" << std::endl;
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
