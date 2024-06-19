#include "reg.h"
#include <windows.h>
#include <vector>
#include "../str.h"

using namespace std;

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

namespace grey::common::win32::reg
{
    bool get_key_info(HKEY hKey,
       unsigned long& subkey_count, unsigned long& value_count,
       unsigned long& max_name_length, unsigned long& max_value_length) {
        TCHAR achKey[MAX_KEY_LENGTH];
        DWORD cbName = MAX_KEY_LENGTH;
        TCHAR achClass[MAX_PATH] = TEXT("");  // buffer for class name 
        DWORD cchClassName = MAX_PATH;
        DWORD cSubKeys = 0;
        DWORD cbMaxSubKey;             // longest subkey size 
        DWORD    cchMaxClass;          // longest class string 
        DWORD    cValues;              // number of values for key
        DWORD    cchMaxValue;          // longest value name 
        DWORD    cbMaxValueData;       // longest value data
        DWORD    cbSecurityDescriptor; // size of security descriptor 
        FILETIME ftLastWriteTime;      // last write time 

        LSTATUS lResult = ::RegQueryInfoKey(hKey,
           achClass,
           &cchClassName,
           nullptr,
           &cSubKeys,  // number of sub-keys (tree in regedit)
           &cbMaxSubKey,
           &cchMaxClass,
           &cValues,   // number of values (right pane in regedit)
           &cchMaxValue,
           &cbMaxValueData,
           &cbSecurityDescriptor,
           &ftLastWriteTime);

        if(lResult == ERROR_SUCCESS) {
            subkey_count = cSubKeys;
            value_count = cValues;
            max_name_length = cchMaxValue;
            max_value_length = cbMaxValueData;
            return true;
        }

        return false;
    }

    HKEY to_hkey(hive h) {
        switch(h) {
            case hive::classes_root:
                return HKEY_CLASSES_ROOT;
            case hive::local_machine:
                return HKEY_LOCAL_MACHINE;
            case hive::current_user:
                return HKEY_CURRENT_USER;
            default:
                return HKEY_CURRENT_USER;
        }
    }

    vector<string> enum_subkeys(hive h, std::string path) {
        vector<string> result;

        HKEY hKey;

        LSTATUS lResult = ::RegOpenKeyEx(to_hkey(h),
           str::to_wstr(path).c_str(),
           0, KEY_READ, &hKey);

        if(lResult != ERROR_SUCCESS) return result;

        TCHAR achKey[MAX_KEY_LENGTH];

        for(int i = 0; ; i++) {
            DWORD cbName = MAX_KEY_LENGTH;

            lResult = ::RegEnumKeyEx(hKey, i, achKey, &cbName, nullptr, nullptr, nullptr, nullptr);
            if(lResult == ERROR_SUCCESS) {
                result.push_back(str::to_str(achKey));
            } else if(lResult == ERROR_NO_MORE_ITEMS) {
                break;
            } else {
                break;
            }
        }

        ::RegCloseKey(hKey);

        return result;
    }

    vector<string> get_value_names(hive h, string path) {
        vector<string> result;
        HKEY hKey;

        LSTATUS lResult = ::RegOpenKeyEx(to_hkey(h),
           str::to_wstr(path).c_str(),
           0, KEY_READ, &hKey);

        if(lResult == ERROR_SUCCESS) {
            unsigned long subkey_count, value_count, max_name_length, max_value_length;
            DWORD type;
            if(get_key_info(hKey, subkey_count, value_count, max_name_length, max_value_length) && value_count) {
                max_name_length += 4;
                max_value_length += 4;
                vector<TCHAR> achValue(max_name_length);
                vector<char> achData(max_value_length);

                for(int i = 0; i < value_count; i++) {
                    unsigned long name_length = max_name_length;
                    lResult = ::RegEnumValue(hKey, i,
                       &achValue[0], &name_length,
                       nullptr,
                       &type,  // todo: include type in result
                       nullptr, nullptr);
                    if(lResult == ERROR_SUCCESS) {
                        wstring name(&achValue[0]);
                        result.push_back(str::to_str(name));
                    }

                }
            }
        }

        ::RegCloseKey(hKey);

        return result;
    }

    std::string get_value(hive h, const std::string& path, const std::string& value_name) {
        HKEY hKey;
        string r;

        LSTATUS lResult = ::RegOpenKeyEx(to_hkey(h),
           str::to_wstr(path).c_str(),
           0, KEY_READ, &hKey);

        if(lResult == ERROR_SUCCESS) {
            DWORD dwType;
            DWORD cbData;
            lResult = ::RegGetValue(hKey,
               nullptr,
               str::to_wstr(value_name).c_str(),
               RRF_RT_ANY,
               &dwType,
               nullptr,
               &cbData);

            if(lResult == ERROR_SUCCESS) {
                // dwType contains data type like REG_SZ
                // cbData is buffer length to allocate

                vector<char> buffer(cbData + 1);
                cbData = buffer.size();

                if(dwType == REG_SZ || dwType == REG_EXPAND_SZ) {
                    lResult = ::RegGetValue(hKey, nullptr,
                       str::to_wstr(value_name).c_str(),
                       RRF_RT_ANY, &dwType,
                       &buffer[0],
                       &cbData);

                    wstring sv(reinterpret_cast<wchar_t*>(&buffer[0]));
                    r = str::to_str(sv);
                } else if(dwType == REG_DWORD) {
                    lResult = ::RegGetValue(hKey, nullptr,
                        str::to_wstr(value_name).c_str(),
                        RRF_RT_REG_DWORD, &dwType,
                        &buffer[0],
                        &cbData);
                    DWORD number = *((DWORD*)&buffer[0]);
                    r = std::to_string(number);
                }
            }

        }

        ::RegCloseKey(hKey);


        return r;
    }

