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

#ifndef SFTRIE_SET_HPP
#define SFTRIE_SET_HPP

#include <string>

#if defined SFTRIE_USE_DECOMPACTION
	#include "set_decompaction.hpp"
	namespace sftrie{
		template<typename text = std::string, typename integer = typename text::size_type>
		using set = set_decompaction<text, integer>;
	};
#elif defined SFTRIE_USE_TAIL
	#include "set_tail.hpp"
	namespace sftrie{
		template<typename text = std::string, typename integer = typename text::size_type>
		using set = set_tail<text, integer>;
	};
#elif defined SFTRIE_USE_BASIC
	#include "set_basic.hpp"
	namespace sftrie{
		template<typename text = std::string, typename integer = typename text::size_type>
		using set = set_basic<text, integer>;
	};
#elif defined SFTRIE_USE_NAIVE
	#include "set_naive.hpp"
	namespace sftrie{
		template<typename text = std::string, typename integer = typename text::size_type>
		using set = set_naive<text, integer>;
	};
#else
	#include "set_decompaction.hpp"
	namespace sftrie{
		template<typename text = std::string, typename integer = typename text::size_type>
		using set = set_decompaction<text, integer>;
	};
#endif

#endif
