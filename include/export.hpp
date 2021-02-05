//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_EXPORT_HPP
#define BONDRIVER_EPGSTATION_EXPORT_HPP

#if defined(_MSC_VER) || defined(__MINGW32__)
    // MSVC & MinGW
    #ifdef BONDRIVER_EPGSTATION_EXPORTS
        #define EXPORT_API __declspec(dllexport)
    #else
        #define EXPORT_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    // GCC/Clang (Unix)
    #ifdef BONDRIVER_EPGSTATION_EXPORTS
        #define EXPORT_API __attribute__((visibility("default")))
    #else
        #define EXPORT_API
    #endif
#else
    #define EXPORT_API
#endif

#endif // BONDRIVER_EPGSTATION_EXPORT_HPP
