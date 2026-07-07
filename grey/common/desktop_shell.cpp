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
}
