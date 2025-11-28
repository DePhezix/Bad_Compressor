#include "Huffman_Universal.h"
#include <map>
#include <string>
#include <vector>
using namespace std;

vector<unsigned char> encodeFile(const string& fileData, map<unsigned char, string>&treeCodes) {
    vector<unsigned char> encodedBytes;
    unsigned char currentByte = 0;
    int bitCount = 0;

    for (unsigned char byte : fileData) {
        const string &code = treeCodes[byte];

        for (char bit : code) {
            currentByte <<= 1;
            if (bit == '1') currentByte |= 1;

            bitCount++;

            if (bitCount == 8) {
                encodedBytes.push_back(currentByte);
                currentByte = 0;
                bitCount = 0;
            }
        }
    }

    if (bitCount > 0) {
        currentByte <<= (8 - bitCount);
        encodedBytes.push_back(currentByte);
    }

    return encodedBytes;
}
