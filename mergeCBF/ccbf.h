#ifndef COUMPOUNDEDCOUNTINGBLOOMFILTER_H_
#define COUMPOUNDEDCOUNTINGBLOOMFILTER_H_

#include<vector>
#include<math.h>
#include<string>
#include<cmath>
using std::string;
using std::vector;


class ComposableCountingrBloomFilter{

public:
	
	ComposableCountingrBloomFilter(int capacity, double false_positive);
	~ComposableCountingrBloomFilter();
	bool insertItem(int value);
	void mergeBloom(ComposableCountingrBloomFilter bloom2);
	bool queryItem(int n);
	bool update_orbarr();
	bool deleteItem(int value);
	void Hash(const std::string key);
	void randMatrix();
	
	vector<int >counter ;
	int  hashes[5];
	vector<int >a;
	
private:
	int g;		//barr_num
	int m;		//bit_num
	vector<vector<int> >barr;//定义二维数组swp[][]，n行 m列
	vector<vector<int> >matrix;
	vector<int >orBarr;
	unsigned int capacity;
	unsigned int counts_per_func;
	double error_rate;
	size_t size;
	size_t nfuncs;
};

#endif //COUNTINGBLOOMFILTER_H_
