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

#ifndef SFTRIE_SET_TAIL
#define SFTRIE_SET_TAIL

#include <cstddef>
#include <vector>
#include <fstream>

#include "util.hpp"

#include "constants.hpp"
#include "file_header.hpp"

namespace sftrie{

template<typename text, typename integer>
class set_tail
{
public:
	using symbol = typename text::value_type;
	using size_type = std::size_t;

	struct node;
	struct virtual_node;
	struct child_iterator;
	struct common_searcher;
	struct subtree_iterator;
	struct prefix_iterator;

	using node_type = virtual_node;

public:
	template<typename random_access_iterator>
	set_tail(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search = constants::default_min_binary_search<integer>,
		integer min_tail = constants::default_min_tail<integer>);
	template<typename random_access_container>
	set_tail(const random_access_container& texts,
		integer min_binary_search = constants::default_min_binary_search<integer>,
		integer min_tail = constants::default_min_tail<integer>);
	template<typename input_stream> set_tail(input_stream& is,
		integer min_binary_search = constants::default_min_binary_search<integer>,
		integer min_tail = constants::default_min_tail<integer>);
	set_tail(std::string path,
		integer min_binary_search = constants::default_min_binary_search<integer>,
		integer min_tail = constants::default_min_tail<integer>);

	// information
	size_type size() const;
	size_type node_size() const;
	size_type trie_size() const;
	size_type space() const;

	// search
	bool exists(const text& pattern) const;
	common_searcher searcher() const;

	// tree operations
	node_type root() const;
	const std::vector<node>& raw_data() const;
	const std::vector<symbol>& raw_tail() const;

	// file I/O
	template<typename output_stream> void save(output_stream& os) const;
	void save(std::string os) const;
	template<typename input_stream> integer load(input_stream& is);
	integer load(std::string path);

private:
	const integer min_binary_search;
	const integer min_tail;

	size_type num_texts;
	std::vector<node> data;
	std::vector<symbol> tails;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current);

	bool check_tail(const text& pattern, integer i, integer current) const;
	bool check_tail_prefix(const text& pattern, integer i, integer current) const;
};


#pragma pack(1)
template<typename text, typename integer>
struct set_tail<text, integer>::node
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	integer tail;
	symbol label;
};
#pragma pack()


// constructors

template<typename text, typename integer>
template<typename random_access_iterator>
set_tail<text, integer>::set_tail(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search, integer min_tail):
	min_binary_search(min_binary_search), min_tail(min_tail),
	num_texts(end - begin), data(1, {false, false, 1, 0, {}}), tails(1, {})
{
	construct(begin, end, 0, 0);
	data.push_back({false, false, container_size<integer>(data), container_size<integer>(tails), {}});
	data.shrink_to_fit();
	for(integer i = container_size<integer>(data) - 2; i > 0 && data[i].tail == 0; --i)
		data[i].tail = data.back().tail;
	tails.push_back({});
	tails.shrink_to_fit();
}

template<typename text, typename integer>
template<typename random_access_container>
set_tail<text, integer>::set_tail(const random_access_container& texts,
		integer min_binary_search, integer min_tail):
	min_binary_search(min_binary_search), min_tail(min_tail), num_texts(std::size(texts))
{
	construct(std::begin(texts), std::end(texts), 0, 0);
	data.push_back({false, false, container_size<integer>(data), {}});
	data.shrink_to_fit();
}

template<typename text, typename integer>
template<typename input_stream>
set_tail<text, integer>::set_tail(input_stream& is,
		integer min_binary_search, integer min_tail):
	min_binary_search(min_binary_search), min_tail(min_tail)
{
	num_texts = load(is);
}

template<typename text, typename integer>
set_tail<text, integer>::set_tail(std::string path,
		integer min_binary_search, integer min_tail):
	min_binary_search(min_binary_search), min_tail(min_tail)
{
	std::ifstream ifs(path);
	num_texts = load(ifs);
}


// public functions

template<typename text, typename integer>
typename set_tail<text, integer>::size_type set_tail<text, integer>::size() const
{
	return num_texts;
}

template<typename text, typename integer>
typename set_tail<text, integer>::size_type set_tail<text, integer>::node_size() const
{
	return sizeof(node);
}

template<typename text, typename integer>
typename set_tail<text, integer>::size_type set_tail<text, integer>::trie_size() const
{
	return data.size();
}

