//
// Created by Mirzokhid on 11/28/2025.
//

#ifndef FRESHMAN_1STSEMESTER_PROJECT_COMPUTER_PROGRAMMING_DECOMPRESS_UNIVERSAL_H
#define FRESHMAN_1STSEMESTER_PROJECT_COMPUTER_PROGRAMMING_DECOMPRESS_UNIVERSAL_H

#include <vector>
#include <cstdint>
#include <fstream>
using namespace std;

vector<uint8_t> readBytes(ifstream& inFile, size_t numBytes, unsigned char mask = 0x00);

#endif // FRESHMAN_1STSEMESTER_PROJECT_COMPUTER_PROGRAMMING_DECOMPRESS_UNIVERSAL_H
