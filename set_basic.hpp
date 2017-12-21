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

#ifndef SFTRIE_SET_BASIC_HPP
#define SFTRIE_SET_BASIC_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename integer>
class set_basic
{
	using symbol = typename text::value_type;

#pragma pack(1)
	struct element
	{
		bool match: 1;
		bool leaf: 1;
		integer index: bit_width<integer>() - 2;
		symbol label;
	};
#pragma pack()

public:
	template<typename random_access_iterator>
	set_basic(random_access_iterator begin, random_access_iterator end,
			integer min_binary_search = 42):
		num_texts(end - begin), data(1, {false, false, 1, {}}),
		min_binary_search(min_binary_search)
	{
		construct(begin, end, 0, 0);
		data.shrink_to_fit();
	}

	std::size_t size() const
	{
		return num_texts;
	}

	std::size_t space() const
	{
		return sizeof(element) * data.size();
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return false;
			integer l = data[current].index, r = data[l].index, m;
			for(integer w = r - l; w > min_binary_search; w = m){
				m = w >> 1;
				l += data[l + m].label < pattern[i] ? w - m : 0;
			}
			for(; l < r && data[l].label < pattern[i]; ++l);
			if(l < r && data[l].label == pattern[i])
				current = l;
			else
				return false;
		}
		return data[current].match;
	}

private:
	const std::size_t num_texts;

	std::vector<element> data;

	const integer min_binary_search;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current)
	{
		if(depth == container_size<integer>(*begin)){
			data[current].match = true;
			if(++begin == end){
				data[current].leaf = true;
				return;
			}
		}

		// reserve siblings first
		std::vector<iterator> head{begin};
		for(iterator i = begin; i < end; head.push_back(i)){
			data.push_back({false, false, 0, (*i)[depth]});
			for(symbol c = (*i)[depth]; i < end && (*i)[depth] == c; ++i);
		}

		// recursively construct subtries
		for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
			integer child = data[current].index + i;
			data[child].index = container_size<integer>(data);
			construct(head[i], head[i + 1], depth + 1, child);
		}
	}
};

};

#endif
