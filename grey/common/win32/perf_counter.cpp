#include "perf_counter.h"
#include <PdhMsg.h>
#include "../str.h"

#pragma comment(lib, "pdh.lib")

namespace grey::common::win32 {
    perf_counter::perf_counter(std::string counter_path) : counter_path{counter_path} {
        auto status = ::PdhOpenQuery(NULL, NULL, &hQuery);
        if(status == ERROR_SUCCESS) {
            auto path = str::to_wstr(counter_path);
            status = ::PdhAddEnglishCounter(hQuery, path.c_str(), NULL, &hCounter);
            ok = true;
        }
    }

    perf_counter::~perf_counter() {
        ::PdhRemoveCounter(hCounter);
        ::PdhCloseQuery(hQuery);
    }

    void perf_counter::collect_sample() {
        if(!ok) return;

        auto status = ::PdhCollectQueryData(hQuery);
        if(status == ERROR_SUCCESS) {
            // Retrieve the counter value
            PDH_FMT_COUNTERVALUE fv;
            if(ERROR_SUCCESS == ::PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL, &fv)) {
                d_value = fv.doubleValue;
            }
        }
    }
}