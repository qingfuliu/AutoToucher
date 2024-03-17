/*
 * @Author: qingfuliu 1805003304@qq.com
 * @Date: 2024-03-11 15:30:05
 * @LastEditors: qingfuliu 1805003304@qq.com
 * @LastEditTime: 2024-03-12 12:07:37
 * @FilePath: /AutoToucher/src/configurator/auto_config.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef AUTOTOUCHER_AUTO_CONFIG_HPP
#define AUTOTOUCHER_AUTO_CONFIG_HPP

#include <array>  // std::array
#include <cassert>
#include <cmath>
#include <cstdint>  // std::int64_t
#include <functional>
#include <iostream>
#include <istream>  // std::basic_istream
#include <limits>
#include <map>
#include <memory>
#include <ostream>  // std::basic_ostream
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>  // std::vector

#define DESERIALIZE_FUNCTION_NAME DeSerialize
#define SERIALIZE_FUNCTION_NAME Serialize

namespace json {
class Json;
}
namespace yaml {
class yaml;
}

namespace clsn {
template <typename _TypeWrapper, typename SourceCharType>
class ConfigItem;

namespace helper {

template <class _ItemTy, class _ValueTy>
std::enable_if_t<std::is_same_v<decltype(std::declval<const _ValueTy &>().SERIALIZE_FUNCTION_NAME()), _ItemTy>, void>
SERIALIZE_FUNCTION_NAME(_ItemTy &item, const _ValueTy &val) {
  item = val.SERIALIZE_FUNCTION_NAME();
}

template <class _ItemTy, class _ValueTy>
std::enable_if_t<
    std::is_same_v<decltype(std::declval<_ValueTy &>().DESERIALIZE_FUNCTION_NAME(std::declval<const _ItemTy &>())),
                   void>,
    void>
DESERIALIZE_FUNCTION_NAME(const _ItemTy &item, _ValueTy &val) {
  val.DESERIALIZE_FUNCTION_NAME(item);
}

#define MEMBER_FUNC_HELPER(FuncName, ItemType, ValueType)                                                       \
  template <typename _ItemTy, class _ValueTy,                                                                   \
            typename v = std::enable_if_t<                                                                      \
                std::is_same_v<void, decltype(FuncName(std::declval<ItemType>(), std::declval<ValueType>()))>>> \
  std::true_type HasMember##FuncName##FuncHelper(int);                                                          \
                                                                                                                \
  template <class sr, typename T>                                                                               \
  std::false_type HasMember##FuncName##FuncHelper(...);

MEMBER_FUNC_HELPER(Serialize, _ItemTy &, const _ValueTy &)

template <typename _ItemTy, class _ValueTy>
struct HasMemberSerializeFunc
    : public decltype(HasMemberSerializeFuncHelper<std::decay_t<_ItemTy>, std::decay_t<_ValueTy>>(0)) {};

template <class _ItemTy, class _ValueTy>
inline constexpr bool HasMemberSerializeFuncV = HasMemberSerializeFunc<_ItemTy, _ValueTy>::value;

MEMBER_FUNC_HELPER(DeSerialize, const _ItemTy &, _ValueTy &)

template <typename _ItemTy, class _ValueTy>
struct HasMemberDeSerializeFunc
    : public decltype(HasMemberDeSerializeFuncHelper<std::decay_t<_ItemTy>, std::decay_t<_ValueTy>>(0)) {};

template <typename _ItemTy, class _ValueTy>
inline constexpr bool HasMemberDeSerializeFuncV = HasMemberDeSerializeFunc<_ItemTy, _ValueTy>::value;

}  // namespace helper

}  // namespace clsn

namespace detail::configor::encoding {

// Copyright (c) 2018-2020 configor - Nomango
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

namespace unicode {
namespace constants {
constexpr auto SURROGATE_BASE = static_cast<uint32_t>(0x10000);
constexpr auto LEAD_SURROGATE_BEGIN = static_cast<uint32_t>(0xD800);
constexpr auto LEAD_SURROGATE_END = static_cast<uint32_t>(0xDBFF);
constexpr auto TRAIL_SURROGATE_BEGIN = static_cast<uint32_t>(0xDC00);
constexpr auto TRAIL_SURROGATE_END = static_cast<uint32_t>(0xDFFF);
constexpr auto TRAIL_SURROGATE_MAX = static_cast<uint32_t>(0x3FF);
constexpr auto SURROGATE_BITS = static_cast<uint32_t>(10);
}  // namespace constants

inline auto IsLeadSurrogate(const uint32_t codepoint) -> bool {
  return constants::LEAD_SURROGATE_BEGIN <= codepoint && codepoint <= constants::LEAD_SURROGATE_END;
}

inline auto IsTrailSurrogate(const uint32_t codepoint) -> bool {
  return constants::TRAIL_SURROGATE_BEGIN <= codepoint && codepoint <= constants::TRAIL_SURROGATE_END;
}

inline auto DecodeSurrogates(uint32_t lead_surrogate, uint32_t trail_surrogate) -> uint32_t {
  uint32_t codepoint = ((lead_surrogate - constants::LEAD_SURROGATE_BEGIN) << constants::SURROGATE_BITS);
  codepoint += (trail_surrogate - constants::TRAIL_SURROGATE_BEGIN);
  codepoint += constants::SURROGATE_BASE;
  return codepoint;
}

inline void EncodeSurrogates(uint32_t codepoint, uint32_t &lead_surrogate, uint32_t &trail_surrogate) {
  codepoint = codepoint - constants::SURROGATE_BASE;
  lead_surrogate = static_cast<uint16_t>(constants::LEAD_SURROGATE_BEGIN + (codepoint >> constants::SURROGATE_BITS));
  trail_surrogate =
      static_cast<uint16_t>(constants::TRAIL_SURROGATE_BEGIN + (codepoint & constants::TRAIL_SURROGATE_MAX));
}

}  // namespace unicode

template <typename _CharTy>
using encoder = void (*)(std::basic_ostream<_CharTy> &, uint32_t);

template <typename _CharTy>
using decoder = bool (*)(std::basic_istream<_CharTy> &, uint32_t &);

template <typename _CharTy>
class Ignore {
 public:
  using char_type = _CharTy;
  using traits_type = std::char_traits<char_type>;
  using istream_type = std::basic_istream<char_type>;
  using ostream_type = std::basic_ostream<char_type>;

  static void Encode(ostream_type &os, uint32_t codepoint) { os.put(static_cast<char_type>(codepoint)); }

  static auto Decode(istream_type &is, uint32_t &codepoint) -> bool {
    codepoint = static_cast<uint32_t>(static_cast<char_type>(is.get()));
    return !is.eof();
  }
};

template <typename _CharTy>
class EncodeSurrogates {
 public:
  using char_type = _CharTy;
  using traits_type = std::char_traits<char_type>;
  using istream_type = std::basic_istream<char_type>;
  using ostream_type = std::basic_ostream<char_type>;

  static void Encode(ostream_type &os, uint32_t codepoint) {
    // Unicode              UTF-8
    // U+0000...U+007F      0xxxxxxx
    // U+0080...U+07FF      110xxxxx 10xxxxxx
    // U+0800...U+FFFF      1110xxxx 10xxxxxx 10xxxxxx
    // U+10000...U+10FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if (codepoint < 0x80) {
      // 0xxxxxxx
      os.put(static_cast<char_type>(codepoint));
    } else if (codepoint <= 0x7FF) {
      // 110xxxxx 10xxxxxx
      os.put(static_cast<char_type>(0xC0 | (codepoint >> 6)));
      os.put(static_cast<char_type>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
      // 1110xxxx 10xxxxxx 10xxxxxx
      os.put(static_cast<char_type>(0xE0 | (codepoint >> 12)));
      os.put(static_cast<char_type>(0x80 | ((codepoint >> 6) & 0x3F)));
      os.put(static_cast<char_type>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0x10FFFF) {
      // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      os.put(static_cast<char_type>(0xF0 | (codepoint >> 18)));
      os.put(static_cast<char_type>(0x80 | ((codepoint >> 12) & 0x3F)));
      os.put(static_cast<char_type>(0x80 | ((codepoint >> 6) & 0x3F)));
      os.put(static_cast<char_type>(0x80 | (codepoint & 0x3F)));
    } else {
      os.setstate(std::ios_base::failbit);
    }
  }

  static auto Decode(istream_type &is, uint32_t &codepoint) -> bool {
    // Unicode              UTF-8
    // U+0000...U+007F      0xxxxxxx
    // U+0080...U+07FF      110xxxxx 10xxxxxx
    // U+0800...U+FFFF      1110xxxx 10xxxxxx 10xxxxxx
    // U+10000...U+10FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    static const std::array<std::uint8_t, 256> encode_surrogates_extra_bytes_256 = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
    };

    static const std::array<std::uint32_t, 6> encode_surrogates_extra_bytes_6 = {
        0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080,
    };

    // peek one byte and check eof
    const auto first_byte = static_cast<uint8_t>(is.peek());

    if (is.eof()) {
      return false;
    }

    codepoint = 0;

    // read bytes
    const auto extra_bytes_to_read = encode_surrogates_extra_bytes_256[first_byte];
    switch (extra_bytes_to_read) {
      case 5:
        codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
        codepoint <<= 6;
        [[fallthrough]];
      case 4:
        codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
        codepoint <<= 6;
        [[fallthrough]];
      case 3:
        codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
        codepoint <<= 6;
        [[fallthrough]];
      case 2:
        codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
        codepoint <<= 6;
        [[fallthrough]];
      case 1:
        codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
        codepoint <<= 6;
        [[fallthrough]];
      case 0:
        codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
    }
    codepoint -= encode_surrogates_extra_bytes_6[extra_bytes_to_read];
    if (codepoint > 0x10FFFF) {
      is.setstate(std::ios_base::failbit);
    }
    return true;
  }
};

template <typename _CharTy>
class Utf16 {
 public:
  using char_type = _CharTy;
  using traits_type = std::char_traits<char_type>;
  using istream_type = std::basic_istream<char_type>;
  using ostream_type = std::basic_ostream<char_type>;

  static_assert(sizeof(char_type) >= 2, "The size of utf16 characters must be larger than 16 bits");

  static void Encode(ostream_type &os, uint32_t codepoint) {
    if (codepoint <= 0xFFFF) {
      os.put(traits_type::to_char_type(static_cast<typename traits_type::int_type>(codepoint)));
    } else if (codepoint <= 0x10FFFF) {
      uint32_t lead_surrogate = 0;
      uint32_t trail_surrogate = 0;
      unicode::EncodeSurrogates(codepoint, lead_surrogate, trail_surrogate);
      os.put(traits_type::to_char_type(static_cast<typename traits_type::int_type>(lead_surrogate)));
      os.put(traits_type::to_char_type(static_cast<typename traits_type::int_type>(trail_surrogate)));
    } else {
      os.setstate(std::ios_base::failbit);
    }
  }

  static auto Decode(istream_type &is, uint32_t &codepoint) -> bool {
    codepoint = static_cast<uint32_t>(static_cast<uint16_t>(is.get()));

    if (is.eof()) {
      return false;
    }

    if (unicode::IsLeadSurrogate(codepoint)) {
      uint32_t lead_surrogate = codepoint;
      auto trail_surrogate = static_cast<uint32_t>(static_cast<uint16_t>(is.get()));

      if (unicode::IsTrailSurrogate(trail_surrogate)) {
        codepoint = unicode::DecodeSurrogates(lead_surrogate, trail_surrogate);
      } else {
        is.setstate(std::ios_base::failbit);
      }
    }

    if (codepoint > 0x10FFFF) {
      is.setstate(std::ios_base::failbit);
    }
    return true;
  }
};

template <typename _CharTy>
class Utf32 {
 public:
  using char_type = _CharTy;
  using traits_type = std::char_traits<char_type>;
  using istream_type = std::basic_istream<char_type>;
  using ostream_type = std::basic_ostream<char_type>;

  static_assert(sizeof(char_type) >= 4, "The size of utf32 characters must be larger than 32 bits");

  static void Encode(ostream_type &os, uint32_t codepoint) {
    if (codepoint > 0x10FFFF) {
      os.setstate(std::ios_base::failbit);
    }
    os.put(traits_type::to_char_type(static_cast<typename traits_type::int_type>(codepoint)));
  }

  static auto Decode(istream_type &is, uint32_t &codepoint) -> bool {
    codepoint = static_cast<uint32_t>(is.get());

    if (is.eof()) {
      return false;
    }

    if (codepoint > 0x10FFFF) {
      is.setstate(std::ios_base::failbit);
    }
    return true;
  }
};

template <typename _CharTy = char>
class AutoUtf {
 public:
  using char_type = _CharTy;
  using traits_type = std::char_traits<char_type>;
  using istream_type = std::basic_istream<char_type>;
  using ostream_type = std::basic_ostream<char_type>;

  static inline void Encode(ostream_type &os, uint32_t codepoint) {
    Encode(os, codepoint, std::integral_constant<int, sizeof(char_type)>());
  }

  static inline auto Decode(istream_type &is, uint32_t &codepoint) -> bool {
    return Decode(is, codepoint, std::integral_constant<int, sizeof(char_type)>());
  }

 private:
  static inline void Encode(ostream_type &os, uint32_t codepoint, std::integral_constant<int, 1> v) {
    EncodeSurrogates<char_type>::Encode(os, codepoint);
  }

  static inline void Encode(ostream_type &os, uint32_t codepoint, std::integral_constant<int, 2> v) {
    Utf16<char_type>::Encode(os, codepoint);
  }

  static inline void Encode(ostream_type &os, uint32_t codepoint, std::integral_constant<int, 4> v) {
    Utf32<char_type>::Encode(os, codepoint);
  }

  static inline auto Decode(istream_type &is, uint32_t &codepoint, std::integral_constant<int, 1> v) -> bool {
    return EncodeSurrogates<char_type>::Decode(is, codepoint);
  }

  static inline auto Decode(istream_type &is, uint32_t &codepoint, std::integral_constant<int, 2> v) -> bool {
    return Utf16<char_type>::Decode(is, codepoint);
  }

  static inline auto Decode(istream_type &is, uint32_t &codepoint, std::integral_constant<int, 4> v) -> bool {
    return Utf32<char_type>::Decode(is, codepoint);
  }
};

//
// type traits
//

template <typename _Encoding>
struct IsUnicodeEncoding : std::false_type {};

template <typename _CharTy>
struct IsUnicodeEncoding<EncodeSurrogates<_CharTy>> : std::true_type {};

template <typename _CharTy>
struct IsUnicodeEncoding<Utf16<_CharTy>> : std::true_type {};

template <typename _CharTy>
struct IsUnicodeEncoding<Utf32<_CharTy>> : std::true_type {};

template <typename _CharTy>
struct IsUnicodeEncoding<AutoUtf<_CharTy>> : std::true_type {};
}  // namespace detail::configor::encoding

/**
 * @Author: qingfuliu
 * @description: definations of error
 * @return {*}
 */
