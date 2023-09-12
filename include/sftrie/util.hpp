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

#ifndef SFTRIE_UTIL
#define SFTRIE_UTIL

#include <string>
#include <algorithm>
#include <locale>
#include <codecvt>

namespace sftrie{

// general utilities

template<typename integer>
constexpr int bit_width()
{
	return 8 * sizeof(integer);
}

template<typename symbol>
constexpr symbol min_char()
{
	return static_cast<symbol>(symbol(0) - 1) < symbol(0) ?
		static_cast<symbol>(symbol(1) << (bit_width<symbol>() - 1)) :
		symbol(0);
}

template<typename symbol>
constexpr symbol max_char()
{
	return static_cast<symbol>(symbol(0) - 1) < symbol(0) ?
		static_cast<symbol>(static_cast<symbol>(symbol(0) - 1) ^ static_cast<symbol>(symbol(1) << (bit_width<symbol>() - 1))) :
		static_cast<symbol>(symbol(0) - 1);
}

template<typename integer, typename container>
integer container_size(const container& t)
{
	return static_cast<integer>(t.size());
}


// sorting

struct text_comparator
{
	template<typename text>
	bool operator()(const text& a, const text& b) const
	{
		for(size_t i = 0; i < std::min(a.size(), b.size()); ++i){
			if(a[i] < b[i])
				return true;
			else if(b[i] < a[i])
				return false;
		}
		return a.size() < b.size();
	}
};

template<typename iterator>
void sort_texts(iterator begin, iterator end)
{
	std::sort(begin, end, text_comparator());
}

struct text_item_pair_comparator
{
	text_comparator compare_text;

	template<typename text, typename item>
	bool operator()(const std::pair<text, item>& a, const std::pair<text, item>& b) const
	{
		return compare_text(a.first, b.first);
	}
};

template<typename iterator>
void sort_text_item_pairs(iterator begin, iterator end)
{
	std::sort(begin, end, text_item_pair_comparator());
}


// string conversion

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

template<typename symbol>
void to_stdstring(const std::basic_string<symbol>& src, std::string& dest)
{
	std::wstring_convert<std::codecvt_utf8<symbol>, symbol> converter;
	dest = converter.to_bytes(src);
}
#pragma GCC diagnostic pop

template<>
inline void from_stdstring(const std::string& src, std::string& dest)
{
	dest = src;
}

template<>
inline void to_stdstring(const std::string& src, std::string& dest)
{
	dest = src;
}

template<typename dest_type, typename src_type>
dest_type cast_string(const src_type& src);

template<typename src_type>
src_type cast_string(const src_type& src)
{
	return src;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
template<typename src_type>
std::string cast_string(const src_type& src)
{
	std::string dest;
	to_stdstring(src, dest);
	return dest;
}

template<typename dest_type>
dest_type cast_string(const std::string& src)
{
	dest_type dest;
	from_stdstring(src, dest);
	return dest;
}
#pragma GCC diagnostic pop

template<typename dest_type>
dest_type cast_string(const char* src)
{
	return cast_string<dest_type>(std::string(src));
}

}

#endif
