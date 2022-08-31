## Suffix tree

This repository contains implementation of suffix tree with polymorphic arbitrary size alphabets of ASCII symbols.

### Alphabet

As far as we know worst property of Suffix trees is large amount of used memory, so to reduce memory footprint was decided to create `Alphabet` which allows to set required letters as template args:

```cpp
template <char... letters>
class Alphabet;
```

`Alphabet` allows to define it's size and character existance at compile time with `constexpr`:

```cpp
    // Size of alphabet.
    static constexpr int32_t size()
    {
        return sizeof...(letters);
    }

    // Checks does letter exist in alphabet.
    static constexpr bool is_exist(char c)
    {
        bool exist = false;
        ((exist = exist || (c == letters)), ...);
        return exist;
    }
```

This property is usefull to define reuired size of connections in each suffix tree's node and reduce redundant overhead.

Alphabet implemented in [own header file](include/Alphabet.h).

### Suffix tree

Suffix tree can be constructed from any string-like objects with `O(n)` time complexity using [Ukkonen's algorithm](https://en.wikipedia.org/wiki/Ukkonen%27s_algorithm) and allows fast substring matching.

```cpp
    /**
     * @brief Substring matching
     * 
     * @param pattern string to match as substring
     * @returns position of first occurrence of patern and `-1` if not found
     */
    int32_t index_of(std::string_view pattern) const;

    /**
     * @brief Substring matching
     * 
     * @param pattern string to match as substring
     * @returns true if pattern is found and false otherwise
     */
    bool contains(std::string_view pattern) const { return index_of(pattern) != -1; }
```

As mentioned above we can provide our own alphabet to reduce memory footprint or extend default alphabet with other required symbols.

```cpp
template <typename Alphabet=StandartSuffixTreeAlphabet>
class SuffixTree
```

Example of usage from [main](main.cpp):

```cpp
    static constexpr std::string_view str("mississipi");
    static constexpr std::string_view pattern("issip");

    custom::SuffixTree<EnglishLowercaseLetters> tree(str);
    std::cout << "position: " << tree.contains(pattern) << std::endl;
```

Suffix tree implemented in [own header file](include/SuffixTree.h). 

### How to use

To use Suffix tree is required to include [SuffixTree](include/SuffixTree.h) header and then compile source file with `-std=c++17`:

```bash
g++ -std=c++17 -Iinclude main.pp -o run
```

-----------------------

**Made by Maksim Bronnikov**
