#ifndef MOLCPP_EXPORTS_H
#define MOLCPP_EXPORTS_H

#ifdef _WIN32
    #ifdef MOLCPP_BUILDING_DLL
        #define MOLCPP_EXPORT __declspec(dllexport)
    #else
        #define MOLCPP_EXPORT __declspec(dllimport)
    #endif
#else
    #define MOLCPP_EXPORT __attribute__((visibility("default")))
#endif

#endif // MOLCPP_EXPORTS_H
