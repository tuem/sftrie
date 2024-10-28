# sftrie: a compact trie representation for integer alphabets

## Features
- Basic trie operations
  - exact matching
  - common-prefix search
  - predictive search
- Integer alphabet support
  - char, wchar, char32_t, etc.
- Set and map implementations

## Using sets

1. Copy [include/sftrie](include/sftrie) to your include path

2. Include header
```c++
#include <sftrie/set.hpp>
```

3. Build trie
```c++
using text = std::string;
std::vector<text> texts = ...;
sftrie::set<text> index(texts);
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

## Using maps

1. Copy [include/sftrie](include/sftrie) to your include path

2. Include header
```c++
#include <sftrie/map.hpp>
```

3. Build trie
```c++
using text = std::u32string;
std::vector<std::pair<text, int>> data = ...; // pairs of keys and values are required
sftrie::set<text> index(data);
```

4. Search texts
```c++
text pattern = "...";
auto searcher = index.searcher();
// exact matching
if(searcher.exists(pattern))
	std::cout << pattern << " found, value=" << index[pattern] << std::endl;
// common-prefix search
for(const auto& result: searcher.prefix(pattern))
	std::cout << result.key() << ", value=" << result.value() << std::endl;
// predictive search
for(const auto& result: searcher.predict(pattern))
	std::cout << result.key() << ", value=" << result.value() << std::endl;
```
