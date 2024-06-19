#pragma once
#include <vector>

namespace grey::common::stl {
    template<class T>
    static bool move(std::vector<T>& vec, int from_pos, int to_pos, bool is_relative) {

        int new_pos = is_relative ? (from_pos + to_pos) : to_pos;
        if(new_pos < 0 || new_pos >= vec.size()) return false;

        auto tenant = vec[from_pos];
        vec.erase(vec.begin() + from_pos);
        vec.insert(vec.begin() + new_pos, tenant);

        return true;
    }
}