template<typename text, typename integer>
typename set_tail<text, integer>::size_type set_tail<text, integer>::space() const
{
	return sizeof(node) * data.size() + sizeof(symbol) * tails.size();
}

template<typename text, typename integer>
bool set_tail<text, integer>::exists(const text& pattern) const
{
	integer current = 0;
	for(integer i = 0; i < pattern.size(); ++i){
		if(data[current].leaf)
			return check_tail(pattern, i, current);
		current = data[current].next;
		integer end = data[current].next;
		for(integer w = end - current, m; w > min_binary_search; w = m){
			m = w >> 1;
			current += data[current + m].label < pattern[i] ? w - m : 0;
		}
		for(; current < end && data[current].label < pattern[i]; ++current);
		if(!(current < end && data[current].label == pattern[i]))
			return false;
	}
	return data[current].match;
}

template<typename text, typename integer>
typename set_tail<text, integer>::common_searcher
set_tail<text, integer>::searcher() const
{
	return common_searcher(*this);
}

template<typename text, typename integer>
typename set_tail<text, integer>::node_type set_tail<text, integer>::root() const
{
	return {*this, static_cast<integer>(0)};
}

template<typename text, typename integer>
const std::vector<typename set_tail<text, integer>::node>&
set_tail<text, integer>::raw_data() const
{
	return data;
}

template<typename text, typename integer>
const std::vector<typename set_tail<text, integer>::symbol>&
set_tail<text, integer>::raw_tail() const
{
	return tails;
}

template<typename text, typename integer>
template<typename output_stream>
void set_tail<text, integer>::save(output_stream& os) const
{
	file_header header = {
		{constants::signature[0], constants::signature[1], constants::signature[2], constants::signature[3]},
		sizeof(file_header),
		constants::current_major_version,
		constants::current_minor_version,
		constants::container_type_set,
		constants::index_type_tail,
		constants::text_charset<text>(),
		constants::text_encoding<text>(),
		constants::integer_type<integer>(),
		sizeof(node),
		0,
		0,
		data.size(),
		tails.size(),
	};
	os.write(reinterpret_cast<const char*>(&header), static_cast<std::streamsize>(sizeof(sftrie::file_header)));

	os.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(sizeof(node) * data.size()));

	os.write(reinterpret_cast<const char*>(tails.data()), static_cast<std::streamsize>(sizeof(symbol) * tails.size()));
}

template<typename text, typename integer>
void set_tail<text, integer>::save(std::string path) const
{
	std::ofstream ofs(path);
	save(ofs);
}

template<typename text, typename integer>
template<typename input_stream>
integer set_tail<text, integer>::load(input_stream& is)
{
	file_header header;
	is.read(reinterpret_cast<char*>(&header), static_cast<std::streamsize>(sizeof(sftrie::file_header)));

	data.resize(header.node_count);
	is.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(sizeof(node) * header.node_count));

	tails.resize(header.tail_length);
	is.read(reinterpret_cast<char*>(tails.data()), static_cast<std::streamsize>(sizeof(symbol) * header.tail_length));

	return std::count_if(data.begin(), data.end(), [](const auto& n){
		return n.match;
	});
}

template<typename text, typename integer>
integer set_tail<text, integer>::load(std::string path)
{
	std::ifstream ifs(path);
	return load(path);
}


// private functions

template<typename text, typename integer>
template<typename iterator>
void set_tail<text, integer>::construct(iterator begin, iterator end, integer depth, integer current)
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
		data.push_back({false, false, 0, 0, (*i)[depth]});
		for(symbol c = (*i)[depth]; i < end && (*i)[depth] == c; ++i);
	}

	// extract tail strings of leaves
	for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
		integer child = data[current].next + i;
		if(head[i + 1] - head[i] == 1 && container_size<integer>(*head[i]) - (depth + 1) >= min_tail){
			data[child].match = container_size<integer>(*head[i]) == depth + 1;
			data[child].leaf = true;
			data[child].tail = container_size<integer>(tails);
			for(integer j = child - 1; j > 0 && data[j].tail == 0; --j)
				data[j].tail = data[child].tail;
			std::copy(std::begin(*head[i]) + depth + 1, std::end(*head[i]), std::back_inserter(tails));
		}
	}

	// recursively construct subtries
	for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
		integer child = data[current].next + i;
		data[child].next = container_size<integer>(data);
		if(head[i + 1] - head[i] != 1 || container_size<integer>(*head[i]) - (depth + 1) < min_tail)
			construct(head[i], head[i + 1], depth + 1, child);
	}
}

