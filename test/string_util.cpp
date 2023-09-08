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

#include "string_util.hpp"

#include <codecvt>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
template<>
void cast_string(const std::string& src, std::wstring& dest)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	dest = converter.from_bytes(src);
}

template<>
void cast_string(const std::wstring& src, std::string& dest)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	dest = converter.to_bytes(src);
}

template<>
void cast_string(const std::string& src, std::u16string& dest)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	dest = converter.from_bytes(src);
}

template<>
void cast_string(const std::u16string& src, std::string& dest)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	dest = converter.to_bytes(src);
}

template<>
void cast_string(const std::string& src, std::u32string& dest)
{
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
	dest = converter.from_bytes(src);
}

template<>
void cast_string(const std::u32string& src, std::string& dest)
{
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
	dest = converter.to_bytes(src);
}
#pragma GCC diagnostic pop
