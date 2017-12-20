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

#ifndef SFTRIE_SET_NAIVE_HPP
#define SFTRIE_SET_NAIVE_HPP

#include <vector>

#include "util.hpp"

namespace sftrie{

template<typename text, typename integer>
class set_naive
{
	using symbol = typename text::value_type;

	struct element
	{
		bool match: 1;
		bool leaf: 1;
		integer index: bit_width<integer>() - 2;
		symbol label;
	};

public:
	template<typename random_access_iterator>
	set_naive(random_access_iterator begin, random_access_iterator end):
		num_texts(end - begin), data(1, {false, false, 1, {}})
	{
		construct(begin, end, 0, 0);
		data.shrink_to_fit();
	}

	std::size_t size() const
	{
		return num_texts;
	}

	std::size_t space() const
	{
		return sizeof(element) * data.size();
	}

	bool exists(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return false;
			for(integer l = data[current].index, r = data[l].index - 1; l <= r; ){
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
			return false;
			NEXT:;
		}
		return data[current].match;
	}

	struct iterator
	{
		const std::vector<element>& data;

		std::vector<integer> path;
		text result;

		iterator(const std::vector<element>& data, const text& prefix, integer root):
			data(data), path(1, root), result(prefix)
		{
			if(root < data.size() && !data[root].match)
				++*this;
		}

		iterator(const std::vector<element>& data): data(data){}

		const text& operator*()
		{
			return result;
		}

		iterator operator++()
		{
			do{
				if(!data[path.back()].leaf){
					integer child = data[path.back()].index;
					path.push_back(child);
					result.push_back(data[child].label);
				}
				else{
					while(path.size() > 1 && path.back() + 1 >= data[data[path[path.size() - 2]].index].index){
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

		bool operator!=(const iterator& i) const
		{
			if(this->path.empty() && i.path.empty())
				return false;
			else if(this->path.size() != i.path.size())
				return true;
			else
				return this->path.back() != i.path.back() || this->path.front() != i.path.front();
		}
	};

	iterator prefix(const text& pattern) const
	{
		integer current = 0;
		for(integer i = 0; i < pattern.size(); ++i){
			if(data[current].leaf)
				return end();
			for(integer l = data[current].index, r = data[l].index - 1; l <= r; ){
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
			return end();
			NEXT:;
		}
		return iterator(data, pattern, current);
	}

	iterator end() const
	{
		return iterator(data);
	}

private:
	const std::size_t num_texts;

	std::vector<element> data;

	template<typename iterator>
	void construct(iterator begin, iterator end, integer depth, integer current)
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
			integer child = data[current].index + i;
			data[child].index = container_size<integer>(data);
			construct(head[i], head[i + 1], depth + 1, child);
		}
	}
};

};

#endif
