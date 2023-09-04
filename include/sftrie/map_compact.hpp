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

#ifndef SFTRIE_MAP_COMPACT
#define SFTRIE_MAP_COMPACT

#include <cstddef>
#include <vector>
#include <fstream>

#include "util.hpp"

#include "constants.hpp"
#include "file_header.hpp"

namespace sftrie{

template<typename text, typename item, typename integer>
class map_compact
{
private:
	using symbol = typename text::value_type;

public:
	struct node;
	struct virtual_node;
	struct child_iterator;
	struct common_searcher;
	struct subtree_iterator;
	struct prefix_iterator;

	using symbol_type = symbol;
	using size_type = std::size_t;
	using node_type = virtual_node;
	using result_type = std::pair<bool, item&>;

public:
	template<typename random_access_iterator>
	map_compact(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search = constants::default_min_binary_search<integer>);
	template<typename random_access_container>
	map_compact(const random_access_container& texts,
		integer min_binary_search = constants::default_min_binary_search<integer>);
	template<typename input_stream> map_compact(input_stream& is,
		integer min_binary_search = constants::default_min_binary_search<integer>);
	map_compact(std::string path, integer min_binary_search = constants::default_min_binary_search<integer>);

	// information
	size_type size() const;
	size_type node_size() const;
	size_type trie_size() const;
	size_type space() const;

	// search
	node_type search(const text& pattern) const;
	result_type find(const text& pattern) const;
	item& operator[](const text& pattern);
	common_searcher searcher();

	// tree operations
	node_type root();
	const std::vector<node>& raw_data() const;
	bool update(integer id, const item& value);
	bool update(const text& key, const item& value);

	// file I/O
	template<typename output_stream> void save(output_stream& os) const;
	void save(std::string os) const;
	template<typename input_stream> integer load(input_stream& is);
	integer load(std::string path);

private:
	const integer min_binary_search;

	size_type num_texts;
	std::vector<node> data;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current);
};


#pragma pack(1)
template<typename text, typename item, typename integer>
struct map_compact<text, item, integer>::node
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	integer ref: integer;
	symbol label;
	item value;
};
#pragma pack()


// constructors

template<typename text, typename item, typename integer>
template<typename random_access_iterator>
map_compact<text, item, integer>::map_compact(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search):
	min_binary_search(min_binary_search),
	num_texts(end - begin), data(1, {false, false, 1, {}, {}})
{
	construct(begin, end, 0, 0);
	data.push_back({false, false, container_size<integer>(data), container_size<integer>(labels), {}, {}});
	data.shrink_to_fit();
}

template<typename text, typename item, typename integer>
template<typename random_access_container>
map_compact<text, item, integer>::map_compact(const random_access_container& texts, integer min_binary_search):
	min_binary_search(min_binary_search), num_texts(std::size(texts))
{
	construct(std::begin(texts), std::end(texts), 0, 0);
	data.push_back({false, false, container_size<integer>(data), container_size<integer>(labels), {}, {}});
	data.shrink_to_fit();
}

template<typename text, typename item, typename integer>
template<typename input_stream>
map_compact<text, item, integer>::map_compact(input_stream& is, integer min_binary_search):
	min_binary_search(min_binary_search)
{
	num_texts = load(is);
}

template<typename text, typename item, typename integer>
map_compact<text, item, integer>::map_compact(std::string path, integer min_binary_search):
	min_binary_search(min_binary_search)
{
	std::ifstream ifs(path);
	num_texts = load(ifs);
}


// public functions

template<typename text, typename item, typename integer>
typename map_compact<text, item, integer>::size_type map_compact<text, item, integer>::size() const
{
	return num_texts;
}

template<typename text, typename item, typename integer>
typename map_compact<text, item, integer>::size_type map_compact<text, item, integer>::node_size() const
{
	return sizeof(node);
}

template<typename text, typename item, typename integer>
typename map_compact<text, item, integer>::size_type map_compact<text, item, integer>::trie_size() const
{
	return data.size();
}

template<typename text, typename item, typename integer>
typename map_compact<text, item, integer>::size_type map_compact<text, item, integer>::space() const
{
	return sizeof(node) * data.size();
}

