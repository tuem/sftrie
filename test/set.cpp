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

using text = std::string;
using integer = std::uint32_t;

const std::vector<text> set_of_one_empty_string = { "" };

TEST_CASE("set/original/char/empty set", "[set]"){
	const std::vector<text> texts;
	sftrie::set_original<text, integer> index(texts.begin(), texts.end());

	SECTION("trie size"){
		CHECK(index.trie_size() == 2); // root and sentinel
	}

	SECTION("search"){
		CHECK(!index.exists(""));
		CHECK(!index.exists("A"));
	}
}

TEST_CASE("set/original/char/set of an empty string", "[set]"){
	const std::vector<text> texts = {
		""
	};
	sftrie::set_original<text, integer> index(texts.begin(), texts.end());

	SECTION("trie size"){
		CHECK(index.trie_size() == 2); // root and sentinel
	}

	SECTION("search"){
		CHECK(index.exists(""));
		CHECK(!index.exists("A"));
	}
}

TEST_CASE("set/compact/char/empty set", "[set]"){
	const std::vector<text> texts;
	sftrie::set_compact<text, integer> index(texts.begin(), texts.end());

	SECTION("trie size"){
		CHECK(index.trie_size() == 2); // root and sentinel
	}

	SECTION("search"){
		CHECK(!index.exists(""));
		CHECK(!index.exists("A"));
	}
}

TEST_CASE("set/compact/char/set of an empty string", "[set]"){
	const std::vector<text> texts = {
		""
	};
	sftrie::set_compact<text, integer> index(texts.begin(), texts.end());

	SECTION("trie size"){
		CHECK(index.trie_size() == 2); // root and sentinel
	}

	SECTION("search"){
		CHECK(index.exists(""));
		CHECK(!index.exists("A"));
	}
}
