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
public:
	sftrie_simple(const std::vector<text>& texts): data(1, {false, 1, 0})
	{
		construct(texts, 0, size(texts), 0, 0);
		data.push_back({false, 0, {}});
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(symbol c: pattern){
			if(data[current].index == 0)
				return false;
			for(integer start = data[current].index, end = data[start].index - 1; start <= end;){
				integer mid = (start + end) / 2;
				if(data[mid].label < c){
					start = mid + 1;
				}
				else if(data[mid].label > c){
					end = mid - 1;
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
	using symbol = typename text::value_type;

	struct element
	{
		integer match: 1;
		integer index: bit_width<integer>() - 1;
		symbol label;
	};
	std::vector<element> data;

	template<typename container> integer size(const container& t)
	{
		return static_cast<integer>(t.size());
	}

	void construct(const std::vector<text>& texts, integer start, integer end, integer depth, integer current)
	{
		if(depth == size(texts[start])){
			data[current].match = true;
			if(++start == end)
				return;
		}

		// reserve siblings first
		std::vector<integer> head{start};
		for(integer i = start; i < end; head.push_back(i)){
			data.push_back({false, 0, texts[i][depth]});
			for(integer old = i; i < end && texts[i][depth] == texts[old][depth]; ++i);
		}

		// recursively construct subtries of siblings
		for(integer i = 0; i < size(head) - 1; ++i){
			integer child = data[current].index + i;
			data[child].index = size(data);
			construct(texts, head[i], head[i + 1], depth + 1, child);
		}
	}
};

#endif