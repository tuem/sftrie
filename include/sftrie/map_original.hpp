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

#ifndef SFTRIE_MAP_ORIGINAL
#define SFTRIE_MAP_ORIGINAL

#include <cstdint>
#include <concepts>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <type_traits>

#include "constants.hpp"
#include "file_header.hpp"
#include "util.hpp"
#include "lexicographically_comparable.hpp"
#include "default_constructible.hpp"
#include "random_access_container.hpp"

namespace sftrie{

template<lexicographically_comparable text, default_constructible item, std::integral integer>
class map_original
{
protected:
	using symbol = typename text::value_type;
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

	// constructors
protected:
	map_original(
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
public:
	template<std::random_access_iterator iterator>
	map_original(iterator begin, iterator end,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
	template<random_access_container container>
	map_original(const container& texts,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
	template<typename input_stream> map_original(input_stream& is,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));
	map_original(const std::string path,
		integer min_binary_search = static_cast<integer>(constants::default_min_binary_search<symbol>()));

	// information
	size_type size() const;
	size_type node_size() const;
	size_type trie_size() const;
	size_type total_space() const;

	// search operations
	bool exists(const text& pattern) const;
	node_type find(const text& pattern) const;
	common_searcher searcher() const;

	// tree operations
	node_type root() const;
	const std::vector<node>& raw_data() const;

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
	const integer min_binary_search;

	size_type num_texts;
	std::vector<node> data;

	template<typename container>
	static integer container_size(const container& c);

	template<typename iterator>
	integer estimate(iterator begin, iterator end);
	template<typename iterator>
	integer estimate(iterator begin, iterator end, integer depth);

	template<typename iterator>
	void construct(iterator begin, iterator end);
	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current);

	integer search(const text& pattern) const;
};


#pragma pack(1)
template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_original<text, item, integer>::node
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	symbol label;
	[[no_unique_address]] item value;
};
#pragma pack()


// constructors

template<lexicographically_comparable text, default_constructible item, std::integral integer>
map_original<text, item, integer>::map_original(integer min_binary_search):
	min_binary_search(min_binary_search), num_texts(0), data(1, {false, false, 1, {}, {}})
{}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<std::random_access_iterator iterator>
map_original<text, item, integer>::map_original(iterator begin, iterator end,
		integer min_binary_search):
	min_binary_search(min_binary_search),
	num_texts(end - begin), data(1, {false, false, 1, {}, {}})
{
	construct(begin, end);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<random_access_container container>
map_original<text, item, integer>::map_original(const container& texts, integer min_binary_search):
	min_binary_search(min_binary_search),
	num_texts(std::size(texts)), data(1, {false, false, 1, 0, {}, {}})
{
	construct(std::begin(texts), std::end(texts));
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename input_stream>
map_original<text, item, integer>::map_original(input_stream& is, integer min_binary_search):
	min_binary_search(min_binary_search)
{
	num_texts = load(is);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
map_original<text, item, integer>::map_original(const std::string path, integer min_binary_search):
	min_binary_search(min_binary_search)
{
	std::ifstream ifs(path);
	num_texts = load(ifs);
}


// public functions

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_original<text, item, integer>::size_type map_original<text, item, integer>::size() const
{
	return num_texts;
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_original<text, item, integer>::size_type map_original<text, item, integer>::node_size() const
{
	return sizeof(node);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_original<text, item, integer>::size_type map_original<text, item, integer>::trie_size() const
{
	return data.size();
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_original<text, item, integer>::size_type map_original<text, item, integer>::total_space() const
{
	return sizeof(node) * data.size();
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
bool map_original<text, item, integer>::exists(const text& pattern) const
{
	return data[search(pattern)].match;
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_original<text, item, integer>::node_type
map_original<text, item, integer>::find(const text& pattern) const
{
	return {*this, search(pattern)};
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_original<text, item, integer>::common_searcher
map_original<text, item, integer>::searcher() const
{
	return common_searcher(*this);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename map_original<text, item, integer>::node_type map_original<text, item, integer>::root() const
{
	return {*this, static_cast<integer>(0)};
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
const std::vector<typename map_original<text, item, integer>::node>&
map_original<text, item, integer>::raw_data() const
{
	return data;
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
bool map_original<text, item, integer>::update(const node_type& n, const item& value)
{
	data[n.id].value = value;
	return true;
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
bool map_original<text, item, integer>::update(const text& key, const item& value)
{
	return update(find(key), value);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
typename trie_traits<text, item, integer>::value_ref_type map_original<text, item, integer>::operator[](const text& key)
{
	auto id = search(key);
	if(!data[id].match)
		id = data.size() - 1;
	return selector::value_ref(data[id].value, id);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename output_stream>
void map_original<text, item, integer>::save(output_stream& os) const
{
	file_header header = {
		{constants::signature[0], constants::signature[1], constants::signature[2], constants::signature[3]},
		sizeof(file_header),
		constants::current_major_version,
		constants::current_minor_version,

		traits::container_type(),
		constants::index_type_original,
		constants::text_charset<text>(),
		constants::text_encoding<text>(),

		constants::integer_type<integer>(),
		sizeof(node),
		constants::value_type<item>(),
		sizeof(item),

		data.size(),
		0,
	};
	os.write(reinterpret_cast<const char*>(&header), static_cast<std::streamsize>(sizeof(sftrie::file_header)));

	os.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(sizeof(node) * data.size()));
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
void map_original<text, item, integer>::save(const std::string path) const
{
	std::ofstream ofs(path);
	save(ofs);
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename input_stream>
integer map_original<text, item, integer>::load(input_stream& is)
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
	if(header.index_type != constants::index_type_original)
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

	if(header.label_count != 0)
		throw std::runtime_error("invalid label count");

	data.resize(header.node_count);
	is.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(sizeof(node) * header.node_count));

	return std::count_if(data.begin(), data.end(), [](const auto& n){
		return n.match;
	});
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
integer map_original<text, item, integer>::load(const std::string path)
{
	std::ifstream ifs(path);
	return load(path);
}


// protected functions

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename container>
typename map_original<text, item, integer>::integer_type
map_original<text, item, integer>::container_size(const container& c)
{
	return static_cast<integer>(c.size());
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename iterator>
integer map_original<text, item, integer>::estimate(iterator begin, iterator end)
{
	integer count = 0;
	count += estimate(begin, end, 0);
	return count + 1; // sentinel
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename iterator>
integer map_original<text, item, integer>::estimate(iterator begin, iterator end, integer depth)
{
	integer count = 1;

	if(begin < end && depth == container_size(selector::key(*begin)))
		++begin;

	if(begin < end){
		for(iterator i = begin; i < end; begin = i){
			for(symbol c = selector::key(*i)[depth];
				i < end && selector::key(*i)[depth] == c; ++i);
			count += estimate(begin, i, depth + 1);
		}
	}

	return count;
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename iterator>
void map_original<text, item, integer>::construct(iterator begin, iterator end)
{
	data.reserve(estimate(begin, end));
	if(begin < end){
		if(selector::key(*begin).size() == 0)
			data[0].value = selector::value(*begin);
		construct(begin, end, 0, 0);
	}
	data.push_back({false, false, container_size(data), {}, {}});
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
template<typename iterator>
void map_original<text, item, integer>::construct(iterator begin, iterator end, integer depth, integer current)
{
	// set flags
	if((data[current].match = (depth == container_size(selector::key(*begin)))))
		if((data[current].leaf = (++begin == end)))
			return;

	// reserve children
	std::vector<iterator> head{begin};
	for(iterator i = begin; i < end; head.push_back(i)){
		data.push_back({false, false, 0, selector::key(*i)[depth], selector::value(*i)});
		for(symbol c = selector::key(*i)[depth]; i < end && selector::key(*i)[depth] == c; ++i);
	}

	// recursively construct subtries
	for(integer i = 0; i < container_size(head) - 1; ++i){
		integer child = data[current].next + i;
		data[child].next = container_size(data);
		construct(head[i], head[i + 1], depth + 1, child);
	}
}

template<lexicographically_comparable text, default_constructible item, std::integral integer>
integer map_original<text, item, integer>::search(const text& pattern) const
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


// subclasses

template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_original<text, item, integer>::virtual_node
{
	const map_original<text, item, integer>& trie;
	integer id;

	virtual_node(const map_original<text, item, integer>& trie, integer id):
		trie(trie), id(id)
	{}

	bool operator==(const virtual_node& n) const
	{
		return id == n.id;
	}

	bool operator!=(const virtual_node& n) const
	{
		return id != n.id;
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
		return valid();
	}

	symbol label() const
	{
		return trie.data[id].label;
	}

	bool match() const
	{
		return trie.data[id].match;
	}

	bool leaf() const
	{
		return trie.data[id].leaf;
	}

	typename traits::value_const_ref_type value() const
	{
		return selector::value_const_ref(trie.data[id].value, id);
	}

	child_iterator children() const
	{
		if(!trie.data[id].leaf)
			return child_iterator(trie, trie.data[id].next, trie.data[trie.data[id].next].next);
		else
			return child_iterator(trie, trie.data.size() - 1, trie.data.size() - 1);
	}
};

template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_original<text, item, integer>::child_iterator
{
	virtual_node current;
	const integer last;

	child_iterator(const map_original<text, item, integer>& trie):
		current(trie, 0), last(1)
	{}

	child_iterator(const map_original<text, item, integer>& trie, integer parent):
		current(trie, trie.data[parent].next),
		last(trie.data[parent].next < trie.data.size() ? trie.data[trie.data[parent].next].next : trie.data.size())
	{}

	child_iterator(const map_original<text, item, integer>& trie, integer id, integer last):
		current(trie, id), last(last)
	{}

	child_iterator begin() const
	{
		return *this;
	}

	child_iterator end() const
	{
		return child_iterator(current.trie, last, last);
	}

	bool incrementable() const
	{
		return current.id < last - 1;
	}

	bool operator==(const child_iterator& i) const
	{
		return current.id == i.current.id;
	}

	bool operator!=(const child_iterator& i) const
	{
		return current.id != i.current.id;
	}

	void operator++()
	{
		++current.id;
	}

	virtual_node operator*() const
	{
		return current;
	}
};

template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_original<text, item, integer>::common_searcher
{
	const map_original<text, item, integer>& trie;
	std::vector<integer> path;
	text result;

	common_searcher(const map_original<text, item, integer>& trie): trie(trie){}

	bool exists(const text& pattern) const
	{
		return trie.exists(pattern);
	}

	virtual_node find(const text& pattern) const
	{
		return virtual_node(trie, trie.search(pattern));
	}

	subtree_iterator predict(const text& pattern)
	{
		auto n = trie.search(pattern);
		if(n < trie.data.size() - 1){
			path.clear();
			result.clear();
			path.push_back(n);
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
struct map_original<text, item, integer>::subtree_iterator
{
	common_searcher& searcher;
	integer current;

	subtree_iterator(common_searcher& searcher, integer n):
		searcher(searcher), current(n)
	{
		if(n < searcher.trie.data.size() - 1 && !searcher.trie.data[n].match){
			if(searcher.trie.data[n].next < searcher.trie.data.size() - 1)
				++*this;
			else
				current = searcher.trie.data.size() - 1;
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

	map_original<text, item, integer>::virtual_node node() const
	{
		return {searcher.trie, current};
	}

	subtree_iterator& begin()
	{
		return *this;
	}

	subtree_iterator end() const
	{
		return subtree_iterator(searcher, searcher.trie.data.size() - 1);
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
				integer child = searcher.trie.data[searcher.path.back()].next;
				searcher.path.push_back(child);
				searcher.result.push_back(searcher.trie.data[child].label);
			}
			else{
				while(searcher.path.size() > 1 && searcher.path.back() + 1 ==
						searcher.trie.data[searcher.trie.data[searcher.path[searcher.path.size() - 2]].next].next){
					// parent
					searcher.path.pop_back();
					searcher.result.pop_back();
				}
				if(searcher.path.size() > 1){
					// next sibling
					searcher.result.back() = searcher.trie.data[++searcher.path.back()].label;
				}
				else{
					// done
					searcher.path.clear();
				}
			}
		}while(!searcher.path.empty() && !searcher.trie.data[searcher.path.back()].match);
		current = !searcher.path.empty() ? searcher.path.back() : searcher.trie.data.size() - 1;
		return *this;
	}
};

template<lexicographically_comparable text, default_constructible item, std::integral integer>
struct map_original<text, item, integer>::prefix_iterator
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

	const prefix_iterator& operator*() const
	{
		return *this;
	}

	prefix_iterator& operator++()
	{
		for(; !searcher.trie.data[current].leaf && depth < pattern.size(); ){
			current = searcher.trie.data[current].next;
			integer end = searcher.trie.data[current].next;
			for(integer w = end - current, m; w > searcher.trie.min_binary_search; w = m){
				m = w >> 1;
				current += searcher.trie.data[current + m].label < pattern[depth] ? w - m : 0;
			}
			for(; current < end && searcher.trie.data[current].label < pattern[depth]; ++current);
			if(!(current < end && searcher.trie.data[current].label == pattern[depth]))
				break;
			searcher.result.push_back(pattern[depth++]);
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

	map_original<text, item, integer>::virtual_node node() const
	{
		return {searcher.trie, current};
	}
};

}

#endif
