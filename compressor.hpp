#pragma once
#include "utils.hpp"
class haffman_compressor
{
public:
  haffman_compressor(byte_freq &bm)
  {
    s_map = get_symbol_map(bm);
  }

  symbol_map get_symbol_map(byte_freq &byte_freq)
  {
    symbol_map huff_map;
    shared_ptr<node> root = get_hff_tree(byte_freq);

    encode(root, "", huff_map);
    return huff_map;
  }

  void encode(shared_ptr<node> root, string str,
              symbol_map &huff_map)
  {
    if (root == nullptr)
      return;

    if (!root->left && !root->right)
    {
      huff_map[(size_t)root->byte] = str;
    }

    encode(root->left, str + "0", huff_map);
    encode(root->right, str + "1", huff_map);
  }

  void compress_bytes(vector<uint8_t> &data)
  {

    for (auto ch : data)
    {
      for (char &bit : s_map[ch])
      {
        int c = ('1' == bit) ? 1 : 0;
        compressed_bits.bit_add(c);
      }
    }
  }

  bits_vector *get_data()
  {
    return &compressed_bits;
  }

  void clear_data()
  {
    compressed_bits.clear();
  }

private:
  symbol_map s_map;
  bits_vector compressed_bits;
};
