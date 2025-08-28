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

#ifndef SFTRIE_MAP_FAST
#define SFTRIE_MAP_FAST

#include <cstdint>
#include <concepts>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <type_traits>

#include "constants.hpp"
#include "lookup_table_mode.hpp"
#include "file_header.hpp"
#include "util.hpp"
#include "lexicographically_comparable.hpp"
#include "default_constructible.hpp"
#include "random_access_container.hpp"

namespace sftrie{

template<lexicographically_comparable text, default_constructible item, std::integral integer>
class map_fast
{
protected:
	using symbol = typename text::value_type;
	using number = float;
	using traits = trie_traits<text, item, integer>;
	using selector = key_value_selector<text, item, integer>;

public:
	using symbol_type = symbol;
	using text_type = text;
	using item_type = item;
	using integer_type = integer;
	using value_type = typename traits::value_type;
	using size_type = std::size_t;

	struct node;
	struct virtual_node;
	struct child_iterator;
	struct common_searcher;
	struct subtree_iterator;
	struct prefix_iterator;

	using node_type = virtual_node;

public:
	// constructors
	map_fast();
	template<std::random_access_iterator iterator>
	map_fast(iterator begin, iterator end, bool two_pass = true,
		lookup_table_mode lut_mode = lookup_table_mode::root_only,
		integer min_lookup_table_children = static_cast<integer>(constants::default_min_lookup_table_children<symbol>()),
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
	template<random_access_container container>
	map_fast(const container& texts, bool two_pass = true,
		lookup_table_mode lut_mode = lookup_table_mode::root_only,
		integer min_lookup_table_children = static_cast<integer>(constants::default_min_lookup_table_children<symbol>()),
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
	template<typename input_stream> map_fast(input_stream& is,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
	map_fast(const std::string path,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));

	// information
	size_type size() const;
	size_type node_size() const;
	size_type trie_size() const;
	size_type total_space() const;

	// construction
	template<typename iterator>
	integer construct(iterator begin, iterator end, bool two_pass = true,
		lookup_table_mode lut_mode = lookup_table_mode::root_only);
	template<random_access_container container>
	integer construct(const container& texts, bool two_pass = true,
		lookup_table_mode lut_mode = lookup_table_mode::root_only);

	// search operations
	bool exists(const text& pattern) const;
	node_type find(const text& pattern) const;
	common_searcher searcher() const;

	// tree operations
	node_type root() const;
	const std::vector<node>& raw_data() const;
	const std::vector<symbol>& raw_labels() const;

	// value operations
	bool update(const node_type& n, const item& value);
	bool update(const text& key, const item& value);
	typename traits::value_ref_type operator[](const text& key);

	// file I/O
	template<typename output_stream> void save(output_stream& os) const;
	void save(const std::string path) const;
	template<typename input_stream> integer load(input_stream& is);
	integer load(const std::string path);

protected:
	std::pair<symbol, symbol> alphabet_range;
	integer alphabet_size;
	integer min_lookup_table_children;
	const integer min_binary_search;

	size_type num_texts;
	std::vector<node> data;
	std::vector<symbol> labels;

	template<typename container>
	static integer container_size(const container& c);

	void reset(integer node_count = static_cast<integer>(0),
		integer label_count = static_cast<integer>(0));
	template<typename iterator>
	std::pair<integer, integer> estimate(iterator begin, iterator end,
		lookup_table_mode lut_mode) const;
	template<typename iterator>
	std::pair<integer, integer> estimate(iterator begin, iterator end,
		integer depth, lookup_table_mode lut_mode) const;
	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current,
		lookup_table_mode lut_mode);
};


#pragma pack(1)
template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_fast<text, item, integer>::node
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	integer ref;
	symbol label;
	[[no_unique_address]] item value;
};
#pragma pack()


// constructors

template<lexicographically_comparable text, default_constructible item, std::integral integer>
map_fast<text, item, integer>::map_fast():
	alphabet_range({0, 0}),  alphabet_size(1),
	min_lookup_table_children(0), min_binary_search(0),
	num_texts(0), data(1, {false, true, 1, 0, {}, {}})
{}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<std::random_access_iterator iterator>
map_fast<text, item, integer>::map_fast(iterator begin, iterator end, bool two_pass,
		lookup_table_mode lut_mode, integer min_lookup_table_children, integer min_binary_search):
	alphabet_range(actual_alphabet_range<text, item, integer>(begin, end)),
	alphabet_size(alphabet_range.second - alphabet_range.first + 1),
	min_lookup_table_children(min_lookup_table_children),
	min_binary_search(min_binary_search),
	num_texts(end - begin), data(1, {false, false, 1, 0, {}, {}})
{
	construct(begin, end, two_pass, lut_mode);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<random_access_container container>
map_fast<text, item, integer>::map_fast(const container& texts, bool two_pass,
		lookup_table_mode lut_mode, integer min_lookup_table_children, integer min_binary_search):
	alphabet_range(actual_alphabet_range<text, item, integer>(std::begin(texts), std::end(texts))),
	alphabet_size(alphabet_range.second - alphabet_range.first + 1),
	min_lookup_table_children(min_lookup_table_children),
	min_binary_search(min_binary_search),
	num_texts(std::size(texts)), data(1, {false, false, 1, 0, {}, {}})
{
	construct(std::begin(texts), std::end(texts), two_pass, lut_mode);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename input_stream>
map_fast<text, item, integer>::map_fast(input_stream& is, integer min_binary_search):
	alphabet_range({0, 0}),  alphabet_size(1),
	min_lookup_table_children(static_cast<integer>((alphabet_range.second - alphabet_range.first) * 0.5)),
	min_binary_search(min_binary_search)
{
	num_texts = load(is);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
map_fast<text, item, integer>::map_fast(const std::string path, integer min_binary_search):
	alphabet_range({0, 0}),  alphabet_size(1),
	min_lookup_table_children(static_cast<integer>((alphabet_range.second - alphabet_range.first) * 0.5)),
	min_binary_search(min_binary_search)
{
	std::ifstream ifs(path);
	num_texts = load(ifs);
}


// public functions

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_fast<text, item, integer>::size_type map_fast<text, item, integer>::size() const
{
	return num_texts;
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_fast<text, item, integer>::size_type map_fast<text, item, integer>::node_size() const
{
	return sizeof(node);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_fast<text, item, integer>::size_type map_fast<text, item, integer>::trie_size() const
{
	return data.size();
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_fast<text, item, integer>::size_type map_fast<text, item, integer>::total_space() const
{
	return sizeof(node) * data.size() + sizeof(symbol) * labels.size();
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
bool map_fast<text, item, integer>::exists(const text& pattern) const
{
	return find(pattern).match();
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_fast<text, item, integer>::node_type
map_fast<text, item, integer>::find(const text& pattern) const
{
	integer current = 0, depth = 0;
	for(integer i = 0; i < pattern.size();){
		if(data[current].leaf)
			return {*this, container_size(data) - 1, 0};
		else if(pattern[i] < alphabet_range.first || alphabet_range.second < pattern[i])
			return {*this, container_size(data) - 1, 0};

		// find child
		current = data[current].next;
		integer end = data[current].next;
		if(end - current == alphabet_size){
			current += pattern[i] - alphabet_range.first;
			if(data[current].label != pattern[i++])
				return {*this, container_size(data) - 1, 0};
		}
		else{
			for(integer w = end - current, m; w > min_binary_search; w = m){
				m = w >> 1;
				current += data[current + m].label < pattern[i] ? w - m : 0;
			}
			for(; current < end && data[current].label < pattern[i]; ++current);
			if(!(current < end && data[current].label == pattern[i++]))
				return {*this, container_size(data) - 1, 0};
		}

		// check compressed labels
		integer jstart = data[current].ref, jend = data[current + 1].ref;
		for(depth = 0; jstart + depth < jend && i < pattern.size(); ++depth, ++i)
			if(labels[jstart + depth] != pattern[i])
				return {*this, container_size(data) - 1, 0};
	}

	return {*this, current, depth};
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_fast<text, item, integer>::common_searcher
map_fast<text, item, integer>::searcher() const
{
	return common_searcher(*this);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
bool map_fast<text, item, integer>::update(const node_type& n, const item& value)
{
	if(!n.match())
		return false;

	data[n.id].value = value;
	return true;
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
bool map_fast<text, item, integer>::update(const text& key, const item& value)
{
	return update(find(key), value);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename trie_traits<text, item, integer>::value_ref_type map_fast<text, item, integer>::operator[](const text& key)
{
	auto n = find(key);
	auto id = n.match() ? n.id : data.size() - 1;
	return selector::value_ref(data[id].value, id);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_fast<text, item, integer>::node_type map_fast<text, item, integer>::root() const
{
	return {*this, static_cast<integer>(0), 0};
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
const std::vector<typename map_fast<text, item, integer>::node>&
map_fast<text, item, integer>::raw_data() const
{
	return data;
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
const std::vector<typename map_fast<text, item, integer>::symbol>&
map_fast<text, item, integer>::raw_labels() const
{
	return labels;
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename output_stream>
void map_fast<text, item, integer>::save(output_stream& os) const
{
	file_header header = {
		{constants::signature[0], constants::signature[1], constants::signature[2], constants::signature[3]},
		sizeof(file_header),
		constants::current_major_version,
		constants::current_minor_version,

		traits::container_type(),
		constants::index_type_fast,
		constants::text_charset<text>(),
		constants::text_encoding<text>(),

		constants::integer_type<integer>(),
		sizeof(node),
		constants::value_type<item>(),
		sizeof(item),

		data.size(),
		labels.size(),
	};
	os.write(reinterpret_cast<const char*>(&header), static_cast<std::streamsize>(sizeof(sftrie::file_header)));

	os.write(reinterpret_cast<const char*>(&alphabet_range.first), static_cast<std::streamsize>(sizeof(symbol)));
	os.write(reinterpret_cast<const char*>(&alphabet_range.second), static_cast<std::streamsize>(sizeof(symbol)));
	os.write(reinterpret_cast<const char*>(&alphabet_size), static_cast<std::streamsize>(sizeof(integer)));
	os.write(reinterpret_cast<const char*>(&min_lookup_table_children), static_cast<std::streamsize>(sizeof(integer)));

	os.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(sizeof(node) * data.size()));

	os.write(reinterpret_cast<const char*>(labels.data()), static_cast<std::streamsize>(sizeof(symbol) * labels.size()));
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
void map_fast<text, item, integer>::save(const std::string path) const
{
	std::ofstream ofs(path);
	save(ofs);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename input_stream>
integer map_fast<text, item, integer>::load(input_stream& is)
{
	file_header header;
	is.read(reinterpret_cast<char*>(&header), static_cast<std::streamsize>(sizeof(sftrie::file_header)));

	if(!std::equal(std::begin(header.signature), std::end(header.signature), std::begin(constants::signature), std::end(constants::signature)))
		throw std::runtime_error("invalid signature");
	if(header.major_version != constants::current_major_version)
		throw std::runtime_error("invalid major version");
	if(header.minor_version != constants::current_minor_version)
		throw std::runtime_error("invalid minor version");

	if(header.container_type != traits::container_type())
		throw std::runtime_error("invalid container type");
	if(header.index_type != constants::index_type_fast)
		throw std::runtime_error("invalid index type");
	if(header.text_charset != constants::text_charset<text>())
		throw std::runtime_error("invalid text charset");
	if(header.text_encoding != constants::text_encoding<text>())
		throw std::runtime_error("invalid text encoding");

	if(header.integer_type!= constants::integer_type<integer>())
		throw std::runtime_error("invalid integer type");
	if(header.node_size != sizeof(node))
		throw std::runtime_error("invalid node size");
	if(header.value_type != constants::value_type<item>())
		throw std::runtime_error("invalid value type");
	if(header.value_size != sizeof(item))
		throw std::runtime_error("invalid value size");

	reset(header.node_count, header.label_count);

	is.read(reinterpret_cast<char*>(&alphabet_range.first), static_cast<std::streamsize>(sizeof(symbol)));
	is.read(reinterpret_cast<char*>(&alphabet_range.second), static_cast<std::streamsize>(sizeof(symbol)));
	is.read(reinterpret_cast<char*>(&alphabet_size), static_cast<std::streamsize>(sizeof(integer)));
	is.read(reinterpret_cast<char*>(&min_lookup_table_children), static_cast<std::streamsize>(sizeof(integer)));

	data.resize(header.node_count);
	is.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(sizeof(node) * header.node_count));

	labels.resize(header.label_count);
	is.read(reinterpret_cast<char*>(labels.data()), static_cast<std::streamsize>(sizeof(symbol) * header.label_count));

	return (num_texts = std::count_if(data.begin(), data.end(), [](const auto& n){
		return n.match;
	}));
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
integer map_fast<text, item, integer>::load(const std::string path)
{
	std::ifstream ifs(path);
	return load(ifs);
}


// protected functions

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename container>
typename map_fast<text, item, integer>::integer_type
map_fast<text, item, integer>::container_size(const container& c)
{
	return static_cast<integer>(c.size());
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
void map_fast<text, item, integer>::reset(integer node_count, integer label_count)
{
	data.clear();
	if(node_count != 0)
		data.reserve(node_count);
	data.push_back({false, false, 1, 0, {}, {}});

	labels.clear();
	if(label_count != 0)
		labels.reserve(label_count);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename iterator>
std::pair<integer, integer> map_fast<text, item, integer>::estimate(iterator begin, iterator end,
	lookup_table_mode lut_mode) const
{
	auto [node_count, label_count] = estimate(begin, end, 0, lut_mode);
	return {node_count + 1, label_count};
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename iterator>
std::pair<integer, integer> map_fast<text, item, integer>::estimate(iterator begin, iterator end,
	integer depth, lookup_table_mode lut_mode) const
{
	integer node_count = 1, label_count = 0;

	if(begin < end && depth == container_size(selector::key(*begin)))
		++begin;

	integer num_children = 0;
	for(iterator i = begin; i < end; ++num_children)
		for(symbol c = selector::key(*i)[depth]; i < end && selector::key(*i)[depth] == c; ++i);
	if(num_children >= min_lookup_table_children && (
		(lut_mode == lookup_table_mode::root_only && depth == 0) ||
		(lut_mode == lookup_table_mode::adaptive)
	))
		node_count += static_cast<integer>(alphabet_size) - num_children;

	for(iterator i = begin; i < end; begin = i){
		for(symbol c = selector::key(*i)[depth]; i < end &&
			selector::key(*i)[depth] == c; ++i);

		integer d = depth + 1;
		while(d < container_size(selector::key(*begin)) &&
				selector::key(*begin)[d] == selector::key(*(i - 1))[d]){
			++d;
			++label_count;
		}

		auto [n, l] = estimate(begin, i, d, lut_mode);
		node_count += n;
		label_count += l;
	}

	return {node_count, label_count};
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename iterator>
integer map_fast<text, item, integer>::construct(iterator begin, iterator end,
	bool two_pass, lookup_table_mode lut_mode)
{
	alphabet_range = actual_alphabet_range<text, item, integer>(begin, end);
	alphabet_size = alphabet_range.second - alphabet_range.first + 1;

	if(two_pass){
		auto [node_count, label_count] = estimate(begin, end, lut_mode);
		reset(node_count, label_count);
	}
	else{
		reset();
	}

	if(begin < end){
		if(selector::key(*begin).size() == 0)
			data[0].value = selector::value(*begin);
		construct(begin, end, 0, 0, lut_mode);
	}
	data.push_back({false, false, container_size(data), container_size(labels), {}, {}});

	if(!two_pass){
		data.shrink_to_fit();
		labels.shrink_to_fit();
	}

	return (num_texts = end - begin);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<random_access_container container>
integer map_fast<text, item, integer>::construct(const container& texts,
	bool two_pass, lookup_table_mode lut_mode)
{
	return construct(std::begin(texts), std::end(texts), two_pass, lut_mode);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename iterator>
void map_fast<text, item, integer>::construct(iterator begin, iterator end, integer depth, integer current,
	lookup_table_mode lut_mode)
{
	// set flags
	if((data[current].match = (depth == container_size((selector::key(*begin))))))
		if((data[current].leaf = (++begin == end)))
			return;

	// count children
	std::vector<iterator> head{begin};
	for(iterator i = begin; i < end; head.push_back(i))
		for(symbol c = selector::key(*i)[depth]; i < end && selector::key(*i)[depth] == c; ++i);

	if(container_size(head) - 1 >= min_lookup_table_children && (
		(lut_mode == lookup_table_mode::root_only && depth == 0) ||
		(lut_mode == lookup_table_mode::adaptive)
	)){
		// reserve children
		integer i = 0;
		for(symbol c = alphabet_range.first; true; ++c){
			if(i < head.size() - 1 && c == selector::key(*head[i])[depth])
				data.push_back({false, false, 0, 0, c, selector::value(*head[i++])});
			else
				data.push_back({false, false, 0, 0, static_cast<symbol>(c - 1), {}});
			if(c == alphabet_range.second)
				break;
		}

		// compress single paths
		std::vector<integer> depths;
		i = 0;
		for(symbol c = alphabet_range.first; true; ++c){
			data[data[current].next + (c - alphabet_range.first)].ref = container_size(labels);
			if(data[data[current].next + (c - alphabet_range.first)].label == c){
				integer d = depth + 1;
				while(d < container_size(selector::key(*head[i])) && selector::key(*head[i])[d] == selector::key(*(head[i + 1] - 1))[d])
					labels.push_back(selector::key(*head[i])[d++]);
				depths.push_back(d);
				++i;
			}
			if(c == alphabet_range.second)
				break;
		}

		// recursively construct subtries
		i = 0;
		for(symbol c = alphabet_range.first; true; ++c){
			data[data[current].next + (c - alphabet_range.first)].next = container_size(data);
			if(data[data[current].next + (c - alphabet_range.first)].label == c){
				construct(head[i], head[i + 1], depths[i], data[current].next + (c - alphabet_range.first), lut_mode);
				++i;
			}
			if(c == alphabet_range.second)
				break;
		}
	}
	else{
		// reserve children
		for(integer i = 0; i < container_size(head) - 1; ++i)
			data.push_back({false, false, 0, 0, selector::key(*head[i])[depth], selector::value(*head[i])});

		// compress single paths
		std::vector<integer> depths;
		for(integer i = 0; i < container_size(head) - 1; ++i){
			data[data[current].next + i].ref = container_size(labels);
			integer d = depth + 1;
			while(d < container_size(selector::key(*head[i])) && selector::key(*head[i])[d] == selector::key(*(head[i + 1] - 1))[d])
				labels.push_back(selector::key(*head[i])[d++]);
			depths.push_back(d);
		}

		// recursively construct subtries
		for(integer i = 0; i < container_size(head) - 1; ++i){
			data[data[current].next + i].next = container_size(data);
			construct(head[i], head[i + 1], depths[i], data[current].next + i, lut_mode);
		}
	}
}


// subclasses

template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_fast<text, item, integer>::virtual_node
{
	const map_fast<text, item, integer>& trie;
	integer id;
	integer depth;

	virtual_node(const map_fast<text, item, integer>& trie, integer id, integer depth):
		trie(trie), id(id), depth(depth)
	{}

	virtual_node(const map_fast<text, item, integer>& trie, integer id):
		trie(trie), id(id), depth(trie.data[id + 1].ref - trie.data[id].ref)
	{}

	bool operator==(const virtual_node& n) const
	{
		return id == n.id && depth == n.depth;
	}

	bool operator!=(const virtual_node& n) const
	{
		return id != n.id || depth != n.depth;
	}

	integer node_id() const
	{
		return id;
	}

	bool valid() const
	{
		return id < trie.data.size() - 1;
	}

	bool physical() const
	{
		return valid() && depth == trie.data[id + 1].ref - trie.data[id].ref;
	}

	symbol label() const
	{
		if(depth == 0)
			return trie.data[id].label;
		else
			return trie.labels[trie.data[id].ref + depth - 1];
	}

	bool match() const
	{
		return trie.data[id].match && trie.data[id].ref + depth == trie.data[id + 1].ref;
	}

	bool leaf() const
	{
		return trie.data[id].leaf && trie.data[id].ref + depth == trie.data[id + 1].ref;
	}

	typename traits::value_const_ref_type value() const
	{
		return selector::value_const_ref(trie.data[id].value, id);
	}

	child_iterator children() const
	{
		if(trie.data[id].ref + depth < trie.data[id + 1].ref)
			return child_iterator(trie, id, depth + 1, 0);
		else if(!trie.data[id].leaf)
			return child_iterator(trie, id);
		else
			return child_iterator(trie, trie.data.size() - 1, 0, trie.data.size() - 1);
	}
};

template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_fast<text, item, integer>::child_iterator
{
	virtual_node current;
	integer last;
	bool lut;

	child_iterator(const map_fast<text, item, integer>& trie):
		current(trie, 0, 0), last(1), lut(false)
	{}

	child_iterator(const map_fast<text, item, integer>& trie, const integer parent):
		current(trie, trie.data[parent].next, 0),
		last(trie.data[parent].next < trie.data.size() ? trie.data[trie.data[parent].next].next : trie.data.size()),
		lut(last - current.id == trie.alphabet_size)
	{
		if(lut){
			integer start = current.id;
			while(current.id < last && trie.data[last - 1].label !=
					static_cast<symbol>(trie.alphabet_range.first + last - 1 - start))
				--last;
			while(current.id < last && trie.data[current.id].label !=
					static_cast<symbol>(trie.alphabet_range.first + (current.id - start)))
				++current.id;
		}
	}

	child_iterator(const map_fast<text, item, integer>& trie, integer id, integer depth, integer last):
		current(trie, id, depth), last(last), lut(last - current.id == trie.alphabet_size)
	{
		if(depth > 0 && lut){
			integer target_diff = current.id - static_cast<integer>(current.label() - current.trie.alphabet_range.first);
			while(current.id < last - 1 &&
					current.id - static_cast<integer>(current.label() - current.trie.alphabet_range.first) != target_diff)
				++current.id;
		}
	}

	child_iterator begin() const
	{
		return *this;
	}

	child_iterator end() const
	{
		return child_iterator(current.trie, last, 0, last);
	}

	bool incrementable() const
	{
		return current.depth == 0 && last > 0 && current.id < last - 1;
	}

	bool operator==(const child_iterator& i) const
	{
		return
			(current.id == i.current.id && current.depth == i.current.depth) ||
			(current.id == last && i.current.id == i.last);
	}

	bool operator!=(const child_iterator& i) const
	{
		return
			(current.id != i.current.id || current.depth != i.current.depth) &&
			(current.id != last || i.current.id != i.last);
	}

	void operator++()
	{
		if(last > 0){
			integer target_diff = current.id - static_cast<integer>(current.label() - current.trie.alphabet_range.first);
			++current.id;
			if(lut){
				while(current.id < last - 1 &&
						current.id - static_cast<integer>(current.label() - current.trie.alphabet_range.first) != target_diff)
					++current.id;
			}
		}
		else{
			current.id = last;
		}
	}

	virtual_node operator*() const
	{
		return current;
	}
};

template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_fast<text, item, integer>::common_searcher
{
	const map_fast<text, item, integer>& trie;
	std::vector<integer> path;
	text result;

	common_searcher(const map_fast<text, item, integer>& trie): trie(trie){}

	bool exists(const text& pattern) const
	{
		return trie.exists(pattern);
	}

	typename map_fast<text, item, integer>::node_type find(const text& pattern) const
	{
		return trie.find(pattern);
	}

	subtree_iterator predict(const text& pattern)
	{
		auto n = trie.find(pattern);
		if(n.id < trie.data.size() - 1){
			result.clear();
			path.clear();
			path.push_back(n.id);
			std::copy(std::begin(pattern), std::end(pattern), std::back_inserter(result));
		}
		return subtree_iterator(*this, n);
	}

	prefix_iterator prefix(const text& pattern)
	{
		result.clear();
		return prefix_iterator(*this, pattern, 0, 0);
	}
};

template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_fast<text, item, integer>::subtree_iterator
{
	common_searcher& searcher;
	integer current;

	subtree_iterator(common_searcher& searcher, const virtual_node& n):
		searcher(searcher), current(n.id)
	{
		if(n.id < searcher.trie.data.size() - 1){
			if(searcher.trie.data[n.id + 1].ref - searcher.trie.data[n.id].ref > n.depth)
				std::copy(searcher.trie.labels.begin() + searcher.trie.data[n.id].ref + n.depth,
					searcher.trie.labels.begin() + searcher.trie.data[n.id + 1].ref,
					std::back_inserter(searcher.result));
			if(!searcher.trie.data[n.id].match){
				if(n.id != 0 || searcher.trie.data[n.id].next < searcher.trie.data.size() - 1)
					++*this;
				else
					current = searcher.trie.data.size() - 1;
			}
		}
	}

	const text& key() const
	{
		return searcher.result;
	}

	typename traits::value_const_ref_type value() const
	{
		return selector::value_const_ref(searcher.trie.data[current].value, current);
	}

	map_fast<text, item, integer>::virtual_node node() const
	{
		return {searcher.trie, current, searcher.trie.data[current + 1].ref - searcher.trie.data[current].ref};
	}

	subtree_iterator& begin()
	{
		return *this;
	}

	subtree_iterator end() const
	{
		return subtree_iterator(searcher, {searcher.trie, container_size(searcher.trie.data) - 1, 0});
	}

	bool operator!=(const subtree_iterator& i) const
	{
		return this->current != i.current;
	}

	const subtree_iterator& operator*() const
	{
		return *this;
	}

	subtree_iterator& operator++()
	{
		do{
			if(!searcher.trie.data[searcher.path.back()].leaf){
				// first child
				integer start = searcher.trie.data[searcher.path.back()].next;
				integer end = searcher.trie.data[start].next;
				integer next = start;
				if(end - start == searcher.trie.alphabet_size){
					while(next < end && searcher.trie.data[next].label !=
							static_cast<symbol>(searcher.trie.alphabet_range.first + (next - start)))
						++next;
				}

				searcher.path.push_back(next);
				searcher.result.push_back(searcher.trie.data[next].label);
				std::copy(searcher.trie.labels.begin() + searcher.trie.data[next].ref,
					searcher.trie.labels.begin() + searcher.trie.data[next + 1].ref,
					std::back_inserter(searcher.result));
			}
			else{
				while(searcher.path.size() > 1){
					while(searcher.path.size() > 1 && searcher.path.back() + 1 ==
							searcher.trie.data[searcher.trie.data[searcher.path[searcher.path.size() - 2]].next].next){
						// parent
						searcher.result.resize(searcher.result.size() -
							(1 + searcher.trie.data[searcher.path.back() + 1].ref - searcher.trie.data[searcher.path.back()].ref));
						searcher.path.pop_back();
					}
					if(searcher.path.size() > 1){
						integer prev = searcher.path.back();
						searcher.result.resize(searcher.result.size() -
							(1 + searcher.trie.data[prev + 1].ref - searcher.trie.data[prev].ref));
						searcher.path.pop_back();

						// next sibling
						integer start = searcher.trie.data[searcher.path.back()].next;
						integer end = searcher.trie.data[start].next;
						integer next = prev + 1;
						if(end - start == searcher.trie.alphabet_size)
							while(next < end && searcher.trie.data[next].label !=
									static_cast<symbol>(searcher.trie.alphabet_range.first + (next - start)))
								++next;

						if(next < end){
							searcher.path.push_back(next);
							searcher.result.push_back(searcher.trie.data[next].label);
							std::copy(searcher.trie.labels.begin() + searcher.trie.data[next].ref,
								searcher.trie.labels.begin() + searcher.trie.data[next + 1].ref,
								std::back_inserter(searcher.result));
							break;
						}
					}
					else{
						searcher.path.clear();
					}
				}
				if(searcher.path.size() <= 1)
					searcher.path.clear();
			}
		}while(!searcher.path.empty() && !searcher.trie.data[searcher.path.back()].match);
		current = !searcher.path.empty() ? searcher.path.back() : searcher.trie.data.size() - 1;
		return *this;
	}
};

template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_fast<text, item, integer>::prefix_iterator
{
	common_searcher& searcher;
	const text& pattern;
	integer current;
	integer depth;

	prefix_iterator(common_searcher& searcher, const text& pattern, integer current, integer depth):
		searcher(searcher), pattern(pattern), current(current), depth(depth)
	{
		if(current == 0 && !searcher.trie.data[current].match){
			if(pattern.empty())
				this->current = searcher.trie.data.size() - 1;
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
		return prefix_iterator(searcher, pattern, searcher.trie.data.size() - 1, pattern.size());
	}

	bool operator!=(const prefix_iterator& i) const
	{
		return this->current != i.current;
	}

	prefix_iterator& operator*()
	{
		return *this;
	}

	prefix_iterator& operator++()
	{
		while(!searcher.trie.data[current].leaf && depth < pattern.size()){
			if(pattern[depth] < searcher.trie.alphabet_range.first || searcher.trie.alphabet_range.second < pattern[depth])
				break;

			// find child
			current = searcher.trie.data[current].next;
			integer end = searcher.trie.data[current].next;
			if(end - current == static_cast<integer>(searcher.trie.alphabet_size)){
				current += pattern[depth] - searcher.trie.alphabet_range.first;
			}
			else{
				for(integer w = end - current, m; w > searcher.trie.min_binary_search; w = m){
					m = w >> 1;
					current += searcher.trie.data[current + m].label < pattern[depth] ? w - m : 0;
				}
				for(; current < end && searcher.trie.data[current].label < pattern[depth]; ++current);
			}

			if(!(current < end && searcher.trie.data[current].label == pattern[depth]))
				break;
			searcher.result.push_back(pattern[depth++]);

			// check compressed labels
			integer j = searcher.trie.data[current].ref, jend = searcher.trie.data[current + 1].ref;
			if(jend > j){
				if(jend - j > pattern.size() - depth || !std::equal(searcher.trie.labels.begin() + j, searcher.trie.labels.begin() + jend, pattern.begin() + depth))
					break;
				std::copy(searcher.trie.labels.begin() + j, searcher.trie.labels.begin() + jend, std::back_inserter(searcher.result));
				depth += jend - j;
			}

			if(searcher.trie.data[current].match)
				return *this;
		}

		current = searcher.trie.data.size() - 1;
		return *this;
	}

	const text& key() const
	{
		return searcher.result;
	}

	typename traits::value_const_ref_type value() const
	{
		return selector::value_const_ref(searcher.trie.data[current].value, current);
	}

	map_fast<text, item, integer>::virtual_node node() const
	{
		return {searcher.trie, current};
	}
};

}

#endif
