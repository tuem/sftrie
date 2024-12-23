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
#include <vector>

#include "constants.hpp"

namespace sftrie{

// empty value for sets

struct empty{};


// general utilities

template<typename integer>
constexpr int bit_width()
{
	return 8 * sizeof(integer);
}

template<typename symbol>
constexpr symbol min_symbol()
{
	return static_cast<symbol>(symbol(0) - 1) < symbol(0) ?
		static_cast<symbol>(symbol(1) << (bit_width<symbol>() - 1)) :
		symbol(0);
}

template<typename symbol>
constexpr symbol max_symbol()
{
	return static_cast<symbol>(symbol(0) - 1) < symbol(0) ?
		static_cast<symbol>(static_cast<symbol>(symbol(0) - 1) ^ static_cast<symbol>(symbol(1) << (bit_width<symbol>() - 1))) :
		static_cast<symbol>(symbol(0) - 1);
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

template<typename src_type>
void cast_text(const src_type& src, src_type& dest)
{
	dest = src;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
template<typename src_type>
void cast_text(const src_type& src, std::string& dest,
	typename std::enable_if<!std::is_same<src_type, std::string>::value>::type* = nullptr,
	typename std::enable_if<!std::is_same<src_type, std::u16string>::value>::type* = nullptr)
{
	std::wstring_convert<std::codecvt_utf8<typename src_type::value_type>, typename src_type::value_type> converter;
	dest = converter.to_bytes(src);
}

inline void cast_text(const std::u16string& src, std::string& dest)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	dest = converter.to_bytes(src);
}

template<typename dest_type>
void cast_text(const std::string& src, dest_type& dest,
	typename std::enable_if<!std::is_same<dest_type, std::string>::value>::type* = nullptr,
	typename std::enable_if<!std::is_same<dest_type, std::u16string>::value>::type* = nullptr)
{
	std::wstring_convert<std::codecvt_utf8<typename dest_type::value_type>, typename dest_type::value_type> converter;
	dest = converter.from_bytes(src);
}

inline void cast_text(const std::string& src, std::u16string& dest)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	dest = converter.from_bytes(src);
}

template<typename src_type, typename dest_type>
void cast_text(const src_type& src, dest_type& dest,
	typename std::enable_if<!std::is_same<src_type, dest_type>::value>::type* = nullptr,
	typename std::enable_if<!std::is_same<src_type, std::string>::value>::type* = nullptr,
	typename std::enable_if<!std::is_same<dest_type, std::string>::value>::type* = nullptr)
{
	std::string dest0;
	std::wstring_convert<std::codecvt_utf8<typename src_type::value_type>, typename src_type::value_type> converter0;
	dest0 = converter0.to_bytes(src);

	std::wstring_convert<std::codecvt_utf8<typename dest_type::value_type>, typename dest_type::value_type> converter;
	dest = converter.from_bytes(dest0);
}
#pragma GCC diagnostic pop

template<typename src_type>
src_type cast_text(const src_type& src)
{
	return src;
}

template<typename dest_type, typename src_type>
dest_type cast_text(const src_type& src,
	typename std::enable_if<!std::is_same<src_type, dest_type>::value>::type* = nullptr,
	typename std::enable_if<std::is_same<src_type, std::string>::value ||
		std::is_same<dest_type, std::string>::value>::type* = nullptr)
{
	dest_type dest;
	cast_text(src, dest);
	return dest;
}

template<typename dest_type, typename src_type>
dest_type cast_text(const src_type& src,
	typename std::enable_if<!std::is_same<src_type, dest_type>::value>::type* = nullptr,
	typename std::enable_if<!std::is_same<src_type, std::string>::value>::type* = nullptr,
	typename std::enable_if<!std::is_same<dest_type, std::string>::value>::type* = nullptr)
{
	return cast_text<dest_type>(cast_text<std::string>(src));
}

template<typename dest_type>
dest_type cast_text(const char* src)
{
	return cast_text<dest_type>(std::string(src));
}

template<typename dest_type, typename src_type>
std::vector<dest_type> cast_texts(const std::vector<src_type>& texts)
{
	std::vector<dest_type> results;
	for(const auto& t: texts)
		results.push_back(sftrie::cast_text<dest_type>(t));
	return results;
}

template<typename dest_type, typename src_type, typename item>
std::vector<std::pair<dest_type, item>>
cast_text_item_pairs(const std::vector<std::pair<src_type, item>>& texts)
{
	std::vector<std::pair<dest_type, item>> results;
	for(const auto& i: texts)
		results.push_back({cast_text<dest_type>(i.first), i.second});
	return results;
}


// type traits and key/value selectors

template<typename text, typename item, typename integer>
struct trie_traits
{
	using original_type = item;
	using original_ref_type = item&;
	using original_const_ref_type = item const&;

	using value_type = item;
	using value_ref_type = item&;
	using value_const_ref_type = item const&;

	using list_element_type = std::pair<text, item>;

	static constexpr std::uint8_t container_type()
	{
		return constants::container_type_map;
	}
};

template<typename text, typename integer>
struct trie_traits<text, empty, integer>
{
	using original_type = empty;
	using original_ref_type = empty;
	using original_const_ref_type = empty;

	using value_type = const integer;
	using value_ref_type = const integer;
	using value_const_ref_type = const integer;

	using list_element_type = text;

	static constexpr std::uint8_t container_type()
	{
		return constants::container_type_set;
	}
};


template<typename text, typename item, typename integer>
struct key_value_selector
{
	static inline const text& key(const typename trie_traits<text, item, integer>::list_element_type& list_element)
	{
		return list_element.first;
	}

	static inline typename trie_traits<text, item, integer>::original_const_ref_type value(const typename trie_traits<text, item, integer>::list_element_type& list_element)
	{
		return list_element.second;
	}

	static inline typename trie_traits<text, item, integer>::value_type value(item value, integer)
	{
		return value;
	}

	static inline typename trie_traits<text, item, integer>::value_ref_type value_ref(item& value, integer)
	{
		return value;
	}

	static inline typename trie_traits<text, item, integer>::value_const_ref_type value_const_ref(const item& value, integer)
	{
		return value;
	}
};

template<typename text, typename integer>
struct key_value_selector<text, empty, integer>
{
	static inline const text& key(const typename trie_traits<text, empty, integer>::list_element_type& list_element)
	{
		return list_element;
	}

	static inline constexpr typename trie_traits<text, empty, integer>::original_const_ref_type value(const typename trie_traits<text, empty, integer>::list_element_type&)
	{
		return {};
	}

	static inline typename trie_traits<text, empty, integer>::value_type value(empty, integer id)
	{
		return id;
	}

	static inline typename trie_traits<text, empty, integer>::value_ref_type value_ref(empty, integer id)
	{
		return id;
	}

	static inline typename trie_traits<text, empty, integer>::value_const_ref_type value_const_ref(empty, integer id)
	{
		return id;
	}
};

}

#endif
