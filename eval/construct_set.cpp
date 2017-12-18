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

#include <stdio.h>

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include <sftrie/set.hpp>

#include "string_util.hpp"

using integer = unsigned int;

template<typename text, typename integer>
std::shared_ptr<sftrie::set<text, integer>> exec(const std::string& corpus_path)
{
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open()){
		std::cerr << "input file is not available: " << corpus_path << std::endl;
		return nullptr;
	}
	std::vector<text> texts;
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		texts.push_back(cast_string<text>(line));
	}
	sftrie::sort_texts(std::begin(texts), std::end(texts));
	return std::make_shared<sftrie::set<text, integer>>(std::begin(texts), std::end(texts));
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

	std::cout << "press any key to construct" << std::flush;
	getchar();

	std::size_t space;
	if(type == "u16"){
		auto index = exec<std::u16string, integer>(corpus_path);
		space = index->space();
		std::cout << "press any key to destruct" << std::flush;
		getchar();
	}
    else if(type == "u32"){
		auto index = exec<std::u32string, integer>(corpus_path);
		space = index->space();
		std::cout << "press any key to destruct" << std::flush;
		getchar();
	}
    else if(type == "w"){
		auto index = exec<std::wstring, integer>(corpus_path);
		space = index->space();
		std::cout << "press any key to destruct" << std::flush;
		getchar();
	}
    else{
		auto index = exec<std::string, integer>(corpus_path);
		space = index->space();
		std::cout << "press any key to destruct" << std::flush;
		getchar();
	}

	std::cout << "press any key to exit" << std::flush;
	getchar();

	std::cout << "size: " << space << std::endl;

	return 0;
}
