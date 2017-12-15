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

#ifndef SFTRIE_TAIL_HPP
#define SFTRIE_TAIL_HPP

#include <vector>
#include <unordered_map>

#include "util.hpp"

template<typename text, typename integer>
class sftrie_tail
{
	using symbol = typename text::value_type;

	struct element
	{
		bool match: 1;
		bool leaf: 1;
		integer index: bit_width<integer>() - 2;
		integer tail;
		integer tailend; // TODO: remove
		symbol label;
	};

public:
	sftrie_tail(const std::vector<text>& texts, integer min_binsearch = 16, integer min_tail = 3):
		data(1, {false, false, 1, 0, 0, {}}), min_binsearch(min_binsearch),
		tailstr(1, {}), min_tail(min_tail)
	{
		construct(texts, 0, container_size<integer>(texts), 0, 0);
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return check_tail(pattern, i, current);
			integer l = data[current].index, r = data[l].index - 1;
			while(l + min_binsearch <= r){
				integer m = (l + r) / 2;
				if(data[m].label < pattern[i]){
					l = m + 1;
				}
				else if(data[m].label > pattern[i]){
					r = m - 1;
				}
				else{
					current = m;
					goto NEXT;
				}
			}
			for(integer j = l; j <= r; ++j){
				if(data[j].label == pattern[i]){
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
	const integer min_binsearch;

	std::vector<symbol> tailstr;
	const integer min_tail;

	void construct(const std::vector<text>& texts, integer start, integer end, integer depth, integer current)
	{
		if(depth == container_size<integer>(texts[start])){
			data[current].match = true;
			if(++start == end){
				data[current].leaf = true;
				return;
			}
		}
		if(start == end - 1 && container_size<integer>(texts[start]) - depth >= min_tail){
			data[current].leaf = true;
			data[current].tail = container_size<integer>(tailstr);
			std::copy(std::begin(texts[start]) + depth, std::end(texts[start]), std::back_inserter(tailstr));
			data[current].tailend = container_size<integer>(tailstr);
			return;
		}

		// reserve siblings first
		std::vector<integer> head{start};
		for(integer i = start; i < end;){
			data.push_back({false, false, 0, 0, 0, texts[i][depth]});
			for(symbol c = texts[i][depth]; i < end && texts[i][depth] == c; ++i);
			head.push_back(i);
		}

		// recursively construct subtries
		for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
			integer child = data[current].index + i;
			data[child].index = container_size<integer>(data);
			construct(texts, head[i], head[i + 1], depth + 1, child);
		}
	}

	bool check_tail(const text& pattern, integer i, integer current) const
	{
		if(data[current].tail == 0)
			return false;
		else if(container_size<integer>(pattern) - i != data[current].tailend - data[current].tail)
			return false;
		for(integer j = i; j < container_size<integer>(pattern); ++j)
			if(pattern[j] != tailstr[data[current].tail + (j - i)])
				return false;
		return true;
	}
};

#endif
