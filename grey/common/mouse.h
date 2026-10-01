#pragma once

namespace grey::common {
    class mouse {
    public:
        static bool get_pos(long& x, long& y);

        static bool set_pos(long x, long y);
    };
}
