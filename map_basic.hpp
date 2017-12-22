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

#ifndef SFTRIE_NAIVE_HPP
#define SFTRIE_NAIVE_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename object, typename integer>
class map_basic
{
	using symbol = typename text::value_type;
	using result = std::pair<bool, const object&>;

#pragma pack(1)
	struct element
	{
		bool match: 1;
		bool leaf: 1;
		integer index: bit_width<integer>() - 2;
		symbol label;
		object value;
	};
#pragma pack()

	struct common_prefix_iterator;

public:
	template<typename random_access_iterator>
	map_basic(random_access_iterator begin, random_access_iterator end,
			integer min_binary_search = 42):
		num_texts(end - begin), data(1, {false, false, 1, {}, {}}), not_found(false, data[0].value),
		min_binary_search(min_binary_search)
	{
		construct(begin, end, 0, 0);
		data.push_back({false, false, container_size<integer>(data), {}, {}});
		data.shrink_to_fit();
	}

	std::size_t size() const
	{
		return num_texts;
	}

	std::size_t space() const
	{
		return sizeof(element) * data.size();
	}

	result find(const text& pattern) const
	{
		integer current = search(pattern);
		return current < data.size() && data[current].match ?
			result(true, data[current].value) : not_found;
	}

	common_prefix_iterator prefix(const text& pattern) const
	{
		integer current = search(pattern);
		return current < data.size() ?
			common_prefix_iterator(data, current, pattern) :
			common_prefix_iterator(data);
	}

private:
	const std::size_t num_texts;

	std::vector<element> data;
	const result not_found;

	const integer min_binary_search;

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
			data.push_back({false, false, 0, i->first[depth], {}});
			for(symbol c = i->first[depth]; i < end && i->first[depth] == c; ++i);
		}

		// recursively construct subtries
		for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
			integer child = data[current].index + i;
			data[child].index = container_size<integer>(data);
			construct(head[i], head[i + 1], depth + 1, child);
		}
	}

	integer search(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return data.size();
			integer l = data[current].index, r = data[l].index;
			for(integer w = r - l, m; w > min_binary_search; w = m){
				m = w >> 1;
				l += data[l + m].label < pattern[i] ? w - m : 0;
			}
			for(; l < r && data[l].label < pattern[i]; ++l);
			if(l < r && data[l].label == pattern[i])
				current = l;
			else
				return data.size();
		}
		return current;
	}
};

template<typename text, typename object, typename integer>
struct map_basic<text, object, integer>::common_prefix_iterator
{
	const std::vector<element>& data;
	std::vector<integer> path;
	text result;

	common_prefix_iterator(const std::vector<element>& data, integer root, const text& prefix):
		data(data), path(1, root), result(prefix)
	{
		if(!data[root].match)
			++*this;
	}

	common_prefix_iterator(const std::vector<element>& data): data(data){}

	common_prefix_iterator& begin()
	{
		return *this;
	}

	common_prefix_iterator end() const
	{
		return common_prefix_iterator(data);
	}

	bool operator!=(const common_prefix_iterator& i) const
	{
		if(this->path.size() != i.path.size())
			return true;
		else if(this->path.empty() && i.path.empty())
			return false;
		else
			return this->path.back() != i.path.back();
	}

	const std::pair<const text&, const object&> operator*() const
	{
		return std::make_pair(result, data[path.back()].value);
	}

	common_prefix_iterator& operator++()
	{
		do{
			if(!data[path.back()].leaf){
				integer child = data[path.back()].index;
				path.push_back(child);
				result.push_back(data[child].label);
			}
			else{
				while(path.size() > 1 && path.back() + 1 == data[data[path[path.size() - 2]].index].index){
					path.pop_back();
					result.pop_back();
				}
				if(path.size() > 1)
					result.back() = data[++path.back()].label;
				else
					path.pop_back();
			}
		}while(!path.empty() && !data[path.back()].match);
		return *this;
	}
};

};

#endif
