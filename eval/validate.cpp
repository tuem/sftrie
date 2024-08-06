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
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <set>
#include <map>

#include <random>
#include <chrono>
#include <stdexcept>

#include <sftrie/set.hpp>
#include <sftrie/map.hpp>
#include <sftrie/util.hpp>
#include <paramset.hpp>

using integer = std::uint32_t;
using item = std::uint32_t;

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
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
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
				if(result.key() != *a)
					++fp;
				else
					++found;
				++a;
			}
		}
		if(found == answers.size()){
			if(found > 0)
				++tp;
			else
				++tn;
		}
	}
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename set>
std::map<std::string, size_t> validate_set_predictive_search(const set& index,
	const std::vector<text>& positive_queries,
	const std::vector<integer>& predictive_search_borders,
	const std::vector<text>& negative_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	auto searcher = index.searcher();
	for(size_t i = 0; i < predictive_search_borders.size(); ++i){
		size_t answer_start = i, answer_end = predictive_search_borders[i];
		bool correct = true;
		size_t j = answer_start, count = 0;
		for(const auto& result: searcher.predict(positive_queries[answer_start])){
			if(result.key() != positive_queries[j]){
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
		for(const auto& result: searcher.predict(query)){
			if(result.key() == query){
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

template<typename text_item_pair, typename map>
std::map<std::string, size_t> validate_map_exact_match(const map& index,
	const std::vector<text_item_pair>& positive_queries, const std::vector<text_item_pair>& negative_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	for(const auto& query: positive_queries){
		auto result = index.find(query.first);
		if(result.match() && result.value() == query.second)
			++tp;
		else
			++fn;
	}
	for(const auto& query: negative_queries){
		auto result = index.find(query.first);
		if(!result.match())
			++tn;
		else
			++fp;
	}
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename map>
std::map<std::string, size_t> validate_map_prefix_search(const map& index,
	const std::vector<text>& queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	auto searcher = index.searcher();
	for(const auto& query: queries){
		std::vector<text> answers;
		text answer = query;
		while(!answer.empty()){
			if(index.find(answer).match())
				answers.push_back(answer);
			answer.pop_back();
		}
		if(index.find(answer).match())
			answers.push_back(answer);
		std::reverse(std::begin(answers), std::end(answers));

		integer found = 0;
		auto a = std::begin(answers);
		for(const auto result: searcher.prefix(query)){
			if(a == std::end(answers)){
				++fn;
				break;
			}
			else{
				if(result.key() != *a)
					++fp;
				else
					++found;
				++a;
			}
		}
		if(found == answers.size()){
			if(found > 0)
				++tp;
			else
				++tn;
		}
	}
	return {{"tp", tp}, {"tn", tn}, {"fp", fp}, {"fn", fn}};
}

template<typename text, typename item, typename map>
std::map<std::string, size_t> validate_map_predictive_search(const map& index,
	const std::vector<std::pair<text, item>>& positive_queries,
	const std::vector<integer>& predictive_search_borders,
	const std::vector<std::pair<text, item>>& negative_queries)
{
	size_t tp = 0, tn = 0, fp = 0, fn = 0;
	auto searcher = index.searcher();
	for(size_t i = 0; i < predictive_search_borders.size(); ++i){
		size_t answer_start = i, answer_end = predictive_search_borders[i];
		bool correct = true;
		size_t j = answer_start, count = 0;
		for(auto result: searcher.predict(positive_queries[answer_start].first)){
			if(result.key() != positive_queries[j].first || result.value() != positive_queries[j].second){
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
		for(const auto result: searcher.predict(query.first)){
			if(result.key() == query.first){
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

template<typename text, typename item, typename integer>
bool exec(const std::string& corpus_path, const std::string& index_type,
	const std::string& optimization_mode, int min_binary_search)
{
	using symbol = typename text::value_type;

	min_binary_search = min_binary_search > 0 ? min_binary_search : sftrie::constants::default_min_binary_search<symbol>();

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
		texts.push_back(sftrie::cast_text<text>(line));
	}
	std::vector<std::pair<text, item>> text_item_pairs;
	if(index_type == "map"){
		item value = 0;
		for(const auto& t: texts)
			text_item_pairs.push_back(std::make_pair(t, value++));
	}
	std::cerr << "done." << std::endl;

	std::cerr << "analyzing texts...";
	std::set<symbol> alphabet;
	symbol min_symbol = texts.front().front(), max_symbol = min_symbol;
	bool first = true;
	size_t min_length = 0, max_length = 0, total_length = 0;
	for(const auto& t: texts){
		for(auto c: t){
			alphabet.insert(c);
			min_symbol = std::min(min_symbol, c);
			max_symbol = std::max(max_symbol, c);
		}
		if(first){
			min_length = t.size();
			max_length = t.size();
			first = false;
		}
		else{
			min_length = std::min(t.size(), min_length);
			max_length = std::max(t.size(), max_length);
		}
		total_length += t.size();
	}
	double average_length = total_length / static_cast<double>(texts.size());
	std::cerr << "done." << std::endl;

	std::cerr << "generating queries...";

	std::vector<text> all_queries = texts;
	std::vector<text> set_positive_queries, set_negative_queries;
	std::vector<text> set_predictive_search_queries;
	std::vector<integer> set_predictive_search_borders;

	std::vector<std::pair<text, item>> map_positive_queries, map_negative_queries;
	std::vector<std::pair<text, item>> map_predictive_search_queries;
	std::vector<integer> map_predictive_search_borders;

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
			set_predictive_search_borders.push_back(end);
		}

		sftrie::sort_texts(std::begin(texts), std::end(texts));

		positive_queries_size = set_positive_queries.size();
		negative_queries_size = set_negative_queries.size();
		predictive_search_queries_size = set_predictive_search_queries.size();
	}
	else if(index_type == "map"){
		std::shuffle(std::begin(text_item_pairs), std::end(text_item_pairs), std::default_random_engine(seed));
		std::copy(std::begin(text_item_pairs), std::begin(text_item_pairs) + text_item_pairs.size() / 2, std::back_inserter(map_positive_queries));
		std::copy(std::begin(text_item_pairs) + text_item_pairs.size() / 2, std::end(text_item_pairs), std::back_inserter(map_negative_queries));
		text_item_pairs.erase(std::begin(text_item_pairs) + text_item_pairs.size() / 2, std::end(text_item_pairs));

		map_predictive_search_queries = map_positive_queries;
		sftrie::sort_text_item_pairs(std::begin(map_predictive_search_queries), std::end(map_predictive_search_queries));
		for(size_t i = 0; i < map_predictive_search_queries.size(); ++i){
			size_t end = i + 1;
			while(end < map_predictive_search_queries.size() &&
					std::equal(std::begin(map_predictive_search_queries[i].first), std::end(map_predictive_search_queries[i].first), std::begin(map_predictive_search_queries[end].first)))
				++end;
			map_predictive_search_borders.push_back(end);
		}

		sftrie::sort_text_item_pairs(std::begin(text_item_pairs), std::end(text_item_pairs));

		positive_queries_size = map_positive_queries.size();
		negative_queries_size = map_negative_queries.size();
		predictive_search_queries_size = map_predictive_search_queries.size();
	}
	else{
		throw std::runtime_error("unknown container type or trie type: " + index_type + " / " + optimization_mode);
	}
	std::cerr << "done." << std::endl;

	std::map<std::string, size_t> result_exact;
	std::map<std::string, size_t> result_prefix;
	std::map<std::string, size_t> result_predict;
	if(index_type == "set" && optimization_mode == "original"){
		std::cerr << "constructing index...";
		sftrie::set_original<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search);
		std::cerr << "done." << std::endl;

		std::cerr << "validating...";
		result_exact = validate_set_exact_match(index, set_positive_queries, set_negative_queries);
		result_prefix = validate_set_prefix_search(index, all_queries);
		result_predict = validate_set_predictive_search(index, set_predictive_search_queries, set_predictive_search_borders, set_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "set" && optimization_mode == "compact"){
		std::cerr << "constructing index...";
		sftrie::set_compact<text, integer> index(std::begin(texts), std::end(texts));
		std::cerr << "done." << std::endl;

		std::cerr << "validating...";
		result_exact = validate_set_exact_match(index, set_positive_queries, set_negative_queries);
		result_prefix = validate_set_prefix_search(index, all_queries);
		result_predict = validate_set_predictive_search(index, set_predictive_search_queries, set_predictive_search_borders, set_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && optimization_mode == "original"){
		std::cerr << "constructing index...";
		sftrie::map_original<text, item, integer> index(std::begin(text_item_pairs), std::end(text_item_pairs),
			min_binary_search);
		std::cerr << "done." << std::endl;

		std::cerr << "validating...";
		result_exact = validate_map_exact_match(index, map_positive_queries, map_negative_queries);
		result_prefix = validate_map_prefix_search(index, all_queries);
		result_predict = validate_map_predictive_search(index, map_predictive_search_queries, map_predictive_search_borders, map_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && optimization_mode == "compact"){
		std::cerr << "constructing index...";
		sftrie::map_compact<text, item, integer> index(std::begin(text_item_pairs), std::end(text_item_pairs), min_binary_search);
		std::cerr << "done." << std::endl;

		std::cerr << "validating...";
		result_exact = validate_map_exact_match(index, map_positive_queries, map_negative_queries);
		result_prefix = validate_map_prefix_search(index, all_queries);
		result_predict = validate_map_predictive_search(index, map_predictive_search_queries, map_predictive_search_borders, map_negative_queries);
		std::cerr << "done." << std::endl;
	}
	else{
		throw std::runtime_error("unknown container type or trie type: " + index_type + " / " + optimization_mode);
	}

	std::cout << std::endl;
	std::cout << "[input]" << std::endl;
	std::cout << std::left << std::setw(20) << "alphabet size" << std::right << std::setw(12) << alphabet.size() << std::endl;
	std::cout << std::left << std::setw(20) << "min symbol" << std::right << std::setw(12) << static_cast<signed long long>(min_symbol) << std::endl;
	std::cout << std::left << std::setw(20) << "max symbol" << std::right << std::setw(12) << static_cast<signed long long>(max_symbol) << std::endl;
	std::cout << std::left << std::setw(20) << "number of texts" << std::right << std::setw(12) << texts.size() << std::endl;
	std::cout << std::left << std::setw(20) << "max length" << std::right << std::setw(12) << max_length << std::endl;
	std::cout << std::left << std::setw(20) << "min length" << std::right << std::setw(12) << min_length << std::endl;
	std::cout << std::left << std::setw(20) << "average length" << std::right << std::setw(12) << average_length << std::endl;
	std::cout << std::left << std::setw(20) << "total length" << std::right << std::setw(12) << total_length << std::endl;
	std::cout << std::left << std::setw(20) << "total bytes" << std::right << std::setw(12) << sizeof(symbol) * total_length << std::endl;
	std::cout << std::left << std::endl;
	std::cout << "[exact match]" << std::endl;
	std::cout << std::left << std::setw(20) << "total queries" << std::right << std::setw(12) << (positive_queries_size + negative_queries_size) << std::endl;
	std::cout << std::left << std::setw(20) << "true positive" << std::right << std::setw(12) << result_exact["tp"] << std::endl;
	std::cout << std::left << std::setw(20) << "true negative" << std::right << std::setw(12) << result_exact["tn"] << std::endl;
	std::cout << std::left << std::setw(20) << "false positive" << std::right << std::setw(12) << result_exact["fp"] << std::endl;
	std::cout << std::left << std::setw(20) << "false negative" << std::right << std::setw(12) << result_exact["fn"] << std::endl;
	std::cout << std::endl;
	std::cout << "[prefix search]" << std::endl;
	std::cout << std::left << std::setw(20) << "total queries" << std::right << std::setw(12) << all_queries.size() << std::endl;
	std::cout << std::left << std::setw(20) << "true positive" << std::right << std::setw(12) << result_prefix["tp"] << std::endl;
	std::cout << std::left << std::setw(20) << "true negative" << std::right << std::setw(12) << result_prefix["tn"] << std::endl;
	std::cout << std::left << std::setw(20) << "false positive" << std::right << std::setw(12) << result_prefix["fp"] << std::endl;
	std::cout << std::left << std::setw(20) << "false negative" << std::right << std::setw(12) << result_prefix["fn"] << std::endl;
	std::cout << std::endl;
	std::cout << "[predictive search]" << std::endl;
	std::cout << std::left << std::setw(20) << "total queries" << std::right << std::setw(12) << (predictive_search_queries_size + negative_queries_size) << std::endl;
	std::cout << std::left << std::setw(20) << "true positive" << std::right << std::setw(12) << result_predict["tp"] << std::endl;
	std::cout << std::left << std::setw(20) << "true negative" << std::right << std::setw(12) << result_predict["tn"] << std::endl;
	std::cout << std::left << std::setw(20) << "false positive" << std::right << std::setw(12) << result_predict["fp"] << std::endl;
	std::cout << std::left << std::setw(20) << "false negative" << std::right << std::setw(12) << result_predict["fn"] << std::endl;

	return
		result_exact["fp"] == 0 &&
		result_exact["fp"] == 0 &&
		result_prefix["fp"] == 0 &&
		result_prefix["fn"] == 0 &&
		result_predict["fp"] == 0 &&
		result_predict["fn"] == 0;
}

int main(int argc, char* argv[])
{
	paramset::definitions defs = {
		{"symbol_type", "char", {"common", "symbol_type"}, "symbol-type", 's', "symbol type (char, wchar, char16_t or char32_t)"},
		{"index_type", "set", {"common", "index_type"}, "index-type", 'i', "index type (set or map)"},
		{"optimization_mode", "compact", {"sftrie", "optimization_mode"}, "optimization-mode", 'o', "sftrie optimization mode (original or compact)"},
		{"min_binary_search", 0, {"sftrie", "min_binary_search"}, "min-binary-search", 'b', "do binary search if number of children is less than the value (set 0 to use default setting)"},
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
		std::string optimization_mode = pm["optimization_mode"];
		int min_binary_search = pm["min_binary_search"];

		std::cout << "[configuration]" << std::endl;
		std::cout << std::setw(20) << std::left << "corpus_path" << corpus_path << std::endl;
		std::cout << std::setw(20) << std::left << "symbol_type" << symbol_type << std::endl;
		std::cout << std::setw(20) << std::left << "index_type" << index_type << std::endl;
		std::cout << std::setw(20) << std::left << "mode" << optimization_mode << std::endl;
		std::cout << std::setw(20) << std::left << "min_binary_search" << min_binary_search << std::endl;
		std::cout << std::endl;

		bool passed;
		if(symbol_type == "char")
			passed = exec<std::string, item, integer>(corpus_path, index_type, optimization_mode, min_binary_search);
		else if(symbol_type == "wchar_t")
			passed = exec<std::wstring, item, integer>(corpus_path, index_type, optimization_mode, min_binary_search);
		else if(symbol_type == "char16_t")
			passed = exec<std::u16string, item, integer>(corpus_path, index_type, optimization_mode, min_binary_search);
		else if(symbol_type == "char32_t")
			passed = exec<std::u32string, item, integer>(corpus_path, index_type, optimization_mode, min_binary_search);
		else
			throw std::runtime_error("unknown symbol type: " + symbol_type);

		if(passed)
			std::cout << std::endl << "all tests paeeed"<< std::endl;

		return passed ? 0 : 1;
	}
	catch(const std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		exit(1);
	}
}
