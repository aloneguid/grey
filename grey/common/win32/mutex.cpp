#include "mutex.h"
#include <windows.h>

using namespace std;

namespace grey::common::win32 {
    mutex::mutex(std::string name) {
        hMutex = ::CreateMutexA(0, true, name.c_str());
        auto err = ::GetLastError();
        is_owner = (hMutex != nullptr && err != ERROR_ALREADY_EXISTS);
    }

    mutex::~mutex() {
        if(hMutex) {
            ::ReleaseMutex(hMutex);
        }
    }
}
