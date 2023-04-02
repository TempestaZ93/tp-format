#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "for_constexpr.hpp"
#include "if_constexpr.hpp"
#include "log2.hpp"

namespace sl {
inline void noop() {}

constexpr inline const char *find(const char *haystack, char needle) {
    return !(haystack != nullptr && *haystack != '\0' && *haystack != needle)
               ? haystack
               : find(haystack + 1, needle);
}

class Formatting {
  public:
    enum class Type { Unknown, Int, Float, Bool, Obj };
    struct Options {
        enum IntegralFormat { Bin, Oct, Dec, Hex, HEX };

        constexpr Options(int precision, IntegralFormat intFormat)
            : precision(precision), intFormat(intFormat) {}
        int precision;
        IntegralFormat intFormat;
    };

    constexpr Formatting &operator=(const Formatting &other) {
        this->type = other.type;
        this->options = other.options;
        return *this;
    }

    constexpr Formatting() : type(Type::Unknown), options(10, Options::IntegralFormat::Dec) {}

    Type type;
    Options options;
};

constexpr bool isDigit(char c) { return c >= '0' && c <= '9'; }

constexpr int stoi_impl(const char *start, const char *end, int value = 0) {
    return start < end ? isDigit(*start) ? stoi_impl(start + 1, end, (*start - '0') + value * 10)
                                         : throw "compile time error: not a digit"
                       : value;
}

constexpr int stoi(const char *begin, const char *end) { return stoi_impl(begin, end); }

constexpr void setFormatting(Formatting *formatting, const char *formatBegin,
                             const char *formatEnd) {
    const size_t len = formatEnd - formatBegin;
    if (len < 2)
        return;
    switch (formatBegin[1]) {
    case '0':
        if (len < 3)
            return;
        switch (formatBegin[2]) {
        case '.':
            if (len < 4)
                return;
            formatting->type = Formatting::Type::Float;
            formatting->options.precision = stoi(formatBegin + 3, formatEnd);
            break;
        case 'x':
            formatting->type = Formatting::Type::Int;
            formatting->options.intFormat = Formatting::Options::IntegralFormat::Hex;
            break;
        case 'X':
            formatting->type = Formatting::Type::Int;
            formatting->options.intFormat = Formatting::Options::IntegralFormat::HEX;
            break;
        case 'o':
            formatting->type = Formatting::Type::Int;
            formatting->options.intFormat = Formatting::Options::IntegralFormat::Oct;
            break;
        case 'b':
            formatting->type = Formatting::Type::Int;
            formatting->options.intFormat = Formatting::Options::IntegralFormat::Bin;
            break;
        default:
            break;
        }

        break;

    case '.':
        if (len < 3)
            return;
        formatting->type = Formatting::Type::Float;
        formatting->options.precision = stoi(formatBegin + 2, formatEnd);
        break;

    case ':':
        assert(false);
        break;
    }
}

template <size_t N> struct Locations {
    constexpr Locations(const char literal[], const char *end) noexcept
        : formatBegin(), formatEnd(), forms() {
        const char *current = literal;
        for (auto i = 0; i < N; i++) {
            const char *formatBegin = find(current, '{');
            const char *formatEnd = find(current, '}');
            this->formatBegin[i] = formatBegin;
            this->formatEnd[i] = formatEnd;
            setFormatting(forms + i, formatBegin, formatEnd);
            current = formatEnd + 1;
            if (current >= end) {
                break;
            }
        }
    }
    static constexpr size_t num = N;
    const char *formatBegin[N];
    const char *formatEnd[N];
    Formatting forms[N];
};

template <typename IntegralType>
typename std::enable_if<std::is_integral<IntegralType>::value, std::string>::type
intToBinaryString(IntegralType num) {
    static_assert(std::is_integral<IntegralType>::value,
                  "Wrong argument for integral type formatting");
    size_t digits_min = log2_64(num) + 1;
    std::string output(digits_min + 2, '0');
    output[0] = '0';
    output[1] = 'b';
    for (int i = 0; i < digits_min; i++) {
        output[digits_min - i + 1] = '0' + ((num >> i) & 1);
    }
    return output;
}

template <typename IntegralType>
typename std::enable_if<!std::is_integral<IntegralType>::value, std::string>::type
intToBinaryString(IntegralType num) {
    return "";
}

template <typename IntegralType>
typename std::enable_if<std::is_integral<IntegralType>::value, std::string>::type
intToOctString(IntegralType num) {
    static_assert(std::is_integral<IntegralType>::value,
                  "Wrong argument for integral type formatting");
    size_t bits = log2_64(num);
    size_t digits_min = bits / 3;
    size_t bits_rest = bits % 3;
    std::string output(digits_min + 3, '0');
    output[0] = '0';
    output[1] = 'o';
    for (int i = 0; i < digits_min; i++) {
        output[digits_min - i + 2] = '0' + ((num >> i * 3) & 0b111);
    }
    output[2] = '0' + ((num >> digits_min * 3) & 0b111);
    return output;
}

template <typename IntegralType>
typename std::enable_if<!std::is_integral<IntegralType>::value, std::string>::type
intToOctString(IntegralType num) {
    return "";
}

template <typename IntegralType>
std::string intToHexString_impl(IntegralType num, char *hexCharSet) {

    static_assert(std::is_integral<IntegralType>::value,
                  "Wrong argument for integral type formatting");
    size_t digits_min = log2_64(num) / 4 + 1;
    std::string output(digits_min + 2, '0');
    output[0] = '0';
    output[1] = 'x';
    for (int i = 0; i < digits_min; i++) {
        output[digits_min - i + 1] = hexCharSet[((num >> i * 4) & 0xF)];
    }
    return output;
}

template <typename IntegralType>
typename std::enable_if<std::is_integral<IntegralType>::value, std::string>::type
intToHexString(IntegralType num) {
    char hexChars[] = "0123456789abcdef";
    return intToHexString_impl(num, hexChars);
}

template <typename IntegralType>
typename std::enable_if<!std::is_integral<IntegralType>::value, std::string>::type
intToHexString(IntegralType num) {
    return "";
}

template <typename IntegralType>
typename std::enable_if<std::is_integral<IntegralType>::value, std::string>::type
intToHEXString(IntegralType num) {
    char hexChars[] = "0123456789ABCDEF";
    return intToHexString_impl(num, hexChars);
}

template <typename IntegralType>
typename std::enable_if<!std::is_integral<IntegralType>::value, std::string>::type
intToHEXString(IntegralType num) {
    return "";
}

template <typename FloatingPointType>
typename std::enable_if<std::is_floating_point<FloatingPointType>::value, std::string>::type
floatToString(FloatingPointType num, int precision) {
    ssize_t wholePart = static_cast<ssize_t>(num);
    FloatingPointType decimalPart = num - wholePart;
    std::string output = std::to_string(wholePart);
    if (precision > 0) {
        output.append(".");
        size_t decimalShifted = decimalPart * std::pow(10, precision);
        output.append(std::to_string(decimalShifted));
    }
    return output;
}
template <typename FloatingPointType>
typename std::enable_if<!std::is_floating_point<FloatingPointType>::value, std::string>::type
floatToString(FloatingPointType num, int precision) {
    return "";
}

template <typename T>
typename std::enable_if<std::is_fundamental<T>::value, std::string>::type
__getString__(T val, Formatting formatting) {
    switch (formatting.type) {
    case Formatting::Type::Int:
        switch (formatting.options.intFormat) {
        case Formatting::Options::IntegralFormat::Bin:
            return intToBinaryString(val);
            break;
        case Formatting::Options::IntegralFormat::Oct:
            return intToOctString(val);
            break;
        case Formatting::Options::IntegralFormat::Dec:
            return std::to_string(val);
            break;
        case Formatting::Options::IntegralFormat::Hex:
            return intToHexString(val);
            break;
        case Formatting::Options::IntegralFormat::HEX:
            return intToHEXString(val);
            break;
        }
        break;
    case Formatting::Type::Float:;
        return floatToString(val, formatting.options.precision);
    case Formatting::Type::Bool:;
    default:
        return std::to_string(val);
    }
    return std::to_string(val);
}

template <typename T>
typename std::enable_if<!std::is_fundamental<T>::value, std::string>::type
__getString__(T val, Formatting formatting) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}