template<typename text, typename item, typename integer>
typename map_compact<text, item, integer>::node_type
map_compact<text, item, integer>::search(const text& pattern) const
{
	integer current = 0;
	for(integer i = 0; i < pattern.size();){
		if(data[current].leaf)
			return {*this, data.size() - 1, 0};

		// find child
		current = data[current].next;
		integer end = data[current].next;
		for(integer w = end - current, m; w > min_binary_search; w = m){
			m = w >> 1;
			current += data[current + m].label < pattern[i] ? w - m : 0;
		}
		for(; current < end && data[current].label < pattern[i]; ++current);
		if(!(current < end && data[current].label == pattern[i]))
			return {*this, data.size() - 1, 0};

		// check compressed labels
		integer jstart = data[current].ref, jend = data[current + 1].ref, j = jstart;
		for(; i < pattern.size() && j < jend; ++i, ++j)
			if(labels[j] != pattern[i])
				return {*this, data.size() - 1, 0};

		if(j < jend)
			return {*this, current, j - jstart};
	}
	return {*this, current, 0};
}


template<typename text, typename item, typename integer>
typename map_compact<text, item, integer>::result_type
map_compact<text, item, integer>::find(const text& pattern) const
{
	integer current = 0;
	for(integer i = 0; i < pattern.size();){
		if(data[current].leaf)
			return {false, data.back().value};

		// find child
		current = data[current].next;
		integer end = data[current].next;
		for(integer w = end - current, m; w > min_binary_search; w = m){
			m = w >> 1;
			current += data[current + m].label < pattern[i] ? w - m : 0;
		}
		for(; current < end && data[current].label < pattern[i]; ++current);
		if(!(current < end && data[current].label == pattern[i]))
			return {false, data.back().value};

		// check compressed labels
		integer j = data[current].ref, jend = data[current + 1].ref;
		if(jend - j > pattern.size() - ++i)
			return {false, data.back().value};
		for(; j < jend; ++i, ++j)
			if(labels[j] != pattern[i])
				return {false, data.back().value};
	}
	return {data[current].match, data[current].value};
}

template<typename text, typename item, typename integer>
item& map_compact<text, item, integer>::operator[](const text& pattern)
{
	auto n = search(pattern);
	return data[n.id].value;
}

template<typename text, typename item, typename integer>
typename map_compact<text, item, integer>::common_searcher
map_compact<text, item, integer>::searcher()
{
	return common_searcher(*this);
}

template<typename text, typename item, typename integer>
bool map_compact<text, item, integer>::update(integer id, const item& value)
{
	data.at(id).value = value;
	return true;
}

template<typename text, typename item, typename integer>
bool map_compact<text, item, integer>::update(const text& key, const item& value)
{
	if(key.empty())
		return update(0, value);

	auto n = search(key);
	if(n.depth > 0 || !data[n.id].match)
		return false;

	return update(n.id, value);
}

template<typename text, typename item, typename integer>
typename map_compact<text, item, integer>::node_type map_compact<text, item, integer>::root()
{
	return {*this, static_cast<integer>(0), 0};
}

template<typename text, typename item, typename integer>
const std::vector<typename map_compact<text, item, integer>::node>&
map_compact<text, item, integer>::raw_data() const
{
	return data;
}

template<typename text, typename item, typename integer>
template<typename output_stream>
void map_compact<text, item, integer>::save(output_stream& os) const
{
	file_header header = {
		{constants::signature[0], constants::signature[1], constants::signature[2], constants::signature[3]},
		sizeof(file_header),
		constants::current_major_version,
		constants::current_minor_version,
		constants::container_type_map,
		constants::index_type_compact,
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

template<typename text, typename item, typename integer>
void map_compact<text, item, integer>::save(std::string path) const
{
	std::ofstream ofs(path);
	save(ofs);
}

template<typename text, typename item, typename integer>
template<typename input_stream>
integer map_compact<text, item, integer>::load(input_stream& is)
{
	file_header header;
	is.read(reinterpret_cast<char*>(&header), static_cast<std::streamsize>(sizeof(sftrie::file_header)));

	data.resize(header.node_count);
	is.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(sizeof(node) * header.node_count));

	return std::count_if(data.begin(), data.end(), [](const auto& n){
		return n.match;
	});
}

template<typename text, typename item, typename integer>
integer map_compact<text, item, integer>::load(std::string path)
{
	std::ifstream ifs(path);
	return load(path);
}


// private functions

template<typename text, typename item, typename integer>
template<typename iterator>
void map_compact<text, item, integer>::construct(iterator begin, iterator end, integer depth, integer current)
{
	// set flags
	if((data[current].match = (depth == container_size<integer>(*begin))))
		if((data[current].leaf = (++begin == end)))
			return;

	// reserve children
	std::vector<iterator> head{begin};
	for(iterator i = begin; i < end; head.push_back(i)){
		data.push_back({false, false, 0, 0, (*i).first[depth], (*i).second});
		for(symbol c = (*i)[depth]; i < end && (*i)[depth] == c; ++i);
	}

	// compress single paths
	std::vector<integer> depths;
	for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
		data[data[current].next + i].ref = container_size<integer>(labels);
		integer d = depth + 1;
		while(d < container_size<integer>(*head[i]) && (*head[i])[d] == (*(head[i + 1] - 1))[d])
			labels.push_back((*head[i])[d++]);
		depths.push_back(d);
	}

	// recursively construct subtries
	for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
		data[data[current].next + i].next = container_size<integer>(data);
		construct(head[i], head[i + 1], depths[i], data[current].next + i);
	}
}


