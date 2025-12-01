#include <bitset>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "Compression/Bad_Huffman/Bad_Huffman.h"
#include "Compression/Bad_LZSS/Bad_LZSS.h"
#include "Compression/Huffman/Huffman.h"

#include "Decompression/Decompress_Huffman/Decompress_Huffman.h"
#include "Decompression/Decompress_LZSS/Decompress_LZSS.h"
using namespace std;

int main() {
    char initialChoice;
    cout << "Compression? (y/n) " << endl;
    cin >> initialChoice;

    if (initialChoice == 'y' || initialChoice == 'Y') {
        int compressionType;
        bool correctNumberInput = false;

        char encryptionChoice;
        string password = "";

        string filePath = "";
        string fileExtension = "";
        string fileData = "";

        do {
            cout << "Choose your compression type (1-5): " << endl;
            cout << "1. Bad Huffman" << endl;
            cout << "2. Huffman" << endl;
            cout << "3. Bad LZSS" << endl;
            cout << "4. LZSS" << endl;
            cout << "5. Huffman + LZSS" << endl;
            cin >> compressionType;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (compressionType <= 5 && compressionType >= 1) {
                correctNumberInput = true;
            } else {
                cout << "Wrong Number Input. Try Again" << endl;
            }
        } while (!correctNumberInput);

        while (true) {
            cout << "File Path: ";
            getline(cin, filePath);
            ifstream originalFile(filePath, ios::in | ios::binary);

            if (originalFile.is_open()) {
                fileData.assign(
                    (istreambuf_iterator<char>(originalFile)),
                    istreambuf_iterator<char>()
                );
                fileExtension = filesystem::path(filePath).extension().string();
                originalFile.close();
                break;
            }
            cout << "File could not be located. Try Again" << endl;
        }

        cout << "Obfuscate the compressed file? (n): ";
        cin >> encryptionChoice;

        if (encryptionChoice == 'y' || encryptionChoice == 'Y') {
            cout << "Password for later decryption: ";
            cin >> password;
        }
        ofstream out("compressed.bin", ios::binary);


        switch (compressionType) {
            case 1:
                Bad_Huffman(out, fileData, password, fileExtension); break;
            case 2:
                Huffman(out, fileData, password, fileExtension); break;
            case 3:
                Bad_LZSS(fileExtension, out, fileData); break;
            case 5:

            default:
                cout << "Error Occured" << endl;
        }

        out.close();
    } else {
        string filePath = "";
        string password = "";
        uint8_t fileType;

        do {
            cout << "File Path: ";

            cin.ignore();
            getline(cin, filePath);
            cin.ignore();

            ifstream originalFile(filePath, ios::in | ios::binary);
            if (originalFile.is_open()) {
                originalFile.read(reinterpret_cast<char*>(&fileType), 1);

                originalFile.close();
                break;
            }
            cout << "File could not be opened. Try Again" << endl;
        } while (true);
        switch (fileType) {
            case 0x01:
            case 0x02: {
                if (fileType == 0x02) {
                    cout << "The file has been detected to be obfuscated. Please enter password: ";
                    getline(cin, password);
                }
                ifstream originalFile(filePath, ios::binary);
                DecompressHuffman(originalFile, password); break;
            }
            case 0x03: {
                ifstream originalFile(filePath, ios::binary);
                Decompress_LZSS(originalFile); break;
            }
            default:
                cerr << "Invalid File Type!" << endl;
        }

    }
    return 0;
}