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

#ifndef SFTRIE_MAP
#define SFTRIE_MAP

#include <cstdint>
#include <string>

#include "map_original.hpp"
#include "map_compact.hpp"

#if defined SFTRIE_MAP_USE_ORIGINAL
	#define SFTRIE_MAP_TYPE map_original
#elif defined SFTRIE_MAP_USE_COMPACT
	#define SFTRIE_MAP_TYPE map_compact
#else
	#define SFTRIE_MAP_TYPE map_compact
#endif

namespace sftrie{
	template<typename text = std::string, typename item = std::uint32_t,
		typename integer = std::uint32_t>
	using map = SFTRIE_MAP_TYPE<text, item, integer>;
};

#undef SFTRIE_MAP_TYPE

#endif
