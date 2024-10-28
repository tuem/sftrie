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

#include <cstdint>
#include <string>
#include <vector>
#include <map>

#include <Catch2/catch.hpp>

#include <sftrie/set.hpp>
#include <sftrie/map.hpp>
#include <sftrie/util.hpp>

#include "util.hpp"


using integer = std::uint32_t;
using item = std::uint32_t;


template<typename set>
void test_set_construction(
	const std::vector<typename set::text_type>& texts,
	bool two_pass,
	const std::vector<size_t>& expected_sizes
)
{
	set index(texts.begin(), texts.end(), two_pass);
	SECTION("# of texts"){
		CHECK(index.size() == texts.size());
	}
	SECTION("node size"){
		CHECK(index.node_size() == sizeof(typename set::node));
	}
	if(expected_sizes[0] > 0){
		SECTION("trie size"){
			CHECK(index.trie_size() == expected_sizes[0]);
		}
	}
	if(expected_sizes[1] > 0){
		SECTION("total space"){
			CHECK(index.total_space() == expected_sizes[1]);
		}
	}

	SECTION("search patterns in texts"){
		for(const auto& pattern: texts)
			CHECK(index.exists(pattern));
	}

/*
	SECTION("reconstruction"){
		index.construct(texts, two_pass);
		CHECK(index.size() == texts.size());
		CHECK(index.node_size() == sizeof(typename set::node));
		if(expected_sizes[0] > 0)
			CHECK(index.trie_size() == expected_sizes[0]);
		if(expected_sizes[1] > 0)
			CHECK(index.total_space() == expected_sizes[1]);
		for(const auto& pattern: texts)
			CHECK(index.exists(pattern));
	}
*/
}

template<typename text, typename integer>
void test_set_construction_all_classes(
	const std::vector<text>& texts,
	const std::vector<size_t>& expected_sizes_original,
	const std::vector<size_t>& expected_sizes_compact
)
{
	SECTION("set_original / 1-pass"){
		test_set_construction<sftrie::set_original<text, integer>>(texts, false, expected_sizes_original);
	}

	SECTION("set_original / 2-pass"){
		test_set_construction<sftrie::set_original<text, integer>>(texts, true, expected_sizes_original);
	}

	SECTION("set_compact / 1-pass"){
		test_set_construction<sftrie::set_compact<text, integer>>(texts, false, expected_sizes_compact);
	}

	SECTION("set_compact / 2-pass"){
		test_set_construction<sftrie::set_compact<text, integer>>(texts, true, expected_sizes_compact);
	}
}

template<typename integer>
void test_set_construction_all(
	const std::vector<std::string>& texts,
	std::map<std::string, std::vector<size_t>>& expected_sizes
)
{
	SECTION("char"){
		test_set_construction_all_classes<std::string, integer>(texts,
			expected_sizes["set/original/char"], expected_sizes["set/compact/char"]);
	}

	SECTION("char16_t"){
		auto texts_u16 = sftrie::cast_texts<std::u16string>(texts);
		test_set_construction_all_classes<std::u16string, integer>(texts_u16,
			expected_sizes["set/original/char16_t"], expected_sizes["set/compact/char16_t"]);
	}

	SECTION("char32_t"){
		auto texts_u32 = sftrie::cast_texts<std::u32string>(texts);
		test_set_construction_all_classes<std::u32string, integer>(texts_u32,
			expected_sizes["set/original/char32_t"], expected_sizes["set/compact/char32_t"]);
	}
}


template<typename map>
void test_map_construction(
	const std::vector<std::pair<typename map::text_type, typename map::value_type>>& texts,
	bool two_pass,
	const std::vector<size_t>& expected_sizes
)
{
	map index(texts.begin(), texts.end(), two_pass);
	SECTION("# of texts"){
		CHECK(index.size() == texts.size());
	}
	SECTION("node size"){
		CHECK(index.node_size() == sizeof(typename map::node));
	}
	if(expected_sizes[0] > 0){
		SECTION("trie size"){
			CHECK(index.trie_size() == expected_sizes[0]);
		}
	}
	if(expected_sizes[1] > 0){
		SECTION("total space"){
			CHECK(index.total_space() == expected_sizes[1]);
		}
	}

	SECTION("search patterns in texts"){
		for(const auto& [pattern, expected_value]: texts)
			CHECK(index[pattern] == expected_value);
	}

/*
	SECTION("reconstruction"){
		index.construct(texts, two_pass);
		CHECK(index.size() == texts.size());
		CHECK(index.node_size() == sizeof(typename map::node));
		if(expected_sizes[0] > 0)
			CHECK(index.trie_size() == expected_sizes[0]);
		if(expected_sizes[1] > 0)
			CHECK(index.total_space() == expected_sizes[1]);
		for(const auto& [pattern, expected_value]: texts)
			CHECK(index[pattern] == expected_value);
	}
*/
}

