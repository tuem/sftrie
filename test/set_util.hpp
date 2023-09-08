/*
trimatch
https://github.com/tuem/trimatch

Copyright 2021 Takashi Uemura

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

#include <Catch2/catch.hpp>


template<typename set>
void test_set_exact_match(
	const std::vector<typename set::text_type>& texts,
	const std::vector<typename set::text_type>& patterns_in_texts,
	const std::vector<typename set::text_type>& patterns_not_in_texts,
	typename set::integer_type expected_size = 0
)
{
	set index(texts.begin(), texts.end());
	if(expected_size > 0){
		SECTION("trie size"){
			CHECK(index.trie_size() == 2); // root and sentinel
		}
	}

	SECTION("search patterns in texts"){
		for(const auto& pattern: patterns_in_texts)
			CHECK(index.exists(pattern));
	}

	SECTION("search patterns not in texts"){
		for(const auto& pattern: patterns_not_in_texts)
			CHECK(!index.exists(pattern));
	}
}
