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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <set>

#include <random>
#include <chrono>

#include <sftrie/map.hpp>

#include "string_util.hpp"
#include "history.hpp"

using object = unsigned int;
using integer = unsigned int;

template<typename text, typename object, typename integer>
int exec(const std::string& corpus_path)
{
	History history;

	std::cerr << "loading corpus...";
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open()){
		std::cerr << "input file is not available: " << corpus_path << std::endl;
		return 1;
	}
	object value = 0;
	std::size_t total_length = 0;
	std::vector<std::pair<text, object>> texts;
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		auto t = cast_string<text>(line);
		texts.push_back(std::make_pair(t, value++));
		total_length += t.size();
	}
	std::cerr << "done." << std::endl;
	history.record("load");

	std::cerr << "sorting texts...";
	sftrie::sort_text_object_pairs(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;
	history.record("sort");

	std::cerr << "generating queries...";
	std::vector<std::pair<text, object>> queries = texts;
	std::vector<std::pair<text, object>> shuffled_queries = queries;
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(std::begin(shuffled_queries), std::end(shuffled_queries), std::default_random_engine(seed));
	std::cerr << "done." << std::endl;
	history.record("(prepare)");

	std::cerr << "constructing index...";
	sftrie::map<text, object, integer> dict(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;
	history.record("construction", texts.size());

	std::cerr << "searching (ordered)...";
	size_t found = 0;
	for(const auto& query: queries)
		if(dict.find(query.first).first)
			++found;
	std::cerr << "done." << std::endl;
	history.record("search (ordered)", queries.size());

	std::cerr << "searching (shuffled)...";
	size_t found_shuffled = 0;
	for(const auto& query: shuffled_queries)
		if(dict.find(query.first).first)
			++found_shuffled;
	std::cerr << "done." << std::endl;
	history.record("search (shuffled)", shuffled_queries.size());

	std::cout << std::endl;
	std::cout << "size:" << std::endl;
	std::cout << std::setw(16) << "# of texts: " << std::setw(16) << dict.size() << std::endl;
	std::cout << std::setw(16) << "total length: " << std::setw(16) << total_length << std::endl;
	std::cout << std::setw(16) << "index size: " << std::setw(16) << dict.space() << std::endl;
	std::cout << std::endl;
	std::cout << "time:" << std::endl;
	history.dump(std::cout, true, true);

	return found == queries.size() && found_shuffled == shuffled_queries.size();
}

int main(int argc, char* argv[])
{
	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " corpus" << std::endl;
		return 0;
	}

	std::string corpus_path = argv[1];
	std::string type = argc > 2 ? argv[2] : "s";
	if(type == "u16")
		return exec<std::u16string, object, integer>(corpus_path);
	else if(type == "u32")
		return exec<std::u32string, object, integer>(corpus_path);
	else if(type == "w")
		return exec<std::wstring, object, integer>(corpus_path);
	else
		return exec<std::string, object, integer>(corpus_path);
}
