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

#include <Catch2/catch.hpp>

#include <sftrie/util.hpp>


TEST_CASE("cast_text/cast to same class", "[cast_text]"){
	std::string src = "これはテストです";
	const std::u16string src_u16 = u"これはテストです";
	std::u32string src_u32 = U"これはテストです";

	SECTION("std::string to std::string, using cast_text(src, dest)"){
		std::string dest;
		sftrie::cast_text(src, dest);
		CHECK(dest == src);
	}

	SECTION("std::string to std::string, using cast_text(src)"){
		auto dest = sftrie::cast_text<std::string>(src);
		CHECK(dest == src);
	}

	SECTION("std::u16string to std::u16string, using cast_text(src, dest)"){
		std::u16string dest;
		sftrie::cast_text(src_u16, dest);
		CHECK(dest == src_u16);
	}

	SECTION("std::u16string to std::u16string, using cast_text(src)"){
		auto dest = sftrie::cast_text<std::u16string>(src_u16);
		CHECK(dest == src_u16);
	}

	SECTION("std::u32string to std::u32string, using cast_text(src, dest)"){
		std::u32string dest;
		sftrie::cast_text(src_u32, dest);
		CHECK(dest == src_u32);
	}

	SECTION("std::u32string to std::u32string, using cast_text(src)"){
		auto dest = sftrie::cast_text<std::u32string>(src_u32);
		CHECK(dest == src_u32);
	}
}

TEST_CASE("cast_text/cast from std::string", "[cast_text]"){
	std::string src = "これはテストです";
	std::u16string src_u16 = u"これはテストです";
	std::u32string src_u32 = U"これはテストです";

	SECTION("std::string to std::u16string, using cast_text(src, dest)"){
		std::u16string dest_u16;
		sftrie::cast_text(src, dest_u16);
		CHECK(dest_u16 == src_u16);
	}

	SECTION("std::string to std::u16string, using cast_text(src)"){
		auto dest_u16 = sftrie::cast_text<std::u16string>(src);
		CHECK(dest_u16 == src_u16);
	}

	SECTION("std::string to std::u32string, using cast_text(src, dest)"){
		std::u32string dest_u32;
		sftrie::cast_text(src, dest_u32);
		CHECK(dest_u32 == src_u32);
	}

	SECTION("std::string to std::u32string, using cast_text(src)"){
		auto dest_u32 = sftrie::cast_text<std::u32string>(src);
		CHECK(dest_u32 == src_u32);
	}
}

TEST_CASE("cast_text/cast to std::string", "[cast_text]"){
	std::string src = "これはテストです";
	const std::u16string src_u16 = u"これはテストです";
	std::u32string src_u32 = U"これはテストです";

	SECTION("std::u16string to std::string, using cast_text(src)"){
		std::string dest_u16;
		sftrie::cast_text(src_u16, dest_u16);
		CHECK(dest_u16 == src);
	}

	SECTION("std::u16string to std::string, using cast_text(src, dest)"){
		auto dest_u16 = sftrie::cast_text<std::string>(src_u16);
		CHECK(dest_u16 == src);
	}

	SECTION("std::u32string to std::string, using cast_text(src)"){
		std::string dest_u32;
		sftrie::cast_text(src_u32, dest_u32);
		CHECK(dest_u32 == src);
	}

	SECTION("std::u32string to std::string, using cast_text(src, dest)"){
		auto dest_u32 = sftrie::cast_text<std::string>(src_u32);
		CHECK(dest_u32 == src);
	}
}

TEST_CASE("cast_text/std::u16string <=> std::u32string", "[cast_text]"){
	std::string src = "これはテストです";
	const std::u16string src_u16 = u"これはテストです";
	std::u32string src_u32 = U"これはテストです";

	SECTION("std::u16string to std::u32string, using cast_text(src)"){
		std::u32string dest_u32;
		sftrie::cast_text(src_u16, dest_u32);
		CHECK(dest_u32 == src_u32);
	}

	SECTION("std::u16string to std::u32string, using cast_text(src, dest)"){
		auto dest_u32 = sftrie::cast_text<std::u32string>(src_u16);
		CHECK(dest_u32 == src_u32);
	}

	SECTION("std::u32string to std::u16string, using cast_text(src)"){
		std::u16string dest_u16;
		sftrie::cast_text(src_u32, dest_u16);
		CHECK(dest_u16 == src_u16);
	}

	SECTION("std::u32string to std::u16string, using cast_text(src, dest)"){
		auto dest_u16 = sftrie::cast_text<std::u16string>(src_u32);
		CHECK(dest_u16 == src_u16);
	}
}
