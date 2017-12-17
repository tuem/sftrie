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

//#define SFTRIE_USE_NAIVE
//#define SFTRIE_USE_BASIC
//#define SFTRIE_USE_TAIL
//#define SFTRIE_USE_DECOMPACTION
#include <sftrie/map.hpp>

using text = std::string;
using integer = unsigned long;
using object = unsigned long;

int main(int argc, char* argv[])
{
	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " corpus" << std::endl;
		return 0;
	}

	std::string corpus_path = argv[1];
	std::cerr << "loading...";
	std::vector<std::pair<text, object>> texts;
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open()){
		std::cerr << "input file is not available: " << corpus_path << std::endl;
		return 1;
	}
	object value = 0;
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		texts.push_back(std::make_pair(line, value++));
	}

	sftrie::sort_text_object_pairs(std::begin(texts), std::end(texts));
	sftrie::map<text, integer, object> dict(std::begin(texts), std::end(texts));
	texts.clear();
	std::cerr << "done." << std::endl;

	while(true){
		std::cerr << "> ";
		std::string query;
		std::getline(std::cin, query);
		if(std::cin.eof() || query == "exit" || query == "quit" || query == "bye")
			break;
		else if(query.empty())
			continue;

		auto result = dict.find(query);
		if(result.first)
			std::cout << query << ": " << "found, value=" << result.second << std::endl;
		else
			std::cout << query << ": " << "not found" << std::endl;
	}

	return 0;
}