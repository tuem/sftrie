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

#ifndef SFTRIE_SET_NAIVE_HPP
#define SFTRIE_SET_NAIVE_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename integer>
class set_naive
{
	using symbol = typename text::value_type;

	struct element;
	struct common_prefix_iterator;

public:
	template<typename random_access_iterator>
	set_naive(random_access_iterator begin, random_access_iterator end);

	std::size_t size() const;
	std::size_t space() const;
	bool exists(const text& pattern) const;
	common_prefix_iterator prefix(const text& pattern) const;

private:
	const std::size_t num_texts;

	std::vector<element> data;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current);

	integer search(const text& pattern) const;
};

template<typename text, typename integer>
struct set_naive<text, integer>::element
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	symbol label;
};

template<typename text, typename integer>
template<typename random_access_iterator>
set_naive<text, integer>::set_naive(random_access_iterator begin, random_access_iterator end):
	num_texts(end - begin), data(1, {false, false, 1, {}})
{
	construct(begin, end, 0, 0);
	data.push_back({false, false, container_size<integer>(data), {}});
	data.shrink_to_fit();
}

template<typename text, typename integer>
std::size_t set_naive<text, integer>::size() const
{
	return num_texts;
}

template<typename text, typename integer>
std::size_t set_naive<text, integer>::space() const
{
	return sizeof(element) * data.size();
}

template<typename text, typename integer>
bool set_naive<text, integer>::exists(const text& pattern) const
{
	integer current = search(pattern);
	return data[current].match;
}

template<typename text, typename integer>
typename set_naive<text, integer>::common_prefix_iterator
set_naive<text, integer>::prefix(const text& pattern) const
{
	integer current = search(pattern);
	return current < data.size() - 1 ?
		common_prefix_iterator(data, current, pattern) :
		common_prefix_iterator(data);
}

template<typename text, typename integer>
template<typename iterator>
void set_naive<text, integer>::construct(iterator begin, iterator end, integer depth, integer current)
{
	if(depth == container_size<integer>(*begin)){
		data[current].match = true;
		if(++begin == end){
			data[current].leaf = true;
			return;
		}
	}

	// reserve siblings first
	std::vector<iterator> head{begin};
	for(iterator i = begin; i < end; head.push_back(i)){
		data.push_back({false, false, 0, (*i)[depth]});
		for(symbol c = (*i)[depth]; i < end && (*i)[depth] == c; ++i);
	}

	// recursively construct subtries
	for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
		integer child = data[current].next + i;
		data[child].next = container_size<integer>(data);
		construct(head[i], head[i + 1], depth + 1, child);
	}
}

template<typename text, typename integer>
integer set_naive<text, integer>::search(const text& pattern) const
{
	integer current = 0;
	for(integer i = 0; i < pattern.size(); ++i){
		if(data[current].leaf)
			return data.size() - 1;
		for(integer l = data[current].next, r = data[l].next - 1; l <= r; ){
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
		return data.size() - 1;
		NEXT:;
	}
	return current;
}

template<typename text, typename integer>
struct set_naive<text, integer>::common_prefix_iterator
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

	const text& operator*()
	{
		return result;
	}

	common_prefix_iterator& operator++()
	{
		do{
			if(!data[path.back()].leaf){
				integer child = data[path.back()].next;
				path.push_back(child);
				result.push_back(data[child].label);
			}
			else{
				while(path.size() > 1 && path.back() + 1 == data[data[path[path.size() - 2]].next].next){
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
