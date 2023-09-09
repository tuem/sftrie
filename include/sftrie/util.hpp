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

#ifndef SFTRIE_UTIL_HPP
#define SFTRIE_UTIL_HPP

#include <algorithm>

namespace sftrie{

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

}

#endif
