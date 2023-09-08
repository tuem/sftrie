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


template<typename set>
void test_set_common_prefix_search(
	const std::vector<typename set::text_type>& texts,
	const std::vector<std::pair<typename set::text_type, std::vector<typename set::text_type>>>& patterns
)
{
	set index(texts.begin(), texts.end());

	auto searcher = index.searcher();
	for(const auto& i: patterns){
		std::vector<typename set::text_type> results;
		for(const auto& p: searcher.prefix(i.first))
			results.push_back(p);
		CHECK(results == i.second);
	}
}

template<typename text>
void test_set_common_prefix_search_all_classes(
	const std::vector<text>& texts,
	const std::vector<std::pair<text, std::vector<text>>>& patterns
)
{
	SECTION("set_original"){
		test_set_common_prefix_search<sftrie::set_original<text, integer>>(texts, patterns);
	}

	SECTION("set_compact"){
		test_set_common_prefix_search<sftrie::set_compact<text, integer>>(texts, patterns);
	}
}

void test_set_common_prefix_search_all(
	const std::vector<std::string>& texts,
	const std::vector<std::pair<std::string, std::vector<std::string>>>& patterns
)
{
	SECTION("char"){
		test_set_common_prefix_search_all_classes(texts, patterns);
	}

	SECTION("char16_t"){
		auto texts_u16 = cast_strings<std::u16string>(texts);
		std::vector<std::pair<std::u16string, std::vector<std::u16string>>> patterns_u16;
		for(const auto& i: patterns){
			std::vector<std::u16string> results;
			for(const auto& j: i.second)
				results.push_back(cast_string<std::u16string>(j));
			patterns_u16.push_back({cast_string<std::u16string>(i.first), results});
		}
		test_set_common_prefix_search_all_classes(texts_u16, patterns_u16);
	}

	SECTION("char32_t"){
		auto texts_u32 = cast_strings<std::u32string>(texts);
		std::vector<std::pair<std::u32string, std::vector<std::u32string>>> patterns_u32;
		for(const auto& i: patterns){
			std::vector<std::u32string> results;
			for(const auto& j: i.second)
				results.push_back(cast_string<std::u32string>(j));
			patterns_u32.push_back({cast_string<std::u32string>(i.first), results});
		}
		test_set_common_prefix_search_all_classes(texts_u32, patterns_u32);
	}
}


template<typename map>
void test_map_common_prefix_search(
	const std::vector<std::pair<typename map::text_type, typename map::value_type>>& texts,
	const std::vector<std::pair<typename map::text_type, std::vector<typename map::text_type>>>& patterns
)
{
	map index(texts.begin(), texts.end());
	auto searcher = index.searcher();
	for(const auto& i: patterns){
		std::vector<typename map::text_type> results;
		for(const auto& p: searcher.prefix(i.first))
			results.push_back(p.key());
		CHECK(results == i.second);
	}
}

template<typename text>
void test_map_common_prefix_search_all_classes(
	const std::vector<std::pair<text, item>>& texts,
	const std::vector<std::pair<text, std::vector<text>>>& patterns
)
{
	SECTION("map_original"){
		test_map_common_prefix_search<sftrie::map_original<text, item, integer>>(texts, patterns);
	}

	SECTION("map_compact"){
		test_map_common_prefix_search<sftrie::map_compact<text, item, integer>>(texts, patterns);
	}
}

void test_map_common_prefix_search_all(
	const std::vector<std::string>& texts,
	const std::vector<std::pair<std::string, std::vector<std::string>>>& patterns
)
{
	auto text_ids = assign_ids<std::string, item>(texts);

	SECTION("char"){
		test_map_common_prefix_search_all_classes(text_ids, patterns);
	}

	SECTION("char16_t"){
		auto text_ids_u16 = cast_strings<std::u16string>(text_ids);
		std::vector<std::pair<std::u16string, std::vector<std::u16string>>> patterns_u16;
		for(const auto& i: patterns){
			std::vector<std::u16string> results;
			for(const auto& j: i.second)
				results.push_back(cast_string<std::u16string>(j));
			patterns_u16.push_back({cast_string<std::u16string>(i.first), results});
		}
		test_map_common_prefix_search_all_classes(text_ids_u16, patterns_u16);
	}

	SECTION("char32_t"){
		auto text_ids_u32 = cast_strings<std::u32string>(text_ids);
		std::vector<std::pair<std::u16string, std::vector<std::u16string>>> patterns_u16;
		std::vector<std::pair<std::u32string, std::vector<std::u32string>>> patterns_u32;
		for(const auto& i: patterns){
			std::vector<std::u32string> results;
			for(const auto& j: i.second)
				results.push_back(cast_string<std::u32string>(j));
			patterns_u32.push_back({cast_string<std::u32string>(i.first), results});
		}
		test_map_common_prefix_search_all_classes(text_ids_u32, patterns_u32);
	}
}


void test_common_prefix_search_all(
	const std::vector<std::string>& texts,
	const std::vector<std::pair<std::string, std::vector<std::string>>>& patterns
)
{
	test_set_common_prefix_search_all(texts, patterns);
	test_map_common_prefix_search_all(texts, patterns);
}


TEST_CASE("common-prefix search/empty set", "[common-prefix search]"){
	using text = std::string;

	const std::vector<text> texts = {
	};
	const std::vector<std::pair<text, std::vector<text>>> patterns = {
		{"", {}},
		{"A", {}},
	};

	test_common_prefix_search_all(texts, patterns);
}

TEST_CASE("common-prefix search/set of an empty text", "[common-prefix search]"){
	using text = std::string;

	const std::vector<text> texts = {
		"",
	};
	const std::vector<std::pair<text, std::vector<text>>> patterns = {
		{"", {""}},
		{"A", {""}},
		{"XYZ", {""}},
	};

	test_common_prefix_search_all(texts, patterns);
}

TEST_CASE("common-prefix search/set of a text with one symbol", "[common-prefix search]"){
	using text = std::string;

	const std::vector<text> texts = {
		"A",
	};
	const std::vector<std::pair<text, std::vector<text>>> patterns = {
		{"", {}},
		{"A", {"A"}},
		{"B", {}},
		{"AB", {"A"}},
	};

	test_common_prefix_search_all(texts, patterns);
}

TEST_CASE("common-prefix search/set of a text", "[common-prefix search]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABC",
	};
	const std::vector<std::pair<text, std::vector<text>>> patterns = {
		{"", {}},
		{"A", {}},
		{"AB", {}},
		{"ABC", {"ABC"}},
		{"ABCD", {"ABC"}},
		{"ABCDEFG", {"ABC"}},
		{"ABX", {}},
	};

	test_common_prefix_search_all(texts, patterns);
}

TEST_CASE("common-prefix search/set of few texts", "[common-prefix search]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABC",
		"D",
		"DEF",
		"DEFGH",
	};
	const std::vector<std::pair<text, std::vector<text>>> patterns = {
		{"", {}},
		{"A", {}},
		{"D", {"D"}},
		{"DE", {"D"}},
		{"DEF", {"D", "DEF"}},
		{"DEFG", {"D", "DEF"}},
		{"DEFGH", {"D", "DEF", "DEFGH"}},
		{"DEFGHI", {"D", "DEF", "DEFGH"}},
		{"DEFGX", {"D", "DEF"}},
	};

	test_common_prefix_search_all(texts, patterns);
}
