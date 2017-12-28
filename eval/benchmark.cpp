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
#include "history.hpp"

using object = unsigned int;
using integer = unsigned int;

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
		for(const auto& result: searcher.common_prefix(query)){
			(void)result;
			++num_result;
			if(max_result != 0 && num_result == max_result)
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
		if(dict.find(query).first)
			++found;
	return found;
}

template<typename text, typename map>
size_t benchmark_map_prefix_search(const map& dict,
	const std::vector<text>& queries, size_t max_result = 0)
{
	size_t found = 0;
	auto searcher = dict.searcher();
	for(const auto& query: queries){
		size_t num_result = 0;
		for(const auto& result: searcher.common_prefix(query)){
			(void)result;
			++num_result;
			if(max_result != 0 && num_result == max_result)
				break;
		}
		found += num_result;
	}
	return found;
}

template<typename text, typename object, typename integer>
void exec(const std::string& corpus_path, const std::string& index_type, int prefix_search_max_result,
	const std::string& sftrie_type, int min_binary_search, int min_tail, int min_decompaction)
{
	History history;

	std::cerr << "loading texts...";
	history.refresh();
	std::size_t total_length = 0;
	std::vector<text> texts;
	std::ifstream ifs(corpus_path);
	if(!ifs.is_open())
		throw std::runtime_error("input file is not available: " + corpus_path);
	while(ifs.good()){
		std::string line;
		std::getline(ifs, line);
		if(ifs.eof())
			break;
		auto t = cast_string<text>(line);
		texts.push_back(t);
		total_length += t.size();
	}
	std::vector<std::pair<text, object>> text_object_pairs;
	if(index_type == "map"){
		object value = 0;
		for(const auto& t: texts)
			text_object_pairs.push_back(std::make_pair(t, value++));
	}
	history.record("load");
	std::cerr << "done." << std::endl;

	std::cerr << "sorting texts...";
	history.refresh();
	sftrie::sort_texts(std::begin(texts), std::end(texts));
	history.record("sort");
	std::cerr << "done." << std::endl;

	std::cerr << "generating queries...";
	history.refresh();
	std::vector<text> queries = texts;
	std::vector<text> shuffled_queries = queries;
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(std::begin(shuffled_queries), std::end(shuffled_queries), std::default_random_engine(seed));
	history.record("prepare");
	std::cerr << "done." << std::endl;

	size_t space = 0;
	size_t found_ordered = 0, found_shuffled = 0, enumerated_ordered = 0, enumerated_shuffled = 0;
	if(index_type == "set" && sftrie_type == "naive"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::set_naive<text, integer> index(std::begin(texts), std::end(texts));
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		std::cerr << "exact match (ordered)...";
		history.refresh();
		found_ordered = benchmark_set_exact_match(index, queries);
		history.record("exact match (ordered)", queries.size());
		std::cerr << "done." << std::endl;
		space = index.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_set_exact_match(index, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_set_prefix_search(index, queries, prefix_search_max_result);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_set_prefix_search(index, shuffled_queries, prefix_search_max_result);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "set" && sftrie_type == "basic"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::set_basic<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		std::cerr << "exact match (ordered)...";
		history.refresh();
		found_ordered = benchmark_set_exact_match(index, queries);
		history.record("exact match (ordered)", queries.size());
		std::cerr << "done." << std::endl;
		space = index.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_set_exact_match(index, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_set_prefix_search(index, queries, prefix_search_max_result);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_set_prefix_search(index, shuffled_queries, prefix_search_max_result);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "set" && sftrie_type == "tail"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::set_tail<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search, min_tail);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		std::cerr << "exact match (ordered)...";
		history.refresh();
		found_ordered = benchmark_set_exact_match(index, queries);
		history.record("exact match (ordered)", queries.size());
		std::cerr << "done." << std::endl;
		space = index.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_set_exact_match(index, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_set_prefix_search(index, queries, prefix_search_max_result);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_set_prefix_search(index, shuffled_queries, prefix_search_max_result);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "set" && sftrie_type == "decompaction"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::set_decompaction<text, integer> index(std::begin(texts), std::end(texts),
			min_binary_search, min_tail, min_decompaction);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		std::cerr << "exact match (ordered)...";
		history.refresh();
		found_ordered = benchmark_set_exact_match(index, queries);
		history.record("exact match (ordered)", queries.size());
		std::cerr << "done." << std::endl;
		space = index.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_set_exact_match(index, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_set_prefix_search(index, queries, prefix_search_max_result);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_set_prefix_search(index, shuffled_queries, prefix_search_max_result);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && sftrie_type == "naive"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_naive<text, object, integer> dict(
			std::begin(text_object_pairs), std::end(text_object_pairs));
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		std::cerr << "exact match (ordered)...";
		history.refresh();
		found_ordered = benchmark_map_exact_match(dict, queries);
		history.record("exact match (ordered)", queries.size());
		std::cerr << "done." << std::endl;
		space = dict.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_map_exact_match(dict, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_map_prefix_search(dict, queries, prefix_search_max_result);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_map_prefix_search(dict, shuffled_queries, prefix_search_max_result);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && sftrie_type == "basic"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_basic<text, object, integer> dict(
			std::begin(text_object_pairs), std::end(text_object_pairs),
			min_binary_search);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		std::cerr << "exact match (ordered)...";
		history.refresh();
		found_ordered = benchmark_map_exact_match(dict, queries);
		history.record("exact match (ordered)", queries.size());
		std::cerr << "done." << std::endl;
		space = dict.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_map_exact_match(dict, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_map_prefix_search(dict, queries, prefix_search_max_result);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_map_prefix_search(dict, shuffled_queries, prefix_search_max_result);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && sftrie_type == "tail"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_tail<text, object, integer> dict(
			std::begin(text_object_pairs), std::end(text_object_pairs),
			min_binary_search, min_tail);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		std::cerr << "exact match (ordered)...";
		history.refresh();
		found_ordered = benchmark_map_exact_match(dict, queries);
		history.record("exact match (ordered)", queries.size());
		std::cerr << "done." << std::endl;
		space = dict.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_map_exact_match(dict, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_map_prefix_search(dict, queries, prefix_search_max_result);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_map_prefix_search(dict, shuffled_queries, prefix_search_max_result);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(index_type == "map" && sftrie_type == "decompaction"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_decompaction<text, object, integer> dict(
			std::begin(text_object_pairs), std::end(text_object_pairs),
			min_binary_search, min_tail, min_decompaction);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;

		std::cerr << "exact match (ordered)...";
		history.refresh();
		found_ordered = benchmark_map_exact_match(dict, queries);
		history.record("exact match (ordered)", queries.size());
		std::cerr << "done." << std::endl;
		space = dict.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_map_exact_match(dict, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_map_prefix_search(dict, queries, prefix_search_max_result);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_map_prefix_search(dict, shuffled_queries, prefix_search_max_result);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else{
		throw std::runtime_error("unknown index type or trie type: " + index_type + " / " + sftrie_type);
	}

	std::cout << std::endl;
	std::cout << "size:" << std::endl;
	std::cout << std::right << std::setw(26) << "# of texts" << std::setw(12) << texts.size() << std::endl;
	std::cout << std::right << std::setw(26) << "total length" << std::setw(12) << total_length << std::endl;
	std::cout << std::right << std::setw(26) << "index size" << std::setw(12) << space << std::endl;
	std::cout << std::endl;
	std::cout << "time:" << std::endl;
	history.dump(std::cout, true, true);
}

int main(int argc, char* argv[])
{
	paramset::definitions defs = {
		{"symbol_type", "char", {"common", "symbol_type"}, "symbol-type", 's', "symbol type (char, wchar, char16_t or char32_t)"},
		{"index_type", "set", {"common", "index_type"}, "index-type", 'i', "index type (set or map)"},
		{"prefix_search_max_result", 0, {"prefix_search", "max_result"}, "prefix-search-max-result", 0, "max number to enumerate texts in common prefix search"},
		{"sftrie_type", "naive", {"sftrie", "type"}, "sftrie-type", 't', "sftrie type (naive, basic, tail or decompaction)"},
		{"min_binary_search", 42, {"sftrie", "min_binary_search"}, "min-binary-search", 0, "minumum number of children for binary search"},
		{"min_tail", 1, {"sftrie", "min_tail"}, "min-tail", 0, "minumum length to copress tail strings"},
		{"min_decompaction", 0, {"sftrie", "min_decompaction"}, "min-decompaction", 0, "minumum number of children to enable decompaction"},
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
		int prefix_search_max_result = pm["prefix_search_max_result"];
		std::string sftrie_type = pm["sftrie_type"];
		int min_binary_search = pm["min_binary_search"];
		int min_tail = pm["min_tail"];
		int min_decompaction = pm["min_decompaction"];

		std::cout << "Configuration" << std::endl;
		std::cout << std::setw(12) << std::left << "  corpus_path: " << corpus_path << std::endl;
		std::cout << std::setw(12) << std::left << "  symbol_type: " << symbol_type << std::endl;
		std::cout << std::setw(12) << std::left << "  index_type: " << index_type << std::endl;
		std::cout << std::setw(12) << std::left << "  prefix_search_max_result: " << prefix_search_max_result << std::endl;
		std::cout << std::setw(12) << std::left << "  sftrie_type: " << sftrie_type << std::endl;
		std::cout << std::setw(12) << std::left << "  min_binary_search: " << min_binary_search << std::endl;
		std::cout << std::setw(12) << std::left << "  min_tail: " << min_tail << std::endl;
		std::cout << std::setw(12) << std::left << "  min_decompaction: " << min_decompaction << std::endl;
		std::cout << std::endl;

		if(symbol_type == "char")
			exec<std::string, object, integer>(corpus_path, index_type, prefix_search_max_result,
				sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "wchar_t")
			exec<std::wstring, object, integer>(corpus_path, index_type, prefix_search_max_result,
				sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char16_t")
			exec<std::u16string, object, integer>(corpus_path, index_type, prefix_search_max_result,
				sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char32_t")
			exec<std::u32string, object, integer>(corpus_path, index_type, prefix_search_max_result,
				sftrie_type, min_binary_search, min_tail, min_decompaction);
		else
			throw std::runtime_error("unknown symbol type: " + symbol_type);

		return 0;
	}
	catch(const std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		exit(1);
	}
}