template<typename text, typename integer>
void test_map_construction_all_classes(
	const std::vector<std::pair<text, item>>& texts,
	const std::vector<size_t>& expected_sizes_original,
	const std::vector<size_t>& expected_sizes_compact
)
{
	SECTION("map_original / 1-pass"){
		test_map_construction<sftrie::map_original<text, item, integer>>(texts, false, expected_sizes_original);
	}

	SECTION("map_original / 2-pass"){
		test_map_construction<sftrie::map_original<text, item, integer>>(texts, true, expected_sizes_original);
	}

	SECTION("map_compact / 1-pass"){
		test_map_construction<sftrie::map_compact<text, item, integer>>(texts, false, expected_sizes_compact);
	}

	SECTION("map_compact / 2-pass"){
		test_map_construction<sftrie::map_compact<text, item, integer>>(texts, true, expected_sizes_compact);
	}
}

template<typename integer>
void test_map_construction_all(
	const std::vector<std::string>& texts,
	std::map<std::string, std::vector<size_t>>& expected_sizes
)
{
	auto text_ids = assign_ids<std::string, item>(texts);

	SECTION("char"){
		test_map_construction_all_classes<std::string, integer>(text_ids,
			expected_sizes["map/original/char"], expected_sizes["map/compact/char"]);
	}

	SECTION("char16_t"){
		auto text_ids_u16 = sftrie::cast_text_item_pairs<std::u16string>(text_ids);
		test_map_construction_all_classes<std::u16string, integer>(text_ids_u16,
			expected_sizes["map/original/char16_t"], expected_sizes["map/compact/char16_t"]);
	}

	SECTION("char32_t"){
		auto text_ids_u32 = sftrie::cast_text_item_pairs<std::u32string>(text_ids);
		test_map_construction_all_classes<std::u32string, integer>(text_ids_u32,
			expected_sizes["map/original/char32_t"], expected_sizes["map/compact/char32_t"]);
	}
}


template<typename integer>
void test_construction_all(
	const std::vector<std::string>& texts,
	std::map<std::string, std::vector<size_t>>& expected_sizes
)
{
	test_set_construction_all<integer>(texts, expected_sizes);
	test_map_construction_all<integer>(texts, expected_sizes);
}


TEST_CASE("construction/empty set", "[construction]"){
	using text = std::string;

	const std::vector<text> texts = {
	};
	std::map<std::string, std::vector<size_t>> expected_sizes = {
		{"set/original/char",     {2, 2 * (sizeof(integer) + sizeof(char))}},
		{"set/original/char16_t", {2, 2 * (sizeof(integer) + sizeof(char16_t))}},
		{"set/original/char32_t", {2, 2 * (sizeof(integer) + sizeof(char32_t))}},
		{"map/original/char",     {2, 2 * (2 * sizeof(integer) + sizeof(char))}},
		{"map/original/char16_t", {2, 2 * (2 * sizeof(integer) + sizeof(char16_t))}},
		{"map/original/char32_t", {2, 2 * (2 * sizeof(integer) + sizeof(char32_t))}},
		{"set/compact/char",      {2, 2 * (2 * sizeof(integer) + sizeof(char))}},
		{"set/compact/char16_t",  {2, 2 * (2 * sizeof(integer) + sizeof(char16_t))}},
		{"set/compact/char32_t",  {2, 2 * (2 * sizeof(integer) + sizeof(char32_t))}},
		{"map/compact/char",      {2, 2 * (3 * sizeof(integer) + sizeof(char))}},
		{"map/compact/char16_t",  {2, 2 * (3 * sizeof(integer) + sizeof(char16_t))}},
		{"map/compact/char32_t",  {2, 2 * (3 * sizeof(integer) + sizeof(char32_t))}},
	};

	test_construction_all<integer>(texts, expected_sizes);
}

