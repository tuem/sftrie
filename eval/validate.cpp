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

#include <sftrie/set.hpp>
#include <sftrie/map.hpp>
#include <paramset.hpp>

#include "string_util.hpp"

using object = unsigned int;
using integer = unsigned int;

template<typename text, typename set>
std::map<std::string, size_t> validate_set_exact_match(const set& index,
	const std::vector<text>& positive_queries, const std::vector<text>& negative_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	for(const auto& query: positive_queries)
		if(index.exists(query))
			++tp;
		else
			++fn;
	for(const auto& query: negative_queries)
		if(index.exists(query))
			++fp;
		else
			++tn;
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename set>
std::map<std::string, size_t> validate_set_prefix_search(const set& index,
	const std::vector<text>& queries)
{
	size_t tp = 0, fp = 0, fn = 0;
	auto searcher = index.searcher();
	for(const auto& query: queries){
		std::vector<text> answers;
		text answer = query;
		while(!answer.empty()){
			if(index.exists(answer))
				answers.push_back(answer);
			answer.pop_back();
		}
		if(index.exists(answer))
			answers.push_back(answer);
		std::reverse(std::begin(answers), std::end(answers));

		integer found = 0;
		auto a = std::begin(answers);
		for(const auto& result: searcher.prefix(query)){
			if(a == std::end(answers)){
				++fn;
				break;
			}
			else{
				if(result != *a)
					++fp;
				else
					++found;
				++a;
			}
		}
		if(found == answers.size())
			++tp;
	}
	return {{"tp", tp}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename set>
std::map<std::string, size_t> validate_set_predictive_search(const set& index,
	const std::vector<text>& positive_queries,
	const std::vector<integer>& traversal_borders,
	const std::vector<text>& negative_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	auto searcher = index.searcher();
	for(size_t i = 0; i < traversal_borders.size(); ++i){
		size_t answer_start = i, answer_end = traversal_borders[i];
		bool correct = true;
		size_t j = answer_start, count = 0;
		for(const auto& result: searcher.traverse(positive_queries[answer_start])){
			if(result != positive_queries[j]){
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
	for(const auto& query: negative_queries){
		bool correct = true;
		for(const auto& result: searcher.traverse(query)){
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
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text_object_pair, typename map>
std::map<std::string, size_t> validate_map_exact_match(const map& dict,
	const std::vector<text_object_pair>& positive_queries, const std::vector<text_object_pair>& negative_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	for(const auto& query: positive_queries){
		auto result = dict.find(query.first);
		if(result.first && result.second == query.second)
			++tp;
		else
			++fn;
	}
	for(const auto& query: negative_queries){
		auto result = dict.find(query.first);
		if(!result.first)
			++tn;
		else
			++fp;
	}
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename map>
std::map<std::string, size_t> validate_map_prefix_search(const map& dict,
	const std::vector<text>& queries)
{
	size_t tp = 0, fp = 0, fn = 0;
	auto searcher = dict.searcher();
	for(const auto& query: queries){
		std::vector<text> answers;
		text answer = query;
		while(!answer.empty()){
			if(dict.find(answer).first)
				answers.push_back(answer);
			answer.pop_back();
		}
		if(dict.find(answer).first)
			answers.push_back(answer);
		std::reverse(std::begin(answers), std::end(answers));

		integer found = 0;
		auto a = std::begin(answers);
		for(const auto& result: searcher.prefix(query)){
			if(a == std::end(answers)){
				++fn;
				break;
			}
			else{
				if(result.first != *a)
					++fp;
				else
					++found;
				++a;
			}
		}
		if(found == answers.size())
			++tp;
	}
	return {{"tp", tp}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename object, typename map>
std::map<std::string, size_t> validate_map_predictive_search(const map& dict,
	const std::vector<std::pair<text, object>>& positive_queries,
	const std::vector<integer>& traversal_borders,
	const std::vector<std::pair<text, object>>& negative_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	auto searcher = dict.searcher();
	for(size_t i = 0; i < traversal_borders.size(); ++i){
		size_t answer_start = i, answer_end = traversal_borders[i];
		bool correct = true;
		size_t j = answer_start, count = 0;
		for(const auto& result: searcher.traverse(positive_queries[answer_start].first)){
			if(result.first != positive_queries[j].first || result.second != positive_queries[j].second){
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
	for(const auto& query: negative_queries){
		bool correct = true;
		for(const auto& result: searcher.traverse(query.first)){
			if(result.first == query.first){
				correct = false;
				break;
			}
		}

		if(correct)
			++tn;
		else
			++fp;
	}
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename object, typename integer>
void exec(const std::string& corpus_path, const std::string& index_type,
	const std::string& sftrie_type, int min_binary_search, int min_tail, int min_decompaction)
{
	using symbol = typename text::value_type;

	std::cerr << "loading texts...";
	std::vector<text> texts;
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open())
		throw std::runtime_error("input file is not available: " + corpus_path);
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		texts.push_back(cast_string<text>(line));
	}
	std::vector<std::pair<text, object>> text_object_pairs;
	if(index_type == "map"){
		object value = 0;
		for(const auto& t: texts)
			text_object_pairs.push_back(std::make_pair(t, value++));
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

	std::vector<text> all_queries = texts;
	std::vector<text> set_positive_queries, set_negative_queries;
	std::vector<text> set_predictive_search_queries;
	std::vector<integer> set_traversal_borders;

	std::vector<std::pair<text, object>> map_positive_queries, map_negative_queries;
	std::vector<std::pair<text, object>> map_predictive_search_queries;
	std::vector<integer> map_traversal_borders;

	size_t positive_queries_size, negative_queries_size, predictive_search_queries_size;
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	if(index_type == "set"){
		std::shuffle(std::begin(texts), std::end(texts), std::default_random_engine(seed));
		std::copy(std::begin(texts), std::begin(texts) + texts.size() / 2, std::back_inserter(set_positive_queries));
		std::copy(std::begin(texts) + texts.size() / 2, std::end(texts), std::back_inserter(set_negative_queries));
		texts.erase(std::begin(texts) + texts.size() / 2, std::end(texts));

		set_predictive_search_queries = set_positive_queries;
		sftrie::sort_texts(std::begin(set_predictive_search_queries), std::end(set_predictive_search_queries));
		for(size_t i = 0; i < set_predictive_search_queries.size(); ++i){
			size_t end = i + 1;
			while(end < set_predictive_search_queries.size() &&
					std::equal(std::begin(set_predictive_search_queries[i]), std::end(set_predictive_search_queries[i]), std::begin(set_predictive_search_queries[end])))
				++end;
			set_traversal_borders.push_back(end);
		}

		sftrie::sort_texts(std::begin(texts), std::end(texts));

		positive_queries_size = set_positive_queries.size();
		negative_queries_size = set_negative_queries.size();
		predictive_search_queries_size = set_predictive_search_queries.size();
	}
	else{
		std::shuffle(std::begin(text_object_pairs), std::end(text_object_pairs), std::default_random_engine(seed));
		std::copy(std::begin(text_object_pairs), std::begin(text_object_pairs) + text_object_pairs.size() / 2, std::back_inserter(map_positive_queries));
		std::copy(std::begin(text_object_pairs) + text_object_pairs.size() / 2, std::end(text_object_pairs), std::back_inserter(map_negative_queries));
		text_object_pairs.erase(std::begin(text_object_pairs) + text_object_pairs.size() / 2, std::end(text_object_pairs));

		map_predictive_search_queries = map_positive_queries;
		sftrie::sort_text_object_pairs(std::begin(map_predictive_search_queries), std::end(map_predictive_search_queries));
		for(size_t i = 0; i < map_predictive_search_queries.size(); ++i){
			size_t end = i + 1;
			while(end < map_predictive_search_queries.size() &&
					std::equal(std::begin(map_predictive_search_queries[i].first), std::end(map_predictive_search_queries[i].first), std::begin(map_predictive_search_queries[end].first)))
				++end;
			map_traversal_borders.push_back(end);
		}

		sftrie::sort_text_object_pairs(std::begin(text_object_pairs), std::end(text_object_pairs));

		positive_queries_size = map_positive_queries.size();
		negative_queries_size = map_negative_queries.size();
		predictive_search_queries_size = map_predictive_search_queries.size();
	}
	std::cerr << "done." << std::endl;

	std::map<std::string, size_t> result_exact;
	std::map<std::string, size_t> result_prefix;
	std::map<std::string, size_t> result_traversal;
	if(index_type == "set" && sftrie_type == "naive"){
		std::cerr << "constructing index...";
		sftrie::set_naive<text, integer> index(std::begin(texts), std::end(texts));
		std::cerr << "done." << std::endl;
		std::cerr << "validating...";
		result_exact = validate_set_exact_match(index, set_positive_queries, set_negative_queries);
		result_prefix = validate_set_prefix_search(index, all_queries);
		result_traversal = validate_set_predictive_search(index, set_predictive_search_queries, set_traversal_borders, set_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "set" && sftrie_type == "basic"){
		std::cerr << "constructing index...";
		sftrie::set_basic<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search);
		std::cerr << "done." << std::endl;
		std::cerr << "validating...";
		result_exact = validate_set_exact_match(index, set_positive_queries, set_negative_queries);
		result_prefix = validate_set_prefix_search(index, all_queries);
		result_traversal = validate_set_predictive_search(index, set_predictive_search_queries, set_traversal_borders, set_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "set" && sftrie_type == "tail"){
		std::cerr << "constructing index...";
		sftrie::set_tail<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search, min_tail);
		std::cerr << "done." << std::endl;
		std::cerr << "validating...";
		result_exact = validate_set_exact_match(index, set_positive_queries, set_negative_queries);
		result_prefix = validate_set_prefix_search(index, all_queries);
		result_traversal = validate_set_predictive_search(index, set_predictive_search_queries, set_traversal_borders, set_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "set" && sftrie_type == "decompaction"){
		std::cerr << "constructing index...";
		sftrie::set_decompaction<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search, min_tail, min_decompaction);
		std::cerr << "done." << std::endl;
		std::cerr << "validating...";
		result_exact = validate_set_exact_match(index, set_positive_queries, set_negative_queries);
		result_prefix = validate_set_prefix_search(index, all_queries);
		result_traversal = validate_set_predictive_search(index, set_predictive_search_queries, set_traversal_borders, set_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && sftrie_type == "naive"){
		std::cerr << "constructing index...";
		sftrie::map_naive<text, object, integer> dict(std::begin(text_object_pairs), std::end(text_object_pairs));
		std::cerr << "done." << std::endl;
		std::cerr << "validating...";
		result_exact = validate_map_exact_match(dict, map_positive_queries, map_negative_queries);
		result_prefix = validate_map_prefix_search(dict, all_queries);
		result_traversal = validate_map_predictive_search(dict, map_predictive_search_queries, map_traversal_borders, map_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && sftrie_type == "basic"){
		std::cerr << "constructing index...";
		sftrie::map_basic<text, object, integer> dict(std::begin(text_object_pairs), std::end(text_object_pairs),
			min_binary_search);
		std::cerr << "done." << std::endl;
		std::cerr << "validating...";
		result_exact = validate_map_exact_match(dict, map_positive_queries, map_negative_queries);
		result_prefix = validate_map_prefix_search(dict, all_queries);
		result_traversal = validate_map_predictive_search(dict, map_predictive_search_queries, map_traversal_borders, map_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && sftrie_type == "tail"){
		std::cerr << "constructing index...";
		sftrie::map_tail<text, object, integer> dict(std::begin(text_object_pairs), std::end(text_object_pairs),
			min_binary_search, min_tail);
		std::cerr << "done." << std::endl;
		std::cerr << "validating...";
		result_exact = validate_map_exact_match(dict, map_positive_queries, map_negative_queries);
		result_prefix = validate_map_prefix_search(dict, all_queries);
		result_traversal = validate_map_predictive_search(dict, map_predictive_search_queries, map_traversal_borders, map_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && sftrie_type == "decompaction"){
		std::cerr << "constructing index...";
		sftrie::map_decompaction<text, object, integer> dict(std::begin(text_object_pairs), std::end(text_object_pairs),
			min_binary_search, min_tail, min_decompaction);
		std::cerr << "done." << std::endl;
		std::cerr << "validating...";
		result_exact = validate_map_exact_match(dict, map_positive_queries, map_negative_queries);
		result_prefix = validate_map_prefix_search(dict, all_queries);
		result_traversal = validate_map_predictive_search(dict, map_predictive_search_queries, map_traversal_borders, map_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else{
		throw std::runtime_error("unknown index type or trie type: " + index_type + " / " + sftrie_type);
	}

	std::cout << "input:" << std::endl;
	std::cout << "  " << std::setw(20) << "alphabet size: " << std::setw(12) << alphabet.size() << std::endl;
	std::cout << "  " << std::setw(20) << "min symbol: " << std::setw(12) << static_cast<signed long long>(min_char) << std::endl;
	std::cout << "  " << std::setw(20) << "max symbol: " << std::setw(12) << static_cast<signed long long>(max_char) << std::endl;
	std::cout << "  " << std::setw(20) << "number of texts: " << std::setw(12) << texts.size() << std::endl;
	std::cout << "  " << std::setw(20) << "total length: " << std::setw(12) << total_length << std::endl;
	std::cout << "exact match:" << std::endl;
	std::cout << "  " << std::setw(20) << "total queries: " << std::setw(12) << (positive_queries_size + negative_queries_size) << std::endl;
	std::cout << "  " << std::setw(20) << "true positive: " << std::setw(12) << result_exact["tp"] << std::endl;
	std::cout << "  " << std::setw(20) << "true negative: " << std::setw(12) << result_exact["tn"] << std::endl;
	std::cout << "  " << std::setw(20) << "false positive: " << std::setw(12) << result_exact["fp"] << std::endl;
	std::cout << "  " << std::setw(20) << "false negative: " << std::setw(12) << result_exact["fn"] << std::endl;
	std::cout << "prefix search:" << std::endl;
	std::cout << "  " << std::setw(20) << "total queries: " << std::setw(12) << all_queries.size() << std::endl;
	std::cout << "  " << std::setw(20) << "true positive: " << std::setw(12) << result_prefix["tp"] << std::endl;
	std::cout << "  " << std::setw(20) << "false positive: " << std::setw(12) << result_prefix["fp"] << std::endl;
	std::cout << "  " << std::setw(20) << "false negative: " << std::setw(12) << result_prefix["fn"] << std::endl;
	std::cout << "predictive search:" << std::endl;
	std::cout << "  " << std::setw(20) << "total queries: " << std::setw(12) << (predictive_search_queries_size + negative_queries_size) << std::endl;
	std::cout << "  " << std::setw(20) << "true positive: " << std::setw(12) << result_traversal["tp"] << std::endl;
	std::cout << "  " << std::setw(20) << "true negative: " << std::setw(12) << result_traversal["tn"] << std::endl;
	std::cout << "  " << std::setw(20) << "false positive: " << std::setw(12) << result_traversal["fp"] << std::endl;
	std::cout << "  " << std::setw(20) << "false negative: " << std::setw(12) << result_traversal["fn"] << std::endl;
}

int main(int argc, char* argv[])
{
	paramset::definitions defs = {
		{"symbol_type", "char", "symbol-type", 's', "symbol type (char, wchar, char16_t or char32_t)"},
		{"index_type", "set", "index-type", 'i', "index type (set or map)"},
		{"sftrie_type", "naive", "sftrie-type", 't', "sftrie type (naive, basic, tail or decompaction)"},
		{"min_binary_search", 42, {"min_binary_search"}, "min-binary-search", 0, "minumum number of children for binary search"},
		{"min_tail", 1, {"min_tail"}, "min-tail", 0, "minumum length to copress tail strings"},
		{"min_decompaction", 0, {"min_decompaction"}, "min-decompaction", 0, "minumum number of children to enable decompaction"},
		{"conf_path", "", "config", 'c', "config file path"}
	};
	paramset::manager pm(defs);

	try{
		pm.load(argc, argv, "config");

		if(!pm.rest.empty())
			pm["corpus_path"] = pm.rest.front().as<std::string>();

		std::string corpus_path = pm["corpus_path"];
		std::string symbol_type = pm["symbol_type"];
		std::string index_type = pm["index_type"];
		std::string sftrie_type = pm["sftrie_type"];
		int min_binary_search = pm["min_binary_search"];
		int min_tail = pm["min_tail"];
		int min_decompaction = pm["min_decompaction"];

		std::cerr << "Configuration" << std::endl;
		std::cerr << std::setw(12) << std::left << "  corpus_path: " << corpus_path << std::endl;
		std::cerr << std::setw(12) << std::left << "  symbol_type: " << symbol_type << std::endl;
		std::cerr << std::setw(12) << std::left << "  index_type: " << index_type << std::endl;
		std::cerr << std::setw(12) << std::left << "  sftrie_type: " << sftrie_type << std::endl;
		std::cerr << std::setw(12) << std::left << "  min_binary_search: " << min_binary_search << std::endl;
		std::cerr << std::setw(12) << std::left << "  min_tail: " << min_tail << std::endl;
		std::cerr << std::setw(12) << std::left << "  min_decompaction: " << min_decompaction << std::endl;
		std::cerr << std::endl;

		if(symbol_type == "char")
			exec<std::string, object, integer>(corpus_path, index_type, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "wchar_t")
			exec<std::wstring, object, integer>(corpus_path, index_type, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char16_t")
			exec<std::u16string, object, integer>(corpus_path, index_type, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char32_t")
			exec<std::u32string, object, integer>(corpus_path, index_type, sftrie_type, min_binary_search, min_tail, min_decompaction);
		else
			throw std::runtime_error("unknown symbol type: " + symbol_type);

		return 0;
	}
	catch(const std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		exit(1);
	}
}
