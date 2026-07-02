#include "uwp.h"
#include <windows.h>
#include <wrl.h>
#include <windows.foundation.h>
#include <windows.management.deployment.h>
#include <shobjidl.h>  // For IApplicationActivationManager
#include "../str.h"

#pragma comment(lib, "WindowsApp.lib")

using namespace std;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Management::Deployment;
using namespace ABI::Windows::System;

#define XF(x) \
{ \
    hr = (x); \
    if(FAILED(hr)) return false; \
}

namespace grey::common::win32 {


    uwp::uwp() {
        // Initialize COM
        HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        initialised = SUCCEEDED(hr);
    }

    uwp::~uwp() {
        if(initialised) {
            ::CoUninitialize();
        }
    }

    bool uwp::start_app(const std::wstring& app_user_mode_id, const std::wstring& arg) {

        // Create an instance of IApplicationActivationManager
        IApplicationActivationManager* appActivationManager = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&appActivationManager));
        if(FAILED(hr)) {
            return false;
        }

        // Activate the UWP application
        DWORD processId;
        hr = appActivationManager->ActivateApplication(
            app_user_mode_id.c_str(),
            arg.empty() ? nullptr : arg.c_str(),
            AO_NONE,
            &processId);
        appActivationManager->Release();

        return SUCCEEDED(hr);
    }

    bool uwp::open_url(const std::wstring& app_user_mode_id, const std::wstring& uri) {

        // Create an instance of IApplicationActivationManager
        IApplicationActivationManager* appActivationManager = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&appActivationManager));
        if(FAILED(hr)) {
            return false;
        }

        // populate second argument (uri)
        IShellItem* shellItem;
        hr = SHCreateItemFromParsingName(uri.c_str(), nullptr, IID_PPV_ARGS(&shellItem));
        IShellItemArray* shellItemArray;
        hr = SHCreateShellItemArrayFromShellItem(shellItem, IID_PPV_ARGS(&shellItemArray));

        // Activate the UWP application for the protocol
        DWORD processId;
        hr = appActivationManager->ActivateForProtocol(app_user_mode_id.c_str(), shellItemArray, &processId);
        appActivationManager->Release();

        return SUCCEEDED(hr);
    }

    bool uwp::open_file(const std::wstring& app_user_mode_id, const std::wstring& file_path) {
        // Create an instance of IApplicationActivationManager
        IApplicationActivationManager* appActivationManager = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&appActivationManager));
        if(FAILED(hr)) {
            return false;
        }

        // populate second argument (file_path)
        IShellItem* shellItem;
        hr = SHCreateItemFromParsingName(file_path.c_str(), nullptr, IID_PPV_ARGS(&shellItem));
        IShellItemArray* shellItemArray;
        hr = SHCreateShellItemArrayFromShellItem(shellItem, IID_PPV_ARGS(&shellItemArray));

        // Activate the UWP application for the file
        DWORD processId;
        hr = appActivationManager->ActivateForFile(app_user_mode_id.c_str(), shellItemArray, nullptr, &processId);
        appActivationManager->Release();

        return SUCCEEDED(hr);
    }

    bool uwp::launch_uri(const std::string& app_package_family_name, const std::string& uri) const {

        RoInitializeWrapper initialize(RO_INIT_SINGLETHREADED);
        if(FAILED(initialize)) {
            return false;
        }

        HRESULT hr;

        ComPtr<ILauncherStatics> launcherStatics;
        XF(GetActivationFactory(HStringReference(RuntimeClass_Windows_System_Launcher).Get(), &launcherStatics));

        ComPtr<IUriRuntimeClassFactory> uriFactory;
        XF(GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_Uri).Get(), &uriFactory));

        ComPtr<IUriRuntimeClass> com_uri;
        wstring w_uri{str::to_wstr(uri)};
        XF(uriFactory->CreateUri(HStringReference(w_uri.c_str()).Get(), &com_uri));

        ComPtr<IAsyncOperation<bool>> asyncOp;

        ComPtr<ILauncherOptions> options;
        XF(ActivateInstance(HStringReference(RuntimeClass_Windows_System_LauncherOptions).Get(), &options));

        // Query for ILauncherOptions2 to set additional options
        ComPtr<ILauncherOptions2> launcherOptions2;
        XF(options.As(&launcherOptions2));
        wstring w_app_package_family_name{str::to_wstr(app_package_family_name)};
        launcherOptions2->put_TargetApplicationPackageFamilyName(HStringReference(w_app_package_family_name.c_str()).Get());

        XF(launcherStatics->LaunchUriWithOptionsAsync(com_uri.Get(), options.Get(), &asyncOp));

        return true;
    }

}
