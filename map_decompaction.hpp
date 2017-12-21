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

#ifndef SFTRIE_MAP_DECOMPACTION_HPP
#define SFTRIE_MAP_DECOMPACTION_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename object, typename integer>
class map_decompaction
{
	using symbol = typename text::value_type;
	using result = std::pair<bool, const object&>;

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
	map_decompaction(random_access_iterator begin, random_access_iterator end,
			integer min_binary_search = 42, integer min_tail = 4,
			symbol min_symbol = min_char<symbol>(), symbol max_symbol = max_char<symbol>(),
			integer min_decompaction = (1 << (bit_width<symbol>() - 3))):
		num_texts(end - begin), data(1, {false, false, 1, 0, {}, {}}), not_found(false, data[0].value),
		min_binary_search(min_binary_search), tails(1, {}), min_tail(min_tail),
		min_symbol(min_symbol), max_symbol(max_symbol), alphabet_size(max_symbol - min_symbol + 1),
		min_decompaction(min_decompaction)
	{
		construct(begin, end, 0, 0);
		data.push_back({false, false, container_size<integer>(data), container_size<integer>(tails), {}, {}});
		data.shrink_to_fit();
		for(integer i = container_size<integer>(data) - 2; i > 0 && data[i].tail == 0; --i)
			data[i].tail = data.back().tail;
		tails.push_back({});
		tails.shrink_to_fit();
	}

	std::size_t size() const
	{
		return num_texts;
	}

	std::size_t space() const
	{
		return sizeof(element) * data.size() + sizeof(symbol) * tails.size();
	}

	result find(const text& pattern) const
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
			while(l + min_binary_search < r){
				integer m = (l + r) / 2;
				if(data[m].label < pattern[i])
					l = m + 1;
				else
					r = m;
			}
			for(; l < r && data[l].label < pattern[i]; ++l);
			if(l < r && data[l].label == pattern[i])
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

	std::vector<symbol> tails;
	const integer min_tail;

	const symbol min_symbol;
	const symbol max_symbol;
	const integer alphabet_size;
	const integer min_decompaction;

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

		// count children
		std::vector<iterator> head{begin};
		for(iterator i = begin; i < end; head.push_back(i))
			for(symbol c = i->first[depth]; i < end && i->first[depth] == c; ++i);

		if(container_size<integer>(head) > min_decompaction){
			// reserve siblings first
			integer old_data_size = container_size<integer>(data);
			for(symbol c = min_symbol; true; ++c){
				data.push_back({false, false, 0, 0, c, {}});
				if(c == max_symbol)
					break;
			}
			integer alphabet_size = container_size<integer>(data) - old_data_size;

			// extract tail strings of leaves
			for(integer i = 0, j = 0; i < alphabet_size; ++i){
				integer child = data[current].index + i;
				if(j == head.size() - 1 || head[j]->first[depth] != data[child].label){
					data[child].leaf = true;
					continue;
				}
				if(head[j + 1] - head[j] == 1 && container_size<integer>(head[j]->first) - (depth + 1) >= min_tail){
					data[child].match = container_size<integer>(head[j]->first) == depth + 1;
					data[child].leaf = true;
					data[child].tail = container_size<integer>(tails);
					data[child].value = head[j]->second;
					for(integer k = child - 1; k > 0 && data[k].tail == 0; --k)
						data[k].tail = data[child].tail;
					std::copy(std::begin(head[j]->first) + depth + 1, std::end(head[j]->first), std::back_inserter(tails));
				}
				++j;
			}

			// recursively construct subtries
			for(integer i = 0, j = 0; i < alphabet_size; ++i){
				integer child = data[current].index + i;
				data[child].index = container_size<integer>(data);
				if(j == head.size() - 1 || head[j]->first[depth] != data[child].label)
					continue;
				if(head[j + 1] - head[j] != 1 || container_size<integer>(head[j]->first) - (depth + 1) < min_tail)
					construct(head[j], head[j + 1], depth + 1, child);
				++j;
			}
		}
		else{
			// reserve siblings first
			for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
				data.push_back({false, false, 0, 0, head[i]->first[depth], {}});
				integer child = data[current].index + i;
				if(head[i + 1] - head[i] == 1 && container_size<integer>(head[i]->first) - (depth + 1) >= min_tail){
					data[child].match = container_size<integer>(head[i]->first) == depth + 1;
					data[child].leaf = true;
					data[child].tail = container_size<integer>(tails);
					data[child].value = head[i]->second;
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
	}

	result check_tail(const text& pattern, integer i, integer current) const
	{
		return container_size<integer>(pattern) - i == data[current + 1].tail - data[current].tail &&
				std::equal(std::begin(pattern) + i, std::end(pattern), std::begin(tails) + data[current].tail) ?
			result(true, data[current].value) : not_found;
	}
};

};

#endif
