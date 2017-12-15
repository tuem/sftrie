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
#include <fstream>
#include <string>
#include <set>

#include <random>
#include <chrono>

//#define SFTRIE_USE_SIMPLE
//#define SFTRIE_USE_TAIL
//#define SFTRIE_USE_DECOMPACTION
#include <sftrie.hpp>

#include "string_util.hpp"
#include "history.hpp"

using integer = unsigned int;

template<typename text, typename integer>
int benchmark(const std::string& corpus_path)
{
	History history;

	std::cerr << "loading corpus...";
	std::vector<text> texts;
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open()){
		std::cerr << "input file is not available: " << corpus_path << std::endl;
		return 1;
	}
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		texts.push_back(cast_string<text>(line));
	}
	std::cerr << "done." << std::endl;
	history.record("load");

	std::cerr << "sorting texts...";
	sort_sftrie_texts(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;
	history.record("sort");

	std::cerr << "generating queries...";
	std::vector<text> queries = texts;
	std::vector<text> shuffled_queries = queries;
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(std::begin(shuffled_queries), std::end(shuffled_queries), std::default_random_engine(seed));
	std::cerr << "done." << std::endl;
	history.record("(prepare)");

	std::cerr << "constructing index...";
	sftrie<text, integer> trie(texts);
	std::cerr << "done." << std::endl;
	history.record("construction", texts.size());

	std::cerr << "searching (ordered)...";
	size_t found = 0;
	for(const auto& query: queries)
		if(trie.exists(query))
			++found;
	std::cerr << "done." << std::endl;
	history.record("search (ordered)", queries.size());

	std::cerr << "searching (shuffled)...";
	size_t found_shuffled = 0;
	for(const auto& query: shuffled_queries)
		if(trie.exists(query))
			++found_shuffled;
	std::cerr << "done." << std::endl;
	history.record("search (shuffled)", shuffled_queries.size());

	std::cout << std::endl;
	history.dump(std::cout, true, true);

	return found == queries.size() && found_shuffled == shuffled_queries.size();
}

int main(int argc, char* argv[])
{
	init_locale();

	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " corpus" << std::endl;
		return 0;
	}

	std::string corpus_path = argv[1];
	bool use_wstring = argc > 2 && std::string(argv[2]) == "wstring"; // TODO: improve
	return use_wstring ?
		benchmark<std::wstring, integer>(corpus_path) :
		benchmark<std::string, integer>(corpus_path);
}
