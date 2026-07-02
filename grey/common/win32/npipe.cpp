#include "npipe.h"
#include "../str.h"
#include <iostream>
#include <vector>

#define PIPE_BUF_SIZE 1024

namespace grey::common::win32 {
    using namespace std;

    npipe::npipe(std::string name) : name{name}, hPipe{0} {
        this->name = "\\\\.\\pipe\\";
        this->name += name;
    }

    npipe::~npipe() {
        if(hPipe) {
            ::CloseHandle(hPipe);
            hPipe = 0;
        }
    }

    bool npipe::listen(std::function<bool(const std::vector<char> &)> callback) {
        HANDLE hPipe;
        bool shutdown = false;

        for(;;) {
            //wcout << "waiting for connection on " << name << endl;

            hPipe = ::CreateNamedPipe(str::to_wstr(name).c_str(),
                                      PIPE_ACCESS_DUPLEX,
                                      PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                      PIPE_UNLIMITED_INSTANCES,
                                      PIPE_BUF_SIZE, PIPE_BUF_SIZE,
                                      NMPWAIT_USE_DEFAULT_WAIT,
                                      nullptr);

            if(INVALID_HANDLE_VALUE == hPipe) {
                //wcout << "failed: " << util::get_win32_last_error() << endl;
                return false;
            }

            ::ConnectNamedPipe(hPipe, nullptr);

            //cout << "reading..." << endl;
            vector<char> buf(PIPE_BUF_SIZE);
            DWORD read{0};
            while(!shutdown && ::ReadFile(hPipe, &buf[0], buf.size(), &read, nullptr)) {
                //cout << "received " << read << " byte (s)" << endl;
                buf.resize(read); // trim to just the data received
                if(!callback(buf)) {
                    //cout << "closing down" << endl;
                    shutdown = true;
                }
            }

            //wcout << "finished" << endl;

            ::CloseHandle(hPipe);

            if(shutdown)
                break;
        }

        return true;
    }

    bool npipe::send(const vector<char> &data) {
        HANDLE hPipe = ::CreateFile(str::to_wstr(name).c_str(),
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    nullptr,
                                    OPEN_EXISTING,
                                    0,
                                    nullptr);

        if(INVALID_HANDLE_VALUE == hPipe) {
            //wcout << L"cannot open pipe: " << util::get_win32_last_error() << endl;
            return false;
        }

        DWORD written = 0;
        bool ok = ::WriteFile(hPipe, &data[0], data.size(), &written, nullptr);
        ::CloseHandle(hPipe);

        return ok;
    }

    bool npipe::send(const std::string &s) {
        vector<char> data(s.begin(), s.end());

        return send(data);
    }
}
