//
// @author magicxqq <xqq@xqq.im>
//

#include <Windows.h>
#include <cstddef>
#include "utils.hpp"

namespace Utils {

std::wstring UTF8ToWideString(const char* input) {
    int length = MultiByteToWideChar(CP_UTF8, 0, input, -1, nullptr, 0);
    std::wstring result;
    if (length > 0) {
        result.resize(length);
        MultiByteToWideChar(CP_UTF8, 0, input, -1, const_cast<LPWSTR>(result.c_str()), length);
    }
    return result;
}

std::wstring UTF8ToWideString(const std::string &input) {
    return UTF8ToWideString(input.c_str());
}

std::string WideStringToUTF8(const wchar_t* input) {
    int src_length = static_cast<int>(wcslen(input));
    int length = WideCharToMultiByte(CP_UTF8, 0, input, src_length, nullptr, 0, nullptr, nullptr);

    std::string result;
    if (length > 0) {
        result.resize(static_cast<size_t>(length + 1));
        WideCharToMultiByte(CP_UTF8, 0, input, src_length, &result[0], length, nullptr, nullptr);
        result[length] = '\0';
    }
    return result;
}

std::string WideStringToUTF8(const std::wstring &input) {
    return WideStringToUTF8(input.c_str());
}

std::string RemoveSuffixSlash(const std::string& input) {
    if (*--input.end() == '/') {
        return std::string(input.begin(), --input.end());
    } else {
        return input;
    }
}

}
