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

#include "util.hpp"


using integer = std::uint32_t;
using item = integer;


template<typename set>
void test_set_exact_match(
	const std::vector<typename set::text_type>& texts,
	const std::vector<typename set::text_type>& patterns_not_in_texts
)
{
	set index(texts.begin(), texts.end());

	SECTION("search patterns NOT in texts"){
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
	const std::vector<text>& patterns_not_in_texts
)
{
	SECTION("set_original"){
		test_set_exact_match<sftrie::set_original<text, integer>>(
			texts, patterns_not_in_texts);
	}

	SECTION("set_compact"){
		test_set_exact_match<sftrie::set_compact<text, integer>>(
			texts, patterns_not_in_texts);
	}
}

void test_set_exact_match_all(
	const std::vector<std::string>& texts,
	const std::vector<std::string>& patterns_not_in_texts
)
{
	SECTION("char"){
		test_set_exact_match_all_classes(texts, patterns_not_in_texts);
	}

	SECTION("char16_t"){
		auto texts_u16 = cast_strings<std::u16string>(texts);
		auto patterns_not_in_texts_u16 = cast_strings<std::u16string>(patterns_not_in_texts);
		test_set_exact_match_all_classes(texts_u16, patterns_not_in_texts_u16);
	}

	SECTION("char32_t"){
		auto texts_u32 = cast_strings<std::u32string>(texts);
		auto patterns_not_in_texts_u32 = cast_strings<std::u32string>(patterns_not_in_texts);
		test_set_exact_match_all_classes(texts_u32, patterns_not_in_texts_u32);
	}
}


template<typename map>
void test_map_exact_match(
	const std::vector<std::pair<typename map::text_type, typename map::value_type>>& texts,
	const std::vector<typename map::text_type>& patterns_not_in_texts
)
{
	map index(texts.begin(), texts.end());

	SECTION("search patterns in texts using all methods"){
		for(const auto& [pattern, expected_value]: texts){
			CHECK(index.exists(pattern));
			auto v0 = index[pattern];
			CHECK(v0 == expected_value);
			auto v1 = index.find(pattern).value();
			CHECK(v1 == expected_value);
			auto v2 = index.raw_data()[index.find(pattern).node_id()].value;
			CHECK(v2 == expected_value);
		}
	}

	SECTION("search patterns NOT in texts"){
		for(const auto& pattern: patterns_not_in_texts){
			CHECK(!index.exists(pattern));
			auto n = index.find(pattern);
			CHECK(!n.match());
		}
	}
}

template<typename text>
void test_map_exact_match_all_classes(
	const std::vector<std::pair<text, item>>& texts,
	const std::vector<text>& patterns_not_in_texts
)
{
	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(
			texts, patterns_not_in_texts);
	}

	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(
			texts, patterns_not_in_texts);
	}
}

void test_map_exact_match_all(
	const std::vector<std::string>& texts,
	const std::vector<std::string>& patterns_not_in_texts
)
{
	auto text_ids = assign_ids<std::string, item>(texts);

	SECTION("char"){
		test_map_exact_match_all_classes(text_ids, patterns_not_in_texts);
	}

	SECTION("char16_t"){
		auto text_ids_u16 = cast_strings<std::u16string>(text_ids);
		auto patterns_not_in_texts_u16 = cast_strings<std::u16string>(patterns_not_in_texts);
		test_map_exact_match_all_classes(text_ids_u16, patterns_not_in_texts_u16);
	}

	SECTION("char32_t"){
		auto text_ids_u32 = cast_strings<std::u32string>(text_ids);
		auto patterns_not_in_texts_u32 = cast_strings<std::u32string>(patterns_not_in_texts);
		test_map_exact_match_all_classes(text_ids_u32, patterns_not_in_texts_u32);
	}
}


void test_exact_match_all(
	const std::vector<std::string>& texts,
	const std::vector<std::string>& patterns_not_in_texts
)
{
	test_set_exact_match_all(texts, patterns_not_in_texts);
	test_map_exact_match_all(texts, patterns_not_in_texts);
}


TEST_CASE("exact match/empty set", "[exact match]"){
	using text = std::string;

	const std::vector<text> texts = {
	};
	const std::vector<text> patterns_not_in_texts = {
		"",
		"A",
	};

	test_exact_match_all(texts, patterns_not_in_texts);
}

TEST_CASE("exact match/set of an empty text", "[exact match]"){
	using text = std::string;

	const std::vector<text> texts = {
		"",
	};
	const std::vector<text> patterns_not_in_texts = {
		"A",
	};

	test_exact_match_all(texts, patterns_not_in_texts);
}

TEST_CASE("exact match/set of a text with a single symbol", "[exact match]"){
	using text = std::string;

	const std::vector<text> texts = {
		"A",
	};
	const std::vector<text> patterns_not_in_texts = {
		"",
		"B",
	};

	test_exact_match_all(texts, patterns_not_in_texts);
}

TEST_CASE("exact match/set of a text", "[exact match]"){
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

	test_exact_match_all(texts, patterns_not_in_texts);
}

TEST_CASE("exact match/few texts", "[exact match]"){
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

	test_exact_match_all(texts, patterns_not_in_texts);
}

TEST_CASE("exact match/a long text", "[construction]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	};

	std::vector<text> patterns_not_in_texts;
	for(integer i = 0; i < texts.front().size() - 1; ++i)
		patterns_not_in_texts.push_back(texts.front().substr(0, i));

	test_exact_match_all(texts, patterns_not_in_texts);
}
