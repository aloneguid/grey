#pragma once
#include <string>
#include <vector>
#include <sstream>

namespace grey::common::str {
    std::wstring to_wstr(const std::string& str);

    std::string to_str(const std::wstring& wstr);

    int to_int(const std::string& str);

    unsigned long to_ulong(const std::string& str, int radix = 10);

    long long to_long_long(const std::string& str);

    void ltrim(std::string& s, const std::string& chars = " \r\n\t");

    void rtrim(std::string& s, const std::string& chars = " \r\n\t");

    void trim(std::string& s, const std::string& chars = " \r\n\t");

    void upper(std::string& s);

    void lower(std::string& s);

    void capitalize(std::string& s);

    void unescape_special_chars(std::string& s);

    void replace_all(std::string& s, const std::string& search, const std::string& replacement);

    std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool trim_lines = false);

    template<class ForwardIterator>
    std::string join(ForwardIterator begin, ForwardIterator end, const std::string& separator) {
        std::ostringstream ss;
        size_t count{};

        for (ForwardIterator it = begin; it != end; ++it, ++count) {
            if (count != 0) ss << separator;

            ss << *it;
        }

        return ss.str();
    }

    std::string to_human_readable_size(unsigned long size);

    std::vector<std::string> match_all_regex(const std::string& expression, const std::string& input);

    std::string get_domain_from_url(const std::string& url);

    std::string humanise(int value, std::string singular, std::string plural = "", std::string once = "", std::string twice = "");

    inline bool is_base64(unsigned char c);

    std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);

    std::string base64_decode(std::string const& encoded_string);

    std::string url_encode(const std::string& value);

    std::string url_decode(const std::string& value);

#if WIN32
    size_t word_count(const std::string& sentence);
#endif

    std::string remove_non_ascii(const std::string& s);

    std::string strip_html(const std::string& s);

    std::string rgx_extract(const std::string& s, const std::string& expr);

    std::vector<std::string> rgx_extract_to_vec(const std::string& s, const std::string& expr);

    std::string deduplicate_lines(const std::string& s);

    /**
     * @brief Case insensitive compare
     * @param haystack 
     * @param needle 
     * @return 
    */
    bool contains_ic(const std::string& haystack, const std::string& needle);

    /**
     * @brief Escapes pipe character in the string with backslash and pipe.
     * @param input 
     * @return 
    */
    std::string escape_pipe(const std::string& input);

    /**
     * @brief Unescapes backslash and pipe character in the string with just pipe.
     * @param input 
     * @return 
    */
    std::string unescape_pipe(const std::string& input);

    /**
     * @brief Joins vector of strings with pipe character, taking into account that some strings may contain pipe character.
     * @param parts 
     * @return 
    */
    std::string join_with_pipe(const std::vector<std::string>& parts);

    /**
     * @brief Split string by pipe character, taking into account that some strings may contain pipe character.
     * @param line 
     * @return 
    */
    std::vector<std::string> split_pipe(const std::string& line);

}