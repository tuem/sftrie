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

#include <sftrie/set.hpp>
#include <sftrie/map.hpp>
#include <sftrie/util.hpp>
#include <paramset.hpp>

#include "history.hpp"

using integer = std::uint32_t;
using item = std::uint32_t;

template<typename text, typename set>
size_t benchmark_set_exact_match(const set& index,
	const std::vector<text>& queries)
{
	size_t found = 0;
	for(const auto& query: queries)
		if(index.exists(query))
			++found;
	return found;
}

template<typename text, typename set>
size_t benchmark_set_prefix_search(const set& index,
	const std::vector<text>& queries, size_t max_result = 0)
{
	size_t found = 0;
	auto searcher = index.searcher();
	for(const auto& query: queries){
		size_t num_result = 0;
		for(const auto& result: searcher.prefix(query)){
			(void)result;
			if(max_result != 0 && ++num_result == max_result)
				break;
		}
		found += num_result;
	}
	return found;
}

template<typename text, typename set>
size_t benchmark_set_predictive_search(const set& index,
	const std::vector<text>& queries, size_t max_result = 0)
{
	size_t found = 0;
	auto searcher = index.searcher();
	for(const auto& query: queries){
		size_t num_result = 0;
		for(const auto& result: searcher.predict(query)){
			(void)result;
			if(max_result != 0 && ++num_result == max_result)
				break;
		}
		found += num_result;
	}
	return found;
}

template<typename text, typename map>
size_t benchmark_map_exact_match(const map& dict,
	const std::vector<text>& queries)
{
	size_t found = 0;
	for(const auto& query: queries)
		if(dict.exists(query))
			++found;
	return found;
}

template<typename text, typename map>
size_t benchmark_map_prefix_search(const map& index,
	const std::vector<text>& queries, size_t max_result = 0)
{
	size_t found = 0;
	auto searcher = index.searcher();
	for(const auto& query: queries){
		size_t num_result = 0;
		for(const auto result: searcher.prefix(query)){
			(void)result;
			if(max_result != 0 && ++num_result == max_result)
				break;
		}
		found += num_result;
	}
	return found;
}

template<typename text, typename map>
size_t benchmark_map_predictive_search(const map& index,
	const std::vector<text>& queries, size_t max_result = 0)
{
	size_t found = 0;
	auto searcher = index.searcher();
	for(const auto& query: queries){
		size_t num_result = 0;
		for(const auto result: searcher.predict(query)){
			(void)result;
			if(max_result != 0 && ++num_result == max_result)
				break;
		}
		found += num_result;
	}
	return found;
}

