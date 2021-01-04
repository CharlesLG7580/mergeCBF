#ifndef COUMPOUNDEDCOUNTINGBLOOMFILTER_H_
#define COUMPOUNDEDCOUNTINGBLOOMFILTER_H_

#include<vector>
#include<math.h>
#include<string>
#include<cmath>
using std::string;
using std::vector;


class dlcbf{
private:
	int byte_num;
	int hash_num;
	double error_rate;
	size_t size;


public:
	int g;		//barr_num
	int m;		//bit_num
	int orbarr;
	int d;    
	int B;		//bucket
	int w;  //cell
	vector<vector<int> >hashTable;//¹þÏ£×Ó±í
	dlcbf(int capacity, double false_positive);
	bool insertItem(int value);
	bool add_f(int value);
	int queryItem(int value);
	bool deleteItem(int value);
	void Hash(const std::string key);
	unsigned int capacity;
	unsigned int counts_per_func;
	vector<int >counter;
	int hashes[5];
	vector<int >a;
	size_t nfuncs;
};

#endif //COUNTINGBLOOMFILTER_H_
