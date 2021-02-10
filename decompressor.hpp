#pragma once
#include "utils.hpp"

class haffman_decompressor
{
public:
  haffman_decompressor(byte_freq &bm)
  {
    root = get_hff_tree(bm);
  }

  void decompress(bits_vector &bits)
  {
    int64_t index = -1;
    while (index < (int64_t)bits.get_bit_count() - 2)
    {
      decode(root, index, bits);
    }
  }

  vector<uint8_t> *get_data()
  {
    return &data;
  }

  void clear_data()
  {
    data.clear();
  }

  void decode(shared_ptr<node> root, int64_t &index, bits_vector &bits)
  {
    if (nullptr == root)
    {
      return;
    }

    if (!root->left && !root->right)
    {
      data.push_back(root->byte);
      return;
    }

    index++;

    if (bits.bit_check(index) == 0)
      decode(root->left, index, bits);
    else
      decode(root->right, index, bits);
  }

private:
  shared_ptr<node> root;
  vector<uint8_t> data;
};
