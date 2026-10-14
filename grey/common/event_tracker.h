#pragma once
#include <string>
#include <map>
#include <vector>
#include "http.h"

namespace grey::common {
    class event_tracker {
    public:
        event_tracker(const std::string& domain, const std::string& app_name, const std::string& version,
            const std::string& cache_file_path = "", unsigned int max_cache_age_hours = 1);
        ~event_tracker();

        void track(const std::map<std::string, std::string>& props);

        void add_constant(std::string name, std::string value);

        void flush();

    private:
        grey::common::http h;
        const std::string url;
        std::string version;
        std::map<std::string, std::string> constants;
        const std::string cache_file_path;
        unsigned int max_cache_age_hours;

        bool write_to_file(const std::string& body);
    };
}