    std::vector<std::string> get_multi_value(hive h, const std::string& path, const std::string& value_name) {
        vector<string> result;

        HKEY hKey;

        LSTATUS lResult = ::RegOpenKeyEx(to_hkey(h),
           str::to_wstr(path).c_str(),
           0, KEY_READ, &hKey);

        if(lResult == ERROR_SUCCESS) {
            DWORD dwType;
            DWORD cbData;
            lResult = ::RegGetValue(hKey,
               nullptr,
               str::to_wstr(value_name).c_str(),
               RRF_RT_ANY,
               &dwType,
               nullptr,
               &cbData);

            if(lResult == ERROR_SUCCESS) {
                // dwType contains data type like REG_SZ
                // cbData is buffer length to allocate

                if(dwType == REG_MULTI_SZ) {
                    vector<wchar_t> buffer((cbData + 1) / sizeof(wchar_t));
                    cbData += sizeof(wchar_t);
                    lResult = ::RegGetValue(hKey, nullptr,
                       str::to_wstr(value_name).c_str(),
                       RRF_RT_ANY, &dwType,
                       &buffer[0],
                       &cbData);

                    string cur;
                    for(auto& ch : buffer) {
                        if(ch == '\0') {
                            if(!cur.empty()) result.push_back(cur);
                            cur.clear();
                        } else {
                            cur += ch;
                        }
                    }
                }
            }

        }

        ::RegCloseKey(hKey);


        return result;
    }

    void delete_key(hive h, const std::string& path) {
        LSTATUS lResult = ::RegDeleteTree(to_hkey(h), str::to_wstr(path).c_str());
    }

    void delete_value(hive h, const std::string& path, const std::string& value_name) {
        HKEY hKey;

        LSTATUS lResult = ::RegOpenKeyEx(to_hkey(h),
           str::to_wstr(path).c_str(), 0, KEY_ALL_ACCESS, &hKey);

        if(lResult == ERROR_SUCCESS) {
            lResult = ::RegDeleteValue(hKey, str::to_wstr(value_name).c_str());

            ::RegCloseKey(hKey);
        }
    }

    void set_value(hive h, const std::string& path, const std::string& value, const std::string& value_name) {
        HKEY hKey;
        LSTATUS lResult = ::RegCreateKeyEx(to_hkey(h),
           str::to_wstr(path).c_str(),
           0, nullptr, 0,
           KEY_ALL_ACCESS,
           nullptr,
           &hKey,
           nullptr);


        if(lResult == ERROR_SUCCESS) {
            wstring wvalue = str::to_wstr(value);
            lResult = ::RegSetValueEx(hKey,
               str::to_wstr(value_name).c_str(),
               0, REG_SZ,
               reinterpret_cast<const BYTE*>(&wvalue[0]),
               wvalue.size() * sizeof(wchar_t));

            ::RegCloseKey(hKey);
        }
    }

    void set_value(hive h, const std::string& path, int32_t value, const std::string& value_name) {
        HKEY hKey;
        LSTATUS lResult = ::RegCreateKeyEx(to_hkey(h),
           str::to_wstr(path).c_str(),
           0, nullptr, 0,
           KEY_ALL_ACCESS,
           nullptr,
           &hKey,
           nullptr);


        if(lResult == ERROR_SUCCESS) {
            lResult = ::RegSetValueEx(hKey,
               str::to_wstr(value_name).c_str(),
               0, REG_DWORD,
               reinterpret_cast<const BYTE*>(&value),
               sizeof(int32_t));

            ::RegCloseKey(hKey);
        }
    }

    void set_value(hive h, const std::string& path, const std::vector<std::string>& value, const std::string& value_name) {
        HKEY hKey;
        LSTATUS lResult = ::RegCreateKeyEx(to_hkey(h),
           str::to_wstr(path).c_str(),
           0, nullptr, 0,
           KEY_ALL_ACCESS,
           nullptr,
           &hKey,
           nullptr);


        if(lResult == ERROR_SUCCESS) {
            vector<wchar_t> rv;
            for(auto& l : value) {
                rv.insert(rv.end(), l.begin(), l.end());
                rv.push_back(L'\0');
            }
            rv.push_back(L'\0');
            lResult = ::RegSetValueEx(hKey,
               str::to_wstr(value_name).c_str(), 0, REG_MULTI_SZ,
               reinterpret_cast<const BYTE*>(&rv[0]),
               rv.size() * sizeof(wchar_t));

            ::RegCloseKey(hKey);
        }
    }
}