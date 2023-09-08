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

#include <sftrie/set.hpp>
#include <sftrie/map.hpp>

#include "string_util.hpp"


using integer = std::uint32_t;
using item = integer;


// utility for set

template<typename set>
void test_set_exact_match(
	const std::vector<typename set::text_type>& texts,
	const std::vector<typename set::text_type>& patterns_not_in_texts,
	typename set::integer_type expected_size = 0
)
{
	set index(texts.begin(), texts.end());
	if(expected_size > 0){
		SECTION("trie size"){
			CHECK(index.trie_size() == expected_size);
		}
	}

	SECTION("search patterns in texts"){
		for(const auto& pattern: texts)
			CHECK(index.exists(pattern));
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
void test_set_exact_match_all_classes(
	const std::vector<text>& texts,
	const std::vector<text>& patterns_not_in_texts,
	size_t expected_size_original = 0,
	size_t expected_size_compact = 0
)
{
	SECTION("set_original"){
		test_set_exact_match<sftrie::set_original<text, integer>>(texts,
			patterns_not_in_texts, expected_size_original);
	}
	SECTION("set_compact"){
		test_set_exact_match<sftrie::set_compact<text, integer>>(texts,
			patterns_not_in_texts, expected_size_compact);
	}
}

void test_set_exact_match_all(
	const std::vector<std::string>& texts,
	const std::vector<std::string>& patterns_not_in_texts,
	std::map<std::string, size_t>& expected_sizes
)
{
	SECTION("char"){
		test_set_exact_match_all_classes(texts, patterns_not_in_texts, expected_sizes["original/char"], expected_sizes["compact/char"]);
	}

	SECTION("char16_t"){
		auto texts_u16 = cast_strings<std::u16string>(texts);
		auto patterns_not_in_texts_u16 = cast_strings<std::u16string>(patterns_not_in_texts);
		test_set_exact_match_all_classes(texts_u16, patterns_not_in_texts_u16, expected_sizes["original/char16_t"], expected_sizes["compact/char16_t"]);
	}

	SECTION("char32_t"){
		auto texts_u32 = cast_strings<std::u32string>(texts);
		auto patterns_not_in_texts_u32 = cast_strings<std::u32string>(patterns_not_in_texts);
		test_set_exact_match_all_classes(texts_u32, patterns_not_in_texts_u32, expected_sizes["original/char32_t"], expected_sizes["compact/char32_t"]);
	}
}


// utility for map

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
	const std::vector<std::string>& texts,
	const std::vector<std::string>& patterns_not_in_texts,
	std::map<std::string, size_t>& expected_sizes
)
{
	auto text_ids = assign_ids<std::string, item>(texts);

	SECTION("char"){
		test_map_exact_match_all_classes(text_ids, patterns_not_in_texts, expected_sizes["original/char"], expected_sizes["compact/char"]);
	}

	SECTION("char16_t"){
		auto text_ids_u16 = cast_strings<std::u16string>(text_ids);
		auto patterns_not_in_texts_u16 = cast_strings<std::u16string>(patterns_not_in_texts);
		test_map_exact_match_all_classes(text_ids_u16, patterns_not_in_texts_u16, expected_sizes["original/char16_t"], expected_sizes["compact/char16_t"]);
	}

	SECTION("char32_t"){
		auto text_ids_u32 = cast_strings<std::u32string>(text_ids);
		auto patterns_not_in_texts_u32 = cast_strings<std::u32string>(patterns_not_in_texts);
		test_map_exact_match_all_classes(text_ids_u32, patterns_not_in_texts_u32, expected_sizes["original/char32_t"], expected_sizes["compact/char32_t"]);
	}
}


void test_exact_match_all(
	const std::vector<std::string>& texts,
	const std::vector<std::string>& patterns_not_in_texts,
	std::map<std::string, size_t>& expected_sizes
)
{
	test_set_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
	test_map_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}


TEST_CASE("exact match/empty set", "[exact match]"){
	using text = std::string;

	const std::vector<text> texts = {
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

	test_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}

TEST_CASE("set/set of an empty string", "[set]"){
	using text = std::string;

	const std::vector<text> texts = {
		"",
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

	test_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}

TEST_CASE("set/set of a string with a single symbol", "[set]"){
	using text = std::string;

	const std::vector<text> texts = {
		"A",
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

	test_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}

TEST_CASE("set/set of a string", "[set]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABC",
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

	test_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}

TEST_CASE("set/few texts", "[set]"){
	using text = std::string;

	const std::vector<text> texts = {
		"AM",
		"AMD",
		"CAD",
		"CAM",
		"CM",
		"DM",
	};
	const std::vector<text> patterns_not_in_texts = {
		"",
		" ",
		"A",
		"B",
		"AD",
		"CA",
		"CD",
		"CAME",
		"E",
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 12},
		{"compact/char", 10},
		{"original/char16_t", 12},
		{"compact/char16_t", 10},
		{"original/char32_t", 12},
		{"compact/char32_t", 10},
	};

	test_exact_match_all(texts, patterns_not_in_texts, expected_sizes);
}
