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

#ifndef SFTRIE_DECOMPACTION_HPP
#define SFTRIE_DECOMPACTION_HPP

#include <vector>
#include <unordered_map>

#include "util.hpp"

template<typename text, typename integer>
class sftrie_decompaction
{
	using symbol = typename text::value_type;

	struct element
	{
		bool match: 1;
		bool leaf: 1;
		integer index: bit_width<integer>() - 2;
		integer tail;
		symbol label;
	};

public:
	sftrie_decompaction(const std::vector<text>& texts, integer min_binary_search = 16, integer min_tail = 1,
			integer min_decompaction = (1 << (bit_width<symbol>() / 2)),
			symbol min_symbol = min_char<symbol>(), symbol max_symbol = max_char<symbol>()):
		data(1, {false, false, 1, 0, {}}), min_binary_search(min_binary_search), tails(1, {}), min_tail(min_tail),
		min_decompaction(min_decompaction), min_symbol(min_symbol), max_symbol(max_symbol)
	{
		construct(texts, 0, container_size<integer>(texts), 0, 0);
		data.push_back({false, false, container_size<integer>(data), container_size<integer>(tails), {}});
		for(integer i = container_size<integer>(data) - 2; i > 0 && data[i].tail == 0; --i)
			data[i].tail = data.back().tail;
		tails.push_back({});
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return check_tail(pattern, i, current);
			integer l = data[current].index;
			if(pattern[i] < data[l].label){
				return false;
			}
			else if(pattern[i] == data[l].label){
				current = l;
				continue;
			}
			integer r = data[l].index - 1;
			if(pattern[i] > data[r].label){
				return false;
			}
			else if(pattern[i] == data[r].label){
				current = r;
				continue;
			}
			if(r - l == static_cast<integer>(static_cast<long long>(max_symbol) - min_symbol)){
				current = data[current].index + static_cast<integer>(pattern[i] - min_symbol);
				continue;
			}
			for(++l, --r; l + min_binary_search <= r; ){
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
	const integer min_binary_search;

	std::vector<symbol> tails;
	const integer min_tail;

	const integer min_decompaction;
	const symbol min_symbol;
	const symbol max_symbol;

	void construct(const std::vector<text>& texts, integer start, integer end, integer depth, integer current)
	{
		if(depth == container_size<integer>(texts[start])){
			data[current].match = true;
			if(++start == end){
				data[current].leaf = true;
				return;
			}
		}

		// count children
		std::vector<integer> head{start};
		for(integer i = start; i < end;){
			for(symbol c = texts[i][depth]; i < end && texts[i][depth] == c; ++i);
			head.push_back(i);
		}

		if(container_size<integer>(head) > min_decompaction){
			// reserve siblings first
			integer old_data_size = container_size<integer>(data);
			for(symbol c = min_symbol; true; ++c){
				data.push_back({false, false, 0, 0, c});
				if(c == max_symbol)
					break;
			}
			integer alphabet_size = container_size<integer>(data) - old_data_size;

			// extract tail strings of leaves
			for(integer i = 0, j = 0; i < alphabet_size; ++i){
				integer child = data[current].index + i;
				if(j == head.size() - 1 || texts[head[j]][depth] != data[child].label){
					data[child].leaf = true;
					continue;
				}
				if(head[j + 1] - head[j] == 1 && container_size<integer>(texts[head[j]]) - (depth + 1) >= min_tail){
					data[child].match = container_size<integer>(texts[head[j]]) == depth + 1;
					data[child].leaf = true;
					data[child].tail = container_size<integer>(tails);
					for(integer k = child - 1; k > 0 && data[k].tail == 0; --k)
						data[k].tail = data[child].tail;
					std::copy(std::begin(texts[head[j]]) + depth + 1, std::end(texts[head[j]]), std::back_inserter(tails));
				}
				++j;
			}

			// recursively construct subtries
			for(integer i = 0, j = 0; i < alphabet_size; ++i){
				integer child = data[current].index + i;
				data[child].index = container_size<integer>(data);
				if(j == head.size() - 1 || texts[head[j]][depth] != data[child].label)
					continue;
				if(head[j + 1] - head[j] != 1 || container_size<integer>(texts[head[j]]) - (depth + 1) < min_tail)
					construct(texts, head[j], head[j + 1], depth + 1, child);
				++j;
			}
		}
		else{
			// reserve siblings first
			for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
				data.push_back({false, false, 0, 0, texts[head[i]][depth]});
				integer child = data[current].index + i;
				if(head[i + 1] - head[i] == 1 && container_size<integer>(texts[head[i]]) - (depth + 1) >= min_tail){
					data[child].match = container_size<integer>(texts[head[i]]) == depth + 1;
					data[child].leaf = true;
					data[child].tail = container_size<integer>(tails);
					for(integer j = child - 1; j > 0 && data[j].tail == 0; --j)
						data[j].tail = data[child].tail;
					std::copy(std::begin(texts[head[i]]) + depth + 1, std::end(texts[head[i]]), std::back_inserter(tails));
				}
			}

			// recursively construct subtries
			for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
				integer child = data[current].index + i;
				data[child].index = container_size<integer>(data);
				if(head[i + 1] - head[i] != 1 || container_size<integer>(texts[head[i]]) - (depth + 1) < min_tail)
					construct(texts, head[i], head[i + 1], depth + 1, child);
			}
		}
	}

	bool check_tail(const text& pattern, integer i, integer current) const
	{
		if(container_size<integer>(pattern) - i != data[current + 1].tail - data[current].tail)
			return false;
		for(integer j = i, k = data[current].tail; j < container_size<integer>(pattern); ++j, ++k)
			if(pattern[j] != tails[k])
				return false;
		return true;
	}
};

#endif
