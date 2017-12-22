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

#ifndef SFTRIE_SET_DECOMPACTION_HPP
#define SFTRIE_SET_DECOMPACTION_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename integer>
class set_decompaction
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
	set_decompaction(random_access_iterator begin, random_access_iterator end,
			integer min_binary_search = 42, integer min_tail = 1,
			symbol min_symbol = min_char<symbol>(), symbol max_symbol = max_char<symbol>(),
			integer min_decompaction = (1 << (bit_width<symbol>() - 3))):
		num_texts(end - begin), data(1, {false, false, 1, {}}),
		min_binary_search(min_binary_search), tail_str(1, {}), tail_pos(1, 0), min_tail(min_tail),
		min_symbol(min_symbol), max_symbol(max_symbol), alphabet_size(max_symbol - min_symbol + 1),
		min_decompaction(min_decompaction)
	{
		construct(begin, end, 0, 0);
		data.push_back({false, false, container_size<integer>(data), {}});
		data.shrink_to_fit();
		tail_pos.push_back(container_size<integer>(tail_str));
		tail_pos.shrink_to_fit();
		for(integer i = container_size<integer>(tail_pos) - 2; i > 0 && tail_pos[i] == 0; --i)
			tail_pos[i] = tail_pos.back();
		tail_str.push_back({});
		tail_str.shrink_to_fit();
		tail_length = tail_str.size();
	}

	std::size_t size() const
	{
		return num_texts;
	}

	std::size_t space() const
	{
		return sizeof(element) * data.size() + sizeof(symbol) * tail_str.size();
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return check_tail(pattern, i, current);
			integer l = data[current].index, r = data[l].index;
			if(l + alphabet_size == r){
				current = l + pattern[i] - min_symbol;
				continue;
			}
			for(integer w = r - l, m; w > min_binary_search; w = m){
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

	std::vector<symbol> tail_str;
	std::vector<integer> tail_pos;
	const integer min_tail;
	integer tail_length;

	const symbol min_symbol;
	const symbol max_symbol;
	const integer alphabet_size;
	const integer min_decompaction;

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

		// count children
		std::vector<iterator> head{begin};
		for(iterator i = begin; i < end; head.push_back(i))
			for(symbol c = (*i)[depth]; i < end && (*i)[depth] == c; ++i);

		if(container_size<integer>(head) > min_decompaction){
			// reserve siblings first
			integer old_data_size = container_size<integer>(data);
			for(symbol c = min_symbol; true; ++c){
				data.push_back({false, false, 0, c});
				tail_pos.push_back(0);
				if(c == max_symbol)
					break;
			}
			integer alphabet_size = container_size<integer>(data) - old_data_size;

			// extract tail strings of leaves
			for(integer i = 0, j = 0; i < alphabet_size; ++i){
				integer child = data[current].index + i;
				if(j == head.size() - 1 || (*head[j])[depth] != data[child].label){
					data[child].leaf = true;
					continue;
				}
				if(head[j + 1] - head[j] == 1 && container_size<integer>(*head[j]) - (depth + 1) >= min_tail){
					data[child].match = container_size<integer>(*head[j]) == depth + 1;
					data[child].leaf = true;
					tail_pos[child] = container_size<integer>(tail_str);
					for(integer k = child - 1; k > 0 && tail_pos[k] == 0; --k)
						tail_pos[k] = tail_pos[child];
					std::copy(std::begin(*head[j]) + depth + 1, std::end(*head[j]), std::back_inserter(tail_str));
				}
				++j;
			}

			// recursively construct subtries
			for(integer i = 0, j = 0; i < alphabet_size; ++i){
				integer child = data[current].index + i;
				data[child].index = container_size<integer>(data);
				if(j == head.size() - 1 || (*head[j])[depth] != data[child].label)
					continue;
				if(head[j + 1] - head[j] != 1 || container_size<integer>(*head[j]) - (depth + 1) < min_tail)
					construct(head[j], head[j + 1], depth + 1, child);
				++j;
			}
		}
		else{
			// reserve siblings first
			for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
				data.push_back({false, false, 0, (*head[i])[depth]});
				tail_pos.push_back(0);
				integer child = data[current].index + i;
				if(head[i + 1] - head[i] == 1 && container_size<integer>(*head[i]) - (depth + 1) >= min_tail){
					data[child].match = container_size<integer>(*head[i]) == depth + 1;
					data[child].leaf = true;
					tail_pos[child] = container_size<integer>(tail_str);
					for(integer k = child - 1; k > 0 && tail_pos[k] == 0; --k)
						tail_pos[k] = tail_pos[child];
					std::copy(std::begin(*head[i]) + depth + 1, std::end(*head[i]), std::back_inserter(tail_str));
				}
			}

			// recursively construct subtries
			for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
				integer child = data[current].index + i;
				data[child].index = container_size<integer>(data);
				if(head[i + 1] - head[i] != 1 || container_size<integer>(*head[i]) - (depth + 1) < min_tail)
					construct(head[i], head[i + 1], depth + 1, child);
			}
		}
	}

	bool check_tail(const text& pattern, integer i, integer current) const
	{
		return tail_pos[current] + pattern.size() - i == tail_pos[current + 1] &&
			std::equal(std::begin(pattern) + i, std::end(pattern), std::begin(tail_str) + tail_pos[current]);
	}
};

};

#endif
