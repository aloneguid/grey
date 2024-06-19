#include <string>
#include <vector>
#include <map>
#if WIN32
#include <windows.h>
#endif
#include <cmath>
#include <sstream>
#include <regex>
#include <algorithm>
#include "str.h"

using namespace std;

namespace grey::common::str {
#if WIN32
    std::wstring to_wstr(const std::string& str) {
        if (str.empty()) return std::wstring();
        int size_needed = ::MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
        std::wstring wstrTo(size_needed, 0);
        ::MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    std::string to_str(const std::wstring& wstr) {
        if (wstr.empty()) return std::string();
        int size_needed = ::WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string strTo(size_needed, 0);
        ::WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
        return strTo;
    }
#else
    std::wstring to_wstr(const std::string& str) {
        return L"";
    }

    std::string to_str(const std::wstring& wstr) {
        return "";
    }
#endif

    int to_int(const std::string& str) {
        return atoi(str.c_str());
    }

    unsigned long to_ulong(const std::string& str, int radix) {
        return strtoul(str.c_str(), nullptr, radix);
    }

    long long to_long_long(const std::string& str) {
        return atoll(str.c_str());
    }

    // trim from start (in place)
    void ltrim(std::string& s, const string& chars) {
        while (s.size() > 0) {
            size_t idx = chars.find(s[0]);
            if (idx == string::npos) return;
            s.erase(0, 1);
        }
    }

    // trim from end (in place)
    void rtrim(std::string& s, const string& chars) {
        while (s.size() > 0) {
            size_t idx = chars.find(*(s.rbegin()));
            if (idx == string::npos) return;
            s.erase(s.size() - 1, 1);
        }
    }

    void trim(std::string& s, const string& chars) {
        ltrim(s, chars);
        rtrim(s, chars);
    }

    void upper(std::string& s) {
        transform(s.begin(), s.end(), s.begin(), ::toupper);
    }

    void lower(std::string& s) {
        transform(s.begin(), s.end(), s.begin(), ::tolower);
    }

    void capitalize(std::string& s) {
        for (int i = 0; i < s.length(); i++) {
            s[i] = i == 0
                ? ::toupper(s[i])
                : ::tolower(s[i]);
        }
	}

    void unescape_special_chars(std::string& s) {
        replace_all(s, "\\r", "\r");
        replace_all(s, "\\t", "\t");
        replace_all(s, "\\b", "\b");
        replace_all(s, "\\n", "\n");
        replace_all(s, "\\f", "\f");
    }

    /// <summary>
    /// Replaces (in-place) all occurances of target with replacement. Taken from : http://stackoverflow.com/questions/3418231/c-replace-part-of-a-string-with-another-string.
    /// </summary>
    /// <param name="str"></param>
    /// <param name="target"></param>
    /// <param name="replecament"></param>
    /// <returns></returns>
    template<class T>
    bool replace_all(std::basic_string<T>& str, const std::basic_string<T>& target, const std::basic_string<T>& replacement) {
        if (target.empty()) {
            return false;
        }

        size_t start_pos = 0;
        const bool found_substring = str.find(target, start_pos) != std::string::npos;

        while ((start_pos = str.find(target, start_pos)) != std::string::npos) {
            str.replace(start_pos, target.length(), replacement);
            start_pos += replacement.length();
        }

        return found_substring;
    }

    std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool trim_lines) {
        std::vector<std::string> strings;

        std::string::size_type pos = 0;
        std::string::size_type prev = 0;
        while ((pos = str.find(delimiter, prev)) != std::string::npos) {
            string el = str.substr(prev, pos - prev);
            if (trim_lines) trim(el);
            strings.push_back(el);
            prev = pos + 1;
        }

        // To get the last substring (or only, if delimiter is not found)
        strings.push_back(prev >= str.size() ? "" : str.substr(prev));

        return strings;
    }

    std::string to_human_readable_size(unsigned long size) {
        int i{};
        double mantissa = size;
        for (; mantissa >= 1024.; mantissa /= 1024., ++i) {}

        //mantissa = std::ceil(mantissa * 10.) / 10.;
        //return std::to_string(mantissa) + "BKMGTPE"[i];

        ostringstream out;
        out.precision(2);
        out << std::fixed << mantissa;
        return out.str() + " " + "BKMGTPE"[i];
    }

    std::vector<std::string> match_all_regex(const std::string& expression, const std::string& input) {
        regex r(expression);
        vector<string> result;
        string source = input;

        smatch sm;
        while (regex_search(source, sm, r)) {
            result.push_back(sm.str());

            source = sm.suffix();
        }

        return result;
    }

    std::string get_domain_from_url(const std::string& url) {
        regex r("https?:\\/\\/(www\\.)?([-a-zA-Z0-9@:%._\\+~#=]{1,256})");
        smatch sm;
        // group 2 is the domain
        if (regex_search(url, sm, r) && sm.size() == 3) {
            return sm[2].str();
        }

        return "";
    }

    void replace_all(std::string& s, const std::string& search, const std::string& replacement) {
        if(search.empty()) return;
        std::string::size_type n = 0;
        while ((n = s.find(search, n)) != std::string::npos) {
            s.replace(n, search.size(), replacement);
            n += replacement.size();
        }
    }

