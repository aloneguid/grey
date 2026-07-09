#pragma once
#include <vector>

namespace grey::common::stl {
    template<typename T>
    static bool move(std::vector<T>& vec, int from_pos, int to_pos, bool is_relative) {

        int new_pos = is_relative ? (from_pos + to_pos) : to_pos;
        if(new_pos < 0 || new_pos >= vec.size()) return false;

        auto tenant = vec[from_pos];
        vec.erase(vec.begin() + from_pos);
        vec.insert(vec.begin() + new_pos, tenant);

        return true;
    }

    template<typename T>
    bool vec_equal(const std::vector<std::shared_ptr<T>>& a,
               const std::vector<std::shared_ptr<T>>& b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(),
            [](const auto& x, const auto& y) {
                if (!x || !y) return x == y;   // handle null shared_ptrs
                return *x == *y;               // requires T::operator==
            });
    }
}