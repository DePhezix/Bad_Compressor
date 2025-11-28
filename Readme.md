# Compression & Decompression Tool

This project is a console-based file **compression** and **decompression** tool created for my Computer Science semester project.  
It supports multiple compression algorithms and optional password-based obfuscation.

---

## ️ Features

- **Compression Algorithms**
    - Bad Huffman
    - Huffman
    - Bad LZSS
    - LZSS (to be implemented)
    - Huffman + LZSS (to be implemented)
- **Optional Obfuscation** for compressed output
- **Automatic File Type Detection** during decompression
- **Binary compression output:** `compressed.bin`
- **Universal decompression output:** `decompressed`

---

## How to Use (Compression)

### 1. Run the Program

Compile and run the program, using the built in build button of your chosen IDE.

### 2. Enter Compression Mode
When starting the program, you will be prompted with:
````bash 
Compression? (y/n)
````
Type ````Y````  or ````y```` to enter the compression mode.

### 3. Choose your Compression Type
You will be prompted with:
````bash
Choose your compression type (1-5):
1. Bad Huffman
2. Huffman
3. Bad LZSS
4. LZSS
5. Huffman + LZSS
````
Enter a number from **1 to 5**, depending on your desired compression method.

### 4. Enter File Path
````bash
File path:
````
Provide the path to the file you want to compress, for example:
```C:\Users\Me\Documents\text.txt```

The program automatically loads the file and automatically extracts its extension once you hit Enter.

**Tip:**
You can quickly copy a file path by either :

- Right-click → **Copy as path**, or

- Select the file → press **Ctrl + Shift + C**

### 5. Optional Obfuscation
````bash
Obfuscate the compressed file? (n):
````
- Type ````Y```` or ````y```` to agree
- Type anything else to disagree

If obfuscation is chosen:
````bash
Password for later decryption:
````

### 6. Output
The compressed file is always saved as:
````compressed.bin````
and will usually be located in the  ````cmake-build-debug```` folder.

---
## How to use (Decompression)

### 1. Run the Program

Compile and run the program, using the built in build button of your chosen IDE.

### 2. Enter Decompression Mode
When starting the program, you will be prompted with:
````bash 
Compression? (y/n)
````
Type ````N````  or ````n```` to enter the decompression mode.

### 3. Enter Path to Compressed File
````bash
File Path:
````

Provide the path to the .bin compressed file.

The program reads the first byte to identify the compression type.
If the file is obfuscated:

````bash
The file has been detected to be obfuscated. Please enter password:
````
Enter the password used during compression. Failure to provide the correct password will lead to a corrupted/empty decompressed file.

### 4. Output
Decompressed text is written to:
````decompressed.[original file extension]```` or ````decompressed.txt```` if it failed decompression.