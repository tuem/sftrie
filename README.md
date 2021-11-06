# sftrie: a compact trie representation

## Usage

1. Copy [include/sftrie](include/sftrie) to your include path

2. Include header
```c++
#include <sftrie/set.hpp>
```

3. Build trie
```c++
std::vector<std::string> texts; // you can also use wstring, u16string, u32string, etc.
...
sftrie::set<text, integer> index(std::begin(texts), std::end(texts));
```

4. Search queries
```c++
std::string query = "...";
auto searcher = index.searcher();
std::cout << searcher.count(query) > 0 ? "found" : "not found" << std::endl;
```
