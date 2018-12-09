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
		for(const auto& result: searcher.prefix(query)){
			(void)result;
			++num_result;
			if(max_result != 0 && num_result == max_result)
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
		for(const auto& result: searcher.traverse(query)){
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
		for(const auto& result: searcher.prefix(query)){
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
size_t benchmark_map_predictive_search(const map& dict,
	const std::vector<text>& queries, size_t max_result = 0)
{
	size_t found = 0;
	auto searcher = dict.searcher();
	for(const auto& query: queries){
		size_t num_result = 0;
		for(const auto& result: searcher.traverse(query)){
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
bool exec(const std::string& corpus_path, const std::string& index_type, int max_result,
	const std::string& sftrie_type, int min_binary_search, int min_tail, int min_decompaction)
{
	using symbol = typename text::value_type;

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
		auto t = cast_string<text>(line);
		texts.push_back(t);
	}
	std::vector<std::pair<text, object>> text_object_pairs;
	if(index_type == "map"){
		object value = 0;
		for(const auto& t: texts)
			text_object_pairs.push_back(std::make_pair(t, value++));
	}
	history.record("loading texts", texts.size());
	std::cerr << "done." << std::endl;

	std::cerr << "analyzing texts...";
	std::set<symbol> alphabet;
	symbol min_char = texts.front().front(), max_char = min_char;
	bool first = true;
	size_t min_length = 0, max_length = 0, total_length = 0;
	for(const auto& t: texts){
		for(auto c: t){
			alphabet.insert(c);
			min_char = std::min(min_char, c);
			max_char = std::max(max_char, c);
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
	sftrie::sort_text_object_pairs(std::begin(text_object_pairs), std::end(text_object_pairs));
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

	size_t node_size = 0, trie_size =0, space = 0;
	size_t found_ordered = 0, found_shuffled = 0, prefixes_ordered = 0, prefixes_shuffled = 0, enumerated_ordered = 0, enumerated_shuffled = 0;
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

		node_size = index.node_size();
		trie_size = index.trie_size();
		space = index.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_set_exact_match(index, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		prefixes_ordered = benchmark_set_prefix_search(index, queries);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		prefixes_shuffled = benchmark_set_prefix_search(index, shuffled_queries);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_set_predictive_search(index, queries, max_result);
		history.record("predictive search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_set_predictive_search(index, shuffled_queries, max_result);
		history.record("predictive search (shuffled)", shuffled_queries.size());
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

		node_size = index.node_size();
		trie_size = index.trie_size();
		space = index.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_set_exact_match(index, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		prefixes_ordered = benchmark_set_prefix_search(index, queries);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		prefixes_shuffled = benchmark_set_prefix_search(index, shuffled_queries);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_set_predictive_search(index, queries, max_result);
		history.record("predictive search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_set_predictive_search(index, shuffled_queries, max_result);
		history.record("predictive search (shuffled)", shuffled_queries.size());
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

		node_size = index.node_size();
		trie_size = index.trie_size();
		space = index.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_set_exact_match(index, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		prefixes_ordered = benchmark_set_prefix_search(index, queries);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		prefixes_shuffled = benchmark_set_prefix_search(index, shuffled_queries);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_set_predictive_search(index, queries, max_result);
		history.record("predictive search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_set_predictive_search(index, shuffled_queries, max_result);
		history.record("predictive search (shuffled)", shuffled_queries.size());
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

		node_size = index.node_size();
		trie_size = index.trie_size();
		space = index.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_set_exact_match(index, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		prefixes_ordered = benchmark_set_prefix_search(index, queries);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		prefixes_shuffled = benchmark_set_prefix_search(index, shuffled_queries);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_set_predictive_search(index, queries, max_result);
		history.record("predictive search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_set_predictive_search(index, shuffled_queries, max_result);
		history.record("predictive search (shuffled)", shuffled_queries.size());
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

		node_size = dict.node_size();
		trie_size = dict.trie_size();
		space = dict.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_map_exact_match(dict, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		prefixes_ordered = benchmark_map_prefix_search(dict, queries);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		prefixes_shuffled = benchmark_map_prefix_search(dict, shuffled_queries);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_map_predictive_search(dict, queries, max_result);
		history.record("predictive search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_map_predictive_search(dict, shuffled_queries, max_result);
		history.record("predictive search (shuffled)", shuffled_queries.size());
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

		node_size = dict.node_size();
		trie_size = dict.trie_size();
		space = dict.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_map_exact_match(dict, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		prefixes_ordered = benchmark_map_prefix_search(dict, queries);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		prefixes_shuffled = benchmark_map_prefix_search(dict, shuffled_queries);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_map_predictive_search(dict, queries, max_result);
		history.record("predictive search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_map_predictive_search(dict, shuffled_queries, max_result);
		history.record("predictive search (shuffled)", shuffled_queries.size());
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

		node_size = dict.node_size();
		trie_size = dict.trie_size();
		space = dict.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_map_exact_match(dict, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		prefixes_ordered = benchmark_map_prefix_search(dict, queries);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		prefixes_shuffled = benchmark_map_prefix_search(dict, shuffled_queries);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_map_predictive_search(dict, queries, max_result);
		history.record("predictive search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_map_predictive_search(dict, shuffled_queries, max_result);
		history.record("predictive search (shuffled)", shuffled_queries.size());
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

		node_size = dict.node_size();
		trie_size = dict.trie_size();
		space = dict.space();

		std::cerr << "exact match (shuffled)...";
		history.refresh();
		found_shuffled = benchmark_map_exact_match(dict, shuffled_queries);
		history.record("exact match (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (ordered)...";
		history.refresh();
		prefixes_ordered = benchmark_map_prefix_search(dict, queries);
		history.record("prefix search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "prefix search (shuffled)...";
		history.refresh();
		prefixes_shuffled = benchmark_map_prefix_search(dict, shuffled_queries);
		history.record("prefix search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (ordered)...";
		history.refresh();
		enumerated_ordered = benchmark_map_predictive_search(dict, queries, max_result);
		history.record("predictive search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "predictive search (shuffled)...";
		history.refresh();
		enumerated_shuffled = benchmark_map_predictive_search(dict, shuffled_queries, max_result);
		history.record("predictive search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else{
		throw std::runtime_error("unknown index type or trie type: " + index_type + " / " + sftrie_type);
	}

	std::cout << std::endl;
	std::cout << "[input]" << std::endl;
	std::cout << std::left << std::setw(30) << "alphabet size" << std::right << std::setw(12) << alphabet.size() << std::endl;
	std::cout << std::left << std::setw(30) << "min symbol" << std::right << std::setw(12) << static_cast<signed long long>(min_char) << std::endl;
	std::cout << std::left << std::setw(30) << "max symbol" << std::right << std::setw(12) << static_cast<signed long long>(max_char) << std::endl;
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
	std::cout << std::left << std::setw(30) << "index size" << std::right << std::setw(12) << space << std::endl;
	std::cout << std::endl;
	std::cout << "[time]" << std::endl;
	history.dump();

	return found_ordered == queries.size() && found_shuffled == shuffled_queries.size() &&
		prefixes_ordered >= queries.size() && prefixes_shuffled >= shuffled_queries.size() &&
		enumerated_ordered >= queries.size() && enumerated_shuffled >= shuffled_queries.size();
}

int main(int argc, char* argv[])
{
	paramset::definitions defs = {
		{"symbol_type", "char", {"common", "symbol_type"}, "symbol-type", 's', "symbol type (char, wchar, char16_t or char32_t)"},
		{"index_type", "set", {"common", "index_type"}, "container-type", 'i', "index type (set or map)"},
		{"mode", "naive", {"sftrie", "mode"}, "mode", 'm', "sftrie optimization mode (naive, basic, tail or decompaction)"},
		{"min_binary_search", 42, {"sftrie", "min_binary_search"}, "min-binary-search", 'b', "do binary search if number of children is less than the value"},
		{"min_tail", 1, {"sftrie", "min_tail"}, "min-tail", 't', "minumum length to compress tail strings"},
		{"min_decompaction", 0, {"sftrie", "min_decompaction"}, "min-decompaction", 'd', "minumum number of children to enable decompaction"},
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
		std::string sftrie_type = pm["mode"];
		int min_binary_search = pm["min_binary_search"];
		int min_tail = pm["min_tail"];
		int min_decompaction = pm["min_decompaction"];
		int max_result = pm["max_result"];

		std::cout << "[configuration]" << std::endl;
		std::cout << std::setw(30) << std::left << "corpus_path" << corpus_path << std::endl;
		std::cout << std::setw(30) << std::left << "symbol_type" << symbol_type << std::endl;
		std::cout << std::setw(30) << std::left << "index_type" << index_type << std::endl;
		std::cout << std::setw(30) << std::left << "mode" << sftrie_type << std::endl;
		std::cout << std::setw(30) << std::left << "min_binary_search" << min_binary_search << std::endl;
		std::cout << std::setw(30) << std::left << "min_tail" << min_tail << std::endl;
		std::cout << std::setw(30) << std::left << "min_decompaction" << min_decompaction << std::endl;
		std::cout << std::setw(30) << std::left << "max_result" << max_result << std::endl;
		std::cout << std::endl;

		if(symbol_type == "char")
			exec<std::string, object, integer>(corpus_path, index_type, max_result,
				sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "wchar_t")
			exec<std::wstring, object, integer>(corpus_path, index_type, max_result,
				sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char16_t")
			exec<std::u16string, object, integer>(corpus_path, index_type, max_result,
				sftrie_type, min_binary_search, min_tail, min_decompaction);
		else if(symbol_type == "char32_t")
			exec<std::u32string, object, integer>(corpus_path, index_type, max_result,
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
