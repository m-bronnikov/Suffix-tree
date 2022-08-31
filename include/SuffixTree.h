#pragma once

#include "Alphabet.h"

#include <string>
#include <string_view>
#include <cstring>
#include <vector>
#include <array>

namespace custom
{

// Terminal symbol for suffix tree
constexpr const char terminal_symbol = '\0';

/**
 * @brief Alphabet for sufix tree must exist unique terminal element: '\0'.
 */
template<char... letters>
using SuffixTreeAlphabet = Alphabet<terminal_symbol, letters...>;

/**
 * @brief Some common alphabet.
 */
using StandartSuffixTreeAlphabet = SuffixTreeAlphabet<
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', 
    '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', 
    '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 
    'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
    '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 
    'y', 'z', '{', '|', '}', '~'
>;

} // custom


namespace custom
{

template <typename Alphabet=StandartSuffixTreeAlphabet>
class SuffixTree
{
private:
    /**
     * @brief Reference to Edge in inner addressation
     * 
     * Represented as 32-bit integer value:
     * 1. 'ref = -1' value means edge by this address not exist.
     * 2. 'ref >= 0' value means valid address of some edge.
     * 3. otherwise address values are invalid.
     */
    using reference_to_edge = int32_t;
    static constexpr reference_to_edge no_connection = -1;

private:
    /**
     * @brief Reference to Node in inner addressation
     * 
     * Represented as 32-bit integer value:
     * 1. 'ref >= 0' value means valid address of some inner node.
     * 2. 'ref < 0' value means edge comes to leaf node with number '-(ref + 1)'
     */
    using reference_to_node = int32_t;

private:
    /**
     * @brief Suffix tree inner node
     * 
     *  Each inner node has suffix connection to some else inner node and can contain 
     *  up to size of alphabet addresses of edges to child nodes.
     */
    struct Node
    {
        constexpr Node();

        reference_to_node suffix_connection;
        std::array<reference_to_edge, Alphabet::size()> edges_to_childs;
    };

private:
    /**
     * @brief Suffix tree edge
     * 
     * Edges comes from some inner node of tree to some child node (leaf or inner).
     * 
     * Edges encodes some substring and includes addres of next node. Next node 
     * could be a leaf: if so, address to next node will a negative value.
     */
    struct Edge
    {
        // substring encoding
        int32_t start_position;
        int32_t length;

        // address of next node
        reference_to_node next_node_addr;
    };

public:
    SuffixTree(std::string_view source);

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

private:
    // entry point to tree building
    void construct_tree();

    // create dummy node - suffix connection of root
    reference_to_node create_dummy_node();

private:
    /**
     * @brief Position inside tree
     * 
     * Position in tree is defied by:
     * 1. Inner node (origin of the edge)
     * 2. Inner edge (which comes from node)
     * 3. Position on this edge (length of the path from edge's begining)
     */
    struct InnerPosition{
        reference_to_node node_addr = -1;
        reference_to_edge edge_addr = no_connection;
        int32_t position = 0; // should be in range [0, length), 0 means edge is undefined, yet
    };
    
    /**
     * @brief Series of first Ukkonen's rule
     * 
     * First rule adds new char to edge which ends in leaf node. Since to each such edge algorithm sets
     * length to matcs with end position of expanded string this step is implicit and does nothing.
     */
    void first_stage(uint32_t new_ch_pos, InnerPosition& iterator) { /* Do nothing */ }

    /**
     * @brief Series of second Ukkonen's rule
     * 
     * Second rule creates new branch in tree. There is 2 cases:
     * 1. Position inside node: create new edge with leaf ending.
     * 2. Position inside edge: create new inner node in this place with 2 outcoming edges:
     *      - Rest of current edge
     *      - New edge with leaf ending
     */
    void second_stage(uint32_t new_ch_pos, InnerPosition& iterator);

    bool is_position_in_edge_without_path(uint32_t new_ch_pos, const InnerPosition& iterator) const;
    bool is_position_in_node_without_path(uint32_t new_ch_pos, const InnerPosition& iterator) const;

    reference_to_node add_node_in(const InnerPosition& iterator);
    void create_new_edge_to_leaf_from_node(uint32_t new_ch_pos, reference_to_node node_addr);

    void go_using_suffix_connection(InnerPosition& iterator) const;

    /**
     * @brief Series of third Ukkonen's rule
     * 
     * Third rule does nothing with tree and just jumps over one letter next in tree.
     */
    void third_stage(uint32_t new_ch_pos, InnerPosition& iterator);

