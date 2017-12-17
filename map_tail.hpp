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

#ifndef SFTRIE_MAP_TAIL_HPP
#define SFTRIE_MAP_TAIL_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename object, typename integer>
class map_tail
{
	using symbol = typename text::value_type;

#pragma pack(1)
	struct element
	{
		bool match: 1;
		bool leaf: 1;
		integer index: bit_width<integer>() - 2;
		integer tail;
		symbol label;
		object value;
	};
#pragma pack()

public:
	template<typename random_access_iterator>
	map_tail(random_access_iterator begin, random_access_iterator end,
			integer min_binary_search = 28, integer min_tail = 4):
		data(1, {false, false, 1, 0, {}, {}}), NOT_FOUND(false, {}),
		min_binary_search(min_binary_search), tails(1, {}), min_tail(min_tail)
	{
		construct(begin, end, 0, 0);
		data.push_back({false, false, container_size<integer>(data), container_size<integer>(tails), {}, {}});
		for(integer i = container_size<integer>(data) - 2; i > 0 && data[i].tail == 0; --i)
			data[i].tail = data.back().tail;
		tails.push_back({});
	}

	std::pair<bool, object> find(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return check_tail(pattern, i, current);
			symbol c = pattern[i];
			integer l = data[current].index;
			if(c == data[l].label){
				current = l;
				continue;
			}
			else if(c < data[l].label){
				return NOT_FOUND;
			}
			integer r = data[l].index - 1;
			if(c == data[r].label){
				current = r;
				continue;
			}
			else if(c > data[r].label){
				return NOT_FOUND;
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
			return NOT_FOUND;
			NEXT:;
		}
		return data[current].match ? std::make_pair(true, data[current].value) : NOT_FOUND;
	}

private:
	std::vector<element> data;
	const std::pair<bool, object> NOT_FOUND;
	const integer min_binary_search;

	std::vector<symbol> tails;
	const integer min_tail;

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
			data.push_back({false, false, 0, 0, i->first[depth], {}});
			for(symbol c = i->first[depth]; i < end && i->first[depth] == c; ++i);
		}

		// TODO
		// extract tail strings of leaves
		for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
			integer child = data[current].index + i;
			if(head[i + 1] - head[i] == 1 && container_size<integer>(head[i]->first) - (depth + 1) >= min_tail){
				data[child].match = container_size<integer>(head[i]->first) == depth + 1;
				data[child].leaf = true;
				data[child].tail = container_size<integer>(tails);
				for(integer j = child - 1; j > 0 && data[j].tail == 0; --j)
					data[j].tail = data[child].tail;
				std::copy(std::begin(head[i]->first) + depth + 1, std::end(head[i]->first), std::back_inserter(tails));
			}
		}

		// recursively construct subtries
		for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
			integer child = data[current].index + i;
			data[child].index = container_size<integer>(data);
			if(head[i + 1] - head[i] != 1 || container_size<integer>(head[i]->first) - (depth + 1) < min_tail)
				construct(head[i], head[i + 1], depth + 1, child);
		}
	}

	std::pair<bool, object> check_tail(const text& pattern, integer i, integer current) const
	{
		if(container_size<integer>(pattern) - i != data[current + 1].tail - data[current].tail)
			return NOT_FOUND;
		for(integer j = i, k = data[current].tail; j < container_size<integer>(pattern); ++j, ++k)
			if(pattern[j] != tails[k])
				return NOT_FOUND;
		return std::make_pair(true, data[current].value);
	}
};

};

#endif
