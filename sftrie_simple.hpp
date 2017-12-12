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
	sftrie_simple(const std::vector<text>& input): data(1, {{}, 1, false, false})
	{
		construct(input, 0, static_cast<integer>(input.size()), 0, 0);
		data.push_back({{}, static_cast<integer>(data.size()), true, false});
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(symbol c: pattern){
			if(data[current].leaf)
				return false;
			for(integer start = data[current].position, end = data[start].position - 1; start <= end;){
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
		symbol label;
		integer position: bit_width<integer>() - 2;
		integer leaf: 1;
		integer match: 1;
	};
	std::vector<element> data;

	void construct(const std::vector<text>& input, integer start, integer end, integer depth, integer current)
	{
		if(input[start].size() == depth){
			data[current].match = true;
			if(++start == end){
				data[current].leaf = true;
				return;
			}
		}

		std::vector<integer> split{start};
		for(integer i = start; i < end;){
			data.push_back({input[i][depth], 0, false, false});
			for(integer head = i; i < end && input[i][depth] == input[head][depth]; ++i);
			split.push_back(i);
		}

		for(integer i = 0; i < static_cast<integer>(split.size()) - 1; ++i){
			integer next = data[current].position + i;
			data[next].position = static_cast<integer>(data.size());
			construct(input, split[i], split[i + 1], depth + 1, next);
		}
	}
};

#endif
