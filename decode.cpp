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

     auto file_r_dec = std::ifstream(inpath, std::ios::binary);

     if (!file_r_dec.is_open())
     {
          cout << " Error reading " << inpath << endl;
          return 1;
     }

     auto file_w_dec = ofstream(outpath, std::ios::binary);
     if (!file_w_dec.is_open())
     {
          cout << " Error writing " << outpath << endl;
          return 2;
     }

     auto bf_r = read_hff_table(file_r_dec);

     vector<uint8_t> rbuf;
     rbuf.reserve(cache_sz);

     haffman_decompressor hff_decomprs(bf_r.first);
     for (;;)
     {
          uint64_t block_size = 0;
          file_r_dec.read((char *)&block_size, sizeof(block_size));
          rbuf.resize(block_size / 8 + 1);
          istream &res_read = file_r_dec.read((char *)rbuf.data(), rbuf.size());
          if (res_read.gcount() > 0)
          {
               rbuf.resize(res_read.gcount());
               bits_vector bd(move(rbuf));
               bd.set_bit_count(block_size);
               hff_decomprs.decompress(bd);
               vector<uint8_t> *result = hff_decomprs.get_data();
               //
               file_w_dec.write((char *)(*result).data(), (*result).size());
               hff_decomprs.clear_data();
          }
          else
          {
               file_w_dec.close();
               break;
          }
     }
}
