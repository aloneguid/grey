#pragma once
#include <string>
#include <map>
#include <vector>

namespace grey::common {
    class url {
    public:
        url(const std::string& abs_url);

        std::string abs_url;
        std::string protocol;
        std::string host;
        std::string query;
        std::string query_without_parameters;

        std::vector<std::pair<std::string, std::string>> parameters;

        std::string to_string();

        /**
         * Opens the given URL in the default web browser.
         */
        static void open_in_browser(const std::string& url);
    };
}