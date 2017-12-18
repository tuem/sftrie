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

#ifndef SFTRIE_MAP_HPP
#define SFTRIE_MAP_HPP

#include <string>

#if defined SFTRIE_MAP_USE_DECOMPACTION
	#include "map_decompaction.hpp"
	namespace sftrie{
		template<typename text = std::string, typename object = typename text::size_type, typename integer = typename text::size_type>
		using map = map_decompaction<text, object, integer>;
	};
#elif defined SFTRIE_MAP_USE_TAIL
	#include "map_tail.hpp"
	namespace sftrie{
		template<typename text = std::string, typename object = typename text::size_type, typename integer = typename text::size_type>
		using map = map_tail<text, object, integer>;
	};
#elif defined SFTRIE_MAP_USE_BASIC
	#include "map_basic.hpp"
	namespace sftrie{
		template<typename text = std::string, typename object = typename text::size_type, typename integer = typename text::size_type>
		using map = map_basic<text, object, integer>;
	};
#elif defined SFTRIE_MAP_USE_NAIVE
	#include "map_naive.hpp"
	namespace sftrie{
		template<typename text = std::string, typename object = typename text::size_type, typename integer = typename text::size_type>
		using map = map_naive<text, object, integer>;
	};
#else
	#include "map_decompaction.hpp"
	namespace sftrie{
		template<typename text = std::string, typename object = typename text::size_type, typename integer = typename text::size_type>
		using map = map_decompaction<text, object, integer>;
	};
#endif

#endif