// subclasses

template<typename text, typename item, typename integer>
struct map_compact<text, item, integer>::virtual_node
{
	map_compact<text, item, integer>& trie;
	integer id;
	integer depth;

	virtual_node(const map_compact<text, item, integer>& trie, integer id, integer depth):
		trie(trie), id(id), depth(depth)
	{}

	integer node_id() const
	{
		return id;
	}

	bool is_root() const
	{
		return id == 0;
	}

	bool is_valid() const
	{
		return id < trie.data.size() - 1;
	}

	bool is_physical() const
	{
		return depth == 0;
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

	item& value()
	{
		return trie.data[id].value;
	}

	bool operator==(const virtual_node& n) const
	{
		return id == n.id && depth == n.depth;
	}

	bool operator!=(const virtual_node& n) const
	{
		return id != n.id || depth != n.depth;
	}

	child_iterator children() const
	{
		if(trie.data[id].ref + depth < trie.data[id + 1].ref)
			return child_iterator(trie, id, depth + 1, 0);
		else if(!trie.data[id].leaf)
			return child_iterator(trie, trie.data[id].next, 0, trie.data[trie.data[id].next].next);
		else
			return child_iterator(trie, trie.data.size() - 1, 0, trie.data.size() - 1);
	}
};

template<typename text, typename item, typename integer>
struct map_compact<text, item, integer>::child_iterator
{
	virtual_node current;
	const integer last;

	child_iterator(const map_compact<text, item, integer>& trie):
		current(trie, 0, 0), last(1)
	{}

	child_iterator(const map_compact<text, item, integer>& trie, const integer parent):
		current(trie, trie.data[parent].next, 0),
		last(trie.data[parent].next < trie.data.size() ? trie.data[trie.data[parent].next].next : trie.data.size())
	{}

	child_iterator(const map_compact<text, item, integer>& trie, integer id, integer depth, integer last):
		current(trie, id, depth), last(last)
	{}

	child_iterator& begin()
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
		if(last > 0)
			++current.id;
		else
			current.id = last;
	}

	virtual_node operator*() const
	{
		return current;
	}
};

template<typename text, typename item, typename integer>
struct map_compact<text, item, integer>::common_searcher
{
	const map_compact<text, item, integer>& trie;
	std::vector<integer> path;
	text result;

	common_searcher(const map_compact<text, item, integer>& trie): trie(trie){}

	typename map_compact<text, item, integer>::node_type find(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size();){
			if(trie.data[current].leaf)
				return {trie, container_size<integer>(trie.data) - 1, 0};

			// find child
			current = trie.data[current].next;
			integer end = trie.data[current].next;
			for(integer w = end - current, m; w > trie.min_binary_search; w = m){
				m = w >> 1;
				current += trie.data[current + m].label < pattern[i] ? w - m : 0;
			}
			for(; current < end && trie.data[current].label < pattern[i]; ++current);
			if(!(current < end && trie.data[current].label == pattern[i]))
				return {trie, container_size<integer>(trie.data) - 1, 0};

			// check compressed labels
			integer j = trie.data[current].ref, jend = trie.data[current + 1].ref;
			if(jend - j > pattern.size() - ++i)
				return {trie, container_size<integer>(trie.data) - 1, 0};
			for(; j < jend; ++i, ++j)
				if(trie.labels[j] != pattern[i])
					return {trie, container_size<integer>(trie.data) - 1, 0};
		}
		if(!trie.data[current].match)
			return {trie, container_size<integer>(trie.data) - 1, 0};

