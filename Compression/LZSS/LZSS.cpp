#include "LZSS.h"
#include <cstdint>
#include <iostream>
#include <variant>
#include <vector>
#include <deque>
#include <string_view>
using namespace std;

using LZSStype = tuple<unsigned long long, unsigned long long>;
using LZSSCompleteType = variant<unsigned char, LZSStype>;

void writeFile(string& fileExtension, ofstream& out, vector<LZSSCompleteType>& results) {
    unsigned char version = 0x03;
    out.write(reinterpret_cast<char*>(&version), 1);

    uint8_t fileExtensionSize = fileExtension.size();
    out.write(reinterpret_cast<char*>(&fileExtensionSize), 1);
    out.write(fileExtension.data(), fileExtensionSize);

    vector<uint8_t> dataVec;
    for (size_t i = 0; i < results.size(); i += 8) {
        uint8_t flagByte = 0;

        for (size_t j = 0; j < 8 && i + j < results.size(); j++) {
            if (holds_alternative<unsigned char>(results[i + j])) {
                flagByte |= (1u << (7 - j));
            }
        }
        dataVec.push_back(flagByte);

        for (size_t j = 0; j < 8 && i + j < results.size(); j++) {
            if (holds_alternative<unsigned char>(results[i + j])) {
                auto& charByte = get<unsigned char>(results[i + j]);
                dataVec.push_back(static_cast<uint8_t>(charByte));
            } else {
                auto [offset, length] = get<LZSStype>(results[i + j]);
                uint16_t packed = static_cast<uint16_t>(offset);
                dataVec.push_back(static_cast<uint8_t>(packed & 0xFF));
                dataVec.push_back(static_cast<uint8_t>((packed >> 8) & 0xFF));
                dataVec.push_back(static_cast<uint8_t>(length));
            }
        }
    }

    uint32_t dataSize = static_cast<uint32_t>(dataVec.size());
    out.write(reinterpret_cast<char*>(&dataSize), 4);
    out.write(reinterpret_cast<const char*>(dataVec.data()), dataVec.size());
}

LZSSCompleteType findLongestMatch(string_view lookAheadBuffer, string_view searchBuffer) {
    int longestMatchLength = 0;
    int longestMatchOffset = 0;

    for (long long i = searchBuffer.size() - 1; i >= 0; i--) {
        int currentMatchLength = 0;

        while (currentMatchLength < lookAheadBuffer.size() &&
               i + currentMatchLength < searchBuffer.size() &&
               searchBuffer[i + currentMatchLength] == lookAheadBuffer[currentMatchLength]) {
            currentMatchLength++;
        }

        if (currentMatchLength > longestMatchLength) {
            longestMatchLength = currentMatchLength;
            longestMatchOffset = searchBuffer.size() - i;
        }
    }

    if (longestMatchLength < 4) {
        return static_cast<unsigned char>(lookAheadBuffer[0]);
    } else {
        LZSStype result = {longestMatchOffset, longestMatchLength};
        return result;
    }
}

void LZSS(string& fileExtension, ofstream& out, string& fileData, int searchBufferSize, int lookAheadBufferSize) {
    deque<char> inSearchBuffer;
    vector<LZSSCompleteType> results;

    for (size_t i = 0; i < fileData.size(); i++) {
        size_t remaining = fileData.size() - i;
        size_t viewSize = min(static_cast<size_t>(lookAheadBufferSize), remaining);

        string_view aheadView(fileData.data() + i, viewSize);
        string searchBufferStr(inSearchBuffer.begin(), inSearchBuffer.end()); // convert deque to string_view safely
        auto result = findLongestMatch(aheadView, searchBufferStr);

        results.push_back(result);

        unsigned long long length = 0;
        if (holds_alternative<LZSStype>(result)) {
            length = get<1>(get<LZSStype>(result));
        }

        if (length == 0) {
            inSearchBuffer.push_back(fileData[i]);
            if (inSearchBuffer.size() > searchBufferSize) {
                inSearchBuffer.pop_front();
            }
        } else {
            for (size_t y = 0; y < length; y++) {
                inSearchBuffer.push_back(fileData[i + y]);
                if (inSearchBuffer.size() > searchBufferSize) {
                    inSearchBuffer.pop_front();
                }
            }
            i += length - 1;
        }
    }

    writeFile(fileExtension, out, results);
}
