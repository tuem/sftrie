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

#include <sftrie/map.hpp>

using text = std::string;
using item = std::uint32_t;

int main(int argc, char* argv[])
{
	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " corpus" << std::endl;
		return 0;
	}

	std::string corpus_path = argv[1];
	std::cerr << "loading...";
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open()){
		std::cerr << "input file is not available: " << corpus_path << std::endl;
		return 1;
	}
	std::vector<std::pair<text, item>> texts;
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		texts.push_back(std::make_pair(line, 0));
	}

	sftrie::sort_text_item_pairs(std::begin(texts), std::end(texts));
	sftrie::map<text, item> index(std::begin(texts), std::end(texts));
	std::cerr << "done, " << texts.size() << " texts" << std::endl;

	auto searcher = index.searcher();
	while(true){
		std::cerr << "> ";
		std::string query;
		std::getline(std::cin, query);
		if(std::cin.eof() || query == "exit" || query == "quit" || query == "bye")
			break;

		size_t count = 0;
		if(query.empty() || (query.back() != '*' && query.back() != '<')){
			// exact match
			if(searcher.exists(query)){
				count++;
				std::cout << query << ": found, count=" << ++index[query] << std::endl;
			}
		}
		else{
			auto back = query.back();
			query.pop_back();
			if(back == '*'){
				// predictive search
				for(const auto& result: searcher.predict(query)){
					index.update(result.key(), result.value() + 1);
					std::cout << std::setw(4) << ++count << ": " << result.key() << ", search count=" << result.value() << std::endl;
				}
			}
			else{
				// common-prefix search
				for(const auto& result: searcher.prefix(query)){
					index.update(result.node(), result.value() + 1);
					std::cout << std::setw(4) << ++count << ": " << result.key() << ", search count=" << result.value() << std::endl;
				}
			}
		}
		if(count == 0)
			std::cout << query << ": " << "not found" << std::endl;
	}

	return 0;
}
