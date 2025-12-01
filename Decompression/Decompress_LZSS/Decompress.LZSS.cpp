#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "../Decompress_Universal/Decompress_Universal.h"
#include "Decompress_LZSS.h"
using namespace std;

void Decompress_LZSS(ifstream& file) {
    file.ignore(1);

    uint8_t fileExtensionLength;
    file.read(reinterpret_cast<char*>(&fileExtensionLength), 1);

    vector<uint8_t> fileExtensionVector = readBytes(file, fileExtensionLength);
    string fileExtension(fileExtensionVector.begin(), fileExtensionVector.end());

    uint32_t dataSize;
    file.read(reinterpret_cast<char*>(&dataSize), 4);

    vector<uint8_t> dataVector = readBytes(file, dataSize);
    bool flagByte = true;
    int indexByFlag = 0;
    array<bool, 8> indicators;
    string decodedData = "";

    for (long long i = 0; i < dataSize; i++) {
        uint8_t byte = dataVector[i];

        if (flagByte) {
            for (int j = 0; j < 8; ++j) {
                bool bit = (byte >> (7 - j)) & 1;
                indicators[j] = bit;
            }
            flagByte = false;
            continue;
        }

        if (indicators[indexByFlag] == 0) {
            decodedData.push_back(static_cast<char>(dataVector[i]));
        } else {
            uint16_t offset = dataVector[i] | (dataVector[i+1] << 8);
            uint8_t length = dataVector[i+2];

            size_t base = decodedData.size() - offset;
            for (int j = 0; j < length; j++) {
                decodedData.push_back(decodedData[base + j]); // may need -1 to account that index is -1 of size
            }

            i += 2;
        }

        ++indexByFlag;
        if (indexByFlag >= 8) {
            flagByte = true;
            indexByFlag = 0;
        }
    }

    string outputName = "decompressed" + fileExtension;
    ofstream out(outputName, ios::binary);
    out.write(decodedData.c_str(), decodedData.length());
    out.close();

    cout << "Decompression successful. " <<  " (" << decodedData.length() << " bytes)." << endl;
}