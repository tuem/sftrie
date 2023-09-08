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

#include <sftrie/set.hpp>


using integer = std::uint32_t;


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
void test_set_exact_match_all(
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

TEST_CASE("set/empty set/char", "[set]"){
	using text = std::string;

	const std::vector<text> texts = {
	};
	const std::vector<text> patterns_not_in_texts = {
		"",
		"A",
	};

	test_set_exact_match_all(texts, patterns_not_in_texts, 2, 2);
}
TEST_CASE("set/empty set/char16_t", "[set]"){
	using text = std::u16string;

	const std::vector<text> texts = {
	};
	const std::vector<text> patterns_not_in_texts = {
		u"",
		u"A",
	};

	test_set_exact_match_all(texts, patterns_not_in_texts, 2, 2);
}
TEST_CASE("set/empty set/char32_t", "[set]"){
	using text = std::u32string;

	const std::vector<text> texts = {
	};
	const std::vector<text> patterns_not_in_texts = {
		U"",
		U"A",
	};

	test_set_exact_match_all(texts, patterns_not_in_texts, 2, 2);
}

TEST_CASE("set/set of an empty string/char", "[set]"){
	using text = std::string;

	const std::vector<text> texts = {
		"",
	};
	const std::vector<text> patterns_not_in_texts = {
		"A",
	};

	test_set_exact_match_all(texts, patterns_not_in_texts, 2, 2);
}
TEST_CASE("set/set of an empty string/char16_t", "[set]"){
	using text = std::u16string;

	const std::vector<text> texts = {
		u"",
	};
	const std::vector<text> patterns_not_in_texts = {
		u"A",
	};

	test_set_exact_match_all(texts, patterns_not_in_texts, 2, 2);
}
TEST_CASE("set/set of an empty string/char32_t", "[set]"){
	using text = std::u32string;

	const std::vector<text> texts = {
		U"",
	};
	const std::vector<text> patterns_not_in_texts = {
		U"A",
	};

	test_set_exact_match_all(texts, patterns_not_in_texts, 2, 2);
}

TEST_CASE("set/set of a string with a single symbol/char", "[set]"){
	using text = std::string;

	const std::vector<text> texts = {
		"A",
	};
	const std::vector<text> patterns_not_in_texts = {
		"",
		"B",
	};

	test_set_exact_match_all(texts, patterns_not_in_texts, 3, 3);
}
TEST_CASE("set/set of a string with a single symbol/char16_t", "[set]"){
	using text = std::u16string;

	const std::vector<text> texts = {
		u"A",
	};
	const std::vector<text> patterns_not_in_texts = {
		u"",
		u"B",
	};

	test_set_exact_match_all(texts, patterns_not_in_texts, 3, 3);
}
TEST_CASE("set/set of a string with a single symbol/char32_t", "[set]"){
	using text = std::u32string;

	const std::vector<text> texts = {
		U"A",
	};
	const std::vector<text> patterns_not_in_texts = {
		U"",
		U"B",
	};

	test_set_exact_match_all(texts, patterns_not_in_texts, 3, 3);
}

TEST_CASE("set/set of a string/char", "[set]"){
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

	test_set_exact_match_all(texts, patterns_not_in_texts, 5, 3);
}
TEST_CASE("set/set of a string/char16_t", "[set]"){
	using text = std::u16string;

	const std::vector<text> texts = {
		u"ABC",
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

	test_set_exact_match_all(texts, patterns_not_in_texts, 5, 3);
}
TEST_CASE("set/set of a string/char32_t", "[set]"){
	using text = std::u32string;

	const std::vector<text> texts = {
		U"ABC",
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

	test_set_exact_match_all(texts, patterns_not_in_texts, 5, 3);
}
