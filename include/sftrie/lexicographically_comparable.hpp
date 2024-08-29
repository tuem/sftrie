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

#ifndef SFTRIE_LEXICOGRAPHICALLY_COMPARABLE
#define SFTRIE_LEXICOGRAPHICALLY_COMPARABLE

#include <iterator>

namespace sftrie{

template<typename text>
concept lexicographically_comparable = requires(text& t, text& s)
{
	typename text::value_type;
	std::size(t);
	std::empty(t);
	std::begin(t) != std::end(t);
	t[0] == s[0];
	t[0] < s[0];
	t == s;
	t < s;
};

}

#endif
