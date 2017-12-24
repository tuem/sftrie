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
#include <map>

#include <random>
#include <chrono>

#include <sftrie/map_naive.hpp>
#include <sftrie/map_basic.hpp>
#include <sftrie/map_tail.hpp>
#include <sftrie/map_decompaction.hpp>
#include <paramset.hpp>

#include "string_util.hpp"

using object = unsigned int;
using integer = unsigned int;

template<typename text_object_pair, typename map>
std::map<std::string, size_t> evaluate(const map& dict,
	const std::vector<text_object_pair>& true_queries, const std::vector<text_object_pair>& false_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	std::cerr << "validating...";
	for(const auto& query: true_queries){
		auto result = dict.find(query.first);
		if(result.first && result.second == query.second)
			++tp;
		else
			++fn;
	}
	for(const auto& query: false_queries){
		auto result = dict.find(query.first);
		if(!result.first)
			++tn;
		else
			++fp;
	}
	std::cerr << "done." << std::endl;
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename object, typename integer>
int exec(const std::string& corpus_path, const std::string& sftrie_type,
	int min_binary_search, int min_tail, int min_decompaction)
{
	using symbol = typename text::value_type;

	std::cerr << "loading texts...";
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open()){
		std::cerr << "input file is not available: " << corpus_path << std::endl;
		return 1;
	}
	object value = 0;
	std::vector<std::pair<text, object>> texts;
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		texts.push_back(std::make_pair(cast_string<text>(line), value++));
	}
	std::cerr << "done." << std::endl;

	std::cerr << "analyzing texts...";
	std::set<symbol> alphabet;
	symbol min_char = texts.front().first.front(), max_char = min_char;
	size_t total_length = 0;
	for(const auto& p: texts){
		for(auto c: p.first){
			alphabet.insert(c);
			min_char = std::min(min_char, c);
			max_char = std::max(max_char, c);
		}
		total_length += p.first.size();
	}
	std::cerr << "done." << std::endl;

	std::cerr << "generating queries...";
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(std::begin(texts), std::end(texts), std::default_random_engine(seed));
	std::vector<std::pair<text, object>> true_queries, false_queries;
	std::copy(std::begin(texts), std::begin(texts) + texts.size() / 2, std::back_inserter(true_queries));
	std::copy(std::begin(texts) + texts.size() / 2, std::end(texts), std::back_inserter(false_queries));
	texts.erase(std::begin(texts) + texts.size() / 2, std::end(texts));
	std::cerr << "done." << std::endl;

	std::cerr << "sorting texts...";
	sftrie::sort_text_object_pairs(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;

	std::map<std::string, size_t> result;
	if(sftrie_type == "naive"){
		std::cerr << "constructing index...";
		sftrie::map_naive<text, object, integer> dict(std::begin(texts), std::end(texts));
		std::cerr << "done." << std::endl;
		result = evaluate(dict, true_queries, false_queries);
	}
	else if(sftrie_type == "basic"){
		std::cerr << "constructing index...";
		sftrie::map_basic<text, object, integer> dict(std::begin(texts), std::end(texts),
			min_binary_search);
		std::cerr << "done." << std::endl;
		result = evaluate(dict, true_queries, false_queries);
	}
	else if(sftrie_type == "tail"){
		std::cerr << "constructing index...";
		sftrie::map_tail<text, object, integer> dict(std::begin(texts), std::end(texts),
			min_binary_search, min_tail);
		std::cerr << "done." << std::endl;
		result = evaluate(dict, true_queries, false_queries);
	}
	else if(sftrie_type == "decompaction"){
		std::cerr << "constructing index...";
		sftrie::map_decompaction<text, object, integer> dict(std::begin(texts), std::end(texts),
			min_binary_search, min_tail, min_decompaction);
		std::cerr << "done." << std::endl;
		result = evaluate(dict, true_queries, false_queries);
	}
	size_t tp = result["tp"], tn = result["tn"], fp = result["fp"], fn = result["fn"];

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
	paramset::definitions defs = {
		{"symbol_type", "char", "map-symbol-type", 's', "symbol type"},
		{"sftrie_type", "naive", "map-sftrie-type", 't', "sftrie type"},
		{"min_binary_search", 42, {"map", "min_binary_search"}, "map-min-binary-search", 0, "minumum number of children for binary search"},
		{"min_tail", 1, {"map", "min_tail"}, "map-min-tail", 0, "minumum length to copress tail strings"},
		{"min_decompaction", 0, {"map", "min_decompaction"}, "map-min-decompaction", 0, "minumum number of children to enable decompaction"},
		{"conf_path", "", "config", 'c', "config file path"}
	};
	paramset::manager pm(defs);

	try{
		pm.load(argc, argv, "config");

		if(!pm.rest.empty())
			pm["corpus_path"] = pm.rest.front().as<std::string>();

		std::string corpus_path = pm["corpus_path"];
		std::string symbol_type = pm["symbol_type"];
		std::string sftrie_type = pm["sftrie_type"];
		int min_binary_search = pm["min_binary_search"];
		int min_tail = pm["min_tail"];
		int min_decompaction = pm["min_decompaction"];

		std::cerr << "Configuration" << std::endl;
		std::cerr << std::setw(12) << std::left << "  symbol_type: " << symbol_type << std::endl;
		std::cerr << std::setw(12) << std::left << "  sftrie_type: " << sftrie_type << std::endl;
		std::cerr << std::setw(12) << std::left << "  min_binary_search: " << min_binary_search << std::endl;
		std::cerr << std::setw(12) << std::left << "  min_tail: " << min_tail << std::endl;
		std::cerr << std::setw(12) << std::left << "  min_decompaction: " << min_decompaction << std::endl;
		std::cerr << std::endl;

		if(symbol_type == "char")
			return exec<std::string, object, integer>(corpus_path, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "wchar_t")
			return exec<std::wstring, object, integer>(corpus_path, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char16_t")
			return exec<std::u16string, object, integer>(corpus_path, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char32_t")
			return exec<std::u32string, object, integer>(corpus_path, sftrie_type, min_binary_search, min_tail, min_decompaction);
	}
	catch(const std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		exit(1);
	}
}