    std::string humanise(int value, string singular, string plural, string once, string twice) {
        if (value == 1 && !once.empty()) {
            return once;
        }

        if (value == 2 && !twice.empty()) {
            return twice;
        }

        string r = std::to_string(value);
        bool is_singular = r.ends_with('1');
        r += " ";

        r += singular;
        if (!is_singular) {
            r += "s";
        }
        return r;
    }

    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i < 4); i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];

            while ((i++ < 3))
                ret += '=';

        }

        return ret;

    }

    std::string base64_decode(const std::string& encoded_string) {
        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;

        while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret += char_array_3[i];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;

            for (j = 0; j < 4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
        }

        return ret;
    }

    // see https://www.programcreek.com/cpp/?code=d2school%2Fda4qi4%2Fda4qi4-master%2Fsrc%2Futilities%2Fhttp_utilities.cpp
    std::string url_encode(const std::string& value) {
        static auto hex_digt = "0123456789ABCDEF";

        std::string result;
        result.reserve(value.size() << 1);

        for (auto ch : value) {
            if ((ch >= '0' && ch <= '9')
                || (ch >= 'A' && ch <= 'Z')
                || (ch >= 'a' && ch <= 'z')
                || ch == '-' || ch == '_' || ch == '!'
                || ch == '\'' || ch == '(' || ch == ')'
                || ch == '*' || ch == '~' || ch == '.')  /* !'()*-._~ */ {
                result.push_back(ch);
            } else {
                result += std::string("%") +
                    hex_digt[static_cast<unsigned char>(ch) >> 4]
                    + hex_digt[static_cast<unsigned char>(ch) & 15];
            }
        }

        return result;
    }

    std::string url_decode(const std::string& value) {
        std::string result;
        result.reserve(value.size());

        for (std::size_t i = 0; i < value.size(); ++i) {
            auto ch = value[i];

            if (ch == '%' && (i + 2) < value.size()) {
                auto hex = value.substr(i + 1, 2);
                auto dec = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
                result.push_back(dec);
                i += 2;
            } else if (ch == '+') {
                result.push_back(' ');
            } else {
                result.push_back(ch);
            }
        }

        return result;
    }

#if WIN32
    size_t word_count(const std::string& sentence) {
        size_t r = 0;

        size_t wl = 0;
        for (char ch : sentence) {
            bool is_word_char = iscsym(ch);

            if (is_word_char) {
                wl += 1;
            } else {
                if (wl > 0) {
                    r += 1;
                    wl = 0;
                }
            }
        }

        if (wl > 0) r += 1;

        return r;
    }
#endif

    std::string remove_non_ascii(const std::string& s) {
        string r;
        for (unsigned char ch : s) {
            if (ch >= 0 && ch <= 127) r += ch;
        }
        return r;
    }

    std::string strip_html(const std::string& s) {
        const regex pattern("\\<.*?\\>");
        return regex_replace(s, pattern, "");
    }

    string rgx_extract(const std::string& s, const std::string& expr) {
        string r;
        regex rgx(expr);
        smatch sm;
        string source = s;
        while (regex_search(source, sm, rgx)) {
            if (!r.empty()) r += "\n";
            r += sm.str();
            source = sm.suffix();
        }
        return r;
    }

    std::vector<std::string> rgx_extract_to_vec(const std::string& s, const std::string& expr) {
        vector<string> r;
        regex rgx(expr);
        smatch sm;
        string source = s;
        while (regex_search(source, sm, rgx)) {
            r.push_back(sm.str());
            source = sm.suffix();
        }
        return r;
    }

    std::string deduplicate_lines(const std::string& s) {
        map<string, bool> rmap;
        for (auto& l : split(s, "\n", true)) {
            if (l.empty()) continue;
            rmap[l] = true;
        }

        string r;
        for (auto& p : rmap) {
            if (!r.empty()) r += "\n";
            r += p.first;
        }
        return r;

    }

    bool contains_ic(const std::string& haystack, const std::string& needle) {
        auto it = std::search(
            haystack.begin(), haystack.end(),
            needle.begin(), needle.end(),
            [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
        );
        return (it != haystack.end());
    }

    std::string escape_pipe(const std::string& input) {
        std::string result;
        for(char c : input) {
            if(c == '|') {
                result += "\\|";
            } else {
                result += c;
            }
        }
        return result;
    }

    std::string unescape_pipe(const std::string& input) {
        std::string result = input;
        size_t position = 0;
        while((position = result.find("\\|", position)) != std::string::npos) {
            result.erase(position, 1);
        }
        return result;
    }

    std::string join_with_pipe(const std::vector<std::string>& parts) {
        std::ostringstream joined;
        for(auto it = parts.begin(); it != parts.end(); ++it) {
            if(it != parts.begin()) {
                joined << "|";
            }
            joined << escape_pipe(*it);
        }
        return joined.str();
    }

    std::vector<std::string> split_pipe(const std::string& line) {

        // This function replaces all escaped pipes (\\|) with a non-printable character (\v),
        // then splits the string by the pipe character (|). After splitting, it replaces the
        // non-printable character back to the pipe character in each substring.
        // Please note that this code assumes that the non - printable character \v is not used in
        // the input string.If it is, you may need to choose a different character for temporary replacement.

        std::string copy(line);
        str::replace_all(copy, "\\|", "\v");
        auto result = str::split(copy, "|");
        for(auto& s : result) {
            str::replace_all(s, "\v", "|");
        }
        return result;
    }
}