template<typename set, typename text>
void benchmark_set(History& history, const set& index,
	const std::vector<text>& queries, const std::vector<text>& shuffled_queries,
	size_t max_result)
{
	std::cerr << "exact match (ordered)...";
	history.refresh();
	benchmark_set_exact_match(index, queries);
	history.record("exact match (ordered)", queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "exact match (shuffled)...";
	history.refresh();
	benchmark_set_exact_match(index, shuffled_queries);
	history.record("exact match (shuffled)", shuffled_queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "prefix search (ordered)...";
	history.refresh();
	benchmark_set_prefix_search(index, queries, max_result);
	history.record("prefix search (ordered)", queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "prefix search (shuffled)...";
	history.refresh();
	benchmark_set_prefix_search(index, shuffled_queries, max_result);
	history.record("prefix search (shuffled)", shuffled_queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "predictive search (ordered)...";
	history.refresh();
	benchmark_set_predictive_search(index, queries, max_result);
	history.record("predictive search (ordered)", queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "predictive search (shuffled)...";
	history.refresh();
	benchmark_set_predictive_search(index, shuffled_queries, max_result);
	history.record("predictive search (shuffled)", shuffled_queries.size());
	std::cerr << "done." << std::endl;
}

template<typename map, typename text>
void benchmark_map(History& history, const map& index,
	const std::vector<text>& queries, const std::vector<text>& shuffled_queries,
	size_t max_result)
{
	std::cerr << "exact match (ordered)...";
	history.refresh();
	benchmark_map_exact_match(index, queries);
	history.record("exact match (ordered)", queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "exact match (shuffled)...";
	history.refresh();
	benchmark_map_exact_match(index, shuffled_queries);
	history.record("exact match (shuffled)", shuffled_queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "prefix search (ordered)...";
	history.refresh();
	benchmark_map_prefix_search(index, queries, max_result);
	history.record("prefix search (ordered)", queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "prefix search (shuffled)...";
	history.refresh();
	benchmark_map_prefix_search(index, shuffled_queries, max_result);
	history.record("prefix search (shuffled)", shuffled_queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "predictive search (ordered)...";
	history.refresh();
	benchmark_map_predictive_search(index, queries, max_result);
	history.record("predictive search (ordered)", queries.size());
	std::cerr << "done." << std::endl;

	std::cerr << "predictive search (shuffled)...";
	history.refresh();
	benchmark_map_predictive_search(index, shuffled_queries, max_result);
	history.record("predictive search (shuffled)", shuffled_queries.size());
	std::cerr << "done." << std::endl;
}

template<typename text, typename item, typename integer>
void exec(const std::string& corpus_path, const std::string& index_type, int max_result,
	const std::string& optimization_mode, bool two_pass,
	sftrie::lookup_table_mode lut_mode, int min_lookup_table_children, int min_binary_search)
{
	using symbol = typename text::value_type;

	min_lookup_table_children = min_lookup_table_children > 0 ? min_lookup_table_children :
		sftrie::constants::default_min_lookup_table_children<symbol>();
	min_binary_search = min_binary_search > 0 ? min_binary_search :
		sftrie::constants::default_min_binary_search<symbol>();

	History history;

	std::cerr << "loading texts...";
	history.refresh();
	std::vector<text> texts;
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open())
		throw std::runtime_error("input file is not available: " + corpus_path);
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		auto t = sftrie::cast_text<text>(line);
		texts.push_back(t);
	}
	std::vector<std::pair<text, item>> text_item_pairs;
	if(index_type == "map"){
		item value = 0;
		for(const auto& t: texts)
			text_item_pairs.push_back(std::make_pair(t, value++));
	}
	history.record("loading texts", texts.size());
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
	history.record("analyzing texts", texts.size());
	std::cerr << "done." << std::endl;

	std::cerr << "sorting texts...";
	history.refresh();
	sftrie::sort_texts(std::begin(texts), std::end(texts));
	sftrie::sort_text_item_pairs(std::begin(text_item_pairs), std::end(text_item_pairs));
	history.record("sorting texts", texts.size());
	std::cerr << "done." << std::endl;

	std::cerr << "generating queries...";
	history.refresh();
	std::vector<text> queries = texts;
	std::vector<text> shuffled_queries = queries;
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(std::begin(shuffled_queries), std::end(shuffled_queries), std::default_random_engine(seed));
	history.record("generating queries", queries.size());
	std::cerr << "done." << std::endl;

	size_t node_size = 0, trie_size = 0, total_space = 0;
	if(index_type == "set" && optimization_mode == "original"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::set_original<text, integer> index(
			std::begin(texts), std::end(texts), two_pass, min_binary_search);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		node_size = index.node_size();
		trie_size = index.trie_size();
		total_space = index.total_space();

		benchmark_set(history, index, queries, shuffled_queries, max_result);
	}
	else if(index_type == "set" && optimization_mode == "compact"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::set_compact<text, integer> index(
			std::begin(texts), std::end(texts), two_pass, min_binary_search);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		node_size = index.node_size();
		trie_size = index.trie_size();
		total_space = index.total_space();

		benchmark_set(history, index, queries, shuffled_queries, max_result);
	}
	else if(index_type == "set" && optimization_mode == "fast"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::set_fast<text, integer> index(
			std::begin(texts), std::end(texts), two_pass, lut_mode, min_lookup_table_children, min_binary_search);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		node_size = index.node_size();
		trie_size = index.trie_size();
		total_space = index.total_space();

		benchmark_set(history, index, queries, shuffled_queries, max_result);
	}
	else if(index_type == "map" && optimization_mode == "original"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_original<text, item, integer> index(
			std::begin(text_item_pairs), std::end(text_item_pairs),
			two_pass, min_binary_search);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		node_size = index.node_size();
		trie_size = index.trie_size();
		total_space = index.total_space();

		benchmark_map(history, index, queries, shuffled_queries, max_result);
	}
	else if(index_type == "map" && optimization_mode == "compact"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_compact<text, item, integer> index(
			std::begin(text_item_pairs), std::end(text_item_pairs),
			two_pass, min_binary_search);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		node_size = index.node_size();
		trie_size = index.trie_size();
		total_space = index.total_space();

		benchmark_map(history, index, queries, shuffled_queries, max_result);
	}
	else if(index_type == "map" && optimization_mode == "fast"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_fast<text, item, integer> index(
			std::begin(text_item_pairs), std::end(text_item_pairs),
			two_pass, lut_mode, min_lookup_table_children, min_binary_search);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		node_size = index.node_size();
		trie_size = index.trie_size();
		total_space = index.total_space();

		benchmark_map(history, index, queries, shuffled_queries, max_result);
	}
	else{
		throw std::runtime_error("unknown index type or trie type: " + index_type + " / " + optimization_mode);
	}

	std::cout << std::endl;
	std::cout << "[input]" << std::endl;
	std::cout << std::left << std::setw(30) << "alphabet size" << std::right << std::setw(12) << alphabet.size() << std::endl;
	std::cout << std::left << std::setw(30) << "min symbol" << std::right << std::setw(12) << static_cast<signed long long>(min_symbol) << std::endl;
	std::cout << std::left << std::setw(30) << "max symbol" << std::right << std::setw(12) << static_cast<signed long long>(max_symbol) << std::endl;
	std::cout << std::left << std::setw(30) << "number of texts" << std::right << std::setw(12) << texts.size() << std::endl;
	std::cout << std::left << std::setw(30) << "max length" << std::right << std::setw(12) << max_length << std::endl;
	std::cout << std::left << std::setw(30) << "min length" << std::right << std::setw(12) << min_length << std::endl;
	std::cout << std::left << std::setw(30) << "average length" << std::right << std::setw(12) << average_length << std::endl;
	std::cout << std::left << std::setw(30) << "total length" << std::right << std::setw(12) << total_length << std::endl;
	std::cout << std::left << std::setw(30) << "total bytes" << std::right << std::setw(12) << sizeof(symbol) * total_length << std::endl;
	std::cout << std::endl;
	std::cout << "[size]" << std::endl;
	std::cout << std::left << std::setw(30) << "symbol size" << std::right << std::setw(12) << sizeof(symbol) << std::endl;
	std::cout << std::left << std::setw(30) << "# of texts" << std::right << std::setw(12) << texts.size() << std::endl;
	std::cout << std::left << std::setw(30) << "total length" << std::right << std::setw(12) << total_length << std::endl;
	std::cout << std::left << std::setw(30) << "total bytes" << std::right << std::setw(12) << (sizeof(symbol) * total_length) << std::endl;
	std::cout << std::left << std::setw(30) << "node size" << std::right << std::setw(12) << node_size << std::endl;
	std::cout << std::left << std::setw(30) << "trie size" << std::right << std::setw(12) << trie_size << std::endl;
	std::cout << std::left << std::setw(30) << "index size" << std::right << std::setw(12) << total_space << std::endl;
	std::cout << std::endl;
	std::cout << "[time]" << std::endl;
	history.dump();
}

int main(int argc, char* argv[])
{
	paramset::definitions defs = {
		{"symbol_type", "char", {"common", "symbol_type"}, "symbol-type", 's', "symbol type (char, wchar_t, char16_t or char32_t)"},
		{"index_type", "set", {"common", "index_type"}, "index-type", 'i', "index type (set or map)"},
		{"optimization_mode", "fast", {"sftrie", "optimization_mode"}, "optimization-mode", 'o', "sftrie optimization mode (original, compact or fast)"},
		{"two_pass", false, {"sftrie", "two_pass_construction"}, "two-pass-construction", 'p', "enable 2-pass construction to save temporary memory consumption in construction (default: false)"},
		{"lut_mode", "root", {"common", "lut_mode"}, "lut-mode", 'l', "lookup table mode (none, root or adaptive)"},
		{"min_lut", 0, {"sftrie", "min_lut"}, "min-lut", 'm', "threshold to use lookup tables for each node"},
		{"min_binary_search", 0, {"sftrie", "min_binary_search"}, "min-binary-search", 'b', "do binary search if number of children is less than the value (set 0 to use default setting)"},
		{"max_result", 0, {"sftrie", "max_result"}, "max-result", 'n', "max number of results in common-prefix search and predictive search"},
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
		bool two_pass = pm["two_pass"];
		sftrie::lookup_table_mode lut_mode = sftrie::lookup_table_mode::root_only;
		if(pm.get<std::string>("lut_mode") == "none")
			lut_mode = sftrie::lookup_table_mode::none;
		else if(pm.get<std::string>("lut_mode") == "adaptive")
			lut_mode = sftrie::lookup_table_mode::adaptive;
		int min_lookup_table_children = pm["min_lut"];
		int min_binary_search = pm["min_binary_search"];
		int max_result = pm["max_result"];

		std::cout << "[configuration]" << std::endl;
		std::cout << std::setw(30) << std::left << "corpus_path" << corpus_path << std::endl;
		std::cout << std::setw(30) << std::left << "symbol_type" << symbol_type << std::endl;
		std::cout << std::setw(30) << std::left << "index_type" << index_type << std::endl;
		std::cout << std::setw(30) << std::left << "mode" << optimization_mode << std::endl;
		std::cout << std::setw(30) << std::left << "two_pass" << (two_pass ? "true" : "false") << std::endl;
		std::cout << std::setw(30) << std::left << "lut_mode";
		if(lut_mode == sftrie::lookup_table_mode::root_only)
			std::cout << "root";
		else if(lut_mode == sftrie::lookup_table_mode::adaptive)
			std::cout << "adaptive";
		else
			std::cout << "none";
		std::cout << std::endl;
		std::cout << std::setw(30) << std::left << "min_lookup_table_children" << min_lookup_table_children << std::endl;
		std::cout << std::setw(30) << std::left << "min_binary_search" << min_binary_search << std::endl;
		std::cout << std::setw(30) << std::left << "max_result" << max_result << std::endl;
		std::cout << std::endl;

		if(symbol_type == "char")
			exec<std::string, item, integer>(corpus_path, index_type, max_result,
				optimization_mode, two_pass, lut_mode, min_lookup_table_children, min_binary_search);
		else if(symbol_type == "wchar_t")
			exec<std::wstring, item, integer>(corpus_path, index_type, max_result,
				optimization_mode, two_pass, lut_mode, min_lookup_table_children, min_binary_search);
		else if(symbol_type == "char16_t")
			exec<std::u16string, item, integer>(corpus_path, index_type, max_result,
				optimization_mode, two_pass, lut_mode, min_lookup_table_children, min_binary_search);
		else if(symbol_type == "char32_t")
			exec<std::u32string, item, integer>(corpus_path, index_type, max_result,
				optimization_mode, two_pass, lut_mode, min_lookup_table_children, min_binary_search);
		else
			throw std::runtime_error("unknown symbol type: " + symbol_type);

		return 0;
	}
	catch(const std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		exit(1);
	}
}
