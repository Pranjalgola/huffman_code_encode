#define _FILE_OFFSET_BITS 64
#include <iostream>
#include <fstream>
#include <cstdint>

#include <array>
#include <vector>

#include "compressor.hpp"
#include "decompressor.hpp"

using namespace std;

int main(int argc, char **argv)
{
     string inpath;
     string outpath;
     if (argc == 3)
     {
          inpath = argv[1];
          outpath = argv[2];
     }
     else
     {
          cout << " using: " << argv[0] << " <input-file> <output-file>" << endl;
          return 0;
     }

     byte_freq bf{};

     bool read_ok = file_binary_read(inpath, [&](vector<uint8_t> rbuf) {
          get_byte_freq((bf), move(rbuf));
          return;
     });

     if (!read_ok)
     {
          cout << " Error reading " << inpath << endl;
          return 1;
     }

     haffman_compressor hff_cmprs(bf);

     ofstream file_o;
     file_o.open(outpath);
     if (!file_o.is_open())
     {
          cout << " Error writing " << outpath << endl;
          return 2;
     }
     if (!write_hff_table(file_o, bf))
     {
          cout << " Error writing " << inpath << endl;
          return 3;
     }

     bool rd_compress_ok = file_binary_read(inpath, [&](vector<uint8_t> rbuf) {
          hff_cmprs.compress_bytes(rbuf);
          bits_vector *bv = hff_cmprs.get_data();
          uint64_t block_size = bv->get_bit_count();
          file_o.write((char *)&block_size, sizeof(block_size));
          file_o.write((char *)bv->bytes()->data(), bv->bytes()->size());
          hff_cmprs.clear_data();
     });

     file_o.close();
     if (!rd_compress_ok)
     {
          cout << " Error reading " << inpath << endl;
          return 4;
     }

     return 0;
}
