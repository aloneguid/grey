#pragma once
#include <string>
#include <map>
#include "platform.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#include <winhttp.h>
#endif

namespace grey::common {
    /**
     * @brief Minimal HTTP functions. On Windows uses WinHTTP to minimise dependencies.
     */
    class http {
    public:
        http();
        ~http();

        std::string get(const std::string& abs_url) const;

        int get_get_headers(const std::string& abs_url, std::map<std::string, std::string>& headers) const;

        void post(const std::string& domain, const std::string& abs_url, const std::string& data, bool is_async = false) const;

    private:
#if PLATFORM_WINDOWS
        HINTERNET hSession{0};

        bool disable_redirects(HINTERNET hRequest) const;
#endif
    };
}