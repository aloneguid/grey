#include "http.h"
#include <vector>
#include <iterator>
#include "str.h"
#include "url.h"

#if PLATFORM_WINDOWS
#pragma comment(lib, "winhttp.lib")
#else
#include <curl/curl.h>
#endif

using namespace std;

namespace grey::common {
    http::http() {
#if PLATFORM_WINDOWS
        hSession = ::WinHttpOpen(
            L"ALG/1.0",
            WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0);
#endif
    }

    http::~http() {
#if PLATFORM_WINDOWS
        if(hSession) {
            ::WinHttpCloseHandle(hSession);
        }
#endif
    }

#if PLATFORM_WINDOWS
    std::string http::get(const std::string& abs_url) const {
        string result;

        if(!hSession) return result;

        url url{abs_url};

        HINTERNET hConnect = ::WinHttpConnect(hSession,
            str::to_wstr(url.host).c_str(),
            INTERNET_DEFAULT_HTTPS_PORT, 0);

        if(hConnect) {
            wstring wurl{str::to_wstr(url.query)};
            HINTERNET hRequest = ::WinHttpOpenRequest(hConnect,
                L"GET",
                wurl.c_str(),
                nullptr,
                WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

            if(hRequest) {
                //wstring headers{ L"Content-Type: application/json" };
                //wstring headers{L"Cache-Control: no-cache"};
                bool ok = ::WinHttpSendRequest(hRequest,
                    WINHTTP_NO_ADDITIONAL_HEADERS,
                    0, WINHTTP_NO_REQUEST_DATA, 0,
                    0, 0);

                ok = ::WinHttpReceiveResponse(hRequest, NULL);
                if(ok) {
                    vector<char> buffer;
                    buffer.resize(1024);
                    DWORD read{0};
                    while(::WinHttpReadData(hRequest, &buffer[0], 1024, &read) && read > 0) {
                        std::copy(buffer.begin(), buffer.begin() + read, back_inserter(result));
                    }
                }

                ::WinHttpCloseHandle(hRequest);
            }


            ::WinHttpCloseHandle(hConnect);
        }

        return result;
    }

    int http::get_get_headers(const std::string& url, map<string, string>& headers) const {
        int response_code = -1;
        headers.clear();
        string domain = str::get_domain_from_url(url);
        string path;
        int idx = url.find_last_of('/');
        if (idx == string::npos) return response_code;
        if (idx < url.size() + 1) path = url.substr(idx + 1);

        if (!hSession) return response_code;

        HINTERNET hConnect = ::WinHttpConnect(hSession,
            str::to_wstr(domain).c_str(),
            INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (hConnect) {

            wstring wurl{ str::to_wstr(path) };
            HINTERNET hRequest = ::WinHttpOpenRequest(hConnect,
                L"GET",
                wurl.c_str(),
                nullptr,
                WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

            disable_redirects(hRequest);

            if (hRequest) {

                bool ok = ::WinHttpSendRequest(hRequest,
                    WINHTTP_NO_ADDITIONAL_HEADERS,
                    0, WINHTTP_NO_REQUEST_DATA, 0,
                    0, 0);

                if (ok) {
                    ok = ::WinHttpReceiveResponse(hRequest, NULL);
                    if (ok) {
                        DWORD dwStatusCode{};
                        DWORD dwSize = sizeof(dwStatusCode);
                        ::WinHttpQueryHeaders(hRequest,
                            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX,
                            &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
                        response_code = static_cast<int>(dwStatusCode);

                        DWORD headerSize{};
                        ok = ::WinHttpQueryHeaders(hRequest,
                            WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, WINHTTP_NO_OUTPUT_BUFFER, &headerSize, WINHTTP_NO_HEADER_INDEX);

                        wstring header_s;

                        if (!ok && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                            header_s.resize(headerSize / sizeof(wchar_t));
                            if (!header_s.empty()) {
                                ok = ::WinHttpQueryHeaders(hRequest,
                                    WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, &header_s[0], &headerSize, WINHTTP_NO_HEADER_INDEX);
                                if (ok) {
                                    // parse headers
                                    for (auto pair_s : str::split(str::to_str(header_s), "\n", true)) {
                                        int idx = pair_s.find_first_of(':');
                                        if (idx == string::npos || idx + 1 >= pair_s.size() ) continue;

                                        string k = pair_s.substr(0, idx);
                                        string v = pair_s.substr(idx + 1);
                                        str::trim(k);
                                        str::trim(v);

                                        headers[k] = v;
                                    }
                                }
                            }
                        }
                    }
                }

                ::WinHttpCloseHandle(hRequest);
            }

            ::WinHttpCloseHandle(hConnect);
        }

        return response_code;
    }

    void http::post(const std::string& domain, const std::string& url, const std::string& data, bool is_async) const {
        // PUT example - https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpreceiveresponse?f1url=%3FappId%3DDev16IDEF1%26l%3DEN-US%26k%3Dk(WINHTTP%252FWinHttpReceiveResponse);k(WinHttpReceiveResponse);k(DevLang-C%252B%252B);k(TargetOS-Windows)%26rd%3Dtrue

        if(!hSession) return;

        HINTERNET hConnect = ::WinHttpConnect(hSession,
            str::to_wstr(domain).c_str(),
            INTERNET_DEFAULT_HTTPS_PORT, 0);

        if(hConnect) {
            wstring wurl{str::to_wstr(url)};
            HINTERNET hRequest = ::WinHttpOpenRequest(hConnect,
                L"POST",
                wurl.c_str(),
                nullptr,
                WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

            // --------- SYNC/ASYNC ----------
            // from here commonality between sync and async ends
            // https://docs.microsoft.com/en-us/archive/msdn-magazine/2008/august/windows-with-c-asynchronous-winhttp

            if(hRequest) {
                //wstring headers{ L"Content-Type: application/json" };
                bool ok = ::WinHttpSendRequest(hRequest,
                    WINHTTP_NO_ADDITIONAL_HEADERS,
                    0, WINHTTP_NO_REQUEST_DATA, 0,
                    data.size(), 0);

                DWORD dwBytesWritten{0};
                ok = ::WinHttpWriteData(hRequest, (LPCVOID)&data[0], data.size(), &dwBytesWritten);
                ok = ::WinHttpReceiveResponse(hRequest, NULL);
            }

            ::WinHttpCloseHandle(hRequest);
        }

        ::WinHttpCloseHandle(hConnect);
    }

    bool http::disable_redirects(HINTERNET hRequest) const {
        DWORD dw = WINHTTP_DISABLE_REDIRECTS;
        return ::WinHttpSetOption(hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &dw, sizeof(dw));
    }

#else

    namespace {
        size_t write_response(char* data, size_t size, size_t count, void* user_data) {
            auto* response = static_cast<std::string*>(user_data);
            response->append(data, size * count);
            return size * count;
        }

        size_t collect_header(char* data, size_t size, size_t count, void* user_data) {
            auto* headers = static_cast<std::map<std::string, std::string>*>(user_data);
            const size_t length = size * count;
            std::string line(data, length);
            const size_t separator = line.find(':');
            if(separator == std::string::npos) return length;

            std::string name = line.substr(0, separator);
            std::string value = line.substr(separator + 1);
            str::trim(name);
            str::trim(value);
            if(!name.empty()) (*headers)[name] = value;
            return length;
        }

        CURL* create_handle(const std::string& request_url) {
            CURL* handle = curl_easy_init();
            if(!handle) return nullptr;

            if(curl_easy_setopt(handle, CURLOPT_URL, request_url.c_str()) != CURLE_OK ||
               curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L) != CURLE_OK ||
               curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 10L) != CURLE_OK ||
               curl_easy_setopt(handle, CURLOPT_TIMEOUT, 30L) != CURLE_OK) {
                curl_easy_cleanup(handle);
                return nullptr;
            }
            return handle;
        }
    }

    std::string http::get(const std::string& abs_url) const {
        std::string result;
        CURL* handle = create_handle(abs_url);
        if(!handle) return result;

        const bool configured =
            curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L) == CURLE_OK &&
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_response) == CURLE_OK &&
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, &result) == CURLE_OK;
        if(configured && curl_easy_perform(handle) != CURLE_OK) result.clear();
        curl_easy_cleanup(handle);
        return result;
    }

