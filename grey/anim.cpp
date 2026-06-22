#include "anim.h"
#include <cmath>

namespace grey {
    float anim::ease_out_cubic(float x) {
        return 1.0f - pow(1.0f - x, 3);
    }

    float anim::ease_in_cubic(float x) {
        return pow(x, 3);
    }
}
