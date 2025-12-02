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
#include "Compression/LZSS/LZSS.h"

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
            cout << "Choose your compression type (1-6): " << endl;
            cout << "1. Bad Huffman" << endl;
            cout << "2. Huffman" << endl;
            cout << "3. Bad LZSS" << endl;
            cout << "4. LZSS" << endl;
            cout << "5. Huffman + LZSS" << endl;
            cout << "6. LZSS + Huffman" << endl;
            cin >> compressionType;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (compressionType <= 6 && compressionType >= 1) {
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
            case 4:
                LZSS(fileExtension, out, fileData); break;
            case 5: {
                ofstream tempOut("temp.bin", ios::binary);
                Huffman(tempOut, fileData, password, fileExtension);
                tempOut.close();

                ifstream compressedFile("temp.bin", ios::binary);
                fileData.assign(istreambuf_iterator<char>(compressedFile), istreambuf_iterator<char>());
                fileExtension = filesystem::path("temp.bin").extension().string();

                compressedFile.close();
                remove("temp.bin");

                LZSS(fileExtension, out, fileData);
                out.seekp(0, ios::beg);
                unsigned char newVersion = password.empty() ? 0x04 : 0x05;
                out.write(reinterpret_cast<char*>(&newVersion), 1);
                break;
            }
            case 6: {
                ofstream tempOut("temp.bin", ios::binary);
                LZSS(fileExtension, tempOut, fileData);
                tempOut.close();

                ifstream compressedFile("temp.bin", ios::binary);
                fileData.assign(istreambuf_iterator<char>(compressedFile), istreambuf_iterator<char>());
                fileExtension = filesystem::path("temp.bin").extension().string();

                compressedFile.close();
                remove("temp.bin");

                Huffman(out, fileData, password, fileExtension);
                out.seekp(0, ios::beg);
                unsigned char newVersion = password.empty() ? 0x06 : 0x07;
                out.write(reinterpret_cast<char*>(&newVersion), 1);
                break;
            }
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

            getline(cin, filePath);

            ifstream originalFile(filePath, ios::in | ios::binary);
            if (originalFile.is_open()) {
                originalFile.read(reinterpret_cast<char*>(&fileType), 1);

                originalFile.close();
                break;
            }
            cout << "File could not be opened. Try Again" << endl;
        } while (true);
        if (fileType == 0x02 || fileType == 0x05 || fileType == 0x07) {
            cout << "The file has been detected to be obfuscated. Please enter password: ";
            getline(cin, password);
        }
        switch (fileType) {
            case 0x01:
            case 0x02: {
                ifstream originalFile(filePath, ios::binary);
                DecompressHuffman(originalFile, password); break;
            }
            case 0x03: {
                ifstream originalFile(filePath, ios::binary);
                Decompress_LZSS(originalFile); break;
            }
            case 0x04:
            case 0x05:
                {
                ifstream originalFile(filePath, ios::binary);
                Decompress_LZSS(originalFile);

                ifstream secondFile("decompressed.bin", ios::binary);
                DecompressHuffman(secondFile, password);

                remove("decompressed.bin");
                break;
            }
            case 0x06:
            case 0x07: {
                ifstream originalFile(filePath, ios::binary);
                DecompressHuffman(originalFile, password);

                ifstream secondFile("decompressed.bin", ios::binary);
                Decompress_LZSS(secondFile);

                remove("decompressed.bin");
                break;
            }
            default:
                cerr << "Invalid File Type!" << endl;
        }

    }
    return 0;
}