    int http::get_get_headers(const std::string& abs_url, std::map<std::string, std::string>& headers) const {
        headers.clear();
        CURL* handle = create_handle(abs_url);
        if(!handle) return -1;

        const bool configured =
            curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L) == CURLE_OK &&
            curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 0L) == CURLE_OK &&
            curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, collect_header) == CURLE_OK &&
            curl_easy_setopt(handle, CURLOPT_HEADERDATA, &headers) == CURLE_OK;
        if(!configured || curl_easy_perform(handle) != CURLE_OK) {
            curl_easy_cleanup(handle);
            return -1;
        }

        long response_code = 0;
        const CURLcode info_result = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_cleanup(handle);
        return info_result == CURLE_OK ? static_cast<int>(response_code) : -1;
    }

    void http::post(const std::string& domain, const std::string& abs_url, const std::string& data, bool is_async) const {
        (void)is_async;
        std::string request_url = domain;
        if(request_url.find("://") == std::string::npos) request_url = "https://" + request_url;
        if(abs_url.find("://") == 0) {
            request_url = abs_url;
        } else if(!abs_url.empty()) {
            if(request_url.back() != '/' && abs_url.front() != '/') request_url += '/';
            request_url += abs_url;
        }

        CURL* handle = create_handle(request_url);
        if(!handle) return;

        const bool configured =
            curl_easy_setopt(handle, CURLOPT_POST, 1L) == CURLE_OK &&
            curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data.data()) == CURLE_OK &&
            curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(data.size())) == CURLE_OK;
        if(configured) curl_easy_perform(handle);
        curl_easy_cleanup(handle);
    }

#endif
}