template <size_t Idx, typename F, typename Head, typename... Tail>
static void for_each_impl(F &&lambda, Head head, Tail... tail) {
    lambda(Idx, head);
    for_each_impl<Idx + 1>(lambda, tail...);
}

// empty argument pack
template <size_t Idx, typename F> static void for_each_impl(F &&lambda) {}

template <typename F, typename Head, typename... Tail>
static void for_each(F &&lambda, Head head, Tail... tail) {
    for_each_impl<0>(lambda, head, tail...);
}

const char *typeToString(Formatting::Type type) {
    switch (type) {
    case Formatting::Type::Int:
        return "Int";
    case Formatting::Type::Float:
        return "Float";
    case Formatting::Type::Bool:
        return "Bool";
    case Formatting::Type::Obj:
        return "Obj";
        break;
    case Formatting::Type::Unknown:
    default:
        return "Unknown";
    }
}

std::string intFormatToString(Formatting::Options::IntegralFormat intFormat) {
    switch (intFormat) {
    case Formatting::Options::Bin:
        return "Bin";
    case Formatting::Options::Oct:
        return "Oct";
    case Formatting::Options::Dec:
        return "Dec";
    case Formatting::Options::Hex:
        return "Hex";
    case Formatting::Options::HEX:
        return "HEX";
        break;
    default:
        return "Unknown";
    }
}

