#ifndef COUNTINGBLOOMFILTER_H
#define COUNTINGBLOOMFILTER_H
#include <cmath>
#include "bitmap.h"
#include <string>
using std::string;

class CountingBloomFilter
{
public:
    CountingBloomFilter(unsigned int capacity, double error_rate);
    ~CountingBloomFilter();

    void Add(const string s);

    void Remove(const string s);

    bool Check(const string s);
    bool isFull();

private:
    void Hash(const string key);
    unsigned int capacity;
    unsigned int counts_per_func;
    vector<uint32_t> hashes;
    size_t nfuncs;
    size_t size;
    size_t num_bytes;
    double error_rate;
    BitMap bitmap;
    uint32_t count;
};

#endif // COUNTINGBLOOMFILTER_H
