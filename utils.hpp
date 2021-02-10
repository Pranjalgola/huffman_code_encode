#pragma once
#include <iostream>
#include <cstdint>
#include <vector>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <array>
#include <chrono>
#include <vector>
#include <list>
#include <numeric>
#include <memory>
#include <atomic>
#include <future>
#include <queue>
#include <iterator>
#include <unordered_map>

#include <utility> // std::pair, std::make_pair

#include <string>
#include <map>
#include <random>
#include <cmath>
#include <functional>

using namespace std;

size_t cache_sz = 256 * 1024;
typedef array<uint64_t, 256> byte_freq;
typedef array<string, 256> symbol_map;

struct node
{
    node(uint8_t p_byte, int p_freq, shared_ptr<node> p_left, shared_ptr<node> p_right) : byte(p_byte), freq(p_freq), left(p_left), right(p_right)
    {
    }
    uint8_t byte;
    uint64_t freq;
    shared_ptr<node> left, right;
};

class bits_vector
{
public:
    using addr_mask = pair<size_t, uint8_t>;
    bits_vector(size_t max_items = 20512)
    {
        vec.reserve(max_items);
        count_bits = 0;
    }

    bits_vector(vector<uint8_t> &data)
    {
        vec = data;
        count_bits = vec.size() * 8;
    }

    bits_vector(vector<uint8_t> &&data)
    {
        vec = move(data);
        count_bits = vec.size() * 8;
    }

    bool bit_check(size_t bit)
    {
        uint8_t *ar = vec.data();
        size_t addr = bit / 8;
        size_t offb = bit - (addr * 8);
        uint8_t mask = 0xff;
        mask >>= offb;
        mask &= 0x01;
        mask <<= offb;
        return (ar[addr] & mask) != 0;
    }

    void bit_clear(size_t bit)
    {
        addr_mask address_mask = prepare_bit_change(bit);
        vec[address_mask.first] &= ~address_mask.second;
    }

    void bit_set(size_t bit)
    {
        addr_mask address_mask = prepare_bit_change(bit);
        vec[address_mask.first] |= address_mask.second;
    }

    void bit_add(size_t bit)
    {
        if (bit)
            bit_set(count_bits);
        else
            bit_clear(count_bits);
    }

    vector<uint8_t> *bytes()
    {
        return &vec;
    }

    void print_bits()
    {
        cout << " cnt " << count_bits << endl;
        for (size_t i = 0; i < count_bits; ++i)
        {
            char c = bit_check(i) ? '1' : '0';
            cout << c;
            if ((i + 1) % 4 == 0)
                cout << ' ';
        }
        cout << endl;
    }
    size_t get_bit_count()
    {
        return count_bits;
    }

    void set_bit_count(size_t c)
    {
        count_bits = c;
        size_t bytes_sz = count_bits / 8;
        if (vec.size() < bytes_sz)
        {
            vec.resize(++bytes_sz);
        }
    }

    void clear()
    {
        count_bits = 0;
        vec.clear();
    }

private:
    addr_mask prepare_bit_change(size_t bit)
    {
        addr_mask address_mask;

        address_mask.first = bit / 8;

        if (count_bits < (bit + 1))
            count_bits = (bit + 1);
        size_t bytes_sz = count_bits / 8;

        if (vec.size() <= bytes_sz)
        {
            vec.resize(++bytes_sz);
        }

        size_t offb = bit - (address_mask.first * 8);
        address_mask.second = 0xff;
        address_mask.second >>= offb;
        address_mask.second &= 0x01;
        address_mask.second <<= offb;
        return address_mask;
    }
    vector<uint8_t> vec;
    size_t count_bits;
};

void get_byte_freq(byte_freq &bm, vector<uint8_t> buf)
{

    for (auto &b : buf)
    {
        ++bm[static_cast<size_t>(b)];
    }
    return;
}

bool write_hff_table(ofstream &file_o, byte_freq &byte_freq)
{
    bool wr_ok = true;
    for (size_t i = 0; i < byte_freq.size(); ++i)
    {
        if (byte_freq[i] != 0)
        {
            char ch = static_cast<char>(i);
            wr_ok &= file_o.write(&ch, sizeof(ch)).good();
            wr_ok &= file_o.write((char *)&byte_freq[i], sizeof(byte_freq[i])).good();
        }
    }

    // null as terminate table
    char term_c = 0x00;
    uint64_t vall_null = 0;
    wr_ok &= file_o.write(&term_c, sizeof(term_c)).good();
    ;
    wr_ok &= file_o.write((char *)&vall_null, sizeof(vall_null)).good();

    return wr_ok;
}

pair<byte_freq, size_t> read_hff_table(ifstream &file_o)
{
    pair<byte_freq, size_t> bf_s;
    size_t counter_ch = 0;
    for (;; ++counter_ch)
    {
        uint8_t byte;
        uint64_t count;
        auto res_read_b = file_o.read((char *)&byte, sizeof(byte)).good();
        auto res_read_8b = file_o.read((char *)&count, sizeof(count)).good();

        if (!byte && !count)
            break;

        if (!res_read_b || !res_read_8b)
            break;
        bf_s.first[byte] = count;
    }

    bf_s.second = counter_ch;

    return bf_s;
}

shared_ptr<node> get_hff_tree(byte_freq byte_freq)
{

    auto node_comp = ([](shared_ptr<node> l, shared_ptr<node> r) {
        return l->freq > r->freq;
    });

    priority_queue<shared_ptr<node>, vector<shared_ptr<node>>, decltype(node_comp)> nodes_pq(node_comp);

    for (size_t i = 0; i < byte_freq.size(); ++i)
    {
        if (byte_freq[i])
        {
            nodes_pq.push(make_shared<node>(static_cast<char>(i), byte_freq[i], nullptr, nullptr));
        }
    }

    while (nodes_pq.size() != 1)
    {
        shared_ptr<node> left = nodes_pq.top();
        nodes_pq.pop();
        shared_ptr<node> right = nodes_pq.top();
        nodes_pq.pop();
        int tn_sum = right->freq + left->freq;
        nodes_pq.push(make_shared<node>(0x00, tn_sum, left, right));
    }

    shared_ptr<node> root = nodes_pq.top();

    return root;
}

using process_data = std::function<void(vector<uint8_t> result)>;

bool file_binary_read(const string filename, process_data file_process)
{
    auto file_r = std::ifstream(filename, std::ios::binary);
    vector<uint8_t> rbuf;
    rbuf.reserve(cache_sz); //

    if (file_r.is_open())
    {
        for (;;)
        {
            rbuf.resize(cache_sz);
            istream &res_read = file_r.read((char *)rbuf.data(), rbuf.size());
            if (res_read.gcount() > 0)
            {
                rbuf.resize(res_read.gcount());
                file_process(move(rbuf));
            }
            else
            {
                file_r.close();
                return true;
            }
        }
    }
    else
        return false;
}
