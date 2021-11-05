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

#ifndef SFTRIE_SET_BASIC_HPP
#define SFTRIE_SET_BASIC_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename integer>
class set_basic
{
	using symbol = typename text::value_type;

	struct element;
	struct common_searcher;
	struct traversal_iterator;
	struct prefix_iterator;

public:
	template<typename random_access_iterator>
	set_basic(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search = 42);

	std::size_t size() const;
	std::size_t node_size() const;
	std::size_t trie_size() const;
	std::size_t space() const;
	bool exists(const text& pattern) const;
	common_searcher searcher() const;

private:
	const std::size_t num_texts;

	std::vector<element> data;

	const integer min_binary_search;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current);

	integer search(const text& pattern) const;
};

#pragma pack(1)
template<typename text, typename integer>
struct set_basic<text, integer>::element
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	symbol label;
};
#pragma pack()

template<typename text, typename integer>
template<typename random_access_iterator>
set_basic<text, integer>::set_basic(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search):
	num_texts(end - begin), data(1, {false, false, 1, {}}),
	min_binary_search(min_binary_search)
{
	construct(begin, end, 0, 0);
	data.push_back({false, false, container_size<integer>(data), {}});
	data.shrink_to_fit();
}

template<typename text, typename integer>
std::size_t set_basic<text, integer>::size() const
{
	return num_texts;
}

template<typename text, typename integer>
std::size_t set_basic<text, integer>::node_size() const
{
	return sizeof(element);
}

template<typename text, typename integer>
std::size_t set_basic<text, integer>::trie_size() const
{
	return data.size();
}

template<typename text, typename integer>
std::size_t set_basic<text, integer>::space() const
{
	return sizeof(element) * data.size();
}

template<typename text, typename integer>
bool set_basic<text, integer>::exists(const text& pattern) const
{
	return data[search(pattern)].match;
}

template<typename text, typename integer>
typename set_basic<text, integer>::common_searcher
set_basic<text, integer>::searcher() const
{
	return common_searcher(*this);
}

template<typename text, typename integer>
template<typename iterator>
void set_basic<text, integer>::construct(iterator begin, iterator end, integer depth, integer current)
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
integer set_basic<text, integer>::search(const text& pattern) const
{
	integer current = 0;
	for(integer i = 0; i < pattern.size(); ++i){
		if(data[current].leaf)
			return data.size() - 1;
		current = data[current].next;
		integer end = data[current].next;
		for(integer w = end - current, m; w > min_binary_search; w = m){
			m = w >> 1;
			current += data[current + m].label < pattern[i] ? w - m : 0;
		}
		for(; current < end && data[current].label < pattern[i]; ++current);
		if(!(current < end && data[current].label == pattern[i]))
			return data.size() - 1;
	}
	return current;
}

template<typename text, typename integer>
struct set_basic<text, integer>::common_searcher
{
	const set_basic<text, integer>& index;
	std::vector<integer> path;
	text result;

	common_searcher(const set_basic<text, integer>& index): index(index){}

	integer find(const text& pattern) const
	{
		auto i = index.search(pattern);
		return index.data[i].match ? i : end();
	}

	integer end() const
	{
		return index.data.size() - 1;
	}

	integer count(const text& pattern) const
	{
		return find(pattern) != end() ? 1 : 0;
	}

	traversal_iterator traverse(const text& pattern)
	{
		integer root = index.search(pattern);
		if(root < index.data.size() - 1){
			path.clear();
			result.clear();
			path.push_back(root);
			std::copy(std::begin(pattern), std::end(pattern), std::back_inserter(result));
		}
		return traversal_iterator(*this, pattern, root);
	}

	prefix_iterator prefix(const text& pattern)
	{
		result.clear();
		return prefix_iterator(*this, pattern, 0, 0);
	}
};

template<typename text, typename integer>
struct set_basic<text, integer>::traversal_iterator
{
	common_searcher& searcher;
	const text& prefix;
	integer current;

	traversal_iterator(common_searcher& searcher, const text& prefix, integer root):
		searcher(searcher), prefix(prefix), current(root)
	{
		if(root < searcher.index.data.size() - 1 && !searcher.index.data[root].match)
			++*this;
	}

	traversal_iterator& begin()
	{
		return *this;
	}

	traversal_iterator end() const
	{
		return traversal_iterator(searcher, prefix, searcher.index.data.size() - 1);
	}

	bool operator!=(const traversal_iterator& i) const
	{
		return this->current != i.current;
	}

	const text& operator*() const
	{
		return searcher.result;
	}

	traversal_iterator& operator++()
	{
		do{
			if(!searcher.index.data[searcher.path.back()].leaf){
				integer child = searcher.index.data[searcher.path.back()].next;
				searcher.path.push_back(child);
				searcher.result.push_back(searcher.index.data[child].label);
			}
			else{
				while(searcher.path.size() > 1 && searcher.path.back() + 1 ==
						searcher.index.data[searcher.index.data[searcher.path[searcher.path.size() - 2]].next].next){
					searcher.path.pop_back();
					searcher.result.pop_back();
				}
				if(searcher.path.size() > 1)
					searcher.result.back() = searcher.index.data[++searcher.path.back()].label;
				else
					searcher.path.pop_back();
			}
		}while(!searcher.path.empty() && !searcher.index.data[searcher.path.back()].match);
		current = !searcher.path.empty() ? searcher.path.back() : searcher.index.data.size() - 1;
		return *this;
	}
};

template<typename text, typename integer>
struct set_basic<text, integer>::prefix_iterator
{
	common_searcher& searcher;
	const text& pattern;
	integer current;
	integer depth;

	prefix_iterator(common_searcher& searcher, const text& pattern, integer current, integer depth):
		searcher(searcher), pattern(pattern), current(current), depth(depth)
	{
		if(current == 0 && !searcher.index.data[current].match){
			if(pattern.empty())
				this->current = searcher.index.data.size() - 1;
			else
				++*this;
		}
	}

	prefix_iterator& begin()
	{
		return *this;
	}

	prefix_iterator end() const
	{
		return prefix_iterator(searcher, pattern, searcher.index.data.size() - 1, pattern.size());
	}

	bool operator!=(const prefix_iterator& i) const
	{
		return this->current != i.current;
	}

	const text& operator*() const
	{
		return searcher.result;
	}

	prefix_iterator& operator++()
	{
		for(; !searcher.index.data[current].leaf && depth < pattern.size(); ){
			current = searcher.index.data[current].next;
			integer end = searcher.index.data[current].next;
			for(integer w = end - current, m; w > searcher.index.min_binary_search; w = m){
				m = w >> 1;
				current += searcher.index.data[current + m].label < pattern[depth] ? w - m : 0;
			}
			for(; current < end && searcher.index.data[current].label < pattern[depth]; ++current);
			if(!(current < end && searcher.index.data[current].label == pattern[depth]))
				break;
			searcher.result.push_back(pattern[depth++]);
			if(searcher.index.data[current].match)
				return *this;
		}
		current = searcher.index.data.size() - 1;
		return *this;
	}
};

};

#endif