namespace clsn {
class OperationError : public std::logic_error {
 public:
  explicit OperationError(const std::string &type, const std::string &operation) noexcept
      : std::logic_error(type + " type objects do not support operation " + operation) {}
  explicit OperationError(const std::string &type, const std::string &operation, const std::string &message) noexcept
      : std::logic_error("An item of " + type + " encountered error " + message + " during operation " + operation) {}
};

}  // namespace clsn
// ConfigItem
namespace clsn {
namespace helper {
/**
 * @Author: qingfuliu
 * @name: HELPER_CONCAT
 * @description: 拼接A和B
 * @return {*}
 */
#define HELPER_CONCAT(A, B) HELPER_CONCAT_IMPL(A, B)
#define HELPER_CONCAT_IMPL(A, B) A##B

/**
 * @Author: qingfuliu
 * @name: HELPER_COMMA_IF
 * @description: N非0，产生一个逗号，最多支持到32
 * @return {*}
 */
#ifndef HELPER_COMMA_IF
#define HELPER_COMMA_IF(N) HELPER_CONCAT(HELPER_COMMA_IF_, N)
#define HELPER_COMMA_IF_0
#define HELPER_COMMA_IF_1 ,
#define HELPER_COMMA_IF_2 ,
#define HELPER_COMMA_IF_3 ,
#define HELPER_COMMA_IF_4 ,
#define HELPER_COMMA_IF_5 ,
#define HELPER_COMMA_IF_6 ,
#define HELPER_COMMA_IF_7 ,
#define HELPER_COMMA_IF_8 ,
#define HELPER_COMMA_IF_9 ,
#define HELPER_COMMA_IF_10 ,
#define HELPER_COMMA_IF_11 ,
#define HELPER_COMMA_IF_12 ,
#define HELPER_COMMA_IF_13 ,
#define HELPER_COMMA_IF_14 ,
#define HELPER_COMMA_IF_15 ,
#define HELPER_COMMA_IF_16 ,
#define HELPER_COMMA_IF_17 ,
#define HELPER_COMMA_IF_19 ,
#define HELPER_COMMA_IF_19 ,
#define HELPER_COMMA_IF_20 ,
#define HELPER_COMMA_IF_21 ,
#define HELPER_COMMA_IF_22 ,
#define HELPER_COMMA_IF_23 ,
#define HELPER_COMMA_IF_24 ,
#define HELPER_COMMA_IF_25 ,
#define HELPER_COMMA_IF_26 ,
#define HELPER_COMMA_IF_27 ,
#define HELPER_COMMA_IF_28 ,
#define HELPER_COMMA_IF_29 ,
#define HELPER_COMMA_IF_30 ,
#define HELPER_COMMA_IF_31 ,
#endif

/**
 * @Author: qingfuliu
 * @name: HELPER_COMMA_IF_NOT(N)
 * @param: 数字
 * @description: N0，产生一个逗号，最多支持到32
 */
#ifndef HELPER_COMMA_IF_NOT
#define HELPER_COMMA_IF_NOT(N) HELPER_CONCAT(HELPER_COMMA_IF_, N)
#define HELPER_COMMA_IF_NOT_0 ,
#define HELPER_COMMA_IF_NOT_1
#define HELPER_COMMA_IF_NOT_2
#define HELPER_COMMA_IF_NOT_3
#define HELPER_COMMA_IF_NOT_4
#define HELPER_COMMA_IF_NOT_5
#define HELPER_COMMA_IF_NOT_6
#define HELPER_COMMA_IF_NOT_7
#define HELPER_COMMA_IF_NOT_8
#define HELPER_COMMA_IF_NOT_9
#define HELPER_COMMA_IF_NOT_10
#define HELPER_COMMA_IF_NOT_11
#define HELPER_COMMA_IF_NOT_12
#define HELPER_COMMA_IF_NOT_13
#define HELPER_COMMA_IF_NOT_14
#define HELPER_COMMA_IF_NOT_15
#define HELPER_COMMA_IF_NOT_16
#define HELPER_COMMA_IF_NOT_17
#define HELPER_COMMA_IF_NOT_19
#define HELPER_COMMA_IF_NOT_19
#define HELPER_COMMA_IF_NOT_20
#define HELPER_COMMA_IF_NOT_21
#define HELPER_COMMA_IF_NOT_22
#define HELPER_COMMA_IF_NOT_23
#define HELPER_COMMA_IF_NOT_24
#define HELPER_COMMA_IF_NOT_25
#define HELPER_COMMA_IF_NOT_26
#define HELPER_COMMA_IF_NOT_27
#define HELPER_COMMA_IF_NOT_28
#define HELPER_COMMA_IF_NOT_29
#define HELPER_COMMA_IF_NOT_30
#define HELPER_COMMA_IF_NOT_31
#endif

/**
 * @Author: qingfuliu
 * @name: HELPER_INVOKE_IF_NOT_EMPTY(N,...)
 * @param: N
 * @description: ...非0，产生一个N，最多支持到32
 */
#ifndef HELPER_INVOKE_IF_NOT_EMPTY
#define HELPER_INVOKE_IF_NOT_EMPTY(FUNC, XX, CTX, ...)                 \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (FUNC, XX, CTX, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_0(...)
#define HELPER_INVOKE_IF_NOT_EMPTY_1(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_2(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_3(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_4(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_5(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_6(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_7(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_8(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_9(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_10(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_11(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_12(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_13(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_14(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_15(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_16(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_17(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_19(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_19(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_20(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_21(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_22(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_23(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_24(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_25(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_26(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_27(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_28(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_29(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_30(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#define HELPER_INVOKE_IF_NOT_EMPTY_31(FUNC, XX, CTX, ARG, ...) FUNC(XX, CTX, ARG, __VA_ARGS__)
#endif

/**
 * @Author: qingfuliu
 * @name: HELPER_NARG
 * @description: 获取一共有多少参数
 */
#define HELPER_ARG_X(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, \
                     t, u, v, w, x, y, z, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y,  \
                     Z, XX, ...)                                                                                      \
  XX

#define HELPER_NARG(...)                                                                                               \
  HELPER_ARG_X("ignore", ##__VA_ARGS__, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43,    \
               42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
               16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/**
 * @Author: qingfuliu
 * @name: HELPER_NARG_RANGE_WITH_FUNC(FUNC, CTX, ...)
 * @param: FUNC 接收两个参数的宏，第二个参数为Item的下标，从0开始
 * @description:  循环调用FUNC(CTX,Arg,Index),最多支持32个
 */
#ifndef HELPER_NARG_RANGE_WITH_FUNC
#define HELPER_NARG_RANGE_WITH_FUNC(FUNC, CTX, ...) \
  HELPER_CONCAT(HELPER_NARG_RANGE_WITH_FUNC_, 0)    \
  (FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_WRAP(FUNC, CTX, B) \
  HELPER_CONCAT(HELPER_NARG_RANGE_WITH_FUNC_, 0)       \
  (FUNC, CTX, B)

#define HELPER_NARG_RANGE_WITH_FUNC_0(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 0)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_1, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_1(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 1)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_2, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_2(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 2)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_3, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_3(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 3)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_4, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_4(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 4)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_5, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_5(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 5)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_6, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_6(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 6)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_7, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_7(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 7)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_8, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_8(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 8)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_9, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_9(FUNC, CTX, Arg, ...)             \
  FUNC(CTX, Arg, 9)                                                    \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_10, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_10(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 10)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_11, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_11(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 11)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_12, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_12(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 12)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_13, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_13(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 13)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_14, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_14(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 14)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_15, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_15(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 15)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_16, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_16(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 16)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_17, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_17(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 17)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_18, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_18(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 18)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_19, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_19(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 19)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_20, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_20(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 20)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_21, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_21(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 21)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_22, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_22(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 22)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_23, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_23(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 23)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_24, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_24(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 24)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_25, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_25(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 25)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_26, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_26(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 26)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_27, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_27(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 27)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_28, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_28(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 28)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_29, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_29(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 29)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_30, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_30(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 30)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_31, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_31(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 31)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_32, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_32(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 32)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_33, FUNC, CTX, __VA_ARGS__)

#define HELPER_NARG_RANGE_WITH_FUNC_33(FUNC, CTX, Arg, ...)            \
  FUNC(CTX, Arg, 33)                                                   \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (HELPER_NARG_RANGE_WITH_FUNC_34, FUNC, CTX, __VA_ARGS__)

#endif
/**
 * @Author: qingfuliu
 * @name: HELPER_EXTRACT_EVEN
 * @description: 提取出偶数项，用逗号连接，最多支持到64
 * @return {*}
 */
#ifndef HELPER_EXTRACT_EVEN
#define HELPER_EXTRACT_EVEN(...) HELPER_CONCAT(HELPER_EXTRACT_EVEN_, HELPER_NARG(__VA_ARGS__))(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_0()

#define HELPER_EXTRACT_EVEN_1(SINGULAR)

#define HELPER_EXTRACT_EVEN_2(SINGULAR, EVEN) EVEN

#define HELPER_EXTRACT_EVEN_3(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_1(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_4(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_2(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_5(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_3(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_6(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_4(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_7(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_5(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_8(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_6(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_9(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_7(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_10(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_8(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_11(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_9(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_12(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_10(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_13(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_11(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_14(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_12(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_15(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_13(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_16(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_14(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_17(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_15(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_18(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_16(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_19(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_17(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_20(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_18(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_21(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_19(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_22(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_20(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_23(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_21(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_24(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_22(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_25(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_23(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_26(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_24(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_27(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_25(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_28(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_26(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_29(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_27(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_30(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_28(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_31(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_29(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_32(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_30(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_33(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_31(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_34(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_32(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_35(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_33(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_36(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_34(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_37(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_35(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_38(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_36(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_39(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_37(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_40(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_38(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_41(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_39(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_42(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_40(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_43(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_41(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_44(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_42(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_45(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_43(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_46(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_44(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_47(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_45(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_48(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_46(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_49(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_47(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_50(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_48(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_51(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_49(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_52(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_50(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_53(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_51(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_54(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_52(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_55(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_53(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_56(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_54(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_57(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_55(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_58(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_56(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_59(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_57(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_60(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_58(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_61(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_59(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_62(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_60(__VA_ARGS__)

#define HELPER_EXTRACT_EVEN_63(SINGULAR, EVEN, ...) EVEN, HELPER_EXTRACT_EVEN_61(__VA_ARGS__)

#endif

/**
 * @Author: qingfuliu
 * @name: HELPER_EXTRACTSINGULAR
 * @description: 提取出奇数数项，用逗号连接，最多支持到64
 * @return {*}
 */
#ifndef HELPER_EXTRACTSINGULAR
#define HELPER_EXTRACT_SINGULAR(...) HELPER_CONCAT(HELPER_EXTRACT_SINGULAR_, HELPER_NARG(__VA_ARGS__))(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_0()

#define HELPER_EXTRACT_SINGULAR_1(SINGULAR)

#define HELPER_EXTRACT_SINGULAR_2(SINGULAR, EVEN) SINGULAR

#define HELPER_EXTRACT_SINGULAR_3(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_1(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_4(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_2(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_5(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_3(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_6(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_4(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_7(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_5(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_8(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_6(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_9(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_7(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_10(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_8(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_11(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_9(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_12(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_10(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_13(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_11(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_14(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_12(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_15(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_13(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_16(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_14(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_17(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_15(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_18(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_16(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_19(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_17(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_20(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_18(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_21(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_19(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_22(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_20(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_23(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_21(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_24(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_22(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_25(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_23(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_26(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_24(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_27(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_25(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_28(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_26(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_29(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_27(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_30(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_28(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_31(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_29(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_32(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_30(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_33(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_31(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_34(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_32(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_35(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_33(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_36(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_34(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_37(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_35(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_38(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_36(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_39(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_37(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_40(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_38(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_41(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_39(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_42(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_40(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_43(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_41(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_44(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_42(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_45(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_43(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_46(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_44(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_47(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_45(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_48(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_46(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_49(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_47(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_50(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_48(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_51(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_49(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_52(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_50(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_53(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_51(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_54(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_52(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_55(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_53(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_56(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_54(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_57(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_55(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_58(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_56(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_59(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_57(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_60(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_58(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_61(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_59(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_62(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_60(__VA_ARGS__)

#define HELPER_EXTRACT_SINGULAR_63(SINGULAR, EVEN, ...) SINGULAR, HELPER_EXTRACT_SINGULAR_61(__VA_ARGS__)
#endif

template <class _X1, class _X2, class... _Args>
struct IsContain : public std::conditional_t<IsContain<_X1, _X2>::value, std::true_type, IsContain<_X1, _Args...>> {};
template <class _X1, class _X2>
struct IsContain<_X1, _X2> : public std::integral_constant<bool, std::is_same_v<std::decay_t<_X1>, std::decay_t<_X2>>> {
};

template <class _X>
struct ISArithmeticHelper : public std::integral_constant<bool, std::is_arithmetic_v<std::decay_t<_X>> &&
                                                                    !std::is_same_v<std::decay_t<_X>, bool>> {
  using type = std::conditional_t<std::is_integral_v<_X>, int64_t, double>;
};

template <class _X1, class _X2, class... _Args>
struct IsContainWeak
    : public std::conditional_t<
          ISArithmeticHelper<std::decay_t<_X1>>::value, ISArithmeticHelper<std::decay_t<_X1>>,
          std::conditional_t<IsContainWeak<_X1, _X2>::value, IsContainWeak<_X1, _X2>, IsContainWeak<_X1, _Args...>>> {};
template <class _X1, class _X2>
struct IsContainWeak<_X1, _X2>
    : public std::integral_constant<bool, std::is_constructible_v<std::decay_t<_X1>, std::decay_t<_X2>> &&
                                              std::is_constructible_v<std::decay_t<_X2>, std::decay_t<_X1>>> {
  using type = _X2;
};

}  // namespace helper
/**
 * @Author: qingfuliu
 * @description: A configuration item
 */
template <typename _TypeWrapper, typename SourceCharType = char>
class ConfigItem {
  /**
   * @Author: qingfuliu
   * @description: 类型萃取
   */
 public:
  using PointerType = ConfigItem *;

