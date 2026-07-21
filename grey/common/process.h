#pragma once
#include "platform.h"
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

#if PLATFORM_WINDOWS
#include <Windows.h>
#else
#include <unistd.h>

typedef void* HWND;
typedef uint32_t DWORD;
#define NORMAL_PRIORITY_CLASS 0
#endif

namespace grey::common {
    /**
    * @brief Process management functions. Interesting fact: on Windows, PID is DWORD, on linux and Mac: pid_t.
    */
    class process {
    public:

#if PLATFORM_WINDOWS
        using PidType = DWORD;
#else
        using PidType = pid_t;
#endif

        /**
         * @brief Creates the process class from a specific PID.
         * @param pid
        */
        explicit process(const PidType pid) : pid{pid} {
        }

        /**
         * @brief Creates the process from the current process.
        */
        process();

        ~process();

        /**
         * @brief Enumerates all processes on the system.
         * @return
        */
        static std::vector<process> enumerate();

        /**
         * @brief Starts a process and returns PID on success
         * @param cmdline
         * @return PID
        */
        static PidType start(const std::string& cmdline, bool wait_for_exit = false);

        /**
         * @brief Executes a command line, captures output and adds to std_out
         * @param cmdline
         * @param std_out
         * @return Process exit code
        */
        static int exec(const std::string& cmdline, std::string& std_out);

        /**
         * @brief Executes a command line, captures output and calls back
         * @param cmdline
         * @param std_out_new_data
         * @return Process exit code
        */
        static int exec(const std::string& cmdline, std::function<void(std::string&)> std_out_new_data);

        std::string get_module_filename() const;

        std::string get_name() const;

        /**
         * @brief Gets a process description similar to the one displayed in Windows Task Manager.
         * @return
         */
        std::string get_description() const;

        HWND find_main_window();

        void set_priority(DWORD priority_class = NORMAL_PRIORITY_CLASS);

        bool enable_efficiency_mode();

        bool is_valid() const { return pid != 0; }

        process get_parent();

        bool get_memory_info(uint64_t& working_set_bytes);

        double get_uptime_sec();

        bool terminate();

        /**
         * @brief Using sampling (windows performance counters), therefore, accurate measurements will only happen after a second or so call with a second delay. This is using the "Process V2" counter available on Windows 10 and above.
         * @return
        */
        double get_cpu_usage_percentage();

    private:
        PidType pid;

#if PLATFORM_WINDOWS
        // cpu perf counter
        bool pdhCpuInitialised{false};
        HANDLE pdhCpuQuery{nullptr};
        HANDLE pdhCpuCounter{nullptr};
#else
        bool cpu_initialised{false};
        uint64_t last_utime{0};
        uint64_t last_stime{0};
        uint64_t last_system_time{0};
#endif

    };
}