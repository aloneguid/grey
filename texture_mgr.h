#pragma once
#include <string>

namespace grey
{
    // todo: rename to something generic, as it's not just textures at the moment
    class texture_mgr {
    public:
        virtual void* load_texture_from_file(
           const std::string& path, int& width, int& height) = 0;

        virtual void* load_texture_from_memory(
           const std::string& cache_name,
           unsigned char* buffer, unsigned int len, int& width, int& height) = 0;

        virtual float get_system_scale() = 0;
    };
}