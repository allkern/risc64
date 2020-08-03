#pragma once

#define LGW_OPTIMIZE
#define LGW_LOG

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

#ifdef __linux__
    #include <X11/Xlib.h>
#endif