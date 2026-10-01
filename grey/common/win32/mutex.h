#pragma once
#include <string>

namespace grey::common::win32 {
    class mutex {
    public:
        mutex(std::string name);

        ~mutex();

        bool is_owner;

    private:
        void *hMutex;
    };
}
