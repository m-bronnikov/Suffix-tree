#pragma once

#include <algorithm>
#include <string>
#include <cassert>

namespace custom
{

/**
 * @brief Alphabet wrapper
 * 
 * This class provides compfortoble interface for some alphabet. Class includes 2 parts:
 *      1. Static constexpr: to define size and char existance at compile time.
 *      2. Common in-memory part: to fast char index definition requests processing.  
 */
template <char... letters>
class Alphabet
{
public:
    // Max possible alphabet size.
    static constexpr int32_t max_alphabet_size = 256;

public:
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

    // Returns all indices of all ASCII letters as array. '-1' means letter is not in alphabet.
    static constexpr std::array<int, max_alphabet_size> indices()
    {
        Accumulator acc;
        for(auto& idx : acc.indices) idx = -1; // fill with -1
        return (acc << ... << letters).indices;
    }

private:
    // Alphabet accumulator with overloaded operator to define alphabet size and all indices of chars.
    struct Accumulator
    {
        std::array<int, max_alphabet_size> indices{};
        int32_t size = 0;
    };

    friend constexpr Accumulator& operator<<(Accumulator& helper, char c)
    {
        helper.indices[c] = helper.size++;
        return helper;
    }

public:
    constexpr Alphabet() : char_to_index(indices()) {}

    // Returns '-1' if c is not in alphabet, otherwise value which '>= 0'.
    int32_t index_of(char c) const
    {
        return char_to_index[c];
    }

    // Checks does provided string produced of this alphabet or not.
    bool is_alphabet_of(const std::string& str) const
    {
        for(char c : str)
            if(index_of(c) < 0)
                return false;
        return true;
    }

private:
    std::array<int, max_alphabet_size> char_to_index;
};

} // namespace custom
