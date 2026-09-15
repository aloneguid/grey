#include <gtest/gtest.h>
#include "common/http.h"

#include <httplib.h>
#include <thread>

using namespace grey::common;

namespace {
    class local_http_server {
    public:
        local_http_server() {
            server.Get("/body", [](const httplib::Request&, httplib::Response& response) {
                response.set_header("X-Test", "present");
                response.set_content("local body", "text/plain");
            });
            server.Get("/redirect", [](const httplib::Request&, httplib::Response& response) {
                response.set_redirect("/body", 302);
            });
            server.Post("/post", [this](const httplib::Request& request, httplib::Response& response) {
                request_body = request.body;
                response.set_content("", "text/plain");
            });
            port = server.bind_to_any_port("127.0.0.1");
            EXPECT_GT(port, 0);
        }

        ~local_http_server() {
            server.stop();
            wait();
        }

        void serve_once() {
            worker = std::thread([this] {
                server.listen_after_bind();
            });
        }

        std::string url(const std::string& path) const {
            return "http://127.0.0.1:" + std::to_string(port) + path;
        }

        std::string domain() const {
            return "http://127.0.0.1:" + std::to_string(port);
        }

        void wait() {
            if(worker.joinable()) worker.join();
        }

        std::string request_body;

    private:
        httplib::Server server;
        int port{0};
        std::thread worker;
    };
}

TEST(Http, GetAndHeaders) {
    local_http_server server;
    server.serve_once();
    http client;
    EXPECT_EQ(client.get(server.url("/body")), "local body");

    local_http_server header_server;
    header_server.serve_once();
    std::map<std::string, std::string> headers{{"stale", "value"}};
    EXPECT_EQ(client.get_get_headers(header_server.url("/body"), headers), 200);
    EXPECT_EQ(headers["X-Test"], "present");
    EXPECT_EQ(headers.count("stale"), 0u);
}

TEST(Http, RedirectsRemainVisible) {
    local_http_server server;
    server.serve_once();
    std::map<std::string, std::string> headers;
    EXPECT_EQ(http{}.get_get_headers(server.url("/redirect"), headers), 302);
    EXPECT_EQ(headers["Location"], "/body");
}

TEST(Http, PostPayloadAndFailure) {
    local_http_server server;
    server.serve_once();
    http client;
    client.post(server.domain(), "/post", "payload", false);
    server.wait();
    EXPECT_EQ(server.request_body, "payload");

    local_http_server empty_server;
    empty_server.serve_once();
    client.post(empty_server.domain(), "/post", "", false);
    empty_server.wait();
    EXPECT_TRUE(empty_server.request_body.empty());

    EXPECT_EQ(client.get("http://127.0.0.1:1/unreachable"), "");
}