TEST_CASE("construction/set of an empty text", "[construction]"){
	using text = std::string;

	const std::vector<text> texts = {
		"",
	};
	std::map<std::string, std::vector<size_t>> expected_sizes = {
		{"set/original/char",     {2, 2 * (sizeof(integer) + sizeof(char))}},
		{"set/original/char16_t", {2, 2 * (sizeof(integer) + sizeof(char16_t))}},
		{"set/original/char32_t", {2, 2 * (sizeof(integer) + sizeof(char32_t))}},
		{"map/original/char",     {2, 2 * (2 * sizeof(integer) + sizeof(char))}},
		{"map/original/char16_t", {2, 2 * (2 * sizeof(integer) + sizeof(char16_t))}},
		{"map/original/char32_t", {2, 2 * (2 * sizeof(integer) + sizeof(char32_t))}},
		{"set/compact/char",      {2, 2 * (2 * sizeof(integer) + sizeof(char))}},
		{"set/compact/char16_t",  {2, 2 * (2 * sizeof(integer) + sizeof(char16_t))}},
		{"set/compact/char32_t",  {2, 2 * (2 * sizeof(integer) + sizeof(char32_t))}},
		{"map/compact/char",      {2, 2 * (3 * sizeof(integer) + sizeof(char))}},
		{"map/compact/char16_t",  {2, 2 * (3 * sizeof(integer) + sizeof(char16_t))}},
		{"map/compact/char32_t",  {2, 2 * (3 * sizeof(integer) + sizeof(char32_t))}},
	};

	test_construction_all<integer>(texts, expected_sizes);
}

TEST_CASE("construction/set of a text with a single symbol", "[construction]"){
	using text = std::string;

	const std::vector<text> texts = {
		"A",
	};
	std::map<std::string, std::vector<size_t>> expected_sizes = {
		{"set/original/char",     {3, 3 * (sizeof(integer) + sizeof(char))}},
		{"set/original/char16_t", {3, 3 * (sizeof(integer) + sizeof(char16_t))}},
		{"set/original/char32_t", {3, 3 * (sizeof(integer) + sizeof(char32_t))}},
		{"map/original/char",     {3, 3 * (sizeof(integer) + sizeof(char) + sizeof(item))}},
		{"map/original/char16_t", {3, 3 * (sizeof(integer) + sizeof(char16_t) + sizeof(item))}},
		{"map/original/char32_t", {3, 3 * (sizeof(integer) + sizeof(char32_t) + sizeof(item))}},
		{"set/compact/char",      {3, 3 * (2 * sizeof(integer) + sizeof(char))}},
		{"set/compact/char16_t",  {3, 3 * (2 * sizeof(integer) + sizeof(char16_t))}},
		{"set/compact/char32_t",  {3, 3 * (2 * sizeof(integer) + sizeof(char32_t))}},
		{"map/compact/char",      {3, 3 * (2 * sizeof(integer) + sizeof(char) + sizeof(item))}},
		{"map/compact/char16_t",  {3, 3 * (2 * sizeof(integer) + sizeof(char16_t) + sizeof(item))}},
		{"map/compact/char32_t",  {3, 3 * (2 * sizeof(integer) + sizeof(char32_t) + sizeof(item))}},
	};

	test_construction_all<integer>(texts, expected_sizes);
}

TEST_CASE("construction/set of a text", "[construction]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABC",
	};
	std::map<std::string, std::vector<size_t>> expected_sizes = {
		{"set/original/char",     {5, 5 * (sizeof(integer) + sizeof(char))}},
		{"set/original/char16_t", {5, 5 * (sizeof(integer) + sizeof(char16_t))}},
		{"set/original/char32_t", {5, 5 * (sizeof(integer) + sizeof(char32_t))}},
		{"map/original/char",     {5, 5 * (sizeof(integer) + sizeof(char) + sizeof(item))}},
		{"map/original/char16_t", {5, 5 * (sizeof(integer) + sizeof(char16_t) + sizeof(item))}},
		{"map/original/char32_t", {5, 5 * (sizeof(integer) + sizeof(char32_t) + sizeof(item))}},
		{"set/compact/char",      {3, 3 * (2 * sizeof(integer) + sizeof(char)) + sizeof(char) * 2}},
		{"set/compact/char16_t",  {3, 3 * (2 * sizeof(integer) + sizeof(char16_t)) + sizeof(char16_t) * 2}},
		{"set/compact/char32_t",  {3, 3 * (2 * sizeof(integer) + sizeof(char32_t)) + sizeof(char32_t) * 2}},
		{"map/compact/char",      {3, 3 * (2 * sizeof(integer) + sizeof(char) + sizeof(item)) + sizeof(char) * 2}},
		{"map/compact/char16_t",  {3, 3 * (2 * sizeof(integer) + sizeof(char16_t) + sizeof(item)) + sizeof(char16_t) * 2}},
		{"map/compact/char32_t",  {3, 3 * (2 * sizeof(integer) + sizeof(char32_t) + sizeof(item)) + sizeof(char32_t) * 2}},
	};

	test_construction_all<integer>(texts, expected_sizes);
}

