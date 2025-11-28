#include "Decompress_Huffman.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <random>

using namespace std;


struct Node {
    optional<uint8_t> character;
    bool isLeaf;
    Node* left;
    Node* right;

    Node(uint8_t ch) : character(ch), isLeaf(true), left(nullptr), right(nullptr) {}

    Node(Node* l, Node* r) : character(nullopt), isLeaf(false), left(l), right(r) {}
};
uint16_t treeIndex = 0;


vector<uint8_t> readBytes(ifstream& inFile, size_t numBytes, unsigned char mask = 0x00) {
    if (numBytes == 0) return {};

    vector<uint8_t> buffer(numBytes);
    inFile.read(reinterpret_cast<char*>(buffer.data()), numBytes);

    if (mask != 0x00) {
        for (int i = 0; i < numBytes; i++) {
            buffer[i] ^= mask;
        }
    }

    if (inFile.gcount() != static_cast<streamsize>(numBytes)) {
        cerr << "Error: Failed to read " << numBytes << " bytes from file. Read " << inFile.gcount() << " bytes instead." << endl;
        return {};
    }
    return buffer;
}


Node* rebuildTree(vector<uint8_t>& treeBytes) {
    uint8_t indicator = treeBytes[treeIndex++];

    if ((indicator & 0x01) == 0) {
        uint8_t byte = treeBytes[treeIndex++];
        return new Node(byte);
    }

    Node* node = new Node(nullptr, nullptr);

    node->right = rebuildTree(treeBytes);
    node->left = rebuildTree(treeBytes);
    return node;
}

void unscrambleTree(Node* node, mt19937 rng) {
    if (node->isLeaf) {
        return;
    }

    if (rng() % 2 == 0) {
        swap(node->left, node->right);
    }

    unscrambleTree(node->right, rng);
    unscrambleTree(node->left, rng);
}

void DecompressHuffman(ifstream& file, string& password) {
    unsigned char mask = 0x00;
    int passwordHash = hash<string>{}(password);
    mt19937 rng(passwordHash);
    file.ignore(1);

    if (!password.empty()) {
        mask = static_cast<unsigned char>(passwordHash & 0xFF);
    }

    uint8_t fileExtensionLength;
    file.read(reinterpret_cast<char*>(&fileExtensionLength), 1);

    vector<uint8_t> fileExtensionVector = readBytes(file, fileExtensionLength, mask);
    string fileExtension(fileExtensionVector.begin(), fileExtensionVector.end());

    uint16_t treeLength;
    file.read(reinterpret_cast<char*>(&treeLength), 2);

    Node* root = nullptr;

    uint32_t encodedBitsLength;

    file.read(reinterpret_cast<char*>(&encodedBitsLength), 4);

    treeIndex = 0;
    vector<uint8_t> treeBuffer = readBytes(file, treeLength, mask);
    vector<uint8_t> encodedBytes = readBytes(file, encodedBitsLength, mask);

    file.close();

    root = rebuildTree(treeBuffer);
    if (mask != 0x00) {
        unscrambleTree(root, rng);
    }
    string decodedData;
    Node* currentNode = root;

    for (size_t i = 0; i < encodedBitsLength; ++i) {
        uint8_t byte = encodedBytes[i];

        for (int j = 0; j < 8; ++j) {
            bool bit = (byte >> (7 - j)) & 1;
            if (bit == 0) {
                currentNode = currentNode->left;
            } else {
                currentNode = currentNode->right;
            }

            if (currentNode && currentNode->isLeaf) {
                decodedData += (char)currentNode->character.value();
                currentNode = root;
            }

            if (!currentNode) {
                cerr << "Error: Corrupt bitstream or tree structure encountered during decoding. Stopping at byte index " << i << "." << endl;
                goto exit;
            }
        }
    }

    exit:
    string outputName = "decompressed" + fileExtension;
    ofstream out(outputName, ios::binary);
    out.write(decodedData.c_str(), decodedData.length());
    out.close();

    delete root;

    cout << "Decompression successful. " <<  " (" << decodedData.length() << " bytes)." << endl;
}