  using CPointerType = const ConfigItem *;

  using SmartPointerType = std::unique_ptr<ConfigItem>;

  template <typename _Ty>
  using AllocatorType = typename _TypeWrapper::template AllocatorType<_Ty>;

  using BooleanType = typename _TypeWrapper::BooleanType;

  using IntegerType = typename _TypeWrapper::IntegerType;

  using FloatType = typename _TypeWrapper::FloatType;

  using CharType = typename _TypeWrapper::CharType;

  using StringType =
      typename _TypeWrapper::template StringType<CharType, std::char_traits<CharType>, AllocatorType<CharType>>;

  using ArrayType = typename _TypeWrapper::template ArrayType<ConfigItem, AllocatorType<ConfigItem>>;

  using MapType = typename _TypeWrapper::template MapType<StringType, ConfigItem, std::less<StringType>,
                                                          AllocatorType<std::pair<const StringType, ConfigItem>>>;

  using BinderType = typename _TypeWrapper::template BinderType<ConfigItem>;

  using ConstructType = typename _TypeWrapper::template ConstructType<std::decay_t<ConfigItem>>;

  using SizeType = std::size_t;

  using DifferenceType = std::ptrdiff_t;

  using IteratorType = typename _TypeWrapper::template Iterator<ConfigItem>;
  using CIteratorType = typename _TypeWrapper::template CIterator<ConfigItem>;

  friend BinderType;
  friend ConstructType;

  inline static constexpr IntegerType INTEGER_MAX = std::numeric_limits<IntegerType>::max();
  inline static constexpr IntegerType INTEGER_MIN = std::numeric_limits<IntegerType>::min();
  inline static constexpr FloatType FLOAT_MAX = std::numeric_limits<FloatType>::max();
  inline static constexpr FloatType FLOAT_MIN = std::numeric_limits<FloatType>::min();
  /**
   * @Author: qingfuliu
   * @description: 成员定义,类型判断函数定义,是否兼容此类型
   * @note: 增加类型请再TY_DEFINEF添加映射
   */
#define XX(CTX, ARG, INDEX) ARG = static_cast<uint8_t>(INDEX),
#define TYPE_JUDGEMENT_FUNCTION(CTX, ARG, INDEX) \
  auto Is##ARG() const noexcept -> bool { return m_value.index() == static_cast<size_t>(ItemType::ARG); }
#define ITEM_TYPE_TO_STRING(CTX, ARG, INDEX) \
  case static_cast<uint8_t>(ItemType::ARG):  \
    return #ARG;

#define TY_DEFINEF(...)                                                                                           \
 private:                                                                                                         \
  StringType m_key_{};                                                                                            \
  std::variant<HELPER_EXTRACT_EVEN(__VA_ARGS__)> m_value{};                                                       \
                                                                                                                  \
 public:                                                                                                          \
  enum class ItemType : uint8_t { HELPER_NARG_RANGE_WITH_FUNC_WRAP(XX, , HELPER_EXTRACT_SINGULAR(__VA_ARGS__)) }; \
  HELPER_NARG_RANGE_WITH_FUNC_WRAP(TYPE_JUDGEMENT_FUNCTION, , HELPER_EXTRACT_SINGULAR(__VA_ARGS__))               \
  template <class _Ty>                                                                                            \
  using SupportedTy = helper::IsContain<_Ty, HELPER_EXTRACT_EVEN(__VA_ARGS__)>;                                   \
  template <class _Ty>                                                                                            \
  using SupportedTyWeak = helper::IsContainWeak<_Ty, HELPER_EXTRACT_EVEN(__VA_ARGS__)>;                           \
  friend auto ItemTypeToString(ItemType type) -> StringType {                                                     \
    switch (static_cast<uint8_t>(type)) {                                                                         \
      HELPER_NARG_RANGE_WITH_FUNC_WRAP(ITEM_TYPE_TO_STRING, , HELPER_EXTRACT_SINGULAR(__VA_ARGS__))               \
    }                                                                                                             \
    return "Unknown";                                                                                             \
  }

  TY_DEFINEF(Undefined, std::monostate, Null, std::nullptr_t, Boolean, BooleanType, Scalar, std::string, Array,
             ArrayType, Object, MapType, Float, FloatType, Integer, IntegerType)
#undef TY_DEFINEF
#undef ITEM_TYPE_TO_STRING
#undef TYPE_JUDGEMENT_FUNCTION
#undef XX

  /**
   * @Author: qingfuliu
   * @description: 构造、拷贝、移动、比较
   * @return {*}
   */
 public:
  ConfigItem() = default;

  virtual ~ConfigItem() = default;

  ConfigItem(ConfigItem &&item) noexcept : m_key_(std::move(item.m_key_)), m_value(std::move(item.m_value)) {}

  ConfigItem(const ConfigItem &item) noexcept : m_key_(item.m_key_), m_value(item.m_value) {}

  template <class _Ty>
  ConfigItem(std::initializer_list<_Ty> &&init_list) {
    BinderType::Bind(*this, std::move(init_list));
  }

  template <class _Ty>
  ConfigItem(const std::initializer_list<_Ty> &init_list) {
    BinderType::Bind(*this, init_list);
  }

  template <class _Ty>
  explicit ConfigItem(_Ty val) {
    BinderType::Bind(*this, std::forward<_Ty>(val));
  }

  auto operator=(ConfigItem &&item) noexcept -> ConfigItem & {
    //    m_key_ = std::move(item.m_key_);
    m_value = std::move(item.m_value);
    return *this;
  }

  auto operator=(const ConfigItem &item) noexcept -> ConfigItem & {
    //    m_key_ = item.m_key_;
    m_value = item.m_value;
    return *this;
  }

