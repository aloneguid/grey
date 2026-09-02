#include "url.h"
#include "str.h"
#include <format>
#include "platform.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace grey::common {
    using namespace std;

    url::url(const std::string& abs_url) {
        parse(abs_url);
    }

    std::string url::to_string() const {
        std::string out;
        out.reserve(scheme.size() + host.size() + port.size() + path.size() + query.size() + 8);

        const bool has_host = !host.empty();

        // UNC paths (\\server\share\...) parse with a non-empty host but no
        // protocol, same as protocol-relative URLs (//host/path) -- the two
        // are told apart here by which separator style `path` already uses.
        const bool looks_like_unc = !path.empty() && path.front() == '\\';

        if (!scheme.empty()) {
            out += scheme;
            out += (has_host ? "://" : ":"); // "https://" vs opaque "mailto:"
        } else if (has_host) {
            out += (looks_like_unc ? "\\\\" : "//"); // UNC vs protocol-relative
        }

        if (has_host) {
            out += host;
            if (!port.empty()) {
                out += ':';
                out += port;
            }
        }

        out += path;

        std::string effective_query = !query.empty()
                                          ? query
                                          : parameters_to_string();
        if (!effective_query.empty()) {
            out += '?';
            out += effective_query;
        }

        return out;
    }

    void url::open_in_browser(const std::string& url) {
#if PLATFORM_WINDOWS
        ::ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
        pid_t pid = fork();
        if(pid == 0) {
            // Child process
            execlp("xdg-open", "xdg-open", url.c_str(), (char *) nullptr);
            _exit(127); // exec failed
        }
        // Parent: don't wait, let it run detached (or waitpid if you want to check success)
#endif
    }

    void url::parse(const std::string& abs_url) {
        if(abs_url.empty()) return;

        string_view sv{abs_url};

        // Option 1. scheme://authority/path?query#fragment
        size_t scheme_sep = sv.find("://");
        if(scheme_sep != string_view::npos) {
            scheme = sv.substr(0, scheme_sep);
            size_t cursor = scheme_sep + 3;
            size_t authority_end = sv.find_first_of("/?#", cursor);
            std::string_view authority = (authority_end == std::string_view::npos)
                                             ? sv.substr(cursor)
                                             : sv.substr(cursor, authority_end - cursor);
            parse_authority(authority);

            size_t restStart = (authority_end == std::string_view::npos) ? sv.size() : authority_end;
            extract_path_and_query(sv.substr(restStart));
            return;
        }

        // Option 2. //authority/path?query#fragment  (protocol-relative)
        if(sv.size() >= 2 && sv[0] == '/' && sv[1] == '/') {
            size_t cursor = 2;
            size_t authorityEnd = sv.find_first_of("/?#", cursor);
            std::string_view authority = (authorityEnd == std::string_view::npos)
                                             ? sv.substr(cursor)
                                             : sv.substr(cursor, authorityEnd - cursor);
            parse_authority(authority);

            size_t restStart = (authorityEnd == std::string_view::npos) ? sv.size() : authorityEnd;
            extract_path_and_query(sv.substr(restStart));
            return;
        }

        // Option 3. Windows drive-letter path: C:\..., C:/... ---
        if(is_windows_drive_path(sv)) {
            path = sv; // native path kept as-is (backslashes preserved)
            return;
        }

        // Option 4. UNC path: \\server\share\... ---
        if(sv.size() >= 2 && sv[0] == '\\' && sv[1] == '\\') {
            const size_t cursor = 2;
            const size_t server_end = sv.find_first_of("\\/", cursor);
            host = (server_end == std::string_view::npos)
                       ? sv.substr(cursor)
                       : sv.substr(cursor, server_end - cursor);
            path = (server_end == std::string_view::npos) ? std::string_view{} : sv.substr(server_end);
            return;
        }

        // Option 5. Rooted path with no scheme: /usr/local/bin, \Users\x ---
        if(sv[0] == '/' || sv[0] == '\\') {
            extract_path_and_query(sv); // still honor a trailing "?query" if present
            return;
        }

        // Option 6. scheme:opaque-data  (mailto:, tel:, urn:, data:, ...) ---
        // A lone ':' with a leading alpha char and everything up to it being
        // legal scheme characters, and NOT already caught by the drive-letter
        // check above (so this is 2+ characters before the colon).
        {
            const size_t colon_pos = sv.find(':');
            if(colon_pos != std::string_view::npos && colon_pos >= 2 &&
               std::isalpha(static_cast<unsigned char>(sv[0]))) {
                bool validScheme = true;
                for(size_t i = 1; i < colon_pos; ++i) {
                    if(!is_scheme_char(sv[i])) {
                        validScheme = false;
                        break;
                    }
                }
                if(validScheme) {
                    scheme = sv.substr(0, colon_pos);
                    extract_path_and_query(sv.substr(colon_pos + 1));
                    return;
                }
            }
        }

        // Option 7. Fallback: relative path (possibly with a "?query" tail) ---
        extract_path_and_query(sv);
    }

    void url::parse_authority(std::string_view authority) {
        size_t at_pos = authority.rfind('@');
        std::string_view hostport = (at_pos == std::string_view::npos)
                                        ? authority
                                        : authority.substr(at_pos + 1);

        if(!hostport.empty() && hostport.front() == '[') {
            size_t close_bracket = hostport.find(']');
            if(close_bracket != std::string_view::npos) {
                host = hostport.substr(0, close_bracket + 1);
                size_t colon_pos = hostport.find(':', close_bracket);
                if(colon_pos != std::string_view::npos) {
                    port = hostport.substr(colon_pos + 1);
                }
            } else {
                host = hostport; // malformed bracket, best effort
            }
            return;
        }

        size_t colon_pos = hostport.find(':');
        if(colon_pos == std::string_view::npos) {
            host = hostport;
        } else {
            host = hostport.substr(0, colon_pos);
            port = hostport.substr(colon_pos + 1);
        }
    }

    void url::extract_path_and_query(std::string_view rest) {
        size_t split_pos = rest.find_first_of("?#");
        path = (split_pos == std::string_view::npos) ? rest : rest.substr(0, split_pos);

        if(split_pos != std::string_view::npos && rest[split_pos] == '?') {
            size_t query_start = split_pos + 1;
            size_t frag_pos = rest.find('#', query_start);
            std::string_view queryView = (frag_pos == std::string_view::npos)
                                             ? rest.substr(query_start)
                                             : rest.substr(query_start, frag_pos - query_start);
            query = queryView;
            parse_parameters(queryView);
        }
    }

    void url::parse_parameters(std::string_view query_view) {
        size_t pos = 0;
        while(pos < query_view.size()) {
            size_t amp = query_view.find('&', pos);
            std::string_view pair = (amp == std::string_view::npos)
                                        ? query_view.substr(pos)
                                        : query_view.substr(pos, amp - pos);

            if(!pair.empty()) {
                const size_t eq = pair.find('=');
                if(eq == std::string_view::npos) {
                    parameters.emplace(percent_decode(pair), std::string{});
                } else {
                    parameters.emplace(percent_decode(pair.substr(0, eq)),
                                       percent_decode(pair.substr(eq + 1)));
                }
            }

            if(amp == std::string_view::npos) break;
            pos = amp + 1;
        }
    }

    std::string url::percent_decode(const std::string_view in) {
        std::string out;
        out.reserve(in.size());

        auto hexVal = [](char c) -> int {
            if(c >= '0' && c <= '9') return c - '0';
            if(c >= 'a' && c <= 'f') return c - 'a' + 10;
            if(c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };

        for(size_t i = 0; i < in.size(); ++i) {
            char c = in[i];
            if(c == '%' && i + 2 < in.size()) {
                const int hi = hexVal(in[i + 1]);
                if(const int lo = hexVal(in[i + 2]); hi >= 0 && lo >= 0) {
                    out.push_back(static_cast<char>((hi << 4) | lo));
                    i += 2;
                    continue;
                }
                out.push_back(c); // malformed escape, pass through literally
            } else if(c == '+') {
                out.push_back(' ');
            } else {
                out.push_back(c);
            }
        }
        return out;
    }

    bool url::is_windows_drive_path(std::string_view sv) {
        return sv.size() >= 2 &&
               std::isalpha(static_cast<unsigned char>(sv[0])) &&
               sv[1] == ':' &&
               (sv.size() == 2 || sv[2] == '\\' || sv[2] == '/');
    }

    bool url::is_scheme_char(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '+' || c == '-' || c == '.';
    }

    std::string url::encode_query_component(std::string_view in) {
        static const char* hexDigits = "0123456789ABCDEF";
        std::string out;
        out.reserve(in.size());

        for (unsigned char c : in) {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                out.push_back(static_cast<char>(c));
            } else if (c == ' ') {
                out.push_back('+');
            } else {
                out.push_back('%');
                out.push_back(hexDigits[c >> 4]);
                out.push_back(hexDigits[c & 0x0F]);
            }
        }
        return out;
    }

    std::string url::parameters_to_string() const {
        std::string out;
        bool first = true;
        for (const auto& [key, value] : parameters) {
            if (!first) out += '&';
            first = false;
            out += encode_query_component(key);
            if (!value.empty()) {
                out += '=';
                out += encode_query_component(value);
            }
        }
        return out;
    }
}
