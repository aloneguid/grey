#include "http.h"
#include <vector>
#include <iterator>
#include "str.h"
#include "url.h"

#if PLATFORM_WINDOWS
#pragma comment(lib, "winhttp.lib")
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

    std::string http::get(const std::string& abs_url) const {
        return "";
    }

    int http::get_get_headers(const std::string& abs_url, std::map<std::string, std::string>& headers) const {
        return 0;
    }

    void http::post(const std::string& domain, const std::string& abs_url, const std::string& data, bool is_async) const {

    }

#endif
}