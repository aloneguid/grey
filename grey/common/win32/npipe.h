#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <functional>

namespace grey::common::win32 {
    class npipe {
    public:
        npipe(std::string name);

        ~npipe();

        /// <summary>
        ///
        /// </summary>
        /// <param name="callback">Callback function for message received. When false is returned, the connection is closed and listening stops.</param>
        /// <returns></returns>
        bool listen(std::function<bool(const std::vector<char> &)> callback);

        bool send(const std::vector<char> &data);

        bool send(const std::string &s);

    private:
        std::string name;
        HANDLE hPipe;
    };
}
