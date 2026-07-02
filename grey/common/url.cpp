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

    const string prot_end("://");

    url::url(const std::string& abs_url) : abs_url{abs_url} {

        string t = abs_url;

        // protocol
        size_t pidx = t.find(prot_end);
        if(pidx != string::npos) {
            protocol = t.substr(0, pidx);
            if(pidx + prot_end.size() < t.size()) {
                t = t.substr(pidx + prot_end.size());
            }
        }

        // host
        size_t hidx = t.find_first_of('/');
        if(hidx == string::npos) {
            host = t;
            t = "";
        }  else {
            host = t.substr(0, hidx);
            t = t.substr(hidx);
        }

        // query
        query = t;

        // parameters
        if(!t.empty()) {
            size_t ridx = query.find_first_of('?');
            if(ridx != string::npos && ridx + 1 < t.size()) {
                query_without_parameters = query.substr(0, ridx);
                t = query.substr(ridx + 1);
                vector<string> parts = str::split(t, "&");
                for(string& part : parts) {
                    size_t eqidx = part.find_first_of('=');
                    string key = part;
                    string value;
                    if(eqidx != string::npos && eqidx < part.size()) {
                        value = key.substr(eqidx + 1);
                        key = key.substr(0, eqidx);
                    }
                    parameters.emplace_back(key, value);
                }
            }
        }
    }

    std::string url::to_string() {
        string r = format("{}://{}{}", protocol, host, query_without_parameters);

        if(!parameters.empty()) {
            r += "?";
            for(int i = 0; i < parameters.size(); i++) {
                if(i > 0) r += "&";
                r += parameters[i].first;
                if(!parameters[i].second.empty()) {
                    r += "=";
                    r += parameters[i].second;
                }
            }
        }

        return r;
    }

    void url::open_in_browser(const std::string &url) {
#if PLATFORM_WINDOWS
        ::ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execlp("xdg-open", "xdg-open", url.c_str(), (char*)nullptr);
            _exit(127); // exec failed
        }
        // Parent: don't wait, let it run detached (or waitpid if you want to check success)
#endif
    }
}
