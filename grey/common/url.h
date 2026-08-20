#pragma once
#include <string>
#include <vector>

namespace grey::common {
    class url {
    public:
        explicit url(const std::string& abs_url);

        /**
         * The original absolute URL.
         */
        std::string abs_url;

        /**
         * Protocol part of the URL, not including "://".
         */
        std::string protocol;

        /**
         * Host part of the URL.
         */
        std::string host;

        /**
         * Query part of the URL, including "?".
         */
        std::string query;

        /**
         * Query part of the URL, without "?" and before that
         */
        std::string query_without_parameters;

        /**
         * Parameters in the query part of the URL.
         */
        std::vector<std::pair<std::string, std::string>> parameters;

        std::string to_string();

        /**
         * Opens the given URL in the default web browser.
         */
        static void open_in_browser(const std::string& url);
    };
}