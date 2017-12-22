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

#ifndef SFTRIE_SET_MINIMAL_HPP
#define SFTRIE_SET_MINIMAL_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename integer>
class set_minimal
{
	using symbol = typename text::value_type;

	struct element
	{
		bool match: 1;
		bool leaf: 1;
		integer index: bit_width<integer>() - 2;
		symbol label;
	};

public:
	template<typename random_access_iterator>
	set_minimal(random_access_iterator begin, random_access_iterator end):
		data(1, {false, false, 1, {}})
	{
		construct(begin, end, 0, 0);
	}

	bool exists(const text& pattern) const
	{
		integer u = 0, v;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[u].leaf)
				return false;
			for(u = data[u].index, v = data[u].index - 1; u < v;){
				integer m = (u + v) / 2;
				if(data[m].label < pattern[i])
					u = m + 1;
				else
					v = m;
			}
			if(!(u <= v && data[u].label == pattern[i]))
				return false;
		}
		return data[u].match;
	}

private:
	std::vector<element> data;

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
			data[data[current].index + i].index = container_size<integer>(data);
			construct(head[i], head[i + 1], depth + 1, data[current].index + i);
		}
	}
};

};

#endif