  template <class _Ty>
  auto operator=(_Ty &&val) noexcept -> ConfigItem & {
    BinderType::Bind(*this, std::forward<_Ty>(val));
    return *this;
  }

  template <class _Ty>
  operator _Ty() const noexcept {
    return ConstructType::template Construct<_Ty>(*this);
  }

  auto operator==(const ConfigItem &item) const noexcept -> BooleanType {
    return m_key_ == item.m_key_ && m_value == item.m_value;
  }

  auto operator!=(const ConfigItem &item) const noexcept -> BooleanType { return !(*this == item); }

  auto Type() const noexcept -> ItemType { return static_cast<ItemType>(m_value.index()); }
  // access function
 public:
  template <class _Ty>
  auto operator[](_Ty &&key)
      -> std::enable_if_t<std::is_constructible_v<StringType, _Ty> && !std::is_same_v<StringType, std::decay_t<_Ty>>,
                          ConfigItem &> {
    return operator[](StringType{std::forward<_Ty>(key)});
  }

  template <class _Ty>
  auto operator[](_Ty &&key) -> std::enable_if_t<std::is_same_v<StringType, std::decay_t<_Ty>>, ConfigItem &> {
    Check(ItemType::Object, "[StringType]");
    MapType &map = GetMap();
    auto it = map.find(key);
    if (it != map.end()) {
      return it->second;
    }
    auto res = Insert({std::forward<_Ty>(key), static_cast<ConfigItem>(nullptr)});
    return res.first->second;
  }

  template <class _Ty>
  auto operator[](_Ty &&key) const
      -> std::enable_if_t<std::is_constructible_v<StringType, _Ty> && !std::is_same_v<StringType, std::decay_t<_Ty>>,
                          const ConfigItem &> {
    return operator[](StringType{std::forward<_Ty>(key)});
  }

  template <class _Ty>
  auto operator[](_Ty &&key) const
      -> std::enable_if_t<std::is_same_v<StringType, std::decay_t<_Ty>>, const ConfigItem &> {
    Check(ItemType::Object, "[StringType]");
    const MapType &map = GetMap();
    auto it = map.find(key);
    if (it == map.end()) {
      throw OperationError(ItemTypeToString(Type()), "operator[](StringType) const");
    }
    return it->second;
  }

  auto operator[](SizeType index) -> ConfigItem & {
    Check(ItemType::Array, "[SizeType]");
    ArrayType &array = GetArray();
    if (array.size() <= index) {
      throw OperationError("[Array]", "[SizeType]", "[Index greater than array length]");
    }
    return array[index];
  }

  auto operator[](SizeType index) const -> const ConfigItem & {
    Check(ItemType::Array, "[SizeType]");
    const ArrayType &array = GetArray();
    if (array.size() <= index) {
      if (array.size() <= index) {
        throw OperationError("[Array]", "[SizeType]", "[Index greater than array length]");
      }
    }
    return array[index];
  }

  auto GetKey() noexcept -> StringType & { return m_key_; }

  auto GetKey() const noexcept -> const StringType & { return m_key_; }

  void SetKey(StringType &&key) noexcept { m_key_ = std::move(key); }

  void SetKey(const StringType &key) noexcept { m_key_ = key; }

  auto Size() const -> SizeType {
    switch (m_value.index()) {
      case static_cast<size_t>(ItemType::Scalar):
        return std::get<StringType>(m_value).size();
      case static_cast<size_t>(ItemType::Array):
        return std::get<ArrayType>(m_value).size();
      case static_cast<size_t>(ItemType::Object):
        return std::get<MapType>(m_value).size();
      default:
        throw OperationError(ItemTypeToString(Type()), "Size");
        break;
    }
    return 0;
  }

  auto Length() const -> SizeType { return Size(); }

  auto Empty() const -> BooleanType { return 0 == Size(); }

  auto GetArray() -> ArrayType & {
    Check(ItemType::Array, "GetArray");
    return std::get<ArrayType>(this->m_value);
  }

  auto GetArray() const -> const ArrayType & {
    Check(ItemType::Array, "GetArray");
    return std::get<ArrayType>(this->m_value);
  }

  auto GetMap() -> MapType & {
    Check(ItemType::Object, "GetMap");
    return std::get<MapType>(this->m_value);
  }

  auto GetMap() const -> const MapType & {
    Check(ItemType::Object, "GetMap");
    return std::get<MapType>(this->m_value);
  }

  auto GetScalar() -> StringType & {
    Check(ItemType::Scalar, "GetScalar");
    return std::get<StringType>(this->m_value);
  }

  auto GetScalar() const -> const StringType & {
    Check(ItemType::Scalar, "GetScalar");
    return std::get<StringType>(this->m_value);
  }

  // Iterator
 public:
  auto Begin() -> IteratorType {
    auto it = IteratorType(this);
    it.SetBegin();
    return it;
  }
  auto End() -> IteratorType {
    auto it = IteratorType(this);
    it.SetEnd();
    return it;
  }
  auto CBegin() const -> CIteratorType {
    auto it = CIteratorType(this);
    it.SetBegin();
    return it;
  }
  auto CEnd() const -> CIteratorType {
    auto it = CIteratorType(this);
    it.SetEnd();
    return it;
  }

 public:
  template <class _Ty>
  void PushBack(_Ty &&val) {
    Check(ItemType::Array, "PushBack");
    std::get<ArrayType>(m_value).push_back(std::forward<_Ty>(val));
  }

  template <class... _Ty>
  void EmplaceBack(_Ty &&...val) {
    Check(ItemType::Array, "Back");
    std::get<ArrayType>(m_value).emplace_back(std::forward<_Ty>(val)...);
  }

  auto Front() const -> const ConfigItem & {
    Check(ItemType::Array, "Front");
    return std::get<ArrayType>(m_value).front();
  }

  auto Front() -> ConfigItem & {
    Check(ItemType::Array, "Front");
    return std::get<ArrayType>(m_value).front();
  }

  auto Back() const -> const ConfigItem & {
    Check(ItemType::Array, "Back");
    return std::get<ArrayType>(m_value).back();
  }

  auto Back() -> ConfigItem & {
    Check(ItemType::Array, "Back");
    return std::get<ArrayType>(m_value).back();
  }

  void PopBack() {
    Check(ItemType::Array, "PopBack");
    std::get<ArrayType>(m_value).pop_back();
  }

  auto Insert(const std::pair<StringType, ConfigItem> &val_pair) {
    Check(ItemType::Object, "Insert");
    auto it = std::get<MapType>(m_value).insert(val_pair);
    it.first->second.m_key_ = val_pair.first;
    return it;
  }

  auto Insert(std::pair<StringType, ConfigItem> &&val_pair) {
    Check(ItemType::Object, "Insert");
    auto it = std::get<MapType>(m_value).insert(std::move(val_pair));
    it.first->second.m_key_ = it.first->first;
    return it;
  }

 private:
  void MakeSelfUndefined() noexcept { m_value = std::monostate{}; }
  void MakeSelfNull() noexcept { m_value = nullptr; }
  void MakeSelfScalar() noexcept { m_value = StringType{}; }
  void MakeSelfArray() noexcept { m_value = ArrayType{}; }
  void MakeSelfObject() noexcept { m_value = MapType{}; }
  void MakeSelfBoolean() noexcept { m_value = BooleanType{}; }
  void MakeSelfFloat() noexcept { m_value = FloatType{}; }
  void MakeSelfInteger() noexcept { m_value = IntegerType{}; }
  void MakeSelf(ItemType type) noexcept {
    switch (static_cast<uint8_t>(type)) {
      case static_cast<uint8_t>(ItemType::Boolean):
        MakeSelfBoolean();
        break;
      case static_cast<uint8_t>(ItemType::Float):
        MakeSelfFloat();
        break;
      case static_cast<uint8_t>(ItemType::Integer):
        MakeSelfInteger();
        break;
      case static_cast<uint8_t>(ItemType::Array):
        MakeSelfArray();
        break;
      case static_cast<uint8_t>(ItemType::Object):
        MakeSelfObject();
        break;
      case static_cast<uint8_t>(ItemType::Scalar):
        MakeSelfScalar();
        break;
      case static_cast<uint8_t>(ItemType::Null):
        MakeSelfNull();
        break;
      default:
        MakeSelfUndefined();
    }
  }

  // fail
  void Check(ItemType type, StringType operation) {
    if (bool same_with_type = type == Type(); !same_with_type && IsUndefined()) {
      MakeSelf(type);
    } else if (!same_with_type) {
      throw OperationError(ItemTypeToString(Type()), std::move(operation));
    }
  }

  // fail
  void Check(ItemType type, StringType operation) const {
    if (type != Type()) {
      throw OperationError(ItemTypeToString(Type()), std::move(operation));
    }
  }
};  // namespace clsn
}  // namespace clsn

// for Iterator
namespace clsn::detail {

struct PrimitiveIterator {
  using difference_type = std::ptrdiff_t;

  inline explicit PrimitiveIterator(difference_type it = 0) : m_it_(it) {}

  inline void SetBegin() { m_it_ = 0; }
  inline void SetEnd() { m_it_ = 1; }

  inline auto operator++() -> PrimitiveIterator & {
    ++m_it_;
    return *this;
  }

  inline auto operator++(int) -> PrimitiveIterator {
    PrimitiveIterator old(m_it_);
    ++(*this);
    return old;
  }

  inline auto operator--() -> PrimitiveIterator & {
    --m_it_;
    return (*this);
  }
  inline auto operator--(int) -> PrimitiveIterator {
    PrimitiveIterator old = (*this);
    --(*this);
    return old;
  }

  inline auto operator==(PrimitiveIterator const &other) const -> bool { return m_it_ == other.m_it_; }
  inline auto operator!=(PrimitiveIterator const &other) const -> bool { return !(*this == other); }

  inline auto operator+(difference_type off) const -> PrimitiveIterator { return PrimitiveIterator(m_it_ + off); }
  inline auto operator-(difference_type off) const -> PrimitiveIterator { return PrimitiveIterator(m_it_ - off); }

  inline auto operator+=(difference_type off) -> PrimitiveIterator & {
    m_it_ += off;
    return (*this);
  }
  inline auto operator-=(difference_type off) -> PrimitiveIterator & {
    m_it_ -= off;
    return (*this);
  }

  inline auto operator-(PrimitiveIterator const &other) const -> difference_type { return m_it_ - other.m_it_; }

  inline auto operator<(PrimitiveIterator const &other) const -> bool { return m_it_ < other.m_it_; }
  inline auto operator<=(PrimitiveIterator const &other) const -> bool { return m_it_ <= other.m_it_; }
  inline auto operator>(PrimitiveIterator const &other) const -> bool { return m_it_ > other.m_it_; }
  inline auto operator>=(PrimitiveIterator const &other) const -> bool { return m_it_ >= other.m_it_; }

 private:
  difference_type m_it_;
};

template <typename _ValTy>
struct Iterator {
  friend _ValTy;
  friend Iterator<std::conditional_t<std::is_const_v<_ValTy>, std::remove_const_t<_ValTy>, const _ValTy>>;

  using value_type = _ValTy;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type &;
  using array_iterator =
      typename std::conditional_t<std::is_const_v<_ValTy>, typename _ValTy::ArrayType::const_iterator,
                                  typename _ValTy::ArrayType::iterator>;

  using map_iterator = typename std::conditional_t<std::is_const_v<_ValTy>, typename _ValTy::MapType::const_iterator,
                                                   typename _ValTy::MapType::iterator>;

  inline explicit Iterator(value_type *v) : m_value_(v) {}