template<typename text, typename integer>
bool set_tail<text, integer>::check_tail(const text& pattern, integer i, integer current) const
{
	return container_size<integer>(pattern) - i == data[current + 1].tail - data[current].tail &&
		std::equal(std::begin(pattern) + i, std::end(pattern), std::begin(tails) + data[current].tail);
}

template<typename text, typename integer>
bool set_tail<text, integer>::check_tail_prefix(const text& pattern, integer i, integer current) const
{
	return container_size<integer>(pattern) - i <= data[current + 1].tail - data[current].tail &&
		std::equal(std::begin(pattern) + i, std::end(pattern), std::begin(tails) + data[current].tail);
}


// subclasses

template<typename text, typename integer>
struct set_tail<text, integer>::virtual_node
{
	const set_tail<text, integer>& trie;
	integer id;
	integer tail_pos;

	virtual_node(const set_tail<text, integer>& trie, integer id, integer tail_pos = 0):
		trie(trie), id(id), tail_pos(tail_pos)
	{}

	integer node_id() const
	{
		return id;
	}

	integer tail() const
	{
		return trie.data[id].tail;
	}

	symbol label() const
	{
		if(tail_pos == 0)
			return trie.data[id].label;
		else
			return trie.tails[trie.data[id].tail + tail_pos - 1];
	}

	bool match() const
	{
		if(tail_pos == 0)
			return trie.data[id].match;
		else
			return trie.data[id].tail + tail_pos == trie.data[id + 1].tail;
	}

	bool leaf() const
	{
		return trie.data[id].leaf && trie.data[id].tail + tail_pos == trie.data[id + 1].tail;
	}

	child_iterator children() const
	{
		if(!trie.data[id].leaf)
			return child_iterator(trie, id);
		else
			return child_iterator(trie, id, id + 1, tail_pos + 1);
	}

	const node& raw_node() const
	{
		return trie.data[id];
	}
};

template<typename text, typename integer>
struct set_tail<text, integer>::child_iterator
{
	virtual_node current;
	const integer last;

	child_iterator(const set_tail<text, integer>& trie):
		current(trie, 0, 0), last(1)
	{}

	child_iterator(const set_tail<text, integer>& trie, const integer parent):
		current(trie, trie.data[parent].next, 0),
		last(trie.data[parent].next < trie.data.size() ? trie.data[trie.data[parent].next].next : trie.data.size())
	{}

	child_iterator(const set_tail<text, integer>& trie, integer id, integer last, integer tail_pos = 0):
		current(trie, id, tail_pos), last(last)
	{}

	child_iterator& begin()
	{
		return *this;
	}

	child_iterator end() const
	{
		return child_iterator(current.trie, last, last, current.tail_pos);
	}

	bool incrementable() const
	{
		return current.id + 1 < last;
	}

	bool operator==(const child_iterator& i) const
	{
		return current.id == i.current.id && current.tail_pos == i.current.tail_pos;
	}

	bool operator!=(const child_iterator& i) const
	{
		return current.id != i.current.id || current.tail_pos != i.current.tail_pos;
	}

	void operator++()
	{
		++current.id;
	}

	virtual_node& operator*()
	{
		return current;
	}
};

template<typename text, typename integer>
struct set_tail<text, integer>::common_searcher
{
	const set_tail<text, integer>& index;

	std::vector<integer> path;
	text result;
	std::vector<integer> path_end;
	text result_end;

	common_searcher(const set_tail<text, integer>& index): index(index){}

