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

#include <string>
#include <vector>
#include <map>

#include <Catch2/catch.hpp>

#include <sftrie/map.hpp>

#include "string_util.hpp"


using integer = std::uint32_t;
using item = integer;


template<typename map>
void test_map_exact_match(
	const std::vector<std::pair<typename map::text_type, typename map::value_type>>& texts,
	const std::vector<typename map::text_type>& patterns_not_in_texts,
	typename map::integer_type expected_size = 0
)
{
	map index(texts.begin(), texts.end());
	if(expected_size > 0){
		SECTION("trie size"){
			CHECK(index.trie_size() == expected_size); // root and sentinel
		}
	}

	SECTION("search patterns in texts"){
		for(const auto& [pattern, expected_value]: texts){
			CHECK(index.exists(pattern));
			auto v0 = index[pattern];
			CHECK(v0 == expected_value);
			auto v1 = index.find(pattern).value();
			CHECK(v1 == expected_value);
		}
	}

	SECTION("search patterns not in texts"){
		for(const auto& pattern: patterns_not_in_texts){
			CHECK(!index.exists(pattern));
			auto n = index.find(pattern);
			if(n != index.root()) // root is always valid
				CHECK(!n.match());
		}
	}
}

template<typename text>
void test_map_exact_match_all_classes(
	const std::vector<std::pair<text, item>>& texts,
	const std::vector<text>& patterns_not_in_texts,
	size_t expected_size_original = 0,
	size_t expected_size_compact = 0
)
{
	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, expected_size_original);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, expected_size_compact);
	}
}

void test_map_exact_match_all(
	const std::vector<std::pair<std::string, item>>& texts,
	const std::vector<std::string>& patterns_not_in_texts,
	std::map<std::string, size_t>& expected_sizes
)
{
	SECTION("char"){
		test_map_exact_match_all_classes(texts, patterns_not_in_texts, expected_sizes["original/char"], expected_sizes["compact/char"]);
	}

	SECTION("char16_t"){
		auto texts_u16 = cast_strings<std::u16string>(texts);
		auto patterns_not_in_texts_u16 = cast_strings<std::u16string>(patterns_not_in_texts);
		test_map_exact_match_all_classes(texts_u16, patterns_not_in_texts_u16, expected_sizes["original/char16_t"], expected_sizes["compact/char16_t"]);
	}

	SECTION("char32_t"){
		auto texts_u32 = cast_strings<std::u32string>(texts);
		auto patterns_not_in_texts_u32 = cast_strings<std::u32string>(patterns_not_in_texts);
		test_map_exact_match_all_classes(texts_u32, patterns_not_in_texts_u32, expected_sizes["original/char32_t"], expected_sizes["compact/char32_t"]);
	}
}

TEST_CASE("map/empty map/char", "[map]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> texts = {
	};
	const std::vector<text> patterns_not_in_texts = {
		"",
		"A",
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 2},
		{"compact/char", 2},
		{"original/char16_t", 2},
		{"compact/char16_t", 2},
		{"original/char32_t", 2},
		{"compact/char32_t", 2},
	};

	test_map_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}

TEST_CASE("map/map of an empty string/char", "[map]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> texts = {
		{"", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		"A",
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 2},
		{"compact/char", 2},
		{"original/char16_t", 2},
		{"compact/char16_t", 2},
		{"original/char32_t", 2},
		{"compact/char32_t", 2},
	};

	test_map_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}

TEST_CASE("map/set of a string with a single symbol/char", "[map]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> texts = {
		{"A", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		"",
		"B",
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 3},
		{"compact/char", 3},
		{"original/char16_t", 3},
		{"compact/char16_t", 3},
		{"original/char32_t", 3},
		{"compact/char32_t", 3},
	};

	test_map_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}

TEST_CASE("map/set of a string/char", "[set]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> texts = {
		{"ABC", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		"",
		"A",
		"AB",
		"ABCD",
		"B",
		"C",
		"BC",
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 5},
		{"compact/char", 3},
		{"original/char16_t", 5},
		{"compact/char16_t", 3},
		{"original/char32_t", 5},
		{"compact/char32_t", 3},
	};

	test_map_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}
