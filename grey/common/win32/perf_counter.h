#pragma once
#include <string>
#include <Pdh.h>    // windows
#include <format>

namespace grey::common::win32 {
    class perf_counter {
    public:
        perf_counter(std::string counter_path);
        ~perf_counter();

        static perf_counter make_process_processor_time(std::string process_name) {
            return perf_counter{std::format("\\Process({})\\% Processor Time", process_name)};
        }

        void collect_sample();

        double get_double_value() { return d_value; }

    private:
        std::string counter_path;
        PDH_HQUERY hQuery{0};
        PDH_HCOUNTER hCounter{0};
        bool ok{false};
        double d_value{0};
    };
}