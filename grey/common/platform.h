#pragma once

#if defined(_WIN32)
    #define PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
    #include <TargetConditionals.h>

    #define PLATFORM_MACOS (!TARGET_OS_IPHONE)
    #define PLATFORM_IOS    TARGET_OS_IPHONE
#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID 1
#elif defined(__linux__)
    #define PLATFORM_LINUX 1
#endif

#ifndef PLATFORM_WINDOWS
    #define PLATFORM_WINDOWS 0
#endif

#ifndef PLATFORM_LINUX
    #define PLATFORM_LINUX 0
#endif

#ifndef PLATFORM_MACOS
    #define PLATFORM_MACOS 0
#endif

#ifndef PLATFORM_ANDROID
    #define PLATFORM_ANDROID 0
#endif

#ifndef PLATFORM_IOS
    #define PLATFORM_IOS 0
#endif

// ...zero-fill the rest similarly, so you can write
// `if constexpr (PLATFORM_WINDOWS)` instead of #ifdef chains where possible