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

#ifndef SFTRIE_STRING_UTIL_HPP
#define SFTRIE_STRING_UTIL_HPP

#include <string>
#include <locale>
#include <codecvt>

template<typename src_type, typename dest_type>
void cast_string(const src_type& src, dest_type& dest);

template<typename src_type>
void cast_string(const src_type& src, src_type& dest)
{
	dest = src;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
template<typename symbol>
void from_stdstring(const std::string& src, std::basic_string<symbol>& dest)
{
	std::wstring_convert<std::codecvt_utf8<symbol>, symbol> converter;
	dest = converter.from_bytes(src);
}

template<>
void from_stdstring(const std::string& src, std::string& dest)
{
	dest = src;
}

template<typename symbol>
void to_stdstring(const std::basic_string<symbol>& src, std::string& dest)
{
	std::wstring_convert<std::codecvt_utf8<symbol>, symbol> converter;
	dest = converter.to_bytes(src);
}

template<>
void to_stdstring(const std::string& src, std::string& dest)
{
	dest = src;
}
#pragma GCC diagnostic pop

template<typename dest_type, typename src_type>
dest_type cast_string(const src_type& src);

template<typename src_type>
src_type cast_string(const src_type& src)
{
	return src;
}

template<typename src_type>
std::string cast_string(const src_type& src)
{
	std::string dest;
	to_stdstring<typename src_type::value_type>(src, dest);
	return dest;
}

template<typename dest_type>
dest_type cast_string(const std::string& src)
{
	dest_type dest;
	from_stdstring<typename dest_type::value_type>(src, dest);
	return dest;
}

template<typename dest_type>
dest_type cast_string(const char* src)
{
	return cast_string<dest_type>(std::string(src));
}

#endif