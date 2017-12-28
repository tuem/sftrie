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

#ifndef SFTRIE_SET_DECOMPACTION_HPP
#define SFTRIE_SET_DECOMPACTION_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename integer>
class set_decompaction
{
	using symbol = typename text::value_type;

	struct element;
	struct common_prefix_searcher;
	struct common_prefix_iterator;

public:
	template<typename random_access_iterator>
	set_decompaction(random_access_iterator begin, random_access_iterator end,
			integer min_binary_search = 42, integer min_tail = 1,
			integer min_decompaction = 1 << (bit_width<symbol>() - 3),
			symbol min_symbol = min_char<symbol>(), symbol max_symbol = max_char<symbol>());

	std::size_t size() const;
	std::size_t space() const;
	bool exists(const text& pattern) const;
	common_prefix_searcher searcher() const;

private:
	const std::size_t num_texts;

	std::vector<element> data;

	const integer min_binary_search;

	std::vector<symbol> tails;
	const integer min_tail;

	const integer min_decompaction;
	const symbol min_symbol;
	const symbol max_symbol;
	const integer alphabet_size;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current);

	bool check_tail(const text& pattern, integer i, integer current) const;
	bool check_tail_prefix(const text& pattern, integer i, integer current) const;
};

#pragma pack(1)
template<typename text, typename integer>
struct set_decompaction<text, integer>::element
{
	bool match: 1;
	bool leaf: 1;
	integer next: bit_width<integer>() - 2;
	integer tail;
	symbol label;
};
#pragma pack()

template<typename text, typename integer>
template<typename random_access_iterator>
set_decompaction<text, integer>::set_decompaction(random_access_iterator begin, random_access_iterator end,
		integer min_binary_search, integer min_tail,
		integer min_decompaction, symbol min_symbol, symbol max_symbol):
	num_texts(end - begin), data(1, {false, false, 1, 0, {}}),
	min_binary_search(min_binary_search), tails(1, {}), min_tail(min_tail),
	min_decompaction(min_decompaction), min_symbol(min_symbol), max_symbol(max_symbol),
	alphabet_size(max_symbol - min_symbol + 1)
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
std::size_t set_decompaction<text, integer>::size() const
{
	return num_texts;
}

template<typename text, typename integer>
std::size_t set_decompaction<text, integer>::space() const
{
	return sizeof(element) * data.size() + sizeof(symbol) * tails.size();
}

