#include <iostream>
#include <any>
#include <array>
#include <fstream>
#include <map>
#include <variant>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>

#include "../Huffman_Universal/Huffman_Universal.h"
#include "Bad_Huffman.h"
using namespace std;

template<typename A, typename B>
pair<B,A> flip_pair(const pair<A,B> &p)
{
    return pair<B,A>(p.second, p.first);
}

template<typename A, typename B, template<class,class,class...> class M, class... Args, typename Func>
multimap<B, typename std::result_of<Func(A)>::type> flip_map_convert(const M<A,B,Args...>& src, Func mapper) {
    using NewA = typename std::result_of<Func(A)>::type;
    multimap<B, NewA> dst;

    for (const auto& pair : src) {
        dst.insert({ pair.second, mapper(pair.first) });
    }

    return dst;
}


void serializeHuffmanArray(ostream& out, const any& node, unsigned char mask = 0x00) {
    if (node.type() == typeid(unsigned char)) {
        auto c = any_cast<unsigned char>(node);
        out.put(0 ^ mask);
        out.put(c ^ mask);
        return;
    }

    auto &v = any_cast<const array<any, 2>&>(node);
    out.put(1 ^ mask);
    serializeHuffmanArray(out, v[1], mask);
    serializeHuffmanArray(out, v[0], mask);
}

void assignCodes(variant<unsigned char, array<any, 2>> &node, map<unsigned char, string>& treeCodes, int nodePos) {
    char charNodePos = '0' + nodePos;
    if (holds_alternative<unsigned char>(node)) {
        unsigned char c = get<unsigned char>(node);
        treeCodes[c] = charNodePos;
    } else {
        auto &firstVector = get<array<any, 2>>(node);
        auto &innerVec = any_cast<vector<unsigned char>&>(firstVector[1]);
        for (unsigned char c : innerVec)
            treeCodes[c].insert(0, 1, charNodePos);
    }
}

void mapNode(variant<unsigned char, array<any, 2>> &node, array<any, 2> &LeftNode, vector<unsigned char> &RightNode, int nodePos) {
    if (!holds_alternative<unsigned char>(node)) {
        auto &firstVector = get<array<any, 2>>(node);
        auto &treeNodes = any_cast<array<any, 2>&>(firstVector[0]);
        vector<unsigned char> &treeChars = any_cast<vector<unsigned char>&>(firstVector[1]);

        LeftNode[nodePos] = treeNodes;
        RightNode.insert(RightNode.end(), treeChars.begin(), treeChars.end());
    } else {
        unsigned char treeChar = get<unsigned char>(node);
        LeftNode[nodePos] = treeChar;
        RightNode.push_back(treeChar);
    }
}

void writeFile(string& fileExtension, ofstream& out, const string& password, int passwordHash, const any& meta, vector<unsigned char>& encodedBytes) {
    ostringstream buffer(std::ios::binary);
    unsigned char version = password.empty() ? 0x01 : 0x02;
    out.write(reinterpret_cast<char*>(&version), 1);

    unsigned char mask = 0x00;
    if (!password.empty()) mask = static_cast<unsigned char>(passwordHash & 0xFF);

    if (mask == 0x00) {
        serializeHuffmanArray(buffer, meta);
    } else {
        serializeHuffmanArray(buffer, meta, mask);
        for (int i = 0; i < fileExtension.size(); i++) {
            fileExtension[i] ^= mask;
        }
    }


    string serializedTree = buffer.str();
    uint8_t fileExtensionSize = static_cast<uint16_t>(fileExtension.size());
    uint16_t treeSize = static_cast<uint16_t>(serializedTree.size());
    uint32_t encodedSize = static_cast<uint32_t>(encodedBytes.size());

    out.write(reinterpret_cast<char*>(&fileExtensionSize), 1);
    out.write(fileExtension.data(), fileExtensionSize);
    out.write(reinterpret_cast<char*>(&treeSize), sizeof(treeSize));
    out.write(reinterpret_cast<char*>(&encodedSize), sizeof(encodedSize));
    out.write(serializedTree.data(), treeSize);

    if (mask == 0x00) {
        out.write(reinterpret_cast<char*>(encodedBytes.data()), encodedBytes.size());
    } else {
        vector<unsigned char> masked(encodedBytes.size());
        for (size_t i = 0; i < encodedBytes.size(); ++i) {
            masked[i] = static_cast<unsigned char>(encodedBytes[i] ^ mask);
        }
        out.write(reinterpret_cast<char*>(masked.data()), masked.size());
    }
}

void scrambleTree(any& node, mt19937 rng) {
    if (node.type() == typeid(unsigned char)) {
        return;
    }

    auto &v = any_cast<array<any, 2>&>(node);
    if (rng() % 2 == 0) {
        swap(v[0], v[1]);
    }

    scrambleTree(v[1], rng);
    scrambleTree(v[0], rng);
}

void Bad_Huffman(ofstream& out, const string& fileData, const string& password, string& fileExtension) {
    map<unsigned char, int> tree;
    int passwordHash = hash<string>{}(password);
    mt19937 rng(passwordHash);

    // Count frequencies
    for (unsigned char byte : fileData)
        tree[byte]++;

    auto sortedTree = flip_map_convert(tree, [](unsigned char c) -> variant<unsigned char, array<any, 2>> {
        return c;
    });

    map<unsigned char, string> treeCodes;


    while ( sortedTree.size() > 1) {
        auto it = sortedTree.begin();

        int firstFrequency = it->first;
        auto firstChar = it->second;
        ++it;
        int secondFrequency = it->first;
        auto secondChar = it->second;
        ++it;

        sortedTree.erase(sortedTree.begin(), it);

        assignCodes(firstChar, treeCodes, 0);
        assignCodes(secondChar, treeCodes, 1);

        int combinedFrequency = firstFrequency + secondFrequency;

        array<any, 2> LeftNode;
        vector<unsigned char> RightNode;

        mapNode(firstChar, LeftNode, RightNode, 0);
        mapNode(secondChar, LeftNode, RightNode, 1);

        array<any, 2> combinedNode = {LeftNode, RightNode};

        sortedTree.insert({ combinedFrequency, combinedNode });
    }

    auto encodedBytes = encodeFile(fileData, treeCodes);

    auto finalTreeParent = get<array<any, 2>>(sortedTree.begin()->second);
    any& meta = finalTreeParent[0];
    scrambleTree(meta, rng);

    writeFile(fileExtension, out, password, passwordHash, meta, encodedBytes);
}