cmake_minimum_required(VERSION 4.0)
project(Freshman_1stSemester_Project_Computer_Programming)

set(CMAKE_CXX_STANDARD 20)

find_package(crow CONFIG REQUIRED)

add_executable(my_server main.cpp)
target_link_libraries(my_server PRIVATE crow)

add_executable(Freshman_1stSemester_Project_Computer_Programming main.cpp
        Compression/Bad_Huffman/Bad_Huffman.cpp
        Compression/Bad_Huffman/Bad_Huffman.h
        Compression/Huffman_Universal/Huffman_Universal.h
        Compression/Huffman_Universal/Huffman_Universal.cpp
        Compression/Huffman/Huffman.cpp
        Compression/Huffman/Huffman.h
        Compression/Bad_LZSS/Bad_LZSS.cpp
        Compression/Bad_LZSS/Bad_LZSS.h
        Decompression/Decompress_Huffman/Decompress_Huffman.cpp
        Decompression/Decompress_LZSS/Decompress_LZSS.h
        Decompression/Decompress_LZSS/Decompress.LZSS.cpp
        Decompression/Decompress_Universal/Decompress_Universal.h
        Decompression/Decompress_Universal/Decompress_Universal.cpp
        Compression/LZSS/LZSS.cpp
        Compression/LZSS/LZSS.h)
