#include "countingbloomfilter.h"
#define SALT_CONSTANT 0x97c29b3a
extern "C"
{
#include "murmur.cpp"
}
#include"iostream"
using namespace std;

CountingBloomFilter::CountingBloomFilter(unsigned int capacity, double error_rate)
{
    this->count=0;
    this->capacity = capacity;
    this->error_rate = error_rate;
    this->nfuncs = (int) ceil(log(1 / error_rate) / log(2));
    this->counts_per_func = (int) ceil(capacity * fabs(log(error_rate)) / (this->nfuncs * pow(log(2), 2)));
    this->size = this->nfuncs * this->counts_per_func;
	/*cout << this->size << " ";*/
    /* rounding-up integer divide by 2 of bloom->size */
    /*this->num_bytes = ((this->size + 1) / 2);*/
	this->num_bytes = this->size;
	/*cout << this->num_bytes;*/
    this->hashes.resize(this->nfuncs);
    this->bitmap.Resize(this->num_bytes);
}

CountingBloomFilter::~CountingBloomFilter()
{
}

void CountingBloomFilter::Hash(const std::string key)
{
    unsigned int i;
    uint32_t checksum[4];
    MurmurHash3_x64_128(key.c_str(), key.length(), SALT_CONSTANT, checksum);
    uint32_t h1 = checksum[0];
    uint32_t h2 = checksum[1];
    for (i = 0; i < this->nfuncs; i++) {			//进行了nfuncs个hash
        hashes[i] = (h1 + i * h2) % this->counts_per_func + i * this->counts_per_func;
		/*cout << hashes[i]<<" ";*/
    }
}


bool CountingBloomFilter::isFull()
{
    return this->count<(this->capacity-1)?false:true;
}



void CountingBloomFilter::Add(const std::string s)
{
    this->Hash(s);			//对s进行hash
    unsigned int i;
    for (i = 0; i < this->nfuncs; i++)
    {
		//cout << this->nfuncs << " " << hashes[i]<<endl;
        this->bitmap.Increase(hashes[i]);
		/*cout << hashes[i] << " ";*/
    }
    this->count++;

}

void CountingBloomFilter::Remove(const std::string s)
{

    this->Hash(s);
    if(this->count==0)
    {
        cerr<<"bloom is empty!"<<endl;
        return;
    }
    unsigned int i;
    for (i = 0; i < this->nfuncs; i++)
    {
        this->bitmap.Decrease(hashes[i]);
    }
    this->count--;
}

bool CountingBloomFilter::Check(const std::string s)
{
    unsigned int i;

    this->Hash(s);

    for (i = 0; i < this->nfuncs; i++)
    {
        if (!this->bitmap.Check(hashes[i])) return false;
    }
    return true;
}
