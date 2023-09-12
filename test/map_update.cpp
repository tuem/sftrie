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


template<typename map>
void test_map_update(
	const std::vector<std::pair<typename map::text_type, typename map::value_type>>& before,
	const std::vector<std::pair<typename map::text_type, typename map::value_type>>& operations,
	const std::vector<std::pair<typename map::text_type, typename map::value_type>>& after
)
{
	SECTION("update by key"){
		map index(before.begin(), before.end());
		for(const auto& op: operations)
			index.update(op.first, op.second);

		for(const auto& [pattern, correct]: after){
			auto n = index.find(pattern);
			CHECK(n.value() == correct);
		}
	}

	SECTION("update node"){
		map index(before.begin(), before.end());
		for(const auto& op: operations){
			auto n = index.find(op.first);
			index.update(n, op.second);
		}

		for(const auto& [pattern, correct]: after){
			auto n = index.find(pattern);
			CHECK(n.value() == correct);
		}
	}

	SECTION("operator[]"){
		map index(before.begin(), before.end());
		for(const auto& op: operations)
			index[op.first] = op.second;

		for(const auto& [pattern, correct]: after){
			auto n = index.find(pattern);
			CHECK(n.value() == correct);
		}
	}
}

template<typename text>
void test_map_update_all_classes(
	const std::vector<std::pair<text, item>>& before,
	const std::vector<std::pair<text, item>>& operations,
	const std::vector<std::pair<text, item>>& after
)
{
	SECTION("map_original"){
		test_map_update<sftrie::map_original<text, item, integer>>(before, operations, after);
	}

	SECTION("map_compact"){
		test_map_update<sftrie::map_compact<text, item, integer>>(before, operations, after);
	}
}

void test_map_update_all(
	const std::vector<std::pair<std::string, item>>& before,
	const std::vector<std::pair<std::string, item>>& operations,
	const std::vector<std::pair<std::string, item>>& after
)
{
	SECTION("char"){
		test_map_update_all_classes(before, operations, after);
	}

	SECTION("char16_t"){
		auto before_u16 = cast_strings<std::u16string>(before);
		auto operations_u16 = cast_strings<std::u16string>(operations);
		auto after_u16 = cast_strings<std::u16string>(after);
		test_map_update_all_classes(before_u16, operations_u16, after_u16);
	}

	SECTION("char32_t"){
		auto before_u32 = cast_strings<std::u32string>(before);
		auto operations_u32 = cast_strings<std::u32string>(operations);
		auto after_u32 = cast_strings<std::u32string>(after);
		test_map_update_all_classes(before_u32, operations_u32, after_u32);
	}
}


TEST_CASE("update/set of an empty string", "[exact match]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> before = {
		{"", 1},
	};
	const std::vector<std::pair<text, item>> operations = {
		{"A", 2},
		{"BC", 3},
	};
	const std::vector<std::pair<text, item>> after = {
		{"", 1},
	};

	test_map_update_all(before, operations, after);
}

TEST_CASE("update/set of a string", "[exact match]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> before = {
		{"A", 1},
	};
	const std::vector<std::pair<text, item>> operations = {
		{"", 2},
		{"A", 3},
		{"BC", 4},
	};
	const std::vector<std::pair<text, item>> after = {
		{"A", 3},
	};

	test_map_update_all(before, operations, after);
}

TEST_CASE("update/set of a string with few letters", "[exact match]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> before = {
		{"ABCDE", 1},
	};
	const std::vector<std::pair<text, item>> operations = {
		{"ABCDE", 2},
		{"", 3},
		{"A", 4},
		{"AB", 5},
		{"ABC", 6},
		{"ABCD", 7},
		{"ABCDEF", 8},
		{"B", 9},
	};
	const std::vector<std::pair<text, item>> after = {
		{"ABCDE", 2},
	};

	test_map_update_all(before, operations, after);
}

TEST_CASE("update/set of few texts", "[exact match]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> before = {
		{"", 1},
		{"A", 2},
		{"ABC", 3},
		{"ABCDE", 4},
		{"ABCFG", 5},
		{"BCD", 6},
	};
	const std::vector<std::pair<text, item>> operations = {
		{"", 7},
		{"BC", 8},
		{"ABCDE", 9},
		{"ABC", 10},
		{"AB", 11},
		{"ABCD", 12},
		{"A", 13},
		{"ABC", 14},
		{"ABCDEF", 115},
	};
	const std::vector<std::pair<text, item>> after = {
		{"", 7},
		{"A", 13},
		{"ABC", 14},
		{"ABCDE", 9},
		{"ABCFG", 5},
		{"BCD", 6},
	};

	test_map_update_all(before, operations, after);
}
