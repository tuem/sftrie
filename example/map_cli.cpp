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

#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <sftrie/map.hpp>
#include <sftrie/util.hpp>

using text = std::string;
using item = std::uint32_t;
using index_type = sftrie::map<text, item>;

template<typename index_type>
void exec(index_type& index)
{
	auto searcher = index.searcher();
	while(true){
		std::cerr << "> ";
		std::string query;
		std::getline(std::cin, query);
		if(std::cin.eof() || query == "exit" || query == "quit" || query == "bye"){
			break;
		}
		else if(query.substr(0, 5) == "save="){
			std::string output_path = query.substr(5);
			index.save(output_path);
			std::cout << "index saved to " << output_path << std::endl;
			continue;
		}


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
					if(!index.update(result.node(), result.value() + 1))
						std::cerr << "update failed" << std::endl;
					std::cout << std::setw(4) << ++count << ": " << result.key() << ", search count=" << result.value() << std::endl;
				}
			}
		}
		if(count == 0)
			std::cout << query << ": " << "not found" << std::endl;
	}
}

int main(int argc, char* argv[])
{
	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " input_path [load_index=false]" << std::endl;
		return 0;
	}

	std::string input_path = argv[1];
	bool load_index = argc > 2 && std::string(argv[2]) == "true";

	if(load_index){
		std::cerr << "loadinag index...";
		index_type index(input_path);
		std::cerr << "done." << std::endl;
		exec(index);
	}
	else{
		std::cerr << "loading texts...";
		std::ifstream ifs(input_path);
		if(!ifs.is_open()){
			std::cerr << "input file is not available: " << input_path << std::endl;
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

		sftrie::sort_text_item_pairs(texts.begin(), texts.end());
		std::cerr << "building index...";
		index_type index(texts.begin(), texts.end());
		std::cerr << "done, " << texts.size() << " texts" << std::endl;
		exec(index);
	}

	return 0;
}
