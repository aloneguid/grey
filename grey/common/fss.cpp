#include "fss.h"
#if WIN32
#include <Windows.h>
#include <ShlObj_core.h>
#include <shellapi.h>
#else
#include <unistd.h>
#endif
#include "str.h"
#include <fstream>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

namespace grey::common::fss {
    std::string get_current_dir() {

#if WIN32
        TCHAR buffer[MAX_PATH] = {0};
        ::GetModuleFileName(nullptr, buffer, MAX_PATH);
        std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
        auto r = std::wstring(buffer).substr(0, pos);
        return str::to_str(r);
#else
        //long size = pathconf(".", _PC_PATH_MAX);
        //getcwd()
        return "";
#endif
    }

    std::string get_home_dir() {
        return std::getenv("HOME");
    }

    std::string get_config_dir() {
        fs::path config_path;

#if defined(_WIN32)
        // Windows: Use the Win32 API to fetch the Roaming AppData folder reliably
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
            config_path = fs::path(path);
        } else {
            // Fallback to environment variable if API fails
            const char* appdata = std::getenv("APPDATA");
            if (appdata) {
                config_path = fs::path(appdata);
            }
        }

#elif defined(__APPLE__)
        // macOS: Use the standard Application Support directory
        const char* home = std::getenv("HOME");
        if (home) {
            config_path = fs::path(home) / "Library" / "Application Support";
        }

#elif defined(__linux__) || defined(__unix__)
        // Linux/Unix: Adhere to XDG Base Directory Specification
        const char* xdg_config = std::getenv("XDG_CONFIG_HOME");
        if (xdg_config && *xdg_config != '\0') {
            config_path = fs::path(xdg_config);
        } else {
            const char* home = std::getenv("HOME");
            if (home) {
                config_path = fs::path(home) / ".config";
            }
        }
#endif

        return config_path.string();
    }

    std::string get_config_file_path(const std::string& application_name, const std::string &filename) {
        fs::path config_path = get_config_dir();
        return (config_path / application_name / filename).string();
    }

    std::string get_current_exec_path() {
#if WIN32
        TCHAR szFileName[MAX_PATH];
        ::GetModuleFileName(nullptr, szFileName, MAX_PATH);
        return str::to_str(szFileName);
#else
        return "";
#endif
    }

    bool file_exists(const std::string& name) {
        std::ifstream f{name};
        return f.is_open();
    }

    size_t get_file_size(const std::string& name) {
        std::ifstream fl(name, std::ifstream::ate | std::ifstream::binary);
        if(!fl) return -1;
        return fl.tellg();
    }

    std::string get_full_path(const std::string& path) {
        if (path.empty() || !path.starts_with('.')) return path;

        string r = path.substr(1);
        r = get_current_dir() + r;
        return r;
    }

    bool read_binary_file(const std::string& name, unsigned char* buffer) {
        std::ifstream fl(name, std::ifstream::ate | std::ifstream::binary);
        if(!fl) return false;
        auto size = fl.tellg();
        fl.seekg(fl.beg);

        fl.read((char*)buffer, size);
        return true;
    }

    std::string read_all_text(const std::string& name) {
        ifstream ifs(name, ios::in | ios::binary | ios::ate);
        if(!ifs) return "";
        ifstream::pos_type sz = ifs.tellg();
        ifs.seekg(0, ios::beg);
        vector<char> bytes(sz);
        ifs.read(bytes.data(), sz);
        return string(bytes.data(), sz);
    }

    void write_all_text(const std::string& filename, const std::string& contents) {
        ofstream ofs(filename, ios::out | ios::binary | ios::ate);
        if(!ofs) return;

        ofs << contents;
        ofs.flush();
    }

    void append_all_text(const std::string& filename, const std::string& contents) {
        ofstream ofs(filename, ios::out | ios::binary | ios::ate | ios::app);
        if(!ofs) return;
        ofs << contents;
        ofs.flush();
    }

    unsigned int get_age_in_seconds(const std::string& filename) {

#if WIN32
        WIN32_FILE_ATTRIBUTE_DATA fileInfo;

        // Retrieve file attributes
        if(!::GetFileAttributesExA(filename.c_str(), GetFileExInfoStandard, &fileInfo)) {
            //throw std::runtime_error("Failed to get file attributes");
            return 0;
        }
        FILETIME creationTime = fileInfo.ftCreationTime;

        // get time now
        SYSTEMTIME now;
        ::GetSystemTime(&now);
        FILETIME nowFileTime;
        ::SystemTimeToFileTime(&now, &nowFileTime);

        // get the difference in seconds
        ULARGE_INTEGER nowTime;
        ULARGE_INTEGER creationTimeInt;
        nowTime.LowPart = nowFileTime.dwLowDateTime;
        nowTime.HighPart = nowFileTime.dwHighDateTime;
        creationTimeInt.LowPart = creationTime.dwLowDateTime;
        creationTimeInt.HighPart = creationTime.dwHighDateTime;
        ULARGE_INTEGER diff;
        diff.QuadPart = nowTime.QuadPart - creationTimeInt.QuadPart;
        unsigned int seconds = static_cast<unsigned int>(diff.QuadPart / 10000000ULL);
        return seconds;
    
#endif

        return 0;
    }

    std::string get_temp_file_path(const string& prefix) {
        WCHAR temp_path[MAX_PATH];
        WCHAR temp_file[MAX_PATH];

        // Get the path to the temporary folder
        DWORD path_len = ::GetTempPath(MAX_PATH, temp_path);
        if (path_len == 0 || path_len > MAX_PATH) {
            throw std::runtime_error("Failed to get temp path");
        }

        // Generate a unique temporary file name
        UINT unique_num = ::GetTempFileName(temp_path, str::to_wstr(prefix).c_str(), 0, temp_file);
        if (unique_num == 0) {
            throw std::runtime_error("Failed to get temp file name");
        }

        return str::to_str(wstring(temp_file));
    }

    bool delete_file(const std::string& path) {
        // use native c++ file system
        return std::remove(path.c_str());
    }
}