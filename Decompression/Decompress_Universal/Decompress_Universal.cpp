#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>

#include "Decompress_Universal.h"
using namespace std;

vector<uint8_t> readBytes(ifstream& inFile, size_t numBytes, unsigned char mask) {
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