std::string optionsToString(Formatting::Options options) {
    std::stringstream ss;
    ss << "[ ";
    ss << "precision: " << options.precision;
    ss << ", intFormat: " << intFormatToString(options.intFormat);
    ss << " ]";
    return ss.str();
}

template <typename... Args, size_t len>
void __format(const char *str, const Locations<len> &locations, std::string *output, Args... args) {
    std::vector<std::string> arg_strs;
    for_each(
        [&arg_strs, &locations](size_t idx, auto arg) {
            arg_strs.push_back(__getString__<decltype(arg)>(arg, locations.forms[idx]));
        },
        args...);

    std::stringstream ss;
    const char *current = str;
    for (int i = 0; i < std::min(locations.num, arg_strs.size()); i++) {
        if (locations.formatBegin[i] != nullptr && locations.formatEnd != nullptr &&
            locations.formatBegin[i][0] != '\0') {
            ss.write(current, locations.formatBegin[i] - current);
            ss << arg_strs[i];
            current = locations.formatEnd[i] + 1;
        }
    }
    ss << current;
    *output = ss.str();
}

#define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
                 _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,   \
                 _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50,   \
                 _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, N, ...)          \
    N
#define PP_RSEQ_N()                                                                                \
    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41,    \
        40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,    \
        18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define format(__literal__, ...)                                                                   \
    {                                                                                              \
        constexpr const char *__str__ = __literal__;                                               \
        constexpr size_t __size__ = sizeof(__literal__);                                           \
        constexpr const char *__end__ = __str__ + __size__ - 1;                                    \
        constexpr auto __locations__ = sl::Locations<PP_NARG(__VA_ARGS__)>(__str__, __end__);      \
        std::string __formatted__ = "";                                                            \
        sl::__format(__str__, __locations__, &__formatted__, ##__VA_ARGS__);                       \
        std::cout << __formatted__ << std::endl;                                                   \
    } // namespace sl
} // namespace sl