	integer find(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(index.data[current].leaf)
				return index.check_tail(pattern, i, current) ? index.data.size() + index.data[current].tail : end();
			current = index.data[current].next;
			integer end = index.data[current].next;
			for(integer w = end - current, m; w > index.min_binary_search; w = m){
				m = w >> 1;
				current += index.data[current + m].label < pattern[i] ? w - m : 0;
			}
			for(; current < end && index.data[current].label < pattern[i]; ++current);
			if(!(current < end && index.data[current].label == pattern[i]))
				return this->end();
		}
		return index.data[current].match ? current : end();
	}

	integer end() const
	{
		return index.data.size() + index.tails.size();
	}

	integer count(const text& pattern) const
	{
		return find(pattern) != end() ? 1 : 0;
	}

	subtree_iterator traverse(const text& pattern)
	{
		path.clear();
		result.clear();

		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(index.data[current].leaf)
				return index.check_tail_prefix(pattern, i, current) ?
					subtree_iterator(index, path, result, path_end, result_end, current, pattern, i) :
					subtree_iterator(index, path_end, result_end);
			current = index.data[current].next;
			integer end = index.data[current].next;
			for(integer w = end - current, m; w > index.min_binary_search; w = m){
				m = w >> 1;
				current += index.data[current + m].label < pattern[i] ? w - m : 0;
			}
			for(; current < end && index.data[current].label < pattern[i]; ++current);
			if(!(current < end && index.data[current].label == pattern[i]))
				return subtree_iterator(index, path_end, result_end);
		}
		return subtree_iterator(index, path, result, path_end, result_end, current, pattern);
	}

	prefix_iterator prefix(const text& pattern)
	{
		result.clear();
		return prefix_iterator(*this, pattern, 0, 0);
	}
};

template<typename text, typename integer>
struct set_tail<text, integer>::subtree_iterator
{
	const set_tail<text, integer>& index;

	std::vector<integer>& path;
	text& result;
	std::vector<integer>& path_end;
	text& result_end;

	subtree_iterator(const set_tail<text, integer>& index,
			std::vector<integer>& path, text& result,
			std::vector<integer>& path_end, text& result_end,
			integer root, const text& prefix, integer length = 0):
		index(index), path(path), result(result), path_end(path_end), result_end(result_end)
	{
		path.push_back(root);
		std::copy(std::begin(prefix), std::begin(prefix) + (length > 0 ? length : prefix.size()),
			std::back_inserter(result));
		if((length != 0 && length < prefix.size()) || !index.data[root].match)
			++*this;
	}

	subtree_iterator(const set_tail<text, integer>& index,
			std::vector<integer>& path, text& result):
		index(index), path(path), result(result), path_end(path), result_end(result) {}

	subtree_iterator& begin()
	{
		return *this;
	}

	subtree_iterator end() const
	{
		return subtree_iterator(index, path_end, result_end);
	}

	bool operator!=(const subtree_iterator& i) const
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

	subtree_iterator& operator++()
	{
		do{
			if(!index.data[path.back()].leaf){
				integer child = index.data[path.back()].next;
				path.push_back(child);
				result.push_back(index.data[child].label);
			}
			else if(index.data[path.back()].tail < index.data[path.back() + 1].tail &&
					(path.size() < 2 || path[path.size() - 2] != path.back())){
				path.push_back(path.back());
				std::copy(std::begin(index.tails) + index.data[path.back()].tail,
					std::begin(index.tails) + index.data[path.back() + 1].tail, std::back_inserter(result));
			}
			else{
				if(path.size() > 1 && path[path.size() - 2] == path.back()){
					path.pop_back();
					result.erase(std::end(result) - (index.data[path.back() + 1].tail - index.data[path.back()].tail),
						std::end(result));
				}
				while(path.size() > 1 && path.back() + 1 == index.data[index.data[path[path.size() - 2]].next].next){
					path.pop_back();
					result.pop_back();
				}
				if(path.size() > 1)
					result.back() = index.data[++path.back()].label;
				else
					path.pop_back();
			}
		}while(!path.empty() && !index.data[path.back()].match &&
			!(path.size() > 1 && path.back() == path[path.size() - 2]));
		return *this;
	}
};

template<typename text, typename integer>
struct set_tail<text, integer>::prefix_iterator
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
		if(!searcher.index.data[current].leaf){
			while(depth < pattern.size()){
				if(searcher.index.data[current].leaf){
					if(
						container_size<integer>(pattern) - depth >= searcher.index.data[current + 1].tail - searcher.index.data[current].tail &&
						std::equal(
							std::begin(pattern) + depth,
							std::begin(pattern) + depth + (searcher.index.data[current + 1].tail - searcher.index.data[current].tail),
							std::begin(searcher.index.tails) + searcher.index.data[current].tail
						)
					)
						std::copy(std::begin(searcher.index.tails) + searcher.index.data[current].tail,
							std::begin(searcher.index.tails) + searcher.index.data[current + 1].tail, std::back_inserter(searcher.result));
					else
						current = searcher.index.data.size() - 1;
					return *this;
				}
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
		}
		current = searcher.index.data.size() - 1;
		return *this;
	}
};

};

#endif
