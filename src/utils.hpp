//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_UTILS_HPP
#define BONDRIVER_EPGSTATION_UTILS_HPP

#include <string>

#if defined(_WIN32) && defined(_UNICODE)
    using PlatformString = std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>;
#else
    using PlatformString = std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
#endif

#if defined(_WIN32) && defined(_UNICODE)
    #define UTF8ToPlatformString(a) Utils::UTF8ToWideString(a)
    #define PlatformStringToUTF8(a) Utils::WideStringToUTF8(a)
#else
    #define UTF8ToPlatformString(a) a
    #define PlatformStringToUTF8(a) a
#endif

namespace Utils {

std::wstring UTF8ToWideString(const char* input);
std::wstring UTF8ToWideString(const std::string& input);
std::string WideStringToUTF8(const wchar_t* input);
std::string WideStringToUTF8(const std::wstring& input);

std::string RemoveSuffixSlash(const std::string& input);

}

#endif // BONDRIVER_EPGSTATION_UTILS_HPP
