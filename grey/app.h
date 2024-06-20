#pragma once
#include <memory>
#include <string>
#include <functional>

namespace grey {
    class app {
    public:
        /**
         * @brief Creates app instance, which will be different implementation depending on platform we run on.
         *        Only one instance of the app should be created per process lifetime.
         * @param title 
         * @return 
         */
        static std::unique_ptr<app> make(const std::string& title);

        float scale{1.0f};

        app();

        virtual void run(std::function<bool(const app&)> render_frame) = 0;

    protected:
        void on_after_initialised();
    };
}