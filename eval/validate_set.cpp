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

#include <sftrie/set_naive.hpp>
#include <sftrie/set_basic.hpp>
#include <sftrie/set_tail.hpp>
#include <sftrie/set_decompaction.hpp>
#include <paramset.hpp>

#include "string_util.hpp"

using integer = unsigned int;

template<typename text, typename set>
std::map<std::string, size_t> evaluate(const set& index,
	const std::vector<text>& true_queries, const std::vector<text>& false_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	std::cerr << "validating...";
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
	std::cerr << "done." << std::endl;
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename set>
std::map<std::string, size_t> evaluate_prefix_search(const set& index,
	const std::vector<text>& true_queries,
	const std::vector<integer>& common_prefix_borders,
	const std::vector<text>& false_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	std::cerr << "validating...";
	for(size_t i = 0; i < common_prefix_borders.size(); ++i){
		size_t answer_start = i, answer_end = common_prefix_borders[i];
		bool correct = true;
		size_t j = answer_start, count = 0;
		for(const auto& result: index.prefix(true_queries[answer_start])){
			if(result != true_queries[j]){
				correct = false;
				break;
			}
			++j;
			++count;
		}
		if(correct && count < answer_end - answer_start)
			correct = false;

		if(correct)
			++tp;
		else
			++fn;
	}
	for(const auto& query: false_queries){
		bool correct = true;
		for(const auto& result: index.prefix(query)){
			if(result == query){
				correct = false;
				break;
			}
		}
		if(correct)
			++tn;
		else
			++fp;
	}
	std::cerr << "done." << std::endl;
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename integer>
int exec(const std::string& corpus_path, const std::string& sftrie_type,
	int min_binary_search, int min_tail, int min_decompaction)
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

	std::cerr << "generating queries for prefix search...";
	std::vector<text> prefix_search_queries = true_queries;
	std::vector<integer> common_prefix_borders;
	sftrie::sort_texts(std::begin(prefix_search_queries), std::end(prefix_search_queries));
	for(size_t i = 0; i < prefix_search_queries.size(); ++i){
		size_t end = i + 1;
		while(end < prefix_search_queries.size() &&
				std::equal(std::begin(prefix_search_queries[i]), std::end(prefix_search_queries[i]), std::begin(prefix_search_queries[end])))
			++end;
		common_prefix_borders.push_back(end);
	}
	std::cerr << "done." << std::endl;

	std::cerr << "sorting texts...";
	sftrie::sort_texts(std::begin(texts), std::end(texts));
	std::cerr << "done." << std::endl;

	std::map<std::string, size_t> result;
	std::map<std::string, size_t> result_ps;
	if(sftrie_type == "naive"){
		std::cerr << "constructing index...";
		sftrie::set_naive<text, integer> index(std::begin(texts), std::end(texts));
		std::cerr << "done." << std::endl;
		result = evaluate(index, true_queries, false_queries);
		result_ps = evaluate_prefix_search(index, prefix_search_queries, common_prefix_borders, false_queries);
	}
	else if(sftrie_type == "basic"){
		std::cerr << "constructing index...";
		sftrie::set_basic<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search);
		std::cerr << "done." << std::endl;
		result = evaluate(index, true_queries, false_queries);
		result_ps = evaluate_prefix_search(index, prefix_search_queries, common_prefix_borders, false_queries);
	}
	else if(sftrie_type == "tail"){
		std::cerr << "constructing index...";
		sftrie::set_tail<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search, min_tail);
		std::cerr << "done." << std::endl;
		result = evaluate(index, true_queries, false_queries);
		result_ps = evaluate_prefix_search(index, prefix_search_queries, common_prefix_borders, false_queries);
	}
	else if(sftrie_type == "decompaction"){
		std::cerr << "constructing index...";
		sftrie::set_decompaction<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search, min_tail, min_decompaction);
		std::cerr << "done." << std::endl;
		result = evaluate(index, true_queries, false_queries);
		result_ps = evaluate_prefix_search(index, prefix_search_queries, common_prefix_borders, false_queries);
	}
	else{
		throw std::runtime_error("unknown trie type: " + sftrie_type);
	}
	size_t tp = result["tp"], tn = result["tn"], fp = result["fp"], fn = result["fn"];
	size_t tp_ps = result_ps["tp"], tn_ps = result_ps["tn"], fp_ps = result_ps["fp"], fn_ps = result_ps["fn"];

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
	std::cout << "prefix search queries:" << std::endl;
	std::cout << "  " << std::setw(25) << "total queries: " << std::setw(12) << (prefix_search_queries.size() + false_queries.size()) << std::endl;
	std::cout << "  " << std::setw(25) << "true positive: " << std::setw(12) << tp_ps << std::endl;
	std::cout << "  " << std::setw(25) << "true negative: " << std::setw(12) << tn_ps << std::endl;
	std::cout << "  " << std::setw(25) << "false positive: " << std::setw(12) << fp_ps << std::endl;
	std::cout << "  " << std::setw(25) << "false negative: " << std::setw(12) << fn_ps << std::endl;

	return (tp == true_queries.size() && tn == false_queries.size() && fp == 0 && fn == 0) ? 0 : 1;
}

int main(int argc, char* argv[])
{
	paramset::definitions defs = {
		{"symbol_type", "char", "set-symbol-type", 's', "symbol type"},
		{"sftrie_type", "naive", "set-sftrie-type", 't', "sftrie type"},
		{"min_binary_search", 42, {"set", "min_binary_search"}, "set-min-binary-search", 0, "minumum number of children for binary search"},
		{"min_tail", 1, {"set", "min_tail"}, "set-min-tail", 0, "minumum length to copress tail strings"},
		{"min_decompaction", 0, {"set", "min_decompaction"}, "set-min-decompaction", 0, "minumum number of children to enable decompaction"},
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
			return exec<std::string, integer>(corpus_path, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "wchar_t")
			return exec<std::wstring, integer>(corpus_path, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char16_t")
			return exec<std::u16string, integer>(corpus_path, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char32_t")
			return exec<std::u32string, integer>(corpus_path, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else
			throw std::runtime_error("unknown symbol type: " + symbol_type);
	}
	catch(const std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		exit(1);
	}
}
