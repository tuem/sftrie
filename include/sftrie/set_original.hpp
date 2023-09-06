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

#ifndef SFTRIE_SET_ORIGINAL
#define SFTRIE_SET_ORIGINAL

#include <cstddef>
#include <vector>
#include <fstream>

#include "util.hpp"

#include "constants.hpp"
#include "file_header.hpp"

namespace sftrie{

template<typename text, typename integer>
class set_original
{
private:
	using symbol = typename text::value_type;

public:
	using symbol_type = symbol;
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
	template<typename random_access_iterator>
	set_original(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search = constants::default_min_binary_search<integer>);
	template<typename random_access_container>
	set_original(const random_access_container& texts,
		integer min_binary_search = constants::default_min_binary_search<integer>);
	template<typename input_stream> set_original(input_stream& is,
		integer min_binary_search = constants::default_min_binary_search<integer>);
	set_original(std::string path, integer min_binary_search = constants::default_min_binary_search<integer>);

	// information
	size_type size() const;
	size_type node_size() const;
	size_type trie_size() const;
	size_type total_space() const;

	// search operations
	bool exists(const text& pattern) const;
	common_searcher searcher() const;

	// tree operations
	node_type root() const;
	const std::vector<node>& raw_data() const;

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

	integer search(const text& pattern) const;
};

#pragma pack(1)
template<typename text, typename integer>
struct set_original<text, integer>::node
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	symbol label;
};
#pragma pack()


// constructors

template<typename text, typename integer>
template<typename random_access_iterator>
set_original<text, integer>::set_original(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search):
	min_binary_search(min_binary_search),
	num_texts(end - begin), data(1, {false, false, 1, {}})
{
	construct(begin, end, 0, 0);
	data.push_back({false, false, container_size<integer>(data), {}});
	data.shrink_to_fit();
}

template<typename text, typename integer>
template<typename random_access_container>
set_original<text, integer>::set_original(const random_access_container& texts, integer min_binary_search):
	min_binary_search(min_binary_search), num_texts(std::size(texts))
{
	construct(std::begin(texts), std::end(texts), 0, 0);
	data.push_back({false, false, container_size<integer>(data), {}});
	data.shrink_to_fit();
}

template<typename text, typename integer>
template<typename input_stream>
set_original<text, integer>::set_original(input_stream& is, integer min_binary_search):
	min_binary_search(min_binary_search)
{
	num_texts = load(is);
}

template<typename text, typename integer>
set_original<text, integer>::set_original(std::string path, integer min_binary_search):
	min_binary_search(min_binary_search)
{
	std::ifstream ifs(path);
	num_texts = load(ifs);
}


// public functions

template<typename text, typename integer>
typename set_original<text, integer>::size_type set_original<text, integer>::size() const
{
	return num_texts;
}

template<typename text, typename integer>
typename set_original<text, integer>::size_type set_original<text, integer>::node_size() const
{
	return sizeof(node);
}

template<typename text, typename integer>
typename set_original<text, integer>::size_type set_original<text, integer>::trie_size() const
{
	return data.size();
}

template<typename text, typename integer>
typename set_original<text, integer>::size_type set_original<text, integer>::total_space() const
{
	return sizeof(node) * data.size();
}

template<typename text, typename integer>
bool set_original<text, integer>::exists(const text& pattern) const
{
	return data[search(pattern)].match;
}

template<typename text, typename integer>
typename set_original<text, integer>::common_searcher
set_original<text, integer>::searcher() const
{
	return common_searcher(*this);
}

template<typename text, typename integer>
typename set_original<text, integer>::node_type set_original<text, integer>::root() const
{
	return {*this, static_cast<integer>(0)};
}

template<typename text, typename integer>
const std::vector<typename set_original<text, integer>::node>&
set_original<text, integer>::raw_data() const
{
	return data;
}

