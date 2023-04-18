#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Bytes {
  unsigned char *data = nullptr;
  size_t size = 0;
  size_t beg = 0;

  Bytes(const std::string &filepath) {
    std::ifstream ifs(filepath, std::ifstream::binary);
    if (!ifs) {
      std::cerr << "ERROR: Failed to open file '" << filepath
                << "': " << std::strerror(errno) << "!\n";
      exit(1);
    }

    // get size of file in bytes
    ifs.seekg(0, std::ifstream::end);
    size = ifs.tellg();
    ifs.seekg(0, std::ifstream::beg);

    // allocate a buffer for storing the bytes
    data = new unsigned char[size];

    // read all bytes
    ifs.read((char *)data, size);

    ifs.close();
  }

  ~Bytes() {
    if (data != nullptr)
      delete[] data;
  }

  bool empty() { return size <= 0; }

  uint64_t get(size_t n) {
    uint64_t result = 0;

    for (size_t i = 0; i < n; ++i) {
      result <<= 8;
      result |= (int)data[beg + i];
    }

    beg += n;
    size -= n;

    return result;
  }

  void printBytes(size_t n) {
    for (size_t i = 0; i < n; i++) {
      std::cout << get(1) << (i < n - 1 ? ", " : "\n");
    }
  }

  void printChars(size_t n) {
    for (size_t i = 0; i < n; i++) {
      std::cout << (char)get(1) << (i < n - 1 ? ", " : "\n");
    }
  }
};

void usage(const char *prog) {
  std::cout << "Usage: " << prog << " <PNG-filepath>\n";
}

int main(int argc, char *argv[]) {

  if (argc - 1 < 1) {
    std::cerr << "ERROR: No input file provided!\n";
    usage(argv[0]);
    exit(1);
  }

  std::string pngfilename = argv[1];

  Bytes b(pngfilename);

  b.printBytes(8);

  while (!b.empty()) { // read chunk
    std::cout << "\nREADING CHUNK\n";
    uint32_t data_len = (uint32_t)b.get(4);
    std::cout << "Chunk Data Len: " << data_len << "\n";
    uint32_t chunk_type = (uint32_t)b.get(4);
    char chunk_code[5];
    chunk_code[0] = (char)(chunk_type >> 3 * 8);
    chunk_code[1] = (char)(chunk_type >> 2 * 8);
    chunk_code[2] = (char)(chunk_type >> 1 * 8);
    chunk_code[3] = (char)(chunk_type >> 0 * 8);
    chunk_code[4] = '\0';
    std::cout << "Chunk Code: " << chunk_code << "\n";

    // std::cout << "Ancillary Bit: "
    //           << (((chunk_type >> 3 * 8) & 0b10000) == 0 ? "Critical"
    //                                                      : "Ancillary")
    //           << "\n";

    // std::cout << "Private Bit: "
    //           << (((chunk_type >> 2 * 8) & 0b10000) == 0 ? "Public"
    //                                                      : "Private")
    //           << "\n";

    // std::cout << "Reserved Bit: "
    //           << (((chunk_type >> 1 * 8) & 0b10000) == 0 ? "0" : "1") <<
    //           "\n";

    // std::cout << "Safe-to-copy Bit: "
    //           << (((chunk_type >> 0 * 8) & 0b10000) == 0 ? "Unsafe" :
    //           "Safe")
    //           << "\n";

    // Read chunk data
    if (data_len > 0) {
      uint8_t *chunk_data = new uint8_t[data_len];
      std::cout << "Data: {";
      for (size_t i = 0; i < data_len; ++i) {
        chunk_data[i] = (uint8_t)b.get(1);
        std::cout << (int)chunk_data[i] << (i < data_len - 1 ? ", " : "");
      }
      std::cout << "}\n";

      delete[] chunk_data;
    }

    // CRC
    uint32_t crc = (uint32_t)b.get(4);
    std::cout << "CRC: " << (int)crc << "\n";
  }

  std::cin.get();
  return 0;
}
