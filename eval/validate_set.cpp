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

#include <sftrie/set.hpp>

#include "string_util.hpp"

using integer = unsigned int;

template<typename text, typename integer>
int exec(const std::string& corpus_path)
{
	using symbol = typename text::value_type;

	std::cerr << "loading texts...";
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

	std::cerr << "analyzing texts...";
	std::set<symbol> alphabet;
	symbol min_char = texts.front().front(), max_char = min_char;
	size_t total_length = 0;
	for(const auto& t: texts){
		for(auto c: t){
			alphabet.insert(c);
			min_char = std::min(min_char, c);
			max_char = std::max(max_char, c);
		}
		total_length += t.size();
	}
	std::cerr << "done." << std::endl;

	std::cerr << "generating queries...";
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(std::begin(texts), std::end(texts), std::default_random_engine(seed));
	std::vector<text> true_queries, false_queries;
	std::copy(std::begin(texts), std::begin(texts) + texts.size() / 2, std::back_inserter(true_queries));
	std::copy(std::begin(texts) + texts.size() / 2, std::end(texts), std::back_inserter(false_queries));
	texts.erase(std::begin(texts) + texts.size() / 2, std::end(texts));
	std::cerr << "done." << std::endl;

	std::cerr << "sorting texts...";
	sftrie::sort_texts(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;

	std::cerr << "constructing index...";
	sftrie::set<text, integer> index(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;

	std::cerr << "validating...";
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	for(const auto& query: true_queries)
		if(index.exists(query))
			++tp;
		else
			++fn;
	for(const auto& query: false_queries)
		if(index.exists(query))
			++fp;
		else
			++tn;
	std::cerr << "done." << std::endl << std::endl;

	std::cout << "texts:" << std::endl;
	std::cout << "  " << std::setw(25) << "alphabet size: " << std::setw(12) << alphabet.size() << std::endl;
	std::cout << "  " << std::setw(25) << "min symbol (as integer): " << std::setw(12) << static_cast<signed long long>(min_char) << std::endl;
	std::cout << "  " << std::setw(25) << "max symbol (as integer): " << std::setw(12) << static_cast<signed long long>(max_char) << std::endl;
	std::cout << "  " << std::setw(25) << "number of texts: " << std::setw(12) << texts.size() << std::endl;
	std::cout << "  " << std::setw(25) << "total length: " << std::setw(12) << total_length << std::endl;
	std::cout << "queries:" << std::endl;
	std::cout << "  " << std::setw(25) << "total queries: " << std::setw(12) << (true_queries.size() + false_queries.size()) << std::endl;
	std::cout << "  " << std::setw(25) << "true positive: " << std::setw(12) << tp << std::endl;
	std::cout << "  " << std::setw(25) << "true negative: " << std::setw(12) << tn << std::endl;
	std::cout << "  " << std::setw(25) << "false positive: " << std::setw(12) << fp << std::endl;
	std::cout << "  " << std::setw(25) << "false negative: " << std::setw(12) << fn << std::endl;

	return (tp == true_queries.size() && tn == false_queries.size() && fp == 0 && fn == 0) ? 0 : 1;
}

int main(int argc, char* argv[])
{
	init_locale();

	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " corpus" << std::endl;
		return 0;
	}

	std::string corpus_path = argv[1];
    std::string type = argc > 2 ? argv[2] : "s";
	if(type == "u16")
		return exec<std::u16string, integer>(corpus_path);
    else if(type == "w")
		return exec<std::wstring, integer>(corpus_path);
    else
		return exec<std::string, integer>(corpus_path);
}