template<typename text, typename integer>
bool set_decompaction<text, integer>::exists(const text& pattern) const
{
	integer current = 0;
	for(integer i = 0; i < pattern.size(); ++i){
		if(data[current].leaf)
			return check_tail(pattern, i, current);
		current = data[current].next;
		integer end = data[current].next;
		if(current + alphabet_size == end){
			current += pattern[i] - min_symbol;
			continue;
		}
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
typename set_decompaction<text, integer>::common_prefix_searcher
set_decompaction<text, integer>::searcher() const
{
	return common_prefix_searcher(*this);
}

template<typename text, typename integer>
template<typename iterator>
void set_decompaction<text, integer>::construct(iterator begin, iterator end, integer depth, integer current)
{
	if(depth == container_size<integer>(*begin)){
		data[current].match = true;
		if(++begin == end){
			data[current].leaf = true;
			return;
		}
	}

	// count children
	std::vector<iterator> head{begin};
	for(iterator i = begin; i < end; head.push_back(i))
		for(symbol c = (*i)[depth]; i < end && (*i)[depth] == c; ++i);

	if(min_decompaction != 0 && container_size<integer>(head) > min_decompaction){
		// reserve siblings first
		integer old_data_size = container_size<integer>(data);
		for(symbol c = min_symbol; true; ++c){
			data.push_back({false, false, 0, 0, c});
			if(c == max_symbol)
				break;
		}
		integer alphabet_size = container_size<integer>(data) - old_data_size;

		// extract tail strings of leaves
		for(integer i = 0, j = 0; i < alphabet_size; ++i){
			integer child = data[current].next + i;
			if(j == head.size() - 1 || (*head[j])[depth] != data[child].label){
				data[child].leaf = true;
				continue;
			}
			if(head[j + 1] - head[j] == 1 && container_size<integer>(*head[j]) - (depth + 1) >= min_tail){
				data[child].match = container_size<integer>(*head[j]) == depth + 1;
				data[child].leaf = true;
				data[child].tail = container_size<integer>(tails);
				for(integer k = child - 1; k > 0 && data[k].tail == 0; --k)
					data[k].tail = data[child].tail;
				std::copy(std::begin(*head[j]) + depth + 1, std::end(*head[j]), std::back_inserter(tails));
			}
			++j;
		}

		// recursively construct subtries
		for(integer i = 0, j = 0; i < alphabet_size; ++i){
			integer child = data[current].next + i;
			data[child].next = container_size<integer>(data);
			if(j == head.size() - 1 || (*head[j])[depth] != data[child].label)
				continue;
			if(head[j + 1] - head[j] != 1 || container_size<integer>(*head[j]) - (depth + 1) < min_tail)
				construct(head[j], head[j + 1], depth + 1, child);
			++j;
		}
	}
	else{
		// reserve siblings first
		for(integer i = 0; i < container_size<integer>(head) - 1; ++i){
			data.push_back({false, false, 0, 0, (*head[i])[depth]});
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
}

template<typename text, typename integer>
bool set_decompaction<text, integer>::check_tail(const text& pattern, integer i, integer current) const
{
	return container_size<integer>(pattern) - i == data[current + 1].tail - data[current].tail &&
		std::equal(std::begin(pattern) + i, std::end(pattern), std::begin(tails) + data[current].tail);
}

template<typename text, typename integer>
bool set_decompaction<text, integer>::check_tail_prefix(const text& pattern, integer i, integer current) const
{
	return container_size<integer>(pattern) - i <= data[current + 1].tail - data[current].tail &&
		std::equal(std::begin(pattern) + i, std::end(pattern), std::begin(tails) + data[current].tail);
}

template<typename text, typename integer>
struct set_decompaction<text, integer>::common_prefix_searcher
{
	const set_decompaction<text, integer>& index;

	std::vector<integer> path;
	text result;
	std::vector<integer> path_end;
	text result_end;

	common_prefix_searcher(const set_decompaction<text, integer>& index): index(index){}

	common_prefix_iterator common_prefix(const text& pattern)
	{
		path.clear();
		result.clear();

		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(index.data[current].leaf)
				return index.check_tail_prefix(pattern, i, current) ?
					common_prefix_iterator(index.data, index.tails, path, result, path_end, result_end, current, pattern, i) :
					common_prefix_iterator(index.data, index.tails, path_end, result_end);
			current = index.data[current].next;
			integer end = index.data[current].next;
			if(current + index.alphabet_size == end){
				current += pattern[i] - index.min_symbol;
				continue;
			}
			for(integer w = end - current, m; w > index.min_binary_search; w = m){
				m = w >> 1;
				current += index.data[current + m].label < pattern[i] ? w - m : 0;
			}
			for(; current < end && index.data[current].label < pattern[i]; ++current);
			if(!(current < end && index.data[current].label == pattern[i]))
				return common_prefix_iterator(index.data, index.tails, path_end, result_end);
		}
		return common_prefix_iterator(index.data, index.tails, path, result, path_end, result_end, current, pattern);
	}
};

template<typename text, typename integer>
struct set_decompaction<text, integer>::common_prefix_iterator
{
	const std::vector<element>& data;
	const std::vector<symbol>& tails;

	std::vector<integer>& path;
	text& result;
	std::vector<integer>& path_end;
	text& result_end;

	common_prefix_iterator(const std::vector<element>& data, const std::vector<symbol>& tails,
			std::vector<integer>& path, text& result,
			std::vector<integer>& path_end, text& result_end,
			integer root, const text& prefix, integer length = 0):
		data(data), tails(tails), path(path), result(result), path_end(path_end), result_end(result_end)
	{
		path.push_back(root);
		std::copy(std::begin(prefix), std::begin(prefix) + (length > 0 ? length : prefix.size()),
			std::back_inserter(result));
		if((length != 0 && length < prefix.size()) || !data[root].match)
			++*this;
	}

	common_prefix_iterator(const std::vector<element>& data, const std::vector<symbol>& tails,
			std::vector<integer>& path, text& result):
		data(data), tails(tails), path(path), result(result), path_end(path), result_end(result) {}

	common_prefix_iterator& begin()
	{
		return *this;
	}

	common_prefix_iterator end() const
	{
		return common_prefix_iterator(data, tails, path_end, result_end);
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
			else if(data[path.back()].tail < data[path.back() + 1].tail &&
					(path.size() < 2 || path[path.size() - 2] != path.back())){
				path.push_back(path.back());
				std::copy(std::begin(tails) + data[path.back()].tail,
					std::begin(tails) + data[path.back() + 1].tail, std::back_inserter(result));
			}
			else{
				if(path.size() > 1 && path[path.size() - 2] == path.back()){
					path.pop_back();
					result.erase(std::end(result) - (data[path.back() + 1].tail - data[path.back()].tail),
						std::end(result));
				}
				while(path.size() > 1 && path.back() + 1 == data[data[path[path.size() - 2]].next].next){
					path.pop_back();
					result.pop_back();
				}
				if(path.size() > 1)
					result.back() = data[++path.back()].label;
				else
					path.pop_back();
			}
		}while(!path.empty() && !data[path.back()].match &&
			!(path.size() > 1 && path.back() == path[path.size() - 2]));
		return *this;
	}
};

};

#endif