  inline Iterator(const Iterator<const _ValTy> &rhs)
      : m_value_(rhs.m_value_),
        m_array_it_(rhs.m_array_it_),
        m_object_it_(rhs.m_object_it_),
        m_primitive_it_(rhs.m_primitive_it_) {}

  auto operator=(const Iterator<const _ValTy> &rhs) -> Iterator & {
    this->m_value_ = rhs.m_value_;
    this->m_array_it_ = rhs.m_array_it_;
    this->m_object_it_ = rhs.m_object_it_;
    this->m_primitive_it_ = rhs.m_primitive_it_;
    return *this;
  }

  inline Iterator(const Iterator<std::remove_const_t<_ValTy>> &rhs)
      : m_value_(rhs.m_value_),
        m_array_it_(rhs.m_array_it_),
        m_object_it_(rhs.m_object_it_),
        m_primitive_it_(rhs.m_primitive_it_) {}

  auto operator=(const Iterator<std::remove_const_t<_ValTy>> &rhs) -> Iterator & {
    this->m_value_ = rhs.m_value_;
    this->m_array_it_ = rhs.m_array_it_;
    this->m_object_it_ = rhs.m_object_it_;
    this->m_primitive_it_ = rhs.m_primitive_it_;
    return *this;
  }

  auto Key() const -> const typename value_type::StringType & {
    CheckData();
    CheckIterator();

    if (!m_value_->IsObject()) {
      throw("cannot use Key() with non-object type");
    }
    return m_object_it_->first;
  }

  auto Value() const -> reference {
    CheckData();
    CheckIterator();

    switch (m_value_->Type()) {
      case value_type::ItemType::Object:
        return m_object_it_->second;
      case value_type::ItemType::Array:
        return *m_array_it_;
      default:
        break;
    }
    return *m_value_;
  }

  inline auto operator*() const -> reference { return this->Value(); }

  inline auto operator->() const -> pointer { return std::addressof(this->operator*()); }

  inline auto operator++(int) -> Iterator {
    Iterator old = (*this);
    ++(*this);
    return old;
  }

  auto operator++() -> Iterator {
    CheckData();

    switch (m_value_->Type()) {
      case value_type::ItemType::Object: {
        std::advance(m_object_it_, 1);
        break;
      }
      case value_type::ItemType::Array: {
        std::advance(m_array_it_, 1);
        break;
      }
      case value_type::ItemType::Null: {
        // DO NOTHING
        break;
      }
      default: {
        ++m_primitive_it_;
        break;
      }
    }
    return *this;
  }

  inline auto operator--(int) -> Iterator {
    Iterator old = (*this);
    --(*this);
    return old;
  }

  auto operator--() -> Iterator & {
    CheckData();

    switch (m_value_->Type()) {
      case value_type::ItemType::Object: {
        std::advance(m_object_it_, -1);
        break;
      }
      case value_type::ItemType::Array: {
        std::advance(m_array_it_, -1);
        break;
      }
      case value_type::ItemType::Null: {
        // DO NOTHING
        break;
      }
      default: {
        --m_primitive_it_;
        break;
      }
    }
    return *this;
  }

  inline auto operator-(difference_type off) const -> Iterator { return operator+(-off); }
  inline auto operator+(difference_type off) const -> Iterator {
    Iterator ret(*this);
    ret += off;
    return ret;
  }

  inline auto operator-=(difference_type off) -> Iterator & { return operator+=(-off); }
  auto operator+=(difference_type off) -> Iterator & {
    CheckData();

    switch (m_value_->Type()) {
      case value_type::ItemType::Object: {
        throw("cannot compute offsets with object type");
        break;
      }
      case value_type::ItemType::Array: {
        std::advance(m_array_it_, off);
        break;
      }
      case value_type::ItemType::Null: {
        // DO NOTHING
        break;
      }
      default: {
        m_primitive_it_ += off;
        break;
      }
    }
    return *this;
  }

  auto operator-(const Iterator &rhs) const -> difference_type {
    CheckData();
    rhs.CheckData();

    if (m_value_ != rhs.m_value_) {
      throw("cannot compute Iterator offsets of different value objects");
    }

    if (m_value_->Type() != value_type::ItemType::Array) {
      throw("cannot compute Iterator offsets with non-array type");
    }
    return m_array_it_ - rhs.m_array_it_;
  }

  inline auto operator!=(Iterator const &other) const -> bool { return !(*this == other); }
  auto operator==(Iterator const &other) const -> bool {
    if (m_value_ == nullptr) {
      return false;
    }

    if (m_value_ != other.m_value_) {
      return false;
    }

    switch (m_value_->Type()) {
      case value_type::ItemType::Object: {
        return m_object_it_ == other.m_object_it_;
      }
      case value_type::ItemType::Array: {
        return m_array_it_ == other.m_array_it_;
      }
      default: {
        return m_primitive_it_ == other.m_primitive_it_;
      }
    }
  }

  inline auto operator>(Iterator const &other) const -> bool { return other.operator<(*this); }
  inline auto operator>=(Iterator const &other) const -> bool { return !operator<(other); }
  inline auto operator<=(Iterator const &other) const -> bool { return !other.operator<(*this); }
  auto operator<(Iterator const &other) const -> bool {
    CheckData();
    other.CheckData();

    if (m_value_ != other.m_value_) {
      throw("cannot compare iterators of different value objects");
    }
    switch (m_value_->Type()) {
      case value_type::ItemType::Object:
        throw("cannot compare iterators with object type");
      case value_type::ItemType::Array:
        return m_array_it_ < other.m_array_it_;
      default:
        return m_primitive_it_ < other.m_primitive_it_;
    }
  }

 private:
  void SetBegin() {
    CheckData();

    switch (m_value_->Type()) {
      case value_type::ItemType::Object: {
        if constexpr (std::is_const<_ValTy>::value) {
          m_object_it_ = m_value_->GetMap().cbegin();
        } else {
          m_object_it_ = m_value_->GetMap().begin();
        }
        break;
      }
      case value_type::ItemType::Array: {
        if constexpr (std::is_const<_ValTy>::value) {
          m_array_it_ = m_value_->GetArray().cbegin();
        } else {
          m_array_it_ = m_value_->GetArray().begin();
        }
        break;
      }
      case value_type::ItemType::Null: {
        // DO NOTHING
        break;
      }
      default: {
        m_primitive_it_.SetBegin();
        break;
      }
    }
  }

  void SetEnd() {
    CheckData();

    switch (m_value_->Type()) {
      case value_type::ItemType::Object: {
        m_object_it_ = m_value_->GetMap().end();
        break;
      }
      case value_type::ItemType::Array: {
        m_array_it_ = m_value_->GetArray().end();
        break;
      }
      case value_type::ItemType::Null: {
        // DO NOTHING
        break;
      }
      default: {
        m_primitive_it_.SetEnd();
        break;
      }
    }
  }

  inline void CheckData() const {
    if (m_value_ == nullptr) {
      throw("Iterator is empty");
    }
  }

  void CheckIterator() const {
    switch (m_value_->Type()) {
      case value_type::ItemType::Object:
        if (m_object_it_ == m_value_->GetMap().end()) {
          throw std::out_of_range("object Iterator out of range");
        }
        break;
      case value_type::ItemType::Array:
        if (m_array_it_ == m_value_->GetArray().end()) {
          throw std::out_of_range("array Iterator out of range");
        }
        break;
      case value_type::ItemType::Null: {
        throw std::out_of_range("null Iterator out of range");
      }
      default:
        if (m_primitive_it_ != PrimitiveIterator{0}) {
          throw std::out_of_range("primitive Iterator out of range");
        }
        break;
    }
  }

 private:
  value_type *m_value_;

  array_iterator m_array_it_;
  map_iterator m_object_it_;
  PrimitiveIterator m_primitive_it_{0};  // for other types
};
// namespace detail

}  // namespace clsn::detail

// for binder and construct
namespace clsn {
template <class _ItemTy>
struct Binder {
  /**
   * @Author: qingfuliu
   * @description: 对于本来就支持的类型，不需要转换
   * @return {*}
   */
  template <class _ValueTy>
  static auto Bind(_ItemTy &item, _ValueTy &&val) -> std::enable_if_t<_ItemTy::template SupportedTy<_ValueTy>::value> {
    item.m_value = std::forward<_ValueTy>(val);
  }

  /**
   * @Author: qingfuliu
   * @description: 对没有包含但是可以进行类型转换的，进行转换之后再赋值
   * @return {*}
   */
  template <class _ValueTy>
  static auto Bind(_ItemTy &item, _ValueTy &&val)
      -> std::enable_if_t<!_ItemTy::template SupportedTy<_ValueTy>::value &&
                          _ItemTy::template SupportedTyWeak<_ValueTy>::value> {
    using type = typename _ItemTy::template SupportedTyWeak<_ValueTy>::type;
    item.m_value = static_cast<type>(val);
  }

  /**
   * @Author: qingfuliu
   * @description: 对于本来就支持的类型，不需要转换,但是需要对数组进行特化
   * @return {*}
   */
  template <class _ValueTy>
  static auto Bind(_ItemTy &item, const std::initializer_list<_ValueTy> &value_list)
      -> std::enable_if_t<_ItemTy ::template SupportedTy<_ValueTy>::value> {
    item.m_value = typename _ItemTy::ArrayType{value_list.size()};
    for (const _ValueTy &val : value_list) {
      item.GetArray().emplace_back(val);
    }
  }

  /**
   * @Author: qingfuliu
   * @description: 对于本来就支持的类型，不需要转换,但是需要对数组进行特化
   * @return {*}
   */
  template <class _ValueTy>
  static auto Bind(_ItemTy &item, std::initializer_list<_ValueTy> &&value_list)
      -> std::enable_if_t<_ItemTy ::template SupportedTy<_ValueTy>::value> {
    item.m_value = typename _ItemTy::ArrayType(value_list.size());
    for (auto &val : value_list) {
      item.GetArray().emplace_back(std::move(val));
    }
  }

  /**
   * @Author: qingfuliu
   * @description: 对于用户自定义类型
   * @return {*}
   */
  template <class _ValueTy>
  static typename std::enable_if_t<!_ItemTy::template SupportedTy<_ValueTy>::value &&
                                       !_ItemTy::template SupportedTyWeak<_ValueTy>::value &&
                                       helper::HasMemberSerializeFuncV<_ItemTy, _ValueTy>,
                                   void>
  Bind(_ItemTy &item, const _ValueTy &val) {
    helper::SERIALIZE_FUNCTION_NAME(item, val);
  }
};

template <class _ItemTy>
struct Constructor {
  /**
   * @Author: qingfuliu
   * @description: 对于本来就支持的类型，不需要转换
   * @return {*}
   */
  template <class _ValueTy>
  static auto Construct(const _ItemTy &item)
      -> std::enable_if_t<_ItemTy::template SupportedTy<std::decay_t<_ValueTy>>::value, _ValueTy> {
    return static_cast<_ValueTy>(std::get<std::decay_t<_ValueTy>>(item.m_value));
  }

  /**
   * @Author: qingfuliu
   * @description: 不支持的但是能够static_cast转换的类型
   * @return {*}
   */
  template <class _ValueTy, class v = typename _ItemTy::template SupportedTyWeak<std::decay_t<_ValueTy>>::type>
  static auto Construct(const _ItemTy &item)
      -> std::enable_if_t<!_ItemTy::template SupportedTy<std::decay_t<_ValueTy>>::value &&
                              _ItemTy::template SupportedTyWeak<std::decay_t<_ValueTy>>::value,
                          _ValueTy> {
    using type = typename _ItemTy::template SupportedTyWeak<_ValueTy>::type;
    return static_cast<std::decay_t<_ValueTy>>(std::get<type>(item.m_value));
  }
  /**
   * @Author: qingfuliu
   * @description: 对于用户自定义类型
   * @return {*}
   */
  template <class _ValueTy>
  static typename std::enable_if_t<!_ItemTy::template SupportedTy<_ValueTy>::value &&
                                       !_ItemTy::template SupportedTyWeak<_ValueTy>::value &&
                                       helper::HasMemberSerializeFuncV<_ItemTy, _ValueTy>,
                                   _ValueTy>
  Construct(const _ItemTy &item) {
    _ValueTy val{};
    helper::DESERIALIZE_FUNCTION_NAME(item, val);
    return val;
  }
};

}  // namespace clsn

