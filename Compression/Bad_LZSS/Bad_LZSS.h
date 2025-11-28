//
// Created by Mirzokhid on 11/27/2025.
//

#ifndef FRESHMAN_1STSEMESTER_PROJECT_COMPUTER_PROGRAMMING_BAD_LZSS_H
#define FRESHMAN_1STSEMESTER_PROJECT_COMPUTER_PROGRAMMING_BAD_LZSS_H

#include <fstream>
#include <string>
using namespace std;

void Bad_LZSS (string& fileExtension, ofstream& out, string& fileData, int searchBufferSize = 8192, int lookAheadBufferSize = 32);


#endif // FRESHMAN_1STSEMESTER_PROJECT_COMPUTER_PROGRAMMING_BAD_LZSS_H
