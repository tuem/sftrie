/*
sftrie
https://github.com/tuem/sftrie

Copyright 2017 Takashi Uemura

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

#ifndef SFTRIE_FILE_HEADER
#define SFTRIE_FILE_HEADER

#include <cstdint>

namespace sftrie{

struct file_header
{
	char signature[4];

	std::uint16_t header_size;
	std::uint8_t major_version;
	std::uint8_t minor_version;

	std::uint8_t container_type;
	std::uint8_t index_type;
	std::uint8_t text_charset;
	std::uint8_t text_encoding;

	std::uint8_t integer_type;
	std::uint8_t node_size;
	std::uint8_t value_size;
	std::uint8_t value_type;

	std::uint64_t node_count;
	std::uint64_t tail_length;
};

};

#endif
