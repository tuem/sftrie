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

#ifndef SFTRIE_MAP_NAIVE_HPP
#define SFTRIE_MAP_NAIVE_HPP

#include <vector>
#include <functional>

#include "util.hpp"

namespace sftrie{

template<typename text, typename object, typename integer>
class map_naive
{
	using symbol = typename text::value_type;
	using result = std::pair<bool, const object&>;

	struct element;
	struct common_prefix_searcher;
	struct common_prefix_iterator;

public:
	template<typename random_access_iterator>
	map_naive(random_access_iterator begin, random_access_iterator end);

	std::size_t size() const;
	std::size_t space() const;
	result find(const text& pattern) const;
	common_prefix_searcher searcher() const;

private:
	const std::size_t num_texts;

	std::vector<element> data;
	const result not_found;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current);

	integer search(const text& pattern) const;
};

template<typename text, typename object, typename integer>
struct map_naive<text, object, integer>::element
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	symbol label;
	object value;
};

template<typename text, typename object, typename integer>
template<typename random_access_iterator>
map_naive<text, object, integer>::map_naive(random_access_iterator begin, random_access_iterator end):
	num_texts(end - begin), data(1, {false, false, 1, {}, {}}), not_found(false, data[0].value)
{
	construct(begin, end, 0, 0);
	data.push_back({false, false, container_size<integer>(data), {}, {}});
	data.shrink_to_fit();
}

template<typename text, typename object, typename integer>
std::size_t map_naive<text, object, integer>::size() const
{
	return num_texts;
}

template<typename text, typename object, typename integer>
std::size_t map_naive<text, object, integer>::space() const
{
	return sizeof(element) * data.size();
}

template<typename text, typename object, typename integer>
typename map_naive<text, object, integer>::result
map_naive<text, object, integer>::find(const text& pattern) const
{
	integer current = search(pattern);
	return data[current].match ? result(true, data[current].value) : not_found;
}

template<typename text, typename object, typename integer>
typename map_naive<text, object, integer>::common_prefix_searcher
map_naive<text, object, integer>::searcher() const
{
	return common_prefix_searcher(*this);
}
/*
template<typename text, typename object, typename integer>
typename map_naive<text, object, integer>::common_prefix_iterator
map_naive<text, object, integer>::prefix(const text& pattern) const
{
	integer current = search(pattern);
	return current < data.size() - 1 ?
		common_prefix_iterator(data, current, pattern) :
		common_prefix_iterator(data);
}
*/

template<typename text, typename object, typename integer>
template<typename iterator>
void map_naive<text, object, integer>::construct(
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
integer map_naive<text, object, integer>::search(const text& pattern) const
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

template<typename text, typename object, typename integer>
struct map_naive<text, object, integer>::common_prefix_searcher
{
	const map_naive<text, object, integer>& index;

	std::vector<integer> path;
	text result;
	std::vector<integer> path_end;
	text result_end;

	common_prefix_searcher(const map_naive<text, object, integer>& index): index(index){}

	common_prefix_iterator common_prefix(const text& pattern)
	{
		path.clear();
		result.clear();
		integer current = index.search(pattern);
		return current < index.data.size() - 1 ?
			common_prefix_iterator(index.data, path, result, path_end, result_end, current, pattern) :
			common_prefix_iterator(index.data, path_end, result_end);
	}
};

template<typename text, typename object, typename integer>
struct map_naive<text, object, integer>::common_prefix_iterator
{
	const std::vector<element>& data;

	std::vector<integer>& path;
	text& result;
	std::vector<integer>& path_end;
	text& result_end;

	common_prefix_iterator(const std::vector<element>& data, std::vector<integer>& path, text& result,
			std::vector<integer>& path_end, text& result_end, integer root, const text& prefix):
		data(data), path(path), result(result), path_end(path_end), result_end(result_end)
	{
		path.push_back(root);
		std::copy(std::begin(prefix), std::end(prefix), std::back_inserter(result));
		if(!data[root].match)
			++*this;
	}

	common_prefix_iterator(const std::vector<element>& data, std::vector<integer>& path, text& result):
		data(data), path(path), result(result), path_end(path), result_end(result) {}
/*
	std::vector<integer> path;
	text result;

	common_prefix_iterator(const std::vector<element>& data, integer root, const text& prefix):
		data(data), path(1, root), result(prefix)
	{
		if(!data[root].match)
			++*this;
	}

	common_prefix_iterator(const std::vector<element>& data): data(data){}
*/

	common_prefix_iterator& begin()
	{
		return *this;
	}

	common_prefix_iterator end() const
	{
		return common_prefix_iterator(data, path_end, result_end);
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
		return std::pair<const text&, const object&>(result, data[path.back()].value);
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