template<typename text, typename integer>
template<typename output_stream>
void set_original<text, integer>::save(output_stream& os) const
{
	file_header header = {
		{constants::signature[0], constants::signature[1], constants::signature[2], constants::signature[3]},
		sizeof(file_header),
		constants::current_major_version,
		constants::current_minor_version,
		constants::container_type_set,
		constants::index_type_original,
		constants::text_charset<text>(),
		constants::text_encoding<text>(),
		constants::integer_type<integer>(),
		sizeof(node),
		0,
		0,
		data.size(),
		0,
	};
	os.write(reinterpret_cast<const char*>(&header), static_cast<std::streamsize>(sizeof(sftrie::file_header)));

	os.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(sizeof(node) * data.size()));
}

template<typename text, typename integer>
void set_original<text, integer>::save(std::string path) const
{
	std::ofstream ofs(path);
	save(ofs);
}

template<typename text, typename integer>
template<typename input_stream>
integer set_original<text, integer>::load(input_stream& is)
{
	file_header header;
	is.read(reinterpret_cast<char*>(&header), static_cast<std::streamsize>(sizeof(sftrie::file_header)));

	data.resize(header.node_count);
	is.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(sizeof(node) * header.node_count));

	return std::count_if(data.begin(), data.end(), [](const auto& n){
		return n.match;
	});
}

template<typename text, typename integer>
integer set_original<text, integer>::load(std::string path)
{
	std::ifstream ifs(path);
	return load(path);
}


// private functions

template<typename text, typename integer>
template<typename iterator>
void set_original<text, integer>::construct(iterator begin, iterator end, integer depth, integer current)
{
	// set flags
	if((data[current].match = (depth == container_size<integer>(*begin))))
		if((data[current].leaf = (++begin == end)))
			return;

	// reserve children
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
integer set_original<text, integer>::search(const text& pattern) const
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

template<typename text, typename integer>
struct set_original<text, integer>::virtual_node
{
	const set_original<text, integer>& trie;
	integer id;

	virtual_node(const set_original<text, integer>& trie, integer id):
		trie(trie), id(id)
	{}

	integer node_id() const
	{
		return id;
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

	child_iterator children() const
	{
		return child_iterator(trie, trie.data[id].next, trie.data[trie.data[id].next].next);
	}
};

template<typename text, typename integer>
struct set_original<text, integer>::child_iterator
{
	virtual_node current;
	const integer last;

	child_iterator(const set_original<text, integer>& trie):
		current(trie, 0), last(1)
	{}

	child_iterator(const set_original<text, integer>& trie, const integer parent):
		current(trie, trie.data[parent].next),
		last(trie.data[parent].next < trie.data.size() ? trie.data[trie.data[parent].next].next : trie.data.size())
	{}

	child_iterator(const set_original<text, integer>& trie, integer id, integer last):
		current(trie, id), last(last)
	{}

	child_iterator& begin()
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

template<typename text, typename integer>
struct set_original<text, integer>::common_searcher
{
	const set_original<text, integer>& index;
	std::vector<integer> path;
	text result;

	common_searcher(const set_original<text, integer>& index): index(index){}

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

	subtree_iterator predict(const text& pattern)
	{
		auto root = index.search(pattern);
		if(root < index.data.size() - 1){
			path.clear();
			result.clear();
			path.push_back(root);
			std::copy(std::begin(pattern), std::end(pattern), std::back_inserter(result));
		}
		return subtree_iterator(*this, pattern, root);
	}

	prefix_iterator prefix(const text& pattern)
	{
		result.clear();
		return prefix_iterator(*this, pattern, 0, 0);
	}
};

template<typename text, typename integer>
struct set_original<text, integer>::subtree_iterator
{
	common_searcher& searcher;
	const text& prefix;
	integer current;

	subtree_iterator(common_searcher& searcher, const text& prefix, integer root):
		searcher(searcher), prefix(prefix), current(root)
	{
		if(root < searcher.index.data.size() - 1 && !searcher.index.data[root].match)
			++*this;
	}

	subtree_iterator& begin()
	{
		return *this;
	}

	subtree_iterator end() const
	{
		return subtree_iterator(searcher, prefix, searcher.index.data.size() - 1);
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
struct set_original<text, integer>::prefix_iterator
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
