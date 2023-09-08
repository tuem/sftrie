# sftrie: a compact trie representation for integer alphabets

## Usage

1. Copy [include/sftrie](include/sftrie) to your include path

2. Include header
```c++
#include <sftrie/set.hpp>
```

3. Build trie
```c++
std::vector<std::string> texts = ...; // you can also use u16string, u32string, etc.
sftrie::set<text> index(texts.begin()), texts.end());
```

4. Search texts
```c++
std::string pattern = "...";
auto searcher = index.searcher();
// exact matching
std::cout << searcher.exists(pattern) ? "found" : "not found" << std::endl;
// common-prefix search
for(const auto& entry: searcher.prefix(pattern))
	std::cout << entry << std::endl;
// predictive search
for(const auto& entry: searcher.predict(pattern))
	std::cout << entry << std::endl;
```
