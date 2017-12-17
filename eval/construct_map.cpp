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
#include <set>
#include <memory>

#include <random>
#include <chrono>

#include <sftrie/map.hpp>

#include "string_util.hpp"
#include "history.hpp"

using object = unsigned int;
using integer = unsigned int;

template<typename text, typename object, typename integer>
std::shared_ptr<sftrie::map<text, object, integer>> construct(const std::string& corpus_path)
{
	History history;

	std::cerr << "loading corpus...";
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open()){
		std::cerr << "input file is not available: " << corpus_path << std::endl;
		exit(1);
	}
	object value = 0;
	std::vector<std::pair<text, object>> texts;
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		texts.push_back(std::make_pair(cast_string<text>(line), value));
	}
	std::cerr << "done." << std::endl;
	history.record("load");

	std::cerr << "sorting texts...";
	sftrie::sort_text_object_pairs(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;
	history.record("sort");

	std::cerr << "constructing index...";
	auto dict = std::make_shared<sftrie::map<text, object, integer>>(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;
	history.record("construction", texts.size());

	std::cout << std::endl;
	history.dump(std::cout, true, true);

	return dict;
}

int main(int argc, char* argv[])
{
	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " corpus" << std::endl;
		return 0;
	}

	std::cout << "press any key to start" << std::endl;
	getchar();

	std::string corpus_path = argv[1];
	bool use_wstring = argc > 2 && std::string(argv[2]) == "w";
	if(!use_wstring){
		init_locale();
		auto dict = construct<std::string, object, integer>(corpus_path);
		std::cout << "press any key to exit" << std::endl;
		getchar();
	}
	else{
		auto dict = construct<std::wstring, object, integer>(corpus_path);
		std::cout << "press any key to exit" << std::endl;
		getchar();
	}

	return 0;
}