// config TypeWrapper
namespace clsn::detail {
/**
 * @Author: qingfuliu
 * @description: Used to enhance scalability for data types
 */
struct TypeWrapper {
  using BooleanType = bool;

  using IntegerType = int64_t;

  using FloatType = double;

  using CharType = char;

  template <class _CharTy, class... _Args>
  using StringType = std::basic_string<_CharTy, _Args...>;

  template <class _Kty, class... _Args>
  using ArrayType = std::vector<_Kty, _Args...>;

  template <class _Kty, class _Ty, class... _Args>
  using MapType = std::map<_Kty, _Ty, _Args...>;

  template <class _Ty>
  using AllocatorType = std::allocator<_Ty>;

  template <class _Ty>
  using BinderType = Binder<_Ty>;

  template <class _Ty>
  using ConstructType = Constructor<_Ty>;

  template <class _Ty>
  using Iterator = Iterator<_Ty>;

  template <class _Ty>
  using CIterator = Iterator<const _Ty>;
};

}  // namespace clsn::detail

// for parser
namespace clsn {

enum class ParserState : uint8_t {
  kUndefined,

  kBooleanTrue,
  kBooleanFalse,
  kNull,

  kString,
  kInteger,
  kFloat,

  kBeginOfArray,
  kEndOfArray,

  kBeginObject,
  kEndObject,

  kNameSeparator,
  kValueSeparator,

  kEndOfInput,
  kErrorInput
};

static inline auto ToString(ParserState parser_state) noexcept -> std::string {
#define XX(CTX, VAL, INDEX)                    \
  case static_cast<uint8_t>(ParserState::VAL): \
    return #VAL;

  switch (static_cast<uint8_t>(parser_state)) {
    HELPER_NARG_RANGE_WITH_FUNC(XX, , kUndefined, kBooleanTrue, kBooleanFalse, kNull, kString, kInteger, kFloat,
                                kBeginOfArray, kEndOfArray, kBeginObject, kEndObject, kNameSeparator, kValueSeparator,
                                kEndOfInput)
    default:
      break;
  }
#undef XX
  return "UnKnown";
}

static inline auto ToString(const std::vector<ParserState> &parser_states) noexcept -> std::string {
  std::string str;
  str.reserve((parser_states.size() << 4));
  str.push_back('<');
  for (size_t i = 0; i < parser_states.size(); ++i) {
    str.append(ToString(parser_states[i]));
    if (i != parser_states.size() - 1) {
      str.push_back(',');
    }
  }
  str.push_back('>');
  return str;
}

class ParserError : public std::logic_error {
 public:
  explicit ParserError(const std::string &str) noexcept : std::logic_error(str) {}

  explicit ParserError(ParserState actual_state) noexcept
      : std::logic_error("Unexpected state encountered during parsing: <" + ToString(actual_state) + ">") {}

  explicit ParserError(ParserState actual_state, const std::string &msg) noexcept
      : std::logic_error("Encountered an error during the parsing process: " + msg + ", the parsing status is " +
                         ToString(actual_state)) {}

  explicit ParserError(ParserState actual_state, ParserState expected_state) noexcept
      : std::logic_error("Unexpected state encountered during parsing: <" + ToString(actual_state) +
                         ">, expected state is <" + ToString(expected_state) + ">") {}

  explicit ParserError(ParserState actual_state, const std::vector<ParserState> &expected_states) noexcept
      : std::logic_error("Unexpected state encountered during parsing: " + ToString(actual_state) +
                         " expected state is " + ToString(expected_states)) {}
};

template <typename ValueType>
class ParserContext {
  using TargetCharType = typename ValueType::CharType;
  using DeCoderType = bool (*)(std::basic_istream<TargetCharType> &, uint32_t &);

 protected:
  std::basic_istream<TargetCharType> m_istream_;
  std::stack<std::pair<typename ValueType::PointerType, ParserState>> m_states_;
  bool m_appeared_wrapper_{false};
  DeCoderType m_decoder_{nullptr};

 public:
  explicit ParserContext(std::basic_istream<TargetCharType> &is) noexcept
      : m_istream_(is.rdbuf())  //, err_handler_(nullptr)
  {
    // TODO(lqf):
    m_istream_.unsetf(std::ios_base::skipws);
    m_istream_.imbue(std::locale(std::locale::classic(), is.getloc(), std::locale::collate | std::locale::ctype));
  }

  virtual ~ParserContext() = default;

  template <class _Coder>
  void SetDecoder() noexcept {
    m_decoder_ = _Coder::Decode;
  }

  virtual void Parser(ValueType &root_val) {
    m_states_.push(std::make_pair(&root_val, ParserState::kUndefined));

    typename ValueType::PointerType cur_val;
    ParserState last_parser_state = ParserState::kUndefined;
    while (!m_states_.empty()) {
      cur_val = m_states_.top().first;
      last_parser_state = m_states_.top().second;

      if (ParserState::kUndefined == last_parser_state) {
        last_parser_state = Scan();
        m_states_.top().second = last_parser_state;
      }

      switch (static_cast<uint8_t>(last_parser_state)) {
        case static_cast<uint8_t>(ParserState::kBooleanTrue):
          *cur_val = true;
          m_states_.pop();
          break;
        case static_cast<uint8_t>(ParserState::kBooleanFalse):
          *cur_val = false;
          m_states_.pop();
          break;
        case static_cast<uint8_t>(ParserState::kNull):
          *cur_val = nullptr;
          m_states_.pop();
          break;
        case static_cast<uint8_t>(ParserState::kString): {
          typename ValueType::StringType str;
          GetString(str);
          *cur_val = std::move(str);
          m_states_.pop();
        } break;
        case static_cast<uint8_t>(ParserState::kInteger): {
          typename ValueType::IntegerType integer;
          GetInteger(integer);
          *cur_val = integer;
          m_states_.pop();
        } break;
        case static_cast<uint8_t>(ParserState::kFloat): {
          typename ValueType::FloatType decimal;
          GetFloat(decimal);
          *cur_val = decimal;
          m_states_.pop();
        } break;
        case static_cast<uint8_t>(ParserState::kBeginOfArray): {
          last_parser_state = Scan();
          switch (static_cast<uint8_t>(last_parser_state)) {
            case static_cast<uint8_t>(ParserState::kBooleanTrue):
            case static_cast<uint8_t>(ParserState::kBooleanFalse):
            case static_cast<uint8_t>(ParserState::kNull):
            case static_cast<uint8_t>(ParserState::kString):
            case static_cast<uint8_t>(ParserState::kInteger):
            case static_cast<uint8_t>(ParserState::kFloat):
            case static_cast<uint8_t>(ParserState::kBeginOfArray):
            case static_cast<uint8_t>(ParserState::kBeginObject):
              if (cur_val->IsArray() && !cur_val->Empty()) {
                CheckAndResetAW(last_parser_state);
              }
              cur_val->PushBack(ValueType());
              m_states_.push(std::make_pair(&(cur_val->Back()), last_parser_state));
              break;
            case static_cast<uint8_t>(ParserState::kValueSeparator):
              // Multiple consecutive commas are not allowed
              CheckAW(last_parser_state, false);
              // the first character cannot be ,
              if (cur_val->Empty()) {
                Fail(last_parser_state, "at least one item is required");
              }
              SetAW(true);
              break;
            case static_cast<uint8_t>(ParserState::kEndOfArray):
              // There should not be a comma before the terminator
              CheckAW(last_parser_state, false);
              cur_val->GetArray().shrink_to_fit();
              m_states_.pop();
              break;
            default:
              Fail(last_parser_state);
              break;
          }
        }  // case static_cast<uint8_t>(ParserState::kBeginOfArray)
        break;
        case static_cast<uint8_t>(ParserState::kBeginObject):
          last_parser_state = Scan();
          switch (static_cast<uint8_t>(last_parser_state)) {
            case static_cast<uint8_t>(ParserState::kString): {
              if (cur_val->IsObject() && !cur_val->Empty()) {
                CheckAndResetAW(ParserState::kBeginObject);
              }
              std::pair<typename ValueType::StringType, ValueType> pair{};
              GetString(pair.first);
              pair.second.SetKey(pair.first);
              last_parser_state = Scan();
              if (last_parser_state != ParserState::kNameSeparator) {
                Fail(last_parser_state, ParserState::kNameSeparator);
              }
              auto it = cur_val->Insert(std::move(pair));
              if (!it.second) {
                Fail(last_parser_state, ParserState::kEndObject);
              }
              m_states_.push(std::make_pair(&(it.first->second), ParserState::kUndefined));
              break;
            }
            case static_cast<uint8_t>(ParserState::kValueSeparator):
              // Multiple consecutive commas are not allowed
              CheckAW(last_parser_state, false);
              // the first character cannot be ,
              if (cur_val->Empty()) {
                Fail(last_parser_state, "at least one item is required");
              }
              SetAW();
              break;
            case static_cast<uint8_t>(ParserState::kEndObject):
              // There should not be a comma before the terminator
              CheckAndResetAW(last_parser_state, false);
              m_states_.pop();
              break;
            default:
              Fail(last_parser_state, ParserState::kEndObject);
              break;
          }  // case static_cast<uint8_t>(ParserState::kBeginObject):
          break;
        case static_cast<uint8_t>(ParserState::kEndOfInput):
          if (m_states_.empty()) {
            break;
          }
        default:
          Fail(last_parser_state);
          break;
      }  // switch (static_cast<uint8_t>(last_parser_state))
    }    // while (!m_states_.empty())
    if (Scan(); !this->m_istream_.eof()) {
      Fail(last_parser_state, ParserState::kEndObject);
    }
  }

 protected:
  virtual auto Scan() -> ParserState = 0;

  virtual void GetInteger(typename ValueType::IntegerType &out) = 0;
  virtual void GetFloat(typename ValueType::FloatType &out) = 0;
  virtual void GetString(typename ValueType::StringType &out) = 0;

  void Fail(const std::string &msg = "unexpected token") { throw ParserError(msg); }

  void Fail(ParserState actual_parser_state, const std::string &msg = "unexpected token") {
    throw ParserError(actual_parser_state, msg);
  }

  void Fail(ParserState actual_state, const std::vector<ParserState> &expected_states,
            const std::string &msg = "unexpected token") {
    throw ParserError(actual_state, expected_states);
  }

  void Fail(ParserState actual_state, ParserState expected_state, const std::string &msg = "unexpected token") {
    throw ParserError(actual_state, expected_state);
  }

  void CheckAW(ParserState actual_state, bool excepted = true) {
    if (excepted != this->m_appeared_wrapper_) {
      Fail(actual_state, ParserState::kValueSeparator);
    }
  }

  void CheckAndResetAW(ParserState actual_state, bool excepted = true) {
    if (excepted != this->m_appeared_wrapper_) {
      Fail(actual_state, ParserState::kValueSeparator);
    }
    this->m_appeared_wrapper_ = false;
  }

