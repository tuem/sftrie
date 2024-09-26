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


// trie

template<typename item, typename integer>
struct trie_value
{
	using original_ref = item&;
	using original_const_ref = item const&;
	using actual = item;
	using actual_ref = item&;
	using actual_const_ref = item const&;
};

template<typename integer>
struct trie_value<empty, integer>
{
	using original_ref = const empty;
	using original_const_ref = const empty;
	using actual = const integer;
	using actual_ref = const integer;
	using actual_const_ref = const integer;
};

template<typename item, typename integer>
struct value_util
{
	static typename trie_value<item, integer>::actual_ref ref(typename trie_value<item, integer>::original_ref value, integer)
	{
		return value;
	}

	static typename trie_value<item, integer>::actual_const_ref const_ref(typename trie_value<item, integer>::original_const_ref value, integer)
	{
		return value;
	}
};

template<typename integer>
struct value_util<empty, integer>
{
	static typename trie_value<empty, integer>::actual_ref ref(typename trie_value<empty, integer>::original_ref, integer id)
	{
		return id;
	}

	static typename trie_value<empty, integer>::actual_const_ref const_ref(typename trie_value<empty, integer>::original_const_ref, integer id)
	{
		return id;
	}
};


template<typename item>
struct list_item
{
	using key_type = item;
};

template<typename key, typename value>
struct list_item<std::pair<key, value>>
{
	using key_type = key;
};

template<typename item_type>
struct key_extractor
{
	static const typename list_item<item_type>::key_type& get_key(const item_type& item)
	{
		return item;
	}
};

template<typename key, typename value>
struct key_extractor<std::pair<key, value>>
{
	static const typename list_item<std::pair<key, value>>::key_type& get_key(const std::pair<key, value>& item)
	{
		return item.first;
	}
};

}

#endif
