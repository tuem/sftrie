# sftrie: a compact trie representation

## Usage

1. Copy [include/sftrie](include/sftrie) to your include path

2. Include header
```c++
#include <sftrie/set.hpp>
```

3. Build trie
```c++
std::vector<std::string> texts; // you can also use u16string, u32string, etc.
...
sftrie::set<text> index(texts.begin()), texts.end());
```

4. Search texts
```c++
std::string query = "...";
auto searcher = index.searcher();
std::cout << searcher.exists(query) ? "found" : "not found" << std::endl;
```