  void SetAW(bool v = true) noexcept { this->m_appeared_wrapper_ = v; }
};

template <class _TypeWrapper, template <typename, typename, typename> class _PaserCtxTy,
          template <typename> class _Coder>
class Parserable {
 public:
  using _ValueTy = ConfigItem<_TypeWrapper>;

  using _SourceCharTy = typename _TypeWrapper::CharType;

  using _CoderTy = _Coder<_SourceCharTy>;

  using _ParserTy = _PaserCtxTy<_ValueTy, _SourceCharTy, _CoderTy>;

  Parserable() = default;

  virtual ~Parserable() = default;

  static auto Parser(std::basic_istream<_SourceCharTy> &is) -> _ValueTy {
    _ValueTy c;
    _ParserTy p{is};
    // p.template SetEncoder<_CoderTy>();
    p.template SetDecoder<_CoderTy>();
    p.Parser(c);
    return c;
  }

  static auto Parser(std::basic_istream<_SourceCharTy> &is, _ValueTy &c) -> _ValueTy {
    _ParserTy p{is};
    // p.template SetEncoder<_CoderTy>();
    p.template SetDecoder<_CoderTy>();
    p.Parser(c);
    return c;
  }

  virtual void ParserFromStream(std::basic_istream<_SourceCharTy> &is) = 0;
};
}  // namespace clsn

// for Serializer
namespace clsn {

template <typename _CharTy>
class Indent {
 public:
  using char_type = _CharTy;
  using string_type = std::basic_string<char_type>;

  Indent(uint8_t step, char_type ch) : m_step_(step == 0 ? 4 : step), m_indent_char_(ch), m_indent_string_() {
    Reverse(static_cast<size_t>(m_step_ * 2));
  }

  inline void operator++() {
    ++m_depth_;
    Reverse(static_cast<size_t>(m_depth_ * m_step_));
  }

  inline void operator--() { --m_depth_; }

  inline void Put(std::basic_ostream<char_type> &os) const {
    os.write(m_indent_string_.c_str(), static_cast<std::streamsize>(m_depth_ * m_step_));
  }

  inline void Put(std::basic_ostream<char_type> &os, int length) {
    Reverse(static_cast<size_t>(length));
    os.write(m_indent_string_.c_str(), static_cast<std::streamsize>(length));
  }

  friend inline auto operator<<(std::basic_ostream<char_type> &os, const Indent &i) -> std::basic_ostream<char_type> & {
    if (i.m_indent_char_ && i.m_step_ > 0 && i.m_depth_ > 0) {
      i.Put(os);
    }
    return os;
  }

 private:
  void Reverse(size_t length) {
    if (m_indent_char_) {
      if (m_indent_string_.size() < length) {
        m_indent_string_.resize(length + static_cast<size_t>(m_step_ * 2), m_indent_char_);
      }
    }
  }

 private:
  uint8_t m_step_{0};
  uint16_t m_depth_{0};
  char_type m_indent_char_{' '};
  string_type m_indent_string_{};
};

template <typename _ValueType>
class SerializeContext {
  using TargetCharType = typename _ValueType::CharType;

  using ItemType = typename _ValueType::ItemType;

  using Iterator = typename _ValueType::CIteratorType;

  using StringType = typename _ValueType::StringType;

  using BooleanType = typename _ValueType::BooleanType;

  using PointerType = typename _ValueType::CPointerType;

  struct SerializeState {
    SerializeState(BooleanType needle, PointerType pointer, Iterator Iterator)
        : m_needle_indent_(needle), m_cur_pointer_(pointer), m_cur_iterator_(Iterator) {}
    BooleanType m_needle_indent_;
    PointerType m_cur_pointer_;
    Iterator m_cur_iterator_;
  };

 protected:
  std::basic_ostream<TargetCharType> m_ostream_;
  std::stack<SerializeState> m_states_;

 public:
  explicit SerializeContext(std::basic_ostream<TargetCharType> &os) noexcept
      : m_ostream_(os.rdbuf())  //, err_handler_(nullptr)
  {
    // TODO(lqf):
    m_ostream_.setf(std::ios_base::skipws);
    m_ostream_.imbue(std::locale(std::locale::classic(), os.getloc(), std::locale::collate | std::locale::ctype));
  }

  virtual ~SerializeContext() = default;

  void Serialize(const _ValueType &root_val) {
    m_states_.emplace(false, &root_val, root_val.CBegin());
    while (!m_states_.empty()) {
      BooleanType needle_indent = m_states_.top().m_needle_indent_;
      typename _ValueType::CPointerType cur_val = m_states_.top().m_cur_pointer_;
      Iterator &last_iterator = m_states_.top().m_cur_iterator_;
      switch (static_cast<uint8_t>(cur_val->Type())) {
        case static_cast<uint8_t>(ItemType::Undefined):
          m_states_.pop();
          break;
        case static_cast<uint8_t>(ItemType::Null):
          if (needle_indent) {
            IndentOutput();
          }
          NullOutput(*cur_val);
          m_states_.pop();
          break;
        case static_cast<uint8_t>(ItemType::Boolean):
          if (needle_indent) {
            IndentOutput();
          }
          BooleanOutput(*cur_val);
          m_states_.pop();
          break;
        case static_cast<uint8_t>(ItemType::Scalar):
          if (needle_indent) {
            IndentOutput();
          }
          ScalarOutput(*cur_val);
          m_states_.pop();
          break;
        case static_cast<uint8_t>(ItemType::Float):
          if (needle_indent) {
            IndentOutput();
          }
          FloatOutput(*cur_val);
          m_states_.pop();
          break;
        case static_cast<uint8_t>(ItemType::Integer):
          if (needle_indent) {
            IndentOutput();
          }
          IntegerOutput(*cur_val);
          m_states_.pop();
          break;
        case static_cast<uint8_t>(ItemType::Array):
          if (cur_val->CBegin() == last_iterator) {
            Next(ParserState::kBeginOfArray);
          } else if (cur_val->CEnd() <= last_iterator) {
            Next(ParserState::kEndOfArray);
            m_states_.pop();
            break;
          } else {
            Next(ParserState::kValueSeparator);
            NewLineOutput();
          }
          m_states_.emplace(true, &*last_iterator, last_iterator->CBegin());
          ++last_iterator;
          break;
        case static_cast<uint8_t>(ItemType::Object):
          if (cur_val->CBegin() == last_iterator) {
            if (needle_indent) {
              IndentOutput();
            }
            Next(ParserState::kBeginObject);
          } else if (cur_val->CEnd() == last_iterator) {
            Next(ParserState::kEndObject);
            m_states_.pop();
            break;
          } else {
            Next(ParserState::kValueSeparator);
            NewLineOutput();
          }
          NameOutput(last_iterator.Key());
          Next(ParserState::kNameSeparator);
          m_states_.emplace(false, &*last_iterator, last_iterator->CBegin());
          ++last_iterator;
          break;
      }
    }
    if (!m_states_.empty()) {
      throw "m_states_ should be empty()";
    }
  }
  virtual void Next(ParserState state) = 0;
  virtual void NewLineOutput() = 0;
  virtual void IntegerOutput(const _ValueType &val) = 0;
  virtual void FloatOutput(const _ValueType &val) = 0;
  virtual void BooleanOutput(const _ValueType &val) = 0;
  virtual void ScalarOutput(const _ValueType &val) = 0;
  virtual void NullOutput(const _ValueType &val) = 0;
  virtual void NameOutput(const StringType &val) = 0;
  virtual void IndentOutput() = 0;

};  // class SerializeContext

template <class _TypeWrapper, template <typename> class _SerializableCtxTy>
class Serializable {
 public:
  using _ValueTy = ConfigItem<_TypeWrapper>;

  using _SourceCharTy = typename _TypeWrapper::CharType;

  using _SerializableTy = _SerializableCtxTy<_ValueTy>;

  Serializable() = default;

  virtual ~Serializable() = default;

  static void Serialize(std::basic_ostream<_SourceCharTy> &os, const _ValueTy &c) {
    _SerializableTy s{os};
    s.Serialize(c);
  }

  virtual void SerializeToStream(std::basic_ostream<_SourceCharTy> &os) = 0;
};

}  // namespace clsn

namespace json {
template <class ValueType, class SourceCharTy,
          class _DefaultEncoding = detail::configor::encoding::AutoUtf<SourceCharTy>>
class JsonParser : public clsn::ParserContext<ValueType> {
  using TargetCharType = typename ValueType::CharType;

  using Base = clsn::ParserContext<ValueType>;

  using ParserState = clsn::ParserState;

  using IntegerType = typename ValueType::IntegerType;

  using FloatType = typename ValueType::FloatType;

  using StringType = typename ValueType::StringType;

  using CharType = typename ValueType::CharType;

 public:
  explicit JsonParser(std::basic_istream<TargetCharType> &is) noexcept : Base(is) {}
  ~JsonParser() override = default;

 private:
  void GetInteger(IntegerType &out) final {
    out = m_interge_;
    m_interge_ = 0;
  }

  void GetFloat(FloatType &out) final {
    out = m_float_;
    m_float_ = 0;
  }

  void GetString(StringType &out) final {
    assert(m_current_ == '"');
    do {
      ReadNext();
      if (m_current_ == '"') {
        break;
      }
      out.push_back(static_cast<CharType>(m_current_));
    } while (true);
    out.shrink_to_fit();
  }

  auto Scan() -> ParserState override {
    ReadNext();
    SkipSpaces();
    if (this->m_istream_.eof()) {
      return ParserState::kEndOfInput;
    }
    switch (static_cast<TargetCharType>(m_current_)) {
      case '{':
        return ParserState::kBeginObject;
      case '}':
        return ParserState::kEndObject;
      case '[':
        return ParserState::kBeginOfArray;
      case ']':
        return ParserState::kEndOfArray;
      case ',':
        return ParserState::kValueSeparator;
      case ':':
        return ParserState::kNameSeparator;
      case 't':
        if (!ScanLiteral({'t', 'r', 'u', 'e'})) {
          Base::Fail(ParserState::kBooleanTrue);
        }
        return ParserState::kBooleanTrue;
      case 'f':
        if (!ScanLiteral({'f', 'a', 'l', 's', 'e'})) {
          Base::Fail(ParserState::kBooleanFalse);
        }
        return ParserState::kBooleanFalse;
      case 'n':
        if (!ScanLiteral({'n', 'u', 'l', 'l'})) {
          Base::Fail(ParserState::kNull);
        }
        return ParserState::kNull;
      case '\"':
        // lazy load
        return ParserState::kString;
      case '-':
      case '.':
      case '+':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        return ScanNumber();
      case '\0':
        return ParserState::kEndOfInput;
    }
    return ParserState::kErrorInput;
  }

 private:
  auto ReadNext() -> uint32_t {
    // TDDO(lqf): Should false be returned when encountering EOF, and do we need to count the number of rows
    if (this->m_decoder_(this->m_istream_, m_current_)) {
      if (this->m_istream_.fail()) {
        Base::Fail("decoding failed with codepoint" + std::to_string(static_cast<SourceCharTy>(m_current_)));
      }
    } else {
      m_current_ = 0;
    }
    return m_current_;
  }

  auto ReadNextIfNot() -> bool {
    // TDDO(lqf): Should false be returned when encountering EOF, and do we need to count the number of rows
    if (this->m_decoder_(this->m_istream_, m_current_)) {
      if (this->m_istream_.fail()) {
        Base::Fail("decoding failed with codepoint" + std::to_string(static_cast<SourceCharTy>(m_current_)));
      }
    } else {
      m_current_ = 0;
    }
    return true;
  }

