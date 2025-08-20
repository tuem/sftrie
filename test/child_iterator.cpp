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
#include <sftrie/util.hpp>

#include "util.hpp"


using integer = std::uint32_t;
using item = integer;

template<typename node, typename text, typename output_iterator>
void extract(node n, text& current, output_iterator o)
{
	if(n.match())
		*o++ = current;
	for(auto c: n.children()){
		current.push_back(c.label());
		extract(c, current, o);
		current.pop_back();
	}
}

template<typename set>
void test_set_child_iterator(
	const std::vector<typename set::text_type>& texts
)
{
	using text = typename set::text_type;

	set index(texts.begin(), texts.end());

	SECTION("dump all symbols in trie"){
		text current;
		std::vector<text> results;
		extract(index.root(), current, std::back_inserter(results));
		
		CHECK(results == texts);
	}
}

template<typename text>
void test_set_child_iterator_all_classes(
	const std::vector<text>& texts
)
{
	SECTION("set_original"){
		test_set_child_iterator<sftrie::set_original<text, integer>>(texts);
	}

	SECTION("set_compact"){
		test_set_child_iterator<sftrie::set_compact<text, integer>>(texts);
	}

	SECTION("set_fast"){
		test_set_child_iterator<sftrie::set_fast<text, integer>>(texts);
	}
}

void test_set_child_iterator_all(
	const std::vector<std::string>& texts
)
{
	SECTION("char"){
		test_set_child_iterator_all_classes(texts);
	}

	SECTION("char16_t"){
		auto texts_u16 = sftrie::cast_texts<std::u16string>(texts);
		test_set_child_iterator_all_classes(texts_u16);
	}

	SECTION("char32_t"){
		auto texts_u32 = sftrie::cast_texts<std::u32string>(texts);
		test_set_child_iterator_all_classes(texts_u32);
	}
}


template<typename map>
void test_map_child_iterator(
	const std::vector<typename map::text_type>& texts
)
{
	using text = typename map::text_type;

	auto text_ids = assign_ids<text, item>(texts);

	map index(text_ids.begin(), text_ids.end());

	SECTION("dump all symbols in trie"){
		text current;
		std::vector<text> results;
		extract(index.root(), current, std::back_inserter(results));

		CHECK(results == texts);
	}
}

template<typename text>
void test_map_child_iterator_all_classes(
	const std::vector<text>& texts
)
{
	SECTION("map_original"){
		test_map_child_iterator<sftrie::map_original<text, item, integer>>(texts);
	}

	SECTION("map_compact"){
		test_map_child_iterator<sftrie::map_compact<text, item, integer>>(texts);
	}

	SECTION("map_fast"){
		test_map_child_iterator<sftrie::map_fast<text, item, integer>>(texts);
	}
}

void test_map_child_iterator_all(
	const std::vector<std::string>& texts
)
{
	SECTION("char"){
		test_map_child_iterator_all_classes(texts);
	}

	SECTION("char16_t"){
		auto texts_u16 = sftrie::cast_texts<std::u16string>(texts);
		test_map_child_iterator_all_classes(texts_u16);
	}

	SECTION("char32_t"){
		auto texts_u32 = sftrie::cast_texts<std::u32string>(texts);
		test_map_child_iterator_all_classes(texts_u32);
	}
}


void test_child_iterator_all(
	const std::vector<std::string>& texts
)
{
	test_set_child_iterator_all(texts);
	test_map_child_iterator_all(texts);
}


TEST_CASE("child_iterator/empty set", "[child_iterator]"){
	using text = std::string;

	const std::vector<text> texts = {
	};

	test_child_iterator_all(texts);
}

TEST_CASE("child_iterator/set of an empty text", "[child_iterator]"){
	using text = std::string;

	const std::vector<text> texts = {
		"",
	};

	test_child_iterator_all(texts);
}

TEST_CASE("child_iterator/set of a text with a single symbol", "[child_iterator]"){
	using text = std::string;

	const std::vector<text> texts = {
		"A",
	};

	test_child_iterator_all(texts);
}

TEST_CASE("child_iterator/set of a text", "[child_iterator]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABC",
	};

	test_child_iterator_all(texts);
}

TEST_CASE("child_iterator/few texts", "[child_iterator]"){
	using text = std::string;

	const std::vector<text> texts = {
		"AM",
		"AMD",
		"CAD",
		"CAM",
		"CM",
		"DM",
	};

	test_child_iterator_all(texts);
}

TEST_CASE("child_iterator/a long text", "[child_iterator]"){
	using text = std::string;

	const std::vector<text> texts = {
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	};

	test_child_iterator_all(texts);
}
