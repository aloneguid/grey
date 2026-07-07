#pragma once
#include <string>

namespace grey::common {
    class guid {
    public:
        /**
         * Creates new V7 GUID.
         */
        static std::string create_guid();
    };
}
