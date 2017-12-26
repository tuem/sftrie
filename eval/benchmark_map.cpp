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

#include <random>
#include <chrono>

#include <sftrie/map_naive.hpp>
#include <sftrie/map_basic.hpp>
#include <sftrie/map_tail.hpp>
#include <sftrie/map_decompaction.hpp>
#include <paramset.hpp>

#include "string_util.hpp"
#include "history.hpp"

using object = unsigned int;
using integer = unsigned int;

template<typename text_object_pair, typename map>
size_t evaluate(const map& dict, const std::vector<text_object_pair>& queries)
{
	size_t found = 0;
	for(const auto& query: queries){
		auto result = dict.find(query.first);
		if(result.first && result.second == query.second)
			++found;
	}
	return found;
}

template<typename text, typename object, typename integer>
int exec(const std::string& corpus_path, const std::string& sftrie_type,
	int min_binary_search, int min_tail, int min_decompaction)
{
	History history;

	std::cerr << "loading corpus...";
	history.refresh();
	object value = 0;
	std::size_t total_length = 0;
	std::vector<std::pair<text, object>> texts;
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
		auto t = cast_string<text>(line);
		texts.push_back(std::make_pair(t, value++));
		total_length += t.size();
	}
	history.record("load");
	std::cerr << "done." << std::endl;

	std::cerr << "sorting texts...";
	history.refresh();
	sftrie::sort_text_object_pairs(std::begin(texts), std::end(texts));
	history.record("sort");
	std::cerr << "done." << std::endl;

	std::cerr << "generating queries...";
	history.refresh();
	std::vector<std::pair<text, object>> queries = texts;
	std::vector<std::pair<text, object>> shuffled_queries = queries;
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(std::begin(shuffled_queries), std::end(shuffled_queries), std::default_random_engine(seed));
	history.record("(prepare)");
	std::cerr << "done." << std::endl;

	size_t space = 0;
	size_t found_ordered = 0, found_shuffled = 0;
	if(sftrie_type == "naive"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_naive<text, object, integer> dict(std::begin(texts), std::end(texts));
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;
		space = dict.space();

		std::cerr << "searching (ordered)...";
		history.refresh();
		found_ordered = evaluate(dict, queries);
		history.record("search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "searching (shuffled)...";
		history.refresh();
		found_shuffled = evaluate(dict, shuffled_queries);
		history.record("search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(sftrie_type == "basic"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_basic<text, object, integer> dict(std::begin(texts), std::end(texts),
			min_binary_search);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;
		space = dict.space();

		std::cerr << "searching (ordered)...";
		history.refresh();
		found_ordered = evaluate(dict, queries);
		history.record("search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "searching (shuffled)...";
		history.refresh();
		found_shuffled = evaluate(dict, shuffled_queries);
		history.record("search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(sftrie_type == "tail"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_tail<text, object, integer> dict(std::begin(texts), std::end(texts),
			min_binary_search, min_tail);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;
		space = dict.space();

		std::cerr << "searching (ordered)...";
		history.refresh();
		found_ordered = evaluate(dict, queries);
		history.record("search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "searching (shuffled)...";
		history.refresh();
		found_shuffled = evaluate(dict, shuffled_queries);
		history.record("search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else if(sftrie_type == "decompaction"){
		std::cerr << "constructing index...";
		history.refresh();
		sftrie::map_decompaction<text, object, integer> dict(std::begin(texts), std::end(texts),
			min_binary_search, min_tail, min_decompaction);
		history.record("construction", texts.size());
		std::cerr << "done." << std::endl;
		space = dict.space();

		std::cerr << "searching (ordered)...";
		history.refresh();
		found_ordered = evaluate(dict, queries);
		history.record("search (ordered)", queries.size());
		std::cerr << "done." << std::endl;

		std::cerr << "searching (shuffled)...";
		history.refresh();
		found_shuffled = evaluate(dict, shuffled_queries);
		history.record("search (shuffled)", shuffled_queries.size());
		std::cerr << "done." << std::endl;
	}
	else{
		throw std::runtime_error("unknown trie type: " + sftrie_type);
	}

	std::cout << std::endl;
	std::cout << "size:" << std::endl;
	std::cout << std::setw(16) << "# of texts: " << std::setw(16) << texts.size() << std::endl;
	std::cout << std::setw(16) << "total length: " << std::setw(16) << total_length << std::endl;
	std::cout << std::setw(16) << "index size: " << std::setw(16) << space << std::endl;
	std::cout << std::endl;
	std::cout << "time:" << std::endl;
	history.dump(std::cout, true, true);

	return (found_ordered == queries.size() && found_shuffled == shuffled_queries.size()) ? 0 : 1;
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
		else
			throw std::runtime_error("unknown symbol type: " + symbol_type);
	}
	catch(const std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		exit(1);
	}
}
