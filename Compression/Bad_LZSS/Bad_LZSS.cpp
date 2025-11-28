#include "Bad_LZSS.h"
#include <cstdint>
#include <sstream>
#include <variant>
#include <vector>
using namespace std;

using LZ77type = tuple<unsigned long long, unsigned long long, unsigned char>;
using LZSStype = tuple<unsigned long long, unsigned long long>;
using LZSSCompleteType = variant<LZ77type, LZSStype>;

void writeFile(string& fileExtension, ofstream& out, vector<LZSSCompleteType>& results, int SearchBufferSize, int LookAheadBufferSize) {
    unsigned char version = 0x03;
    out.write(reinterpret_cast<char*>(&version), 1);

    uint8_t fileExtensionSize = fileExtension.size();
    out.write(reinterpret_cast<char*>(&fileExtensionSize), 1);
    out.write(fileExtension.data(), fileExtensionSize);

    SearchBufferSize = static_cast<uint16_t>(SearchBufferSize);
    out.write(reinterpret_cast<char*>(&SearchBufferSize), 2);

    ostringstream buffer(ios::binary);
    for (size_t i = 0; i < results.size(); i += 8) {
        uint8_t flagByte = 0;

        for (size_t j = 0; j < 8 && i + j < results.size(); j++) {
            if (holds_alternative<LZ77type>(results[i + j])) {
                flagByte |= (1 << (7 - j));
            }
        }
        buffer.put(flagByte);

        for (size_t j = 0; j < 8 && i + j < results.size(); j++) {
            if (holds_alternative<LZ77type>(results[i + j])) {
                auto& lz77 = get<LZ77type>(results[i + j]);
                unsigned char val3 = get<2>(lz77);
                buffer.write(reinterpret_cast<char*>(&val3), 1);
            } else {
                auto& [offset, length] = get<LZSStype>(results[i+j]);
                uint16_t packed = offset;
                uint8_t packed2 = length;
                buffer.write(reinterpret_cast<char*>(&packed), 2);
                buffer.write(reinterpret_cast<char*>(&packed2), 1);
            }
        }
    }

    string serializedData = buffer.str();
    uint16_t serializedDataSize = static_cast<uint16_t>(serializedData.size());

    out.write(reinterpret_cast<char*>(&serializedDataSize), sizeof(serializedDataSize));
    out.write(serializedData.data(), serializedDataSize);
}

LZSStype SearchBuffer(string& fileHistory, string& bytesForSearch) {
    const int bytesLength = bytesForSearch.size();
    bool success = true;
    unsigned long long bytesRead = 0;
    for (long long i = fileHistory.size(); i >= 0; i--) { // search through the fileHistory but from the last to first.
        success = true;
        bytesRead = 0;

        for (long long y = 0; y < bytesLength; y++) {
            long long bufIndex = i + y;

            if (fileHistory[bufIndex] != bytesForSearch[y]) {
                success = false;
                break;
            }

            bytesRead++;
        }

        if (success && bytesRead == bytesLength) {
            return {fileHistory.size() - i, bytesForSearch.length()};
        }
    }
    return {0,0};
}

LZSSCompleteType LookAheadBuffer(const string& aheadSubStr, string& searchBuffer) {
    string toSearch = "";
    LZSStype previous;
    for (const unsigned char &byte : aheadSubStr) {
        toSearch += byte;
        const auto &searchResult = SearchBuffer(searchBuffer, toSearch);
        if (get<0>(searchResult) == 0 && get<1>(searchResult) == 0) {
            break;
        }
        previous = searchResult;
    }

    if (toSearch.size() == 1) {
        LZ77type result = {0,0, aheadSubStr[0]};
        return result;
    }

    LZSStype result = {get<0>(previous), get<1>(previous)};
    return result;
}

void Bad_LZSS(string& fileExtension, ofstream& out, string& fileData, int searchBufferSize, int lookAheadBufferSize) {
    string inSearchBuffer = "";
    vector<LZSSCompleteType> results;
    for (int i = 0; i < fileData.size(); i++) {
        string aheadSubStr = fileData.substr(i, lookAheadBufferSize);
        auto result = LookAheadBuffer(aheadSubStr, inSearchBuffer);
        results.push_back(result);

        unsigned long long length = 0;
        if (holds_alternative<LZSStype>(result)) {
            length = get<1>(get<LZSStype>(result));
        } else if (holds_alternative<LZ77type>(result)) {
            length = get<1>(get<LZ77type>(result));
        }

        if (length == 0 ) {
            inSearchBuffer += fileData[i];
            if (i >= searchBufferSize) {
                inSearchBuffer.erase(0, 1);
            }
        } else {
            for (int y = 0; y < length; y++) {
                inSearchBuffer += fileData[i + y];
                if (inSearchBuffer.size() > searchBufferSize) {
                    inSearchBuffer.erase(0, 1);
                }
            }

            i += length - 1;
        }
    }

    writeFile(fileExtension, out, results, searchBufferSize, lookAheadBufferSize);
}