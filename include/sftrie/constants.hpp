/*
sftrie
https://github.com/tuem/sftrie

Copyright 2017 Takashi Uemura

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef SFTRIE_CONSTANTS
#define SFTRIE_CONSTANTS

#include <cstdint>

namespace sftrie{

struct empty; // defined in util.hpp

namespace constants{

// performance tuning

template<typename symbol> constexpr std::uint32_t default_min_binary_search() { return 42; }
template<> inline constexpr std::uint32_t default_min_binary_search<char16_t>() { return 64; }
template<> inline constexpr std::uint32_t default_min_binary_search<char32_t>() { return 64; }


// header values

inline constexpr char signature[4] = {'S', 'F', 'T', 'I'};

inline constexpr std::uint8_t current_major_version = 1;
inline constexpr std::uint8_t current_minor_version = 0;

inline constexpr std::uint8_t container_type_set = 0;
inline constexpr std::uint8_t container_type_map = 1;

inline constexpr std::uint8_t index_type_original = 0;
inline constexpr std::uint8_t index_type_compact = 1;

inline constexpr std::uint8_t text_charset_system_default = 0;
inline constexpr std::uint8_t text_charset_unicode = 1;
inline constexpr std::uint8_t text_charset_user_defined = 0xff;

inline constexpr std::uint8_t text_encoding_system_default = 0;
inline constexpr std::uint8_t text_encoding_utf8 = 1;
inline constexpr std::uint8_t text_encoding_utf16 = 2;
inline constexpr std::uint8_t text_encoding_utf32 = 3;
inline constexpr std::uint8_t text_encoding_user_defined = 0xff;

inline constexpr std::uint8_t integer_type_uint8 = 0;
inline constexpr std::uint8_t integer_type_int8 = 1;
inline constexpr std::uint8_t integer_type_uint16 = 2;
inline constexpr std::uint8_t integer_type_int16 = 3;
inline constexpr std::uint8_t integer_type_uint32 = 4;
inline constexpr std::uint8_t integer_type_int32 = 5;
inline constexpr std::uint8_t integer_type_uint64 = 6;
inline constexpr std::uint8_t integer_type_int64 = 7;
inline constexpr std::uint8_t integer_type_user_defined = 0xff;

inline constexpr std::uint8_t value_type_empty = 0;
inline constexpr std::uint8_t value_type_uint8 = 1;
inline constexpr std::uint8_t value_type_int8 = 2;
inline constexpr std::uint8_t value_type_uint16 = 3;
inline constexpr std::uint8_t value_type_int16 = 4;
inline constexpr std::uint8_t value_type_uint32 = 5;
inline constexpr std::uint8_t value_type_int32 = 6;
inline constexpr std::uint8_t value_type_uint64 = 7;
inline constexpr std::uint8_t value_type_int64 = 8;
inline constexpr std::uint8_t value_type_user_defined = 0xff;


template<typename text> constexpr std::uint8_t text_charset() { return text_charset_system_default; }

template<> inline constexpr std::uint8_t text_charset<std::string>(){ return text_charset_unicode; }
template<> inline constexpr std::uint8_t text_charset<std::u16string>(){ return text_charset_unicode; }
template<> inline constexpr std::uint8_t text_charset<std::u32string>(){ return text_charset_unicode; }


template<typename text> constexpr std::uint8_t text_encoding() { return text_encoding_system_default; }

template<> inline constexpr std::uint8_t text_encoding<std::string>(){ return text_encoding_utf8; }
template<> inline constexpr std::uint8_t text_encoding<std::u16string>(){ return text_encoding_utf16; }
template<> inline constexpr std::uint8_t text_encoding<std::u32string>(){ return text_encoding_utf32; }


template<typename integer> constexpr std::uint8_t integer_type() {return integer_type_user_defined; }

template<> inline constexpr std::uint8_t integer_type<std::int8_t>(){ return integer_type_int8; }
template<> inline constexpr std::uint8_t integer_type<std::uint8_t>(){ return integer_type_uint8; }
template<> inline constexpr std::uint8_t integer_type<std::int16_t>(){ return integer_type_int16; }
template<> inline constexpr std::uint8_t integer_type<std::uint16_t>(){ return integer_type_uint16; }
template<> inline constexpr std::uint8_t integer_type<std::int32_t>(){ return integer_type_int32; }
template<> inline constexpr std::uint8_t integer_type<std::uint32_t>(){ return integer_type_uint32; }
template<> inline constexpr std::uint8_t integer_type<std::int64_t>(){ return integer_type_int64; }
template<> inline constexpr std::uint8_t integer_type<std::uint64_t>(){ return integer_type_uint64; }


template<typename value> constexpr std::uint8_t value_type(){ return value_type_user_defined; }

template<> inline constexpr std::uint8_t value_type<empty>(){ return value_type_empty; }
template<> inline constexpr std::uint8_t value_type<std::int8_t>(){ return value_type_int8; }
template<> inline constexpr std::uint8_t value_type<std::uint8_t>(){ return value_type_uint8; }
template<> inline constexpr std::uint8_t value_type<std::int16_t>(){ return value_type_int16; }
template<> inline constexpr std::uint8_t value_type<std::uint16_t>(){ return value_type_uint16; }
template<> inline constexpr std::uint8_t value_type<std::int32_t>(){ return value_type_int32; }
template<> inline constexpr std::uint8_t value_type<std::uint32_t>(){ return value_type_uint32; }
template<> inline constexpr std::uint8_t value_type<std::int64_t>(){ return value_type_int64; }
template<> inline constexpr std::uint8_t value_type<std::uint64_t>(){ return value_type_uint64; }

}

}

#endif
