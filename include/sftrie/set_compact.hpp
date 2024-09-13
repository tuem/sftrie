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

#ifndef SFTRIE_SET_COMPACT
#define SFTRIE_SET_COMPACT

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

#include "constants.hpp"
#include "file_header.hpp"
#include "util.hpp"
#include "map_compact.hpp"

namespace sftrie{

template<lexicographically_comparable text, std::integral integer>
class set_compact: public map_compact<text, empty, integer>
{
protected:
	using symbol = typename text::value_type;

public:
	using symbol_type = symbol;
	using text_type = text;
	using integer_type = integer;
	using value_type = integer;
	using size_type = std::size_t;

	using node = typename map_compact<text, empty, integer>::node;
	using common_searcher = typename map_compact<text, empty, integer>::common_searcher;
	using node_type = typename map_compact<text, empty, integer>::virtual_node;

public:
	// constructors
	template<typename random_access_iterator>
	set_compact(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
	template<typename random_access_container>
	set_compact(const random_access_container& texts,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
	template<typename input_stream> set_compact(input_stream& is,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
	set_compact(const std::string path,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));

protected:
	std::uint8_t container_type() const override;

	template<typename iterator>
	std::pair<integer, integer> estimate(iterator begin, iterator end);
	template<typename iterator>
	std::pair<integer, integer> estimate(iterator begin, iterator end, integer depth);

	template<typename iterator>
	void construct(iterator begin, iterator end);
	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current);
};


// constructors

template<lexicographically_comparable text, std::integral integer>
template<typename random_access_iterator>
set_compact<text, integer>::set_compact(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search):
	map_compact<text, empty, integer>(min_binary_search)
{
	construct(begin, end);
}

template<lexicographically_comparable text, std::integral integer>
template<typename random_access_container>
set_compact<text, integer>::set_compact(const random_access_container& texts, integer min_binary_search):
	map_compact<text, empty, integer>(min_binary_search)
{
	construct(std::begin(texts), std::end(texts));
}

template<lexicographically_comparable text, std::integral integer>
template<typename input_stream>
set_compact<text, integer>::set_compact(input_stream& is, integer min_binary_search):
	map_compact<text, empty, integer>(min_binary_search)
{
	this->num_texts = this->load(is);
}

template<lexicographically_comparable text, std::integral integer>
set_compact<text, integer>::set_compact(const std::string path, integer min_binary_search):
	map_compact<text, empty, integer>(min_binary_search)
{
	std::ifstream ifs(path);
	this->num_texts = this->load(ifs);
}


// protected functions

template<lexicographically_comparable text, std::integral integer>
std::uint8_t set_compact<text, integer>::container_type() const
{
	return constants::container_type_set;
}

template<lexicographically_comparable text, std::integral integer>
template<typename iterator>
std::pair<integer, integer> set_compact<text, integer>::estimate(iterator begin, iterator end)
{
	auto [node_count, label_count] = estimate(begin, end, 0);
	return {node_count + 1, label_count}; // sentinel
}

template<lexicographically_comparable text, std::integral integer>
template<typename iterator>
std::pair<integer, integer> set_compact<text, integer>::estimate(iterator begin, iterator end, integer depth)
{
	integer node_count = 1, label_count = 0;

	if(begin < end && depth == this->container_size(*begin))
		++begin;

	if(begin < end){
		for(iterator i = begin; i < end; begin = i){
			for(symbol c = (*i)[depth]; i < end && (*i)[depth] == c; ++i);
			integer d = depth + 1;
			while(d < this->container_size(*begin) && (*begin)[d] == (*(i - 1))[d]){
				++d;
				++label_count;
			}
			auto [n, l] = estimate(begin, i, d);
			node_count += n;
			label_count += l;
		}
	}

	return {node_count, label_count};
}

template<lexicographically_comparable text, std::integral integer>
template<typename iterator>
void set_compact<text, integer>::construct(iterator begin, iterator end)
{
	auto [node_count, label_count] = estimate(begin, end, 0);
	this->data.reserve(node_count);
	this->labels.reserve(label_count);
	if(begin < end)
		construct(begin, end, 0, 0);
	this->data.push_back({false, false, this->container_size(this->data), this->container_size(this->labels), {}, {}});
}

template<lexicographically_comparable text, std::integral integer>
template<typename iterator>
void set_compact<text, integer>::construct(iterator begin, iterator end, integer depth, integer current)
{
	// set flags
	if((this->data[current].match = (depth == this->container_size(*begin))))
		if((this->data[current].leaf = (++begin == end)))
			return;

	// reserve children
	std::vector<iterator> head{begin};
	for(iterator i = begin; i < end; head.push_back(i)){
		this->data.push_back({false, false, 0, 0, (*i)[depth], {}});
		for(symbol c = (*i)[depth]; i < end && (*i)[depth] == c; ++i);
	}

	// compress single paths
	std::vector<integer> depths;
	for(integer i = 0; i < this->container_size(head) - 1; ++i){
		this->data[this->data[current].next + i].ref = this->container_size(this->labels);
		integer d = depth + 1;
		while(d < this->container_size(*head[i]) && (*head[i])[d] == (*(head[i + 1] - 1))[d])
			this->labels.push_back((*head[i])[d++]);
		depths.push_back(d);
	}

	// recursively construct subtries
	for(integer i = 0; i < this->container_size(head) - 1; ++i){
		this->data[this->data[current].next + i].next = this->container_size(this->data);
		construct(head[i], head[i + 1], depths[i], this->data[current].next + i);
	}
}

}

#endif