  void SkipSpaces() {
    while (m_current_ == ' ' || m_current_ == '\t' || m_current_ == '\n' || m_current_ == '\r') {
      ReadNext();
    }

    // skip comments
    if (m_current_ == '/') {
      SkipComments();
    }
  }

  void SkipComments() {
    ReadNext();
    if (m_current_ == '/') {
      // one line comment
      while (true) {
        ReadNext();
        if (m_current_ == '\n' || m_current_ == '\r') {
          // end of comment
          SkipSpaces();
          break;
        }

        if (this->m_istream_.eof()) {
          break;
        }
      }
    } else if (m_current_ == '*') {
      // multiple line comment
      while (true) {
        if (ReadNext() == '*') {
          if (ReadNext() == '/') {
            // end of comment
            ReadNext();
            break;
          }
        }

        if (this->m_istream_.eof()) {
          Base::Fail("unexpected eof while reading comment");
        }
      }
      SkipSpaces();
    } else {
      Base::Fail("unexpected character '/'");
    }
  }

  auto ScanLiteral(std::initializer_list<char> text) -> bool {
    bool res = std::all_of(text.begin(), text.end(), [this](char ch) -> bool {
      if (ch != std::char_traits<char>::to_char_type(this->m_current_)) {
        return false;
      }
      this->ReadNext();
      return true;
    });
    RollBack();
    return res;
  }

  auto ScanNumber() -> ParserState {
    if (m_current_ == '-' || m_current_ == '+') {
      m_is_negative_ = (m_current_ == '-');
      ReadNext();
    }
    if (m_current_ == '0') {
      ReadNext();
      switch (m_current_) {
        case 'E':
        case 'e':
          return ScanExponent();
        case '.':
          return ScanFloat();
        default:
          if (IsDigit()) {
            Base::Fail(ParserState::kInteger, "json numbers should not start with 0");
          }
      }
    }
    return ScanInteger();
  }

  auto ScanInteger() -> ParserState {
    assert(IsDigit());

    while (true) {
      switch (m_current_) {
        case 'E':
        case 'e':
          return ScanExponent();
        case '.':
          return ScanFloat();
      }
      if (!IsDigit()) {
        break;
      }
      if (IsIntrgetOverFlow(m_interge_, static_cast<IntegerType>(m_current_))) {
        Base::Fail(ParserState ::kInteger, "Integer OverFlow");
      }
      m_interge_ = (m_interge_ << 3) + (m_interge_ << 2) + static_cast<IntegerType>(m_current_ - '0');
      ReadNext();
    }
    RollBack();
    return ParserState::kInteger;
  }

  auto ScanFloat() -> ParserState {
    assert(IsDecimalSymbol());
    ReadNext();

    FloatType factor = 0.1;
    m_float_ += static_cast<FloatType>(m_interge_);
    m_interge_ = 0;
    while (true) {
      if (IsDigit()) {
        if (IsFloatOverFlow(m_float_, static_cast<FloatType>(m_current_ - '0') * factor)) {
          Base::Fail(ParserState ::kFloat, "Float OverFlow");
        }
        m_float_ += static_cast<FloatType>(m_current_ - '0') * factor;
        factor *= 0.1;
        ReadNext();
        continue;
      }
      if (IsExponentSymbol()) {
        return ScanExponent();
      }
      break;
    }
    RollBack();
    return ParserState::kFloat;
  }

  auto ScanExponent() -> ParserState {
    assert(IsExponentSymbol());
    if (m_interge_ != 0) {
      m_float_ = static_cast<FloatType>(m_interge_);
      m_interge_ = 0;
    }
    IntegerType exponent{0};
    bool is_exponent_negative{false};

    if (ReadNext(); m_current_ == '+' || m_current_ == '-') {
      is_exponent_negative = (m_current_ == '-');
      ReadNext();
    }

    while (true) {
      if (!IsDigit()) {
        break;
      }
      if (IsIntrgetOverFlow(exponent, m_current_ - static_cast<IntegerType>('0'))) {
        Base::Fail(ParserState ::kFloat, "Integer OverFlow");
      }
      exponent = (exponent << 3) + (exponent << 1) + (m_current_ - static_cast<IntegerType>('0'));
      ReadNext();
    }
    if (is_exponent_negative) {
      exponent = -exponent;
    }
    m_float_ = std::pow(m_float_, exponent);
    RollBack();
    return ParserState::kFloat;
  }

  auto IsDigit() const noexcept -> bool { return m_current_ <= '9' && m_current_ >= '0'; }

  auto IsDecimalSymbol() const noexcept -> bool { return m_current_ == '.'; }

  auto IsExponentSymbol() const noexcept -> bool { return m_current_ == 'E' || m_current_ == 'e'; }

  auto IsIntrgetOverFlow(IntegerType original, IntegerType increment) -> bool {
    return (original > ValueType::INTEGER_MAX / 10 || (original == ValueType::INTEGER_MAX / 10 && increment > 7)) ||
           (original < ValueType::INTEGER_MIN / 10 || (original == ValueType::INTEGER_MIN / 10 && increment < -8));
  }

  auto IsFloatOverFlow(FloatType original, FloatType increment) const noexcept -> bool {
    return (original > ValueType::FLOAT_MAX / 10 - increment / 10) ||
           (original < ValueType::FLOAT_MIN / 10 - increment / 10);
  }

  void RollBack() {
    this->m_istream_.seekg(this->m_istream_.tellg() -
                           static_cast<typename std::basic_istream<TargetCharType>::pos_type>(1));
  }

 private:
  uint32_t m_current_{0};
  IntegerType m_interge_{0};
  FloatType m_float_{0};
  typename ValueType::BooleanType m_is_negative_{false};
};

template <class ValueType>
class JsonSerializer : public clsn::SerializeContext<ValueType> {
  using Base = clsn::SerializeContext<ValueType>;

  using TargetCharType = typename ValueType::CharType;

  using ParserState = clsn::ParserState;

  using IntegerType = typename ValueType::IntegerType;

  using FloatType = typename ValueType::FloatType;

  using StringType = typename ValueType::StringType;

  using CharType = typename ValueType::CharType;

 private:
  clsn::Indent<CharType> m_indent_;

 public:
  explicit JsonSerializer(std::basic_ostream<TargetCharType> &os) noexcept
      : Base(os), m_indent_(this->m_ostream_.width(), this->m_ostream_.fill()) {}

  ~JsonSerializer() override = default;

 private:
  void Next(ParserState state) override {
    switch (static_cast<uint8_t>(state)) {
      case static_cast<uint8_t>(ParserState::kBeginObject):
        this->m_ostream_ << "{\n";
        ++m_indent_;
        break;
      case static_cast<uint8_t>(ParserState::kEndObject):
        --m_indent_;
        this->m_ostream_ << "\n" << m_indent_ << "}";
        break;
      case static_cast<uint8_t>(ParserState::kBeginOfArray):
        this->m_ostream_ << "[\n";
        ++m_indent_;
        break;
      case static_cast<uint8_t>(ParserState::kEndOfArray):
        --m_indent_;
        this->m_ostream_ << "\n" << m_indent_ << "]";
        break;
      case static_cast<uint8_t>(ParserState::kNameSeparator):
        this->m_ostream_ << ":";
        m_indent_.Put(this->m_ostream_, 1);
        break;
      case static_cast<uint8_t>(ParserState::kValueSeparator):
        this->m_ostream_ << ",";
        break;
      default:
        break;
    }
  }
  void NewLineOutput() override { this->m_ostream_ << '\n'; }
  void IntegerOutput(const ValueType &val) override {
    IntegerType v = val;
    this->m_ostream_ << std::to_string(v);
  }
  void FloatOutput(const ValueType &val) override {
    FloatType v = val;
    this->m_ostream_ << std::to_string(v);
  }
  void BooleanOutput(const ValueType &val) override {
    if (bool v = val; v) {
      this->m_ostream_ << "true";
    } else {
      this->m_ostream_ << "false";
    }
  }
  void ScalarOutput(const ValueType &val) override {
    //    const StringType &v = static_cast<const StringType &>(val);
    this->m_ostream_ << "\"" << static_cast<const StringType &>(val) << "\"";
  }
  void NullOutput(const ValueType &val) override { this->m_ostream_ << "null"; }
  void NameOutput(const StringType &name) override { this->m_ostream_ << m_indent_ << "\"" << name << "\""; }
  void IndentOutput() override { this->m_ostream_ << m_indent_; }
};

class Json : public clsn::ConfigItem<clsn::detail::TypeWrapper>,
             public clsn::Parserable<clsn::detail::TypeWrapper, JsonParser, detail::configor::encoding::AutoUtf>,
             public clsn::Serializable<clsn::detail::TypeWrapper, JsonSerializer> {
  using _SourceCharTy = typename clsn::detail::TypeWrapper::CharType;

  using BasicParserable = clsn::Parserable<clsn::detail::TypeWrapper, JsonParser, detail::configor::encoding::AutoUtf>;

  using BasicSerializable = clsn::Serializable<clsn::detail::TypeWrapper, JsonSerializer>;

  using Base = clsn::ConfigItem<clsn::detail::TypeWrapper>;

 public:
  using value = clsn::ConfigItem<clsn::detail::TypeWrapper>;

  Json() = default;

  ~Json() override = default;

  void ParserFromStream(std::basic_istream<_SourceCharTy> &is) override { BasicParserable::Parser(is, *this); }

  void SerializeToStream(std::basic_ostream<_SourceCharTy> &os) override { BasicSerializable::Serialize(os, *this); }
};

#define DESERIALIZE_HELPER(XX, CTX, ...) \
  HELPER_CONCAT(DESERIALIZE_HELPER_, HELPER_NARG(__VA_ARGS__))(XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_0(XX, CTX, ...)

#define DESERIALIZE_HELPER_1(XX, CTX, X1)

#define DESERIALIZE_HELPER_2(XX, CTX, X1, X2, ...)                     \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_3, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_3(XX, CTX, X1, X2, ...)                     \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_4, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_4(XX, CTX, X1, X2, ...)                     \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_5, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_5(XX, CTX, X1, X2, ...)                     \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_6, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_6(XX, CTX, X1, X2, ...)                     \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_7, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_7(XX, CTX, X1, X2, ...)                     \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_8, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_8(XX, CTX, X1, X2, ...)                     \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_9, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_9(XX, CTX, X1, X2, ...)                     \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_10, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_10(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_11, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_11(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_12, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_12(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_13, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_13(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_14, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_14(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_15, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_15(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_16, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_16(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_17, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_17(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_18, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_18(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_19, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_19(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_20, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_20(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_21, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_21(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_22, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_22(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_23, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_23(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_24, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_24(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_25, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_25(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_26, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_26(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_27, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_27(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_28, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_28(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_29, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_29(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_30, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_30(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_31, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_31(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_32, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_HELPER_32(XX, CTX, X1, X2, ...)                    \
  XX(CTX, X1, X2)                                                      \
  HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \
  (DESERIALIZE_HELPER_33, XX, CTX, __VA_ARGS__)

#define DESERIALIZE_XX(CTX, X1, X2) X2 = json[X1];
#define SERIALIZE_XX(CTX, X1, X2) json[X1] = X2;

#define DESERIALIZE(...) \
  void DESERIALIZE_FUNCTION_NAME(const json::Json::value &json) { DESERIALIZE_HELPER(DESERIALIZE_XX, , __VA_ARGS__) }

#define SERIALIZE(...)                                \
  json::Json::value SERIALIZE_FUNCTION_NAME() const { \
    json::Json::value json;                           \
    DESERIALIZE_HELPER(SERIALIZE_XX, , __VA_ARGS__)   \
    return json;                                      \
  }

}  // namespace json

#endif  // AUTOTOUCHER_CONFIGENTRY_H