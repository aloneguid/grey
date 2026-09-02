#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

namespace grey::common {
    class url {
    public:
        explicit url(const std::string& abs_url);

        /**
         * Scheme part of the URL e.g. "https".
         */
        std::string scheme;

        /**
         * Host e.g. "www.example.com" without port part.
         */
        std::string host;

        /**
         * Port e.g. "8080". Empty if not present.
         */
        std::string port;

        /**
         * Path e.g. "/path/to/resource", including leading '/'.
         */
        std::string path;

        /**
         * Raw query string, no leading '?'
         */
        std::string query;

        /**
         * Decoded key-value parameters
         */
        std::unordered_map<std::string, std::string> parameters;

        [[nodiscard]] std::string to_string() const;

        /**
         * Opens the given URL in the default web browser.
         */
        static void open_in_browser(const std::string& url);

    private:
        void parse(const std::string& abs_url);

        /**
         * Parse "host[:port]" (with optional "user:pass@" prefix and optional
         * "[ipv6]" bracket form) into host / port.
         * Any piece that doesn't fit is simply left empty.
         */
        void parse_authority(std::string_view authority);

        /**
         * Given the remainder of the input after any scheme/authority has been
         * consumed, split off "path" (up to '?' or '#') and, if present, "query"
         * (up to '#'), then decode query into parameters. Works equally well for
         * URL paths and for bare filesystem paths that happen to carry a trailing
         * "?query" (harmless no-op if there's no '?').
         */
        void extract_path_and_query(std::string_view rest);

        /**
         * Split "key=value&key2=value2" into the parameters map. Tolerant of
         * malformed pairs (missing '=', empty segments, stray '&'s). O(n) single pass.
         */
        void parse_parameters(std::string_view query);

        /**
         * Decode %XX and '+' (only meaningful in query strings) into a real std::string.
         * O(k) in the length of the input token. Malformed escapes are passed through
         * literally rather than treated as errors.
         */
        static std::string percent_decode(std::string_view in);

        /**
         * Windows drive-letter path: "C:", "C:\...", "C:/...".
         * Deliberately checked BEFORE generic scheme detection because a single
         * letter followed by ':' is (in practice) always a drive letter, never a
         * real URI scheme.
         */
        static bool is_windows_drive_path(std::string_view sv);

        /**
         * True for characters legal in a URI scheme token (after the first letter).
         */
        static bool is_scheme_char(char c);

        static std::string encode_query_component(std::string_view in);

        [[nodiscard]] std::string parameters_to_string() const;
    };
}
