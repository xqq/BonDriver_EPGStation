//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_LOG_HPP
#define BONDRIVER_EPGSTATION_LOG_HPP

using cstr = const char * const;

static constexpr const char* file_name(const char* path) {
    const char* name = path;
    while (*path) {
        const char ch = *path++;
        if (ch == '\\' || ch == '/') {
            name = path;
        }
    }
    return name;
}

#ifndef __SHORT_FILE__
    #define __SHORT_FILE__ file_name(__FILE__)
#endif

#ifndef __FUNCTION_NAME__
    #ifdef _WIN32
        #define __FUNCTION_NAME__   __FUNCTION__
    #else
        #define __FUNCTION_NAME__   __func__
    #endif
#endif

#ifndef LOG_FILE_FUNCTION
    #define LOG_FILE_FUNCTION "%s:%d:%s()",__SHORT_FILE__,__LINE__,__FUNCTION_NAME__
#endif

#ifndef LOG_FILE_FUNCTION_MESSAGE
    #define LOG_FILE_FUNCTION_MESSAGE(m) "%s:%d:%s():%s",__SHORT_FILE__,__LINE__,__FUNCTION_NAME__,m
#endif

class Log {
public:
    static void Info(const char* str);
    static void InfoF(const char* format, ...);
    static void Error(const char* str);
    static void ErrorF(const char* format, ...);
};


#endif // BONDRIVER_EPGSTATION_LOG_HPP
