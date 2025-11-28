#include <algorithm>
#include <optional>
#include <queue>
#include <sstream>
#include <random>

#include "../Huffman_Universal/Huffman_Universal.h"
#include "Huffman.h"
using namespace std;

struct Node {
    int frequency;
    optional<unsigned char> character;
    bool isLeaf;
    Node* left;
    Node* right;

    Node(int freq, unsigned char ch) : frequency(freq), character(ch), isLeaf(true), left(nullptr), right(nullptr) {}

    Node(int freq, Node* l, Node* r) : frequency(freq), character(nullopt),isLeaf(false), left(l), right(r) {}
};

struct CompareNodes {
    bool operator()(Node* a, Node* b) const {
        return a->frequency > b->frequency;
    }
};

void assignCodes(Node* root, map<unsigned char, string>& treeCodes, const string treeCode) {
    if (root->left == nullptr && root->right == nullptr) { //hit leaf
        treeCodes.insert({*root->character, treeCode});
        return;
    }
    assignCodes(root->right, treeCodes, treeCode + '1');
    assignCodes(root->left, treeCodes, treeCode + '0');
}

void serializeTree(ostream& out, Node* root, unsigned char mask = 0x00) {
    if (root->left == nullptr && root->right == nullptr) {
        out.put(0 ^ mask);
        out.put(*root->character ^ mask);
        return;
    }
    out.put(1 ^ mask);
    serializeTree(out, root->right, mask);
    serializeTree(out, root->left, mask);
}

void writeFile(string& fileExtension, ofstream& out, const string& password, int passwordHash, Node* root, vector<unsigned char>& encodedBytes) {
    unsigned char version = password.empty() ? 0x01 : 0x02;
    out.write(reinterpret_cast<char*>(&version), 1);

    unsigned char mask = 0x00;
    if (!password.empty()) mask = static_cast<unsigned char>(passwordHash & 0xFF);

    ostringstream buffer(std::ios::binary);
    if (password.empty()) {
        serializeTree(buffer, root);
    } else {
        serializeTree(buffer, root, mask);
        for (int i = 0; i < fileExtension.size(); i++) {
            fileExtension[i] ^= mask;
        }
    }

    string serializedTree = buffer.str();

    uint8_t fileExtensionSize = fileExtension.size();
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

void scrambleTree(Node* node, mt19937 rng) {
    if (node->isLeaf) {
        return;
    }

    if (rng() % 2 == 0) {
        swap(node->left, node->right);
    }

    scrambleTree(node->right, rng);
    scrambleTree(node->left, rng);
}

void Huffman(ofstream& out, const string& fileData, const string& password, string& fileExtension) {
    int passwordHash = hash<string>{}(password);
    mt19937 rng(passwordHash);
    map<unsigned char, int> tree;
    for (unsigned char byte : fileData) {
        tree[byte]++;
    }

    priority_queue<Node*, vector<Node*>, CompareNodes> NodesArray;


    for (const auto& pair : tree) {
        Node* nodePointer = new Node(pair.second, pair.first);
        NodesArray.push(nodePointer);
    }

    while (NodesArray.size() > 1) {
        auto leftNode = NodesArray.top();
        NodesArray.pop();

        auto rightNode = NodesArray.top();
        NodesArray.pop();


        Node* newInternalNode = new Node(leftNode->frequency + rightNode->frequency, leftNode, rightNode);
        NodesArray.push(newInternalNode);
    }

    Node* root = NodesArray.top();

    map<unsigned char, string> treeCodes;
    assignCodes(root, treeCodes, "");

    if (!password.empty()) {
        scrambleTree(root, rng);
    }

    auto encodedBytes = encodeFile(fileData, treeCodes);
    writeFile(fileExtension, out, password, passwordHash, root, encodedBytes);

    delete root;
}