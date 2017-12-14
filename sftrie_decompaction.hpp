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
public:
	sftrie_decompaction(const std::vector<text>& texts, integer threshold = (1 << bit_width<symbol>()) / 2):
		data(1, {false, false, false, 1, {}}), threshold(threshold)
	{
		construct(texts, 0, container_size<integer>(texts), 0, 0);
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return check_tail(pattern, i, current);
			if(data[current].expanded){
				current = data[current].index + static_cast<integer>(pattern[i] - min_char<symbol>());
				continue;
			}
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
	using symbol = typename text::value_type;

	struct element
	{
		integer match: 1;
		integer leaf: 1;
		integer expanded: 1;
		integer index: bit_width<integer>() - 3;
		symbol label;
	};
	std::vector<element> data;
	std::unordered_map<integer, std::vector<symbol>> tail;
	const integer threshold;

	void construct(const std::vector<text>& texts, integer start, integer end, integer depth, integer current)
	{
		if(depth == container_size<integer>(texts[start])){
			data[current].match = true;
			if(++start == end){
				data[current].leaf = true;
				return;
			}
		}
		else if(start == end - 1){
			data[current].leaf = true;
			std::vector<symbol> tailstr;
			std::copy(std::begin(texts[start]) + depth, std::end(texts[start]), std::back_inserter(tailstr));
			tail[current] = tailstr;
			return;
		}

		std::vector<integer> head{start};
		for(integer i = start; i < end;){
			for(integer old = i; i < end && texts[i][depth] == texts[old][depth]; ++i);
			head.push_back(i);
		}

		if(container_size<integer>(head) > threshold){
			data[current].expanded = true;
			integer node_end = 0;
			for(symbol c = min_char<symbol>(); true; ++c, ++node_end){
				data.push_back({false, false, false, current + (1 << bit_width<symbol>()), c});
				if(c == max_char<symbol>())
					break;
			}

            integer k = 0;
			integer new_start = start, new_end = new_start;
			for(integer j = 0; new_start < end && j < node_end; ++j){
				integer child = data[current].index + j;
				data[child].index = container_size<integer>(data);
				if(texts[new_start][depth] != data[child].label){
				    data[child].leaf = true;
					continue;
                }
                new_end = head[k++];
				construct(texts, new_start, head[k], depth + 1, child);
				new_start = new_end;
			}
		}
		else{
			// reserve siblings first
			integer j = 0;
			for(integer i = start; i < end;){
				data.push_back({false, false, false, 0, texts[i][depth]});
				i = head[++j];
			}

			// recursively construct subtries of siblings
			for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
				integer child = data[current].index + i;
				data[child].index = container_size<integer>(data);
				construct(texts, head[i], head[i + 1], depth + 1, child);
			}
		}
	}

	bool check_tail(const text& pattern, integer i, integer current) const
	{
		auto p = tail.find(current);
		if(p != std::end(tail)){
			const auto& tailstr = p->second;
			if(container_size<integer>(pattern) - i != container_size<integer>(tailstr))
				return false;
			for(integer j = i; j < container_size<integer>(pattern); ++j)
				if(pattern[j] != tailstr[j - i])
					return false;
			return true;
		}
		return false;
	}
};

#endif
