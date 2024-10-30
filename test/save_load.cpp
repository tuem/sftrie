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
#include <sstream>
#include <map>

#include <Catch2/catch.hpp>

#include <sftrie/set.hpp>
#include <sftrie/map.hpp>
#include <sftrie/util.hpp>

#include "util.hpp"


using integer = std::uint32_t;
using item = integer;


template<typename set>
void test_set_save_load(
	const std::vector<typename set::text_type>& texts,
	typename set::integer_type expected_size
)
{
	set index0(texts.begin(), texts.end());
	std::stringstream ss;
	index0.save(ss);

	set index1(ss);
	if(expected_size > 0){
		SECTION("trie size"){
			CHECK(index1.trie_size() == expected_size);
		}
	}

	SECTION("search patterns from loaded index"){
		for(const auto& pattern: texts)
			CHECK(index1.exists(pattern));
	}

	SECTION("reload"){
		ss.seekg(0);
		index0.load(ss);
		CHECK(index0.trie_size() == expected_size);
	}
}

template<typename text>
void test_set_save_load_all_classes(
	const std::vector<text>& texts,
	size_t expected_size_original,
	size_t expected_size_compact
)
{
	SECTION("set_original"){
		test_set_save_load<sftrie::set_original<text, integer>>(texts, expected_size_original);
	}

	SECTION("set_compact"){
		test_set_save_load<sftrie::set_compact<text, integer>>(texts, expected_size_compact);
	}
}

void test_set_save_load_all(
	const std::vector<std::string>& texts,
	std::map<std::string, size_t>& expected_sizes
)
{
	SECTION("char"){
		test_set_save_load_all_classes(texts,
			expected_sizes["original/char"], expected_sizes["compact/char"]);
	}

	SECTION("char16_t"){
		auto texts_u16 = sftrie::cast_texts<std::u16string>(texts);
		test_set_save_load_all_classes(texts_u16,
			expected_sizes["original/char16_t"], expected_sizes["compact/char16_t"]);
	}

	SECTION("char32_t"){
		auto texts_u32 = sftrie::cast_texts<std::u32string>(texts);
		test_set_save_load_all_classes(texts_u32,
			expected_sizes["original/char32_t"], expected_sizes["compact/char32_t"]);
	}
}


template<typename map>
void test_map_save_load(
	const std::vector<std::pair<typename map::text_type, typename map::value_type>>& texts,
	typename map::integer_type expected_size
)
{
	map index0(texts.begin(), texts.end());
	std::stringstream ss;
	index0.save(ss);

	map index1(ss);
	if(expected_size > 0){
		SECTION("trie size"){
			CHECK(index1.trie_size() == expected_size); // root and sentinel
		}
	}

	SECTION("search patterns in texts"){
		for(const auto& [pattern, expected_value]: texts)
			CHECK(index1[pattern] == expected_value);
	}

	SECTION("reload"){
		ss.seekg(0);
		index0.load(ss);
		CHECK(index0.trie_size() == expected_size);
	}
}

template<typename text>
void test_map_save_load_all_classes(
	const std::vector<std::pair<text, item>>& texts,
	size_t expected_size_original,
	size_t expected_size_compact
)
{
	SECTION("map_original"){
		test_map_save_load<sftrie::map_original<text, item, integer>>(texts, expected_size_original);
	}

	SECTION("map_compact"){
		test_map_save_load<sftrie::map_compact<text, item, integer>>(texts, expected_size_compact);
	}
}

void test_map_save_load_all(
	const std::vector<std::string>& texts,
	std::map<std::string, size_t>& expected_sizes
)
{
	auto text_ids = assign_ids<std::string, item>(texts);

	SECTION("char"){
		test_map_save_load_all_classes(text_ids,
			expected_sizes["original/char"], expected_sizes["compact/char"]);
	}

	SECTION("char16_t"){
		auto text_ids_u16 = sftrie::cast_text_item_pairs<std::u16string>(text_ids);
		test_map_save_load_all_classes(text_ids_u16,
			expected_sizes["original/char16_t"], expected_sizes["compact/char16_t"]);
	}

	SECTION("char32_t"){
		auto text_ids_u32 = sftrie::cast_text_item_pairs<std::u32string>(text_ids);
		test_map_save_load_all_classes(text_ids_u32,
			expected_sizes["original/char32_t"], expected_sizes["compact/char32_t"]);
	}
}


void test_save_load_all(
	const std::vector<std::string>& texts,
	std::map<std::string, size_t>& expected_sizes
)
{
	test_set_save_load_all(texts, expected_sizes);
	test_map_save_load_all(texts, expected_sizes);
}


TEST_CASE("save_load/empty set", "[save/load]"){
	using text = std::string;

	const std::vector<text> texts = {
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 2},
		{"compact/char", 2},
		{"original/char16_t", 2},
		{"compact/char16_t", 2},
		{"original/char32_t", 2},
		{"compact/char32_t", 2},
	};

	test_save_load_all(texts, expected_sizes);
}

TEST_CASE("save_load/set of an empty text", "[save_load]"){
	using text = std::string;

	const std::vector<text> texts = {
		"",
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 2},
		{"compact/char", 2},
		{"original/char16_t", 2},
		{"compact/char16_t", 2},
		{"original/char32_t", 2},
		{"compact/char32_t", 2},
	};

	test_save_load_all(texts, expected_sizes);
}

TEST_CASE("save_load/set of a text with a single symbol", "[save_load]"){
	using text = std::string;

	const std::vector<text> texts = {
		"A",
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 3},
		{"compact/char", 3},
		{"original/char16_t", 3},
		{"compact/char16_t", 3},
		{"original/char32_t", 3},
		{"compact/char32_t", 3},
	};

	test_save_load_all(texts, expected_sizes);
}

TEST_CASE("save_load/set of a text", "[save_load]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABC",
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 5},
		{"compact/char", 3},
		{"original/char16_t", 5},
		{"compact/char16_t", 3},
		{"original/char32_t", 5},
		{"compact/char32_t", 3},
	};

	test_save_load_all(texts, expected_sizes);
}

TEST_CASE("save_load/few texts", "[save_load]"){
	using text = std::string;

	const std::vector<text> texts = {
		"AM",
		"AMD",
		"CAD",
		"CAM",
		"CM",
		"DM",
	};
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", 12},
		{"compact/char", 10},
		{"original/char16_t", 12},
		{"compact/char16_t", 10},
		{"original/char32_t", 12},
		{"compact/char32_t", 10},
	};

	test_save_load_all(texts, expected_sizes);
}

TEST_CASE("save_load/a long text", "[save_load]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	};
	auto l = texts.front().size();
	std::map<std::string, size_t> expected_sizes = {
		{"original/char", l + 2},
		{"compact/char", 3},
		{"original/char16_t", l + 2},
		{"compact/char16_t", 3},
		{"original/char32_t", l + 2},
		{"compact/char32_t", 3},
	};

	test_save_load_all(texts, expected_sizes);
}
