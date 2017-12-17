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

#ifndef SFTRIE_BASIC_HPP
#define SFTRIE_BASIC_HPP

#include <vector>

#include "util.hpp"

template<typename text, typename integer>
class sftrie_basic
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
	sftrie_basic(random_access_iterator begin, random_access_iterator end,
			integer min_binary_search = 28):
		data(1, {false, false, 1, {}}), min_binary_search(min_binary_search)
	{
		construct(begin, end, 0, 0);
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return false;
			symbol c = pattern[i];
			integer l = data[current].index;
			if(c == data[l].label){
				current = l;
				continue;
			}
			else if(c < data[l].label){
				return false;
			}
			integer r = data[l].index - 1;
			if(c == data[r].label){
				current = r;
				continue;
			}
			else if(c > data[r].label){
				return false;
			}
			for(++l, --r; l + min_binary_search <= r; ){
				integer m = (l + r) / 2;
				if(data[m].label < c){
					l = m + 1;
				}
				else if(data[m].label > c){
					r = m - 1;
				}
				else{
					current = m;
					goto NEXT;
				}
			}
			for(integer j = l; j <= r; ++j){
				if(data[j].label == c){
					current = j;
					goto NEXT;
				}
			}
			return false;
			NEXT:;
		}
		return data[current].match;
	}

private:
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

#endif