		return {trie, current, trie.data[current + 1].ref - trie.data[current].ref};
	}

	typename map_compact<text, item, integer>::node_type end() const
	{
		return {trie, container_size<integer>(trie.data) - 1, 0};
	}

	integer count(const text& pattern) const
	{
		return find(pattern) != end() ? 1 : 0;
	}

	subtree_iterator traverse(const text& pattern)
	{
		auto root = find(pattern);
		if(root != end()){
			result.clear();
			path.clear();
			path.push_back(root.id);
			std::copy(std::begin(pattern), std::end(pattern), std::back_inserter(result));
		}
		return subtree_iterator(*this, root);
	}

	prefix_iterator prefix(const text& pattern)
	{
		result.clear();
		return prefix_iterator(*this, pattern, 0, 0);
	}
};

template<typename text, typename item, typename integer>
struct map_compact<text, item, integer>::subtree_iterator
{
	common_searcher& searcher;
	integer current;

	subtree_iterator(common_searcher& searcher, const virtual_node& root):
		searcher(searcher), current(root.id)
	{
		if(root.id < searcher.trie.data.size() - 1 && !searcher.trie.data[root.id].match){
			for(integer d = root.depth; searcher.trie.data[root.id].ref + d < searcher.trie.data[root.id + 1].ref; ++d)
				searcher.result.push_back(searcher.trie.labels[searcher.trie.data[root.id].ref + d]);
			++*this;
		}
	}

	const text& key() const
	{
		return searcher.result;
	}

	item& value()
	{
		return searcher.index.data[current].value;
	}

	map_compact<text, item, integer>::virtual_node node()
	{
		return map_compact<text, item, integer>::virtual_node(searcher.index, current);
	}

	subtree_iterator& begin()
	{
		return *this;
	}

	subtree_iterator end() const
	{
		return subtree_iterator(searcher, {searcher.trie, container_size<integer>(searcher.trie.data) - 1, 0});
	}

	bool operator!=(const subtree_iterator& i) const
	{
		return this->current != i.current;
	}

	const text& operator*() const
	{
		return searcher.result;
	}

	subtree_iterator& operator++()
	{
		do{
			if(!searcher.trie.data[searcher.path.back()].leaf){
				// first child
				integer n = searcher.trie.data[searcher.path.back()].next;
				searcher.path.push_back(n);
				searcher.result.push_back(searcher.trie.data[n].label);
				std::copy(searcher.trie.labels.begin() + searcher.trie.data[n].ref, searcher.trie.labels.begin() + searcher.trie.data[n + 1].ref, std::back_inserter(searcher.result));
			}
			else{
				while(searcher.path.size() > 1 && searcher.path.back() + 1 ==
						searcher.trie.data[searcher.trie.data[searcher.path[searcher.path.size() - 2]].next].next){
					// parent
					searcher.result.resize(searcher.result.size() - (1 + searcher.trie.data[searcher.path.back() + 1].ref - searcher.trie.data[searcher.path.back()].ref));
					searcher.path.pop_back();
				}
				if(searcher.path.size() > 1){
					// next sibling
					searcher.result.resize(searcher.result.size() - (1 + searcher.trie.data[searcher.path.back() + 1].ref - searcher.trie.data[searcher.path.back()].ref));
					integer n = ++searcher.path.back();
					searcher.result.push_back(searcher.trie.data[n].label);
					std::copy(searcher.trie.labels.begin() + searcher.trie.data[n].ref, searcher.trie.labels.begin() + searcher.trie.data[n + 1].ref, std::back_inserter(searcher.result));
				}
				else{
					searcher.path.clear();
				}
			}
		}while(!searcher.path.empty() && !searcher.trie.data[searcher.path.back()].match);
		current = !searcher.path.empty() ? searcher.path.back() : searcher.trie.data.size() - 1;
		return *this;
	}
};

template<typename text, typename item, typename integer>
struct map_compact<text, item, integer>::prefix_iterator
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

	const text& key() const
	{
		return searcher.result;
	}

	item& value()
	{
		return searcher.index.data[current].value;
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

	const text& operator*() const
	{
		return searcher.result;
	}

	prefix_iterator& operator++()
	{
		while(!searcher.trie.data[current].leaf && depth < pattern.size()){
			// find child
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
};

};

#endif
