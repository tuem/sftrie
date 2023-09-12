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

#ifndef SFTRIE_TEST_UTIL
#define SFTRIE_TEST_UTIL

#include <string>
#include <codecvt>

#include <sftrie/util.hpp>

template<typename text>
std::vector<text> cast_strings(const std::vector<std::string>& texts)
{
	std::vector<text> results;
	for(const auto& t: texts)
		results.push_back(sftrie::cast_string<text>(t));
	return results;
}

template<typename text, typename item>
std::vector<std::pair<text, item>> cast_strings(const std::vector<std::pair<std::string, item>>& texts)
{
	std::vector<std::pair<text, item>> results;
	for(const auto& i: texts)
		results.push_back({sftrie::cast_string<text>(i.first), i.second});
	return results;
}

template<typename text, typename item>
std::vector<std::pair<text, item>> assign_ids(const std::vector<text>& texts, item start = static_cast<item>(1))
{
	std::vector<std::pair<text, item>> results;
	auto i = start;
	for(const auto& t: texts)
		results.push_back({t, ++i});
	return results;
}

#endif
