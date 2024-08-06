# sftrie: a compact trie representation for integer alphabets

## Usage

1. Copy [include/sftrie](include/sftrie) to your include path

2. Include header
```c++
#include <sftrie/set.hpp>
```

3. Build trie
```c++
using text = std::string; // you can also use u16string, u32string, etc.
std::vector<text> texts = ...;
sftrie::set<text> index(texts.begin()), texts.end());
```

4. Search texts
```c++
text pattern = "...";
auto searcher = index.searcher();
// exact matching
std::cout << searcher.exists(pattern) ? "found" : "not found" << std::endl;
// common-prefix search
for(const auto& result: searcher.prefix(pattern))
	std::cout << result.key() << std::endl;
// predictive search
for(const auto& result: searcher.predict(pattern))
	std::cout << result.key() << std::endl;
```
