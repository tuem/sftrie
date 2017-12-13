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

#ifndef SFTRIE_UTIL_HPP
#define SFTRIE_UTIL_HPP

template<typename integer>
constexpr int bit_width()
{
	return 8 * sizeof(integer);
}

struct text_comparator
{
    template<typename text>
    bool operator()(const text& a, const text& b) const
    {
        for(size_t i = 0; i < std::min(a.size(), b.size()); ++i){
            if(a[i] < b[i])
                return true;
            else if(b[i] < a[i])
                return false;
        }
        return a.size() < b.size();
    }
};

#endif