TEST_CASE("construction/few texts", "[construction]"){
	using text = std::string;

	const std::vector<text> texts = {
		"AM",
		"AMD",
		"CAD",
		"CAM",
		"CM",
		"DM",
	};
	std::map<std::string, std::vector<size_t>> expected_sizes = {
		{"set/original/char",     {12, 12 * (sizeof(integer) + sizeof(char))}},
		{"set/original/char16_t", {12, 12 * (sizeof(integer) + sizeof(char16_t))}},
		{"set/original/char32_t", {12, 12 * (sizeof(integer) + sizeof(char32_t))}},
		{"map/original/char",     {12, 12 * (sizeof(integer) + sizeof(char) + sizeof(item))}},
		{"map/original/char16_t", {12, 12 * (sizeof(integer) + sizeof(char16_t) + sizeof(item))}},
		{"map/original/char32_t", {12, 12 * (sizeof(integer) + sizeof(char32_t) + sizeof(item))}},
		{"set/compact/char",      {10, 10 * (2 * sizeof(integer) + sizeof(char)) + sizeof(char) * 2}},
		{"set/compact/char16_t",  {10, 10 * (2 * sizeof(integer) + sizeof(char16_t)) + sizeof(char16_t) * 2}},
		{"set/compact/char32_t",  {10, 10 * (2 * sizeof(integer) + sizeof(char32_t)) + sizeof(char32_t) * 2}},
		{"map/compact/char",      {10, 10 * (2 * sizeof(integer) + sizeof(char) + sizeof(item)) + sizeof(char) * 2}},
		{"map/compact/char16_t",  {10, 10 * (2 * sizeof(integer) + sizeof(char16_t) + sizeof(item)) + sizeof(char16_t) * 2}},
		{"map/compact/char32_t",  {10, 10 * (2 * sizeof(integer) + sizeof(char32_t) + sizeof(item)) + sizeof(char32_t) * 2}},
	};

	test_construction_all<integer>(texts, expected_sizes);
}

TEST_CASE("construction/a long text", "[construction]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	};
	auto l = texts.front().size();
	std::map<std::string, std::vector<size_t>> expected_sizes = {
		{"set/original/char",     {l + 2, (l + 2) * (sizeof(integer) + sizeof(char))}},
		{"set/original/char16_t", {l + 2, (l + 2) * (sizeof(integer) + sizeof(char16_t))}},
		{"set/original/char32_t", {l + 2, (l + 2) * (sizeof(integer) + sizeof(char32_t))}},
		{"map/original/char",     {l + 2, (l + 2) * (sizeof(integer) + sizeof(char) + sizeof(item))}},
		{"map/original/char16_t", {l + 2, (l + 2) * (sizeof(integer) + sizeof(char16_t) + sizeof(item))}},
		{"map/original/char32_t", {l + 2, (l + 2) * (sizeof(integer) + sizeof(char32_t) + sizeof(item))}},
		{"set/compact/char",      {1 + 2, (1 + 2) * (2 * sizeof(integer) + sizeof(char)) + sizeof(char) * (l - 1)}},
		{"set/compact/char16_t",  {1 + 2, (1 + 2) * (2 * sizeof(integer) + sizeof(char16_t)) + sizeof(char16_t) * (l - 1)}},
		{"set/compact/char32_t",  {1 + 2, (1 + 2) * (2 * sizeof(integer) + sizeof(char32_t)) + sizeof(char32_t) * (l - 1)}},
		{"map/compact/char",      {1 + 2, (1 + 2) * (2 * sizeof(integer) + sizeof(char) + sizeof(item)) + sizeof(char) * (l - 1)}},
		{"map/compact/char16_t",  {1 + 2, (1 + 2) * (2 * sizeof(integer) + sizeof(char16_t) + sizeof(item)) + sizeof(char16_t) * (l - 1)}},
		{"map/compact/char32_t",  {1 + 2, (1 + 2) * (2 * sizeof(integer) + sizeof(char32_t) + sizeof(item)) + sizeof(char32_t) * (l - 1)}},
	};

	test_construction_all<integer>(texts, expected_sizes);
}
