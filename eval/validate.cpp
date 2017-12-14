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

using text = std::string;
using symbol = typename text::value_type;
using integer = unsigned long;

int main(int argc, char* argv[])
{
	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " corpus" << std::endl;
		return 0;
	}

	std::string corpus_path = argv[1];
	std::cerr << "loading...";
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
		texts.push_back(line);
	}
	std::cerr << "done." << std::endl;

	integer total_length = 0;
    symbol min_char = texts.front().front(), max_char = min_char;
    for(const auto& text: texts){
		total_length += container_size<integer>(text);
        for(auto c: text){
            min_char = std::min(min_char, c);
            max_char = std::max(max_char, c);
        }
    }
	std::cout << "total length: " << total_length << std::endl;
	std::cout << "min symbol(int): " << static_cast<signed long long>(min_char) << std::endl;
	std::cout << "max symbol(int): " << static_cast<signed long long>(max_char) << std::endl;

	std::cerr << "generating queries...";
	std::vector<text> all_queries = texts;
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(std::begin(all_queries), std::end(all_queries), std::default_random_engine(seed));
	std::vector<text> true_queries, false_queries;
    std::copy(std::begin(all_queries), std::begin(all_queries) + texts.size() / 2, std::back_inserter(true_queries));
    std::copy(std::begin(all_queries) + texts.size() / 2, std::end(all_queries), std::back_inserter(false_queries));
    texts = all_queries;
    texts.erase(std::begin(texts) + texts.size() / 2, std::end(texts));

    seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(std::begin(true_queries), std::end(true_queries), std::default_random_engine(seed));
    seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(std::begin(false_queries), std::end(false_queries), std::default_random_engine(seed));

	std::vector<text> shuffled_true_queries = true_queries, shuffled_false_queries = false_queries;
	sort_sftrie_texts(std::begin(true_queries), std::end(true_queries));
	sort_sftrie_texts(std::begin(false_queries), std::end(false_queries));
	std::cerr << "done." << std::endl;

	std::cerr << "sorting texts...";
	sort_sftrie_texts(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;

	std::cerr << "constructing index...";
	sftrie<text, integer> trie(texts);
	std::cerr << "done." << std::endl;

	std::cerr << "validating...";
    size_t tp, tn, fp, fn;
    tp = tn = fp = fn = 0;
    for(const auto& query: true_queries)
        if(trie.exists(query))
            ++tp;
        else
            ++fn;
    for(const auto& query: false_queries)
        if(trie.exists(query))
            ++fp;
        else
            ++tn;
	std::cerr << "done." << std::endl;

	std::cout << "total queries: " << all_queries.size() << std::endl;
	std::cout << "true positive: " << tp << std::endl;
	std::cout << "true negative: " << tn << std::endl;
	std::cout << "false positive: " << fp << std::endl;
	std::cout << "false negative: " << fn << std::endl;
    
	return (tp == true_queries.size() && tn == false_queries.size() && fp == 0 && fn == 0) ? 0 : 1;
}
