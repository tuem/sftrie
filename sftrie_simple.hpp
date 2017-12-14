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

#ifndef SFTRIE_SIMPLE_HPP
#define SFTRIE_SIMPLE_HPP

#include <vector>

#include "util.hpp"

template<typename text, typename integer>
class sftrie_simple
{
private:
	using symbol = typename text::value_type;

	struct element
	{
		bool match: 1;
		bool leaf: 1;
		integer index: bit_width<integer>() - 2;
		symbol label;
	};

public:
	sftrie_simple(const std::vector<text>& texts): data(1, {false, false, 1, {}})
	{
		construct(texts, 0, container_size<integer>(texts), 0, 0);
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return false;
			integer start = data[current].index, end = data[start].index;
			while(start < end){
				integer mid = (start + end) / 2;
				if(data[mid].label < pattern[i]){
					start = mid + 1;
				}
				else if(data[mid].label > pattern[i]){
					end = mid;
				}
				else{
					current = mid;
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

	void construct(const std::vector<text>& texts, integer start, integer end, integer depth, integer current)
	{
		if(depth == container_size<integer>(texts[start])){
			data[current].match = true;
			if(++start == end){
				data[current].leaf = true;
				return;
			}
		}

		// reserve siblings first
		std::vector<integer> head{start};
		for(integer i = start; i < end;){
			data.push_back({false, false, 0, texts[i][depth]});
			for(symbol c = texts[i][depth]; i < end && texts[i][depth] == c; ++i);
			head.push_back(i);
		}

		// recursively construct subtries of siblings
		for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
			integer child = data[current].index + i;
			data[child].index = container_size<integer>(data);
			construct(texts, head[i], head[i + 1], depth + 1, child);
		}
	}
};

#endif
