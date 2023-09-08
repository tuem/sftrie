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

#include <sftrie/map.hpp>


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


TEST_CASE("map/empty map/char", "[map]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> texts = {
	};
	const std::vector<text> patterns_not_in_texts = {
		"",
		"A",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
}
TEST_CASE("map/empty map/char16_t", "[map]"){
	using text = std::u16string;

	const std::vector<std::pair<text, item>> texts = {
	};
	const std::vector<text> patterns_not_in_texts = {
		u"",
		u"A",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
}
TEST_CASE("map/empty map/char32_t", "[map]"){
	using text = std::u32string;

	const std::vector<std::pair<text, item>> texts = {
	};
	const std::vector<text> patterns_not_in_texts = {
		U"",
		U"A",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
}

TEST_CASE("map/map of an empty string/char", "[map]"){
	using text = std::string;

	const std::vector<std::pair<text, item>> texts = {
		{"", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		"A",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
}
TEST_CASE("map/map of an empty string/char16_t", "[map]"){
	using text = std::u16string;

	const std::vector<std::pair<text, item>> texts = {
		{u"", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		u"A",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
}
TEST_CASE("map/map of an empty string/char32_t", "[map]"){
	using text = std::u32string;

	const std::vector<std::pair<text, item>> texts = {
		{U"", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		U"A",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 2);
	}
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

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 3);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 3);
	}
}
TEST_CASE("map/set of a string with a single symbol/char16_t", "[map]"){
	using text = std::u16string;

	const std::vector<std::pair<text, item>> texts = {
		{u"A", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		u"",
		u"B",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 3);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 3);
	}
}
TEST_CASE("map/set of a string with a single symbol/char32_t", "[map]"){
	using text = std::u32string;

	const std::vector<std::pair<text, item>> texts = {
		{U"A", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		U"",
		U"B",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 3);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 3);
	}
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

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 5);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 3);
	}
}
TEST_CASE("map/set of a string/char16_t", "[set]"){
	using text = std::u16string;

	const std::vector<std::pair<text, item>> texts = {
		{u"ABC", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		u"",
		u"A",
		u"AB",
		u"ABCD",
		u"B",
		u"C",
		u"BC",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 5);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 3);
	}
}
TEST_CASE("map/set of a string/char32_t", "[map]"){
	using text = std::u32string;

	const std::vector<std::pair<text, item>> texts = {
		{U"ABC", 1},
	};
	const std::vector<text> patterns_not_in_texts = {
		U"",
		U"A",
		U"AB",
		U"ABCD",
		U"B",
		U"C",
		U"BC",
	};

	SECTION("map_original"){
		test_map_exact_match<sftrie::map_original<text, item, integer>>(texts,
			patterns_not_in_texts, 5);
	}
	SECTION("map_compact"){
		test_map_exact_match<sftrie::map_compact<text, item, integer>>(texts,
			patterns_not_in_texts, 3);
	}
}
