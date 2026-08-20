#include "keyboard.h"
#include "platform.h"
#include <string>

#if PLATFORM_WINDOWS
#include <windows.h>
#elif PLATFORM_LINUX
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace grey::common {
#if PLATFORM_WINDOWS
    void keyboard::refresh_state() {
        ::GetKeyboardState(state_);
    }

    bool keyboard::is_key_down(key k, bool rescan) {
        if(rescan) {
            refresh_state();
        }

        return (state_[static_cast<int>(k)] & 0x80) != 0;
    }
#endif

#if PLATFORM_LINUX
    namespace {
        constexpr size_t bits_per_long = 8 * sizeof(unsigned long);

        bool is_bit_set(const unsigned long* bits, int bit) {
            return (bits[bit / bits_per_long] & (1UL << (bit % bits_per_long))) != 0;
        }

        // scans /dev/input/event* devices and ORs each device's key bitmask into `state`, since
        // there is no single global keyboard state API on Linux (unlike GetKeyboardState on Windows).
        void scan_all_keys(unsigned long* state, size_t state_size) {
            std::memset(state, 0, state_size * sizeof(unsigned long));

            DIR* dir = ::opendir("/dev/input");
            if(!dir) {
                return;
            }

            dirent* entry;
            while((entry = ::readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if(name.rfind("event", 0) != 0) {
                    continue;
                }

                std::string path = "/dev/input/" + name;
                int fd = ::open(path.c_str(), O_RDONLY | O_NONBLOCK);
                if(fd < 0) {
                    continue;
                }

                unsigned long key_bits[(KEY_MAX / bits_per_long) + 1]{};
                if(::ioctl(fd, EVIOCGKEY(sizeof(key_bits)), key_bits) >= 0) {
                    for(size_t i = 0; i < state_size; i++) {
                        state[i] |= key_bits[i];
                    }
                }

                ::close(fd);
            }

            ::closedir(dir);
        }

        // scans /dev/input/event* devices for a lit LED, used for caps lock state.
        bool is_evdev_led_on(int code) {
            if(code < 0 || code > LED_MAX) {
                return false;
            }

            DIR* dir = ::opendir("/dev/input");
            if(!dir) {
                return false;
            }

            bool on = false;
            dirent* entry;
            while(!on && (entry = ::readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if(name.rfind("event", 0) != 0) {
                    continue;
                }

                std::string path = "/dev/input/" + name;
                int fd = ::open(path.c_str(), O_RDONLY | O_NONBLOCK);
                if(fd < 0) {
                    continue;
                }

                unsigned long led_bits[(LED_MAX / bits_per_long) + 1]{};
                if(::ioctl(fd, EVIOCGLED(sizeof(led_bits)), led_bits) >= 0 && is_bit_set(led_bits, code)) {
                    on = true;
                }

                ::close(fd);
            }

            ::closedir(dir);
            return on;
        }
    }

    void keyboard::refresh_state() {
        scan_all_keys(state_, sizeof(state_) / sizeof(state_[0]));
    }

    bool keyboard::is_key_down(key k, bool rescan) {
        if(rescan) {
            refresh_state();
        }

        int code = static_cast<int>(k);
        if(code < 0 || code > KEY_MAX) {
            return false;
        }

        return is_bit_set(state_, code);
    }

#endif

    bool keyboard::are_keys_down(const std::vector<key>& keys, bool rescan) {
        if(rescan) {
            refresh_state();
        }

        for(key k : keys) {
            if(!is_key_down(k, false)) {
                return false;
            }
        }

        return true;
    }

    bool keyboard::is_any_key_down(const std::vector<key>& keys, bool rescan) {
        if(rescan) {
            refresh_state();
        }

        for(key k : keys) {
            if(is_key_down(k, false)) {
                return true;
            }
        }

        return false;
    }
}
