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

#ifndef SFTRIE_NAIVE_HPP
#define SFTRIE_NAIVE_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename object, typename integer>
class map_basic
{
	using symbol = typename text::value_type;
	using result = std::pair<bool, const object&>;

#pragma pack(1)
	struct element
	{
		bool match: 1;
		bool leaf: 1;
		integer index: bit_width<integer>() - 2;
		symbol label;
		object value;
	};
#pragma pack()

public:
	template<typename random_access_iterator>
	map_basic(random_access_iterator begin, random_access_iterator end,
			integer min_binary_search = 42):
		num_texts(end - begin), data(1, {false, false, 1, {}, {}}), not_found(false, data[0].value),
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

	result find(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return not_found;
			symbol c = pattern[i];
			integer l = data[current].index, r = data[l].index - 1;
			while(l + min_binary_search < r){
				integer m = (l + r) / 2;
				if(data[m].label < c)
					l = m + 1;
				else
					r = m;
			}
			for(; l <= r && data[l].label < c; ++l);
			if(l <= r && data[l].label == c)
				current = l;
			else
				return not_found;
		}
		return data[current].match ? result(true, data[current].value) : not_found;
	}

private:
	const std::size_t num_texts;

	std::vector<element> data;
	const result not_found;

	const integer min_binary_search;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current)
	{
		if(depth == container_size<integer>(begin->first)){
			data[current].match = true;
			data[current].value = begin->second;
			if(++begin == end){
				data[current].leaf = true;
				return;
			}
		}

		// reserve siblings first
		std::vector<iterator> head{begin};
		for(iterator i = begin; i < end; head.push_back(i)){
			data.push_back({false, false, 0, i->first[depth], {}});
			for(symbol c = i->first[depth]; i < end && i->first[depth] == c; ++i);
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
