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

	struct element;
	struct common_searcher;
	struct traversal_iterator;
	struct prefix_iterator;

public:
	template<typename random_access_iterator>
	map_basic(random_access_iterator begin, random_access_iterator end,
			integer min_binary_search = 42);

	std::size_t size() const;
	std::size_t node_size() const;
	std::size_t trie_size() const;
	std::size_t space() const;
	result find(const text& pattern) const;
	common_searcher searcher() const;

private:
	const std::size_t num_texts;

	std::vector<element> data;
	const result not_found;

	const integer min_binary_search;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current);

	integer search(const text& pattern) const;
};

#pragma pack(1)
template<typename text, typename object, typename integer>
struct map_basic<text, object, integer>::element
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	symbol label;
	object value;
};
#pragma pack()

template<typename text, typename object, typename integer>
template<typename random_access_iterator>
map_basic<text, object, integer>::map_basic(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search):
	num_texts(end - begin), data(1, {false, false, 1, {}, {}}), not_found(false, data[0].value),
	min_binary_search(min_binary_search)
{
	construct(begin, end, 0, 0);
	data.push_back({false, false, container_size<integer>(data), {}, {}});
	data.shrink_to_fit();
}

template<typename text, typename object, typename integer>
std::size_t map_basic<text, object, integer>::size() const
{
	return num_texts;
}

template<typename text, typename object, typename integer>
std::size_t map_basic<text, object, integer>::node_size() const
{
	return sizeof(element);
}

template<typename text, typename object, typename integer>
std::size_t map_basic<text, object, integer>::trie_size() const
{
	return data.size();
}

template<typename text, typename object, typename integer>
std::size_t map_basic<text, object, integer>::space() const
{
	return sizeof(element) * data.size();
}

template<typename text, typename object, typename integer>
typename map_basic<text, object, integer>::result
map_basic<text, object, integer>::find(const text& pattern) const
{
	integer current = search(pattern);
	return current < data.size() && data[current].match ?
		result(true, data[current].value) : not_found;
}

template<typename text, typename object, typename integer>
typename map_basic<text, object, integer>::common_searcher
map_basic<text, object, integer>::searcher() const
{
	return common_searcher(*this);
}

template<typename text, typename object, typename integer>
template<typename iterator>
void map_basic<text, object, integer>::construct(
	iterator begin, iterator end, integer depth, integer current)
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
		integer child = data[current].next + i;
		data[child].next = container_size<integer>(data);
		construct(head[i], head[i + 1], depth + 1, child);
	}
}

template<typename text, typename object, typename integer>
integer map_basic<text, object, integer>::search(const text& pattern) const
{
	integer current = 0;
	for(integer i = 0; i < pattern.size(); ++i){
		if(data[current].leaf)
			return data.size();
		current = data[current].next;
		integer end = data[current].next;
		for(integer w = end - current, m; w > min_binary_search; w = m){
			m = w >> 1;
			current += data[current + m].label < pattern[i] ? w - m : 0;
		}
		for(; current < end && data[current].label < pattern[i]; ++current);
		if(!(current < end && data[current].label == pattern[i]))
			return data.size();
	}
	return current;
}

template<typename text, typename object, typename integer>
struct map_basic<text, object, integer>::common_searcher
{
	const map_basic<text, object, integer>& index;

	std::vector<integer> path;
	text result;
	std::vector<integer> path_end;
	text result_end;

	common_searcher(const map_basic<text, object, integer>& index): index(index){}

	traversal_iterator traverse(const text& pattern)
	{
		path.clear();
		result.clear();
		integer current = index.search(pattern);
		return current < index.data.size() - 1 ?
			traversal_iterator(index.data, path, result, path_end, result_end, current, pattern) :
			traversal_iterator(index.data, path_end, result_end);
	}

	prefix_iterator prefix(const text& pattern)
	{
		result.clear();
		return prefix_iterator(*this, pattern, 0, 0);
	}
};

template<typename text, typename object, typename integer>
struct map_basic<text, object, integer>::traversal_iterator
{
	const std::vector<element>& data;

	std::vector<integer>& path;
	text& result;
	std::vector<integer>& path_end;
	text& result_end;

	traversal_iterator(const std::vector<element>& data, std::vector<integer>& path, text& result,
			std::vector<integer>& path_end, text& result_end, integer root, const text& prefix):
		data(data), path(path), result(result), path_end(path_end), result_end(result_end)
	{
		path.push_back(root);
		std::copy(std::begin(prefix), std::end(prefix), std::back_inserter(result));
		if(!data[root].match)
			++*this;
	}

	traversal_iterator(const std::vector<element>& data, std::vector<integer>& path, text& result):
		data(data), path(path), result(result), path_end(path), result_end(result) {}

	traversal_iterator& begin()
	{
		return *this;
	}

	traversal_iterator end() const
	{
		return traversal_iterator(data, path_end, result_end);
	}

	bool operator!=(const traversal_iterator& i) const
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
		return std::pair<const text&, const object&>(result, data[path.back()].value);
	}

	traversal_iterator& operator++()
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

template<typename text, typename object, typename integer>
struct map_basic<text, object, integer>::prefix_iterator
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

	const std::pair<const text&, const object&> operator*() const
	{
		return std::pair<const text&, const object&>(searcher.result, searcher.index.data[current].value);
	}

	prefix_iterator& operator++()
	{
		for(; !searcher.index.data[current].leaf && depth < pattern.size();){
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