    void go_over_one_letter_next(char ch, InnerPosition& iterator) const;


private:
    int32_t length_to_end_from(int32_t start_pos) const { return expanded_string.size() - start_pos; }

private:
    // abstractions to create and access to edges
    reference_to_edge allocate_edge();
    const Edge& get_edge_by(reference_to_edge ref) const { return edge_allocator[ref]; }
    Edge& get_edge_by(reference_to_edge ref) { return edge_allocator[ref]; }

    // abstractions to create and access to nodes
    reference_to_node allocate_node();
    const Node& get_node_by(reference_to_node ref) const { return node_allocator[ref]; }
    Node& get_node_by(reference_to_node ref) { return node_allocator[ref]; }

    // abstractions over leafs, leafs is just negative references
    bool is_leaf(reference_to_node ref) const { return ref < 0; }
    int32_t leaf_num_of(reference_to_node ref) const;

    // methods to access dummy node (suffix connection of root node)
    reference_to_node get_dummy() const { return get_node_by(root_addr).suffix_connection; }
    bool is_dummy(reference_to_node node_addr) const { return node_addr == get_node_by(root_addr).suffix_connection; }



private:
    // source string expanded with terminal symbol
    std::string expanded_string;

    // alphabet for letters of expanded string
    static inline constexpr Alphabet alphabet{};


private:
    reference_to_node root_addr;

private:
    std::vector<Node> node_allocator;
    std::vector<Edge> edge_allocator;
};


template<typename Alphabet>
constexpr SuffixTree<Alphabet>::Node::Node()
{
    for(auto& edge_addr : edges_to_childs){
        edge_addr = no_connection;
    }
}


template <typename Alphabet>
int32_t SuffixTree<Alphabet>::leaf_num_of(reference_to_node ref) const
{
    assert(is_leaf(ref));
    return -ref - 1;
}


template<typename Alphabet>
typename SuffixTree<Alphabet>::reference_to_node SuffixTree<Alphabet>::allocate_node()
{
    node_allocator.emplace_back();
    return node_allocator.size() - 1;
}

template<typename Alphabet>
typename SuffixTree<Alphabet>::reference_to_edge SuffixTree<Alphabet>::allocate_edge()
{
    edge_allocator.emplace_back();
    return edge_allocator.size() - 1;
}


template<typename Alphabet>
typename SuffixTree<Alphabet>::reference_to_node SuffixTree<Alphabet>::create_dummy_node()
{
    // create dummy node
    auto dummy_addr = allocate_node();
    Node& dummy = get_node_by(dummy_addr);

    // suffix connection of dummy node could be any, let's it be dummy node itself
    dummy.suffix_connection = dummy_addr;

    // root is only one child of dummy by each alphabet's symbol
    for(auto& edge_addr : dummy.edges_to_childs)
    {
        edge_addr = allocate_edge();

        // dummy is suffix connection of root, so length of each edge 
        // must be '1' since jump by suffix connection decreases suffix 
        // length by 1 letter
        Edge& edge = get_edge_by(edge_addr);
        {
            edge.start_position = -1; // some invalid start position
            edge.length = 1;
            edge.next_node_addr = root_addr;
        }
    }

    return dummy_addr;
}


template<typename Alphabet>
SuffixTree<Alphabet>::SuffixTree(std::string_view source) : expanded_string(std::string(source) + terminal_symbol)
{
    // alphabet must contain all symbols of expanded string
    assert(alphabet.is_alphabet_of(expanded_string));

    // create root and dummy nodes
    root_addr = allocate_node();
    auto dummy_addr = create_dummy_node();

    // root has suffix connection to dummy node
    Node& root = get_node_by(root_addr);
    root.suffix_connection = dummy_addr;

    // do main routine and construct suffix tree
    construct_tree();
}


template<typename Alphabet>
void SuffixTree<Alphabet>::go_using_suffix_connection(InnerPosition& iterator) const
{
    // save source edge which contains current position to take chars to jump from it
    const Edge& source_edge = get_edge_by(iterator.edge_addr);
    assert(source_edge.start_position >= 0);

    // jump to node by suffix connection
    iterator.node_addr = get_node_by(iterator.node_addr).suffix_connection;
    if(iterator.position == 0)
    {
        iterator.edge_addr = no_connection;
        return;
    }

    // update edge
    {
        const Node& node = get_node_by(iterator.node_addr);

        char ch = expanded_string[source_edge.start_position];
        iterator.edge_addr = node.edges_to_childs[alphabet.index_of(ch)];
        assert(iterator.edge_addr != -1);
    }

    // go over edges until position is larger than edge size
    int32_t processed_length = 0;
    while(iterator.position >= get_edge_by(iterator.edge_addr).length)
    {
        const Edge& edge = get_edge_by(iterator.edge_addr);

        // update node and position
        {
            iterator.node_addr = edge.next_node_addr;
            iterator.position -= edge.length;
            processed_length += edge.length;
            assert(processed_length <= source_edge.length);
        }

        // skip if immediatelly in node
        if(iterator.position == 0)
        {
            iterator.edge_addr = no_connection;
            break;
        }

        // update edge
        {
            const Node& node = get_node_by(iterator.node_addr);

            char ch = expanded_string[source_edge.start_position + processed_length];
            iterator.edge_addr = node.edges_to_childs[alphabet.index_of(ch)];
            assert(iterator.edge_addr != -1);
        }
    }
}


template <typename Alphabet>
bool SuffixTree<Alphabet>::is_position_in_node_without_path(uint32_t new_ch_pos, const InnerPosition& iterator) const
{
    const Node& node = get_node_by(iterator.node_addr);
    char ch = expanded_string[new_ch_pos];

    // statement is true if position inside inner node and no connection exist for new char
    if(iterator.position == 0)
        if(node.edges_to_childs[alphabet.index_of(ch)] == no_connection)
            return true;

    return false;
}


template <typename Alphabet>
bool SuffixTree<Alphabet>::is_position_in_edge_without_path(uint32_t new_ch_pos, const InnerPosition& iterator) const
{
    // statement is false if in ineer node
    if(iterator.position == 0)
        return false;

    // first letter in the string is always in inner node
    assert(new_ch_pos > 0);

    // current edge
    const Edge& edge = get_edge_by(iterator.edge_addr);
    assert(edge.start_position >= 0);

    // check previous letter is mathes and placed in right position
    assert(iterator.position < edge.length);
    assert(expanded_string[new_ch_pos - 1] == expanded_string[edge.start_position + iterator.position - 1]);
    
    // statement is false if next char of edge matches with char from 'new_ch_pos'
    char ch = expanded_string[new_ch_pos];
    if(expanded_string[edge.start_position + iterator.position] == ch)
        return false;

    return true;
}

template <typename Alphabet>
typename SuffixTree<Alphabet>::reference_to_node SuffixTree<Alphabet>::add_node_in(const InnerPosition& iterator)
{
    // allocate new node and edge
    auto new_edge_addr = allocate_edge();
    auto new_node_addr = allocate_node();

    Edge& edge = get_edge_by(iterator.edge_addr);
    assert(edge.start_position >= 0);

    // position must be inside edge
    assert(iterator.position > 0);
    assert(iterator.position < edge.length);

    uint32_t ch_pos = edge.start_position + iterator.position;
    char ch = expanded_string[ch_pos];

    // create edge incoming from new node, this edge - second part of source edge split
    Edge& new_edge = get_edge_by(new_edge_addr);
    {
        new_edge.length = edge.length - iterator.position;
        new_edge.start_position = ch_pos;
        new_edge.next_node_addr = edge.next_node_addr;
    }

    // create new node and set created edge
    Node& new_node = get_node_by(new_node_addr);
    {
        new_node.edges_to_childs[alphabet.index_of(ch)] = new_edge_addr;
    }

    // update exist edge, this edge - reuse of source edge as first part of split
    edge.length = iterator.position;
    edge.next_node_addr = new_node_addr;

    return new_node_addr;
}

template <typename Alphabet>
void SuffixTree<Alphabet>::create_new_edge_to_leaf_from_node(uint32_t new_ch_pos, reference_to_node node_addr)
{
    static reference_to_node leaf_addr_allocator = 0;
    reference_to_node new_leaf_addr = --leaf_addr_allocator;

    Node& node = get_node_by(node_addr);
    char ch = expanded_string[new_ch_pos];
    
    // edge by char must not exist before
    assert(node.edges_to_childs[alphabet.index_of(ch)] == no_connection);

    auto edge_addr = allocate_edge();
    Edge& edge = get_edge_by(edge_addr);
    {
        edge.start_position = new_ch_pos;
        edge.length = length_to_end_from(new_ch_pos);
        edge.next_node_addr = new_leaf_addr;
    }

    node.edges_to_childs[alphabet.index_of(ch)] = edge_addr;
}


template <typename Alphabet>
void SuffixTree<Alphabet>::second_stage(uint32_t new_ch_pos, InnerPosition& iterator)
{
    // define initial node to pass suffix connections
    reference_to_node last_node_addr = get_node_by(root_addr).suffix_connection; // init with dummy node because dummy's suffix connection value is no matter
    if(is_position_in_edge_without_path(new_ch_pos, iterator))
    {
        last_node_addr = add_node_in(iterator);
        create_new_edge_to_leaf_from_node(new_ch_pos, last_node_addr);
        go_using_suffix_connection(iterator);
    }

    while(is_position_in_edge_without_path(new_ch_pos, iterator))
    {
        // connect last node with new created using suffix connection
        reference_to_node new_node_addr = add_node_in(iterator);
        get_node_by(last_node_addr).suffix_connection = new_node_addr;
        last_node_addr = new_node_addr;
        create_new_edge_to_leaf_from_node(new_ch_pos, new_node_addr);
        go_using_suffix_connection(iterator);
    }

    // connect last node to obtained using suffix connection
    get_node_by(last_node_addr).suffix_connection = iterator.node_addr;

    while(is_position_in_node_without_path(new_ch_pos, iterator))
    {
        // don't need to connect nodes due to them already created and therefore already connected
        create_new_edge_to_leaf_from_node(new_ch_pos, iterator.node_addr);
        go_using_suffix_connection(iterator);
    }
}


template <typename Alphabet>
void SuffixTree<Alphabet>::third_stage(uint32_t new_ch_pos, InnerPosition& iterator)
{
    go_over_one_letter_next(expanded_string[new_ch_pos], iterator);
}


template <typename Alphabet>
void SuffixTree<Alphabet>::go_over_one_letter_next(char ch, InnerPosition& iterator) const
{
    // if current position in inner node: define edge
    if(iterator.position == 0)
    {
        const Node& node = get_node_by(iterator.node_addr);

        iterator.edge_addr = node.edges_to_childs[alphabet.index_of(ch)];
        assert(iterator.edge_addr != no_connection);
    }

    const Edge& edge = get_edge_by(iterator.edge_addr);
    assert(is_dummy(iterator.node_addr) || expanded_string[edge.start_position + iterator.position] == ch);

    // increment length in node
    iterator.position++;

    // update node addr if end is obtained
    if(iterator.position == edge.length)
    {
        iterator.node_addr = edge.next_node_addr;
        iterator.edge_addr = -1; // undefine
        iterator.position = 0;
    }
}


template <typename Alphabet>
void SuffixTree<Alphabet>::construct_tree()
{
    InnerPosition iterator = {root_addr, no_connection, 0};

    for(uint32_t pos = 0; pos < expanded_string.size(); ++pos)
    {
        first_stage(pos, iterator);

        second_stage(pos, iterator);

        third_stage(pos, iterator);
    }
}


template <typename Alphabet>
int32_t SuffixTree<Alphabet>::index_of(std::string_view pattern) const
{
    InnerPosition iterator = {root_addr, no_connection, 0};

    if(pattern.size() == 0) {
        return 0;
    }

    reference_to_node dummy_addr = get_dummy();
    reference_to_edge last_edge_addr = get_node_by(dummy_addr).edges_to_childs.front();

    for(char ch : pattern)
    {
        // define node
        const Node& node = get_node_by(iterator.node_addr);
        
        // define edge
        if(iterator.edge_addr == no_connection)
        {
            // update edge if possible
            iterator.edge_addr = node.edges_to_childs[alphabet.index_of(ch)];
            if(iterator.edge_addr == no_connection)
            {
                return -1;
            }
        }
        const Edge& edge = get_edge_by(iterator.edge_addr);
        
        // false if encoded string on edge not matches with pattern
        if(expanded_string[edge.start_position + iterator.position] != ch)
        {
            return -1;
        }

        // update position
        ++iterator.position;

        // update node if needed
        if(edge.length == iterator.position)
        {
            last_edge_addr = iterator.edge_addr;

            iterator.node_addr = edge.next_node_addr;
            iterator.edge_addr = no_connection;
            iterator.position = 0;

            // leaf must not be accessed due to terminal
            assert(!is_leaf(iterator.node_addr));
        }
    }

    reference_to_edge edge_addr = iterator.edge_addr;
    int32_t edge_position = iterator.position;

    // use last edge if current position in node
    if(iterator.edge_addr == no_connection)
    {
        edge_addr = last_edge_addr;
        edge_position = get_edge_by(edge_addr).length;
    }

    // define position of first occurence
    const Edge& edge = get_edge_by(edge_addr);
    int32_t position = edge.start_position + edge_position - pattern.size();
    assert(position >= 0);
    return position;
}

} // custom
