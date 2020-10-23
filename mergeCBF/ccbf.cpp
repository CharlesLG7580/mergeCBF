#include "ccbf.h"
#include"iostream"
#include"time.h"
#include<vector>
using namespace std;

#define SALT_CONSTANT 0x97c29b3a
extern "C"
{
#include "murmur.cpp"
}
const int barr_num = 32;
const int bit_num = 640000;  //每个比特数组的位数


ComposableCountingrBloomFilter::ComposableCountingrBloomFilter(int capacity, double error_rate){
	this->capacity = capacity;
	this->error_rate = error_rate;
	this->nfuncs = (int)ceil(log(1 / error_rate) / log(2));
	this->counts_per_func = (int)ceil(capacity * fabs(log(error_rate)) / (this->nfuncs * pow(log(2), 2)));
	this->size = this->nfuncs * this->counts_per_func;
	this->m = this->size;
	this->g = barr_num;    //比特数组的个数
	this->counter.resize(bit_num);
	this->a.resize(bit_num);
	this->orBarr.resize(bit_num);
	this->barr.resize(barr_num);
	this->matrix.resize(bit_num);
	for (int i = 0; i < barr_num; i++)
	{
		this->barr[i].resize(bit_num);
	}
	for(int i = 0; i < bit_num;i++)
	{
		this->matrix[i].resize(barr_num);
	}
	this->randMatrix();
}
ComposableCountingrBloomFilter::~ComposableCountingrBloomFilter()
{
}

bool ComposableCountingrBloomFilter::update_orbarr(){
	int result = 0;
	for (int i = 0; i < m; i++){
		for (int j = 0; j < g; j++){
			result = result | this->barr[j][i];		//进行或运算
		}
		this->orBarr[i] = result;
		result = 0;
	}
	return true;
}
bool ComposableCountingrBloomFilter::insertItem(int n){
	this->Hash(to_string(n));
	unsigned int i;
	int x,t;
	for (i = 0; i < nfuncs; i++){
		x = hashes[i];
		t = this->matrix[x][this->counter[x]] - 1;	//要选取的数组
		this->counter[x] += 1;	//计数
		this->barr[t][x] = 1;
	}
	return true;
}
bool ComposableCountingrBloomFilter::queryItem(int n){
	this->Hash(to_string(n));
	unsigned int i;
	for (i = 0; i < nfuncs; i++){
		if (this->orBarr[(hashes[i])] == 0){			//直接查看orBarr的对应单元是否为1，orBarr是g个bit数组或的结果
			return false;
		}
	}
	return true;
}

bool ComposableCountingrBloomFilter::deleteItem(int n){
	if (queryItem(n)){
		this->Hash(to_string(n));
		unsigned int i;
		int pos, x;
		for (i = 0; i < nfuncs; i++){
			x = hashes[i];					//找上次的添加的位置，this->counter会记录hash值x到第几个了
			pos = this->matrix[x][this->counter[x] - 1];
			this->barr[pos - 1][x] = 0;
		}
	}
	return true;
}

void ComposableCountingrBloomFilter::mergeBloom(ComposableCountingrBloomFilter bloom2)  //按顺序合并各个比特数组
{

	//int size1, size2;
	//if (size1 + size2 >= n){		//判断一下合并的数量是否超过容量
	//	return false;
	//}
	for (int i = 0; i < g; i++){	//g个比特数组的每一位
		for (int j = 0; j < m; j++){
			this->barr[i][j] = bloom2.barr[i][j] | this->barr[i][j];		//十六位把每一位都或一下
		}
	}

	for (int i = 0; i < m; i++)
	{
		this->orBarr[i] = this->orBarr[i] | bloom2.orBarr[i];
	}
	//更新一下orbarr
}

void ComposableCountingrBloomFilter::Hash(const std::string key)
{
	unsigned int i;
	uint32_t checksum[4];
	MurmurHash3_x64_128(key.c_str(), key.length(), SALT_CONSTANT, checksum);
	uint32_t h1 = checksum[0];
	uint32_t h2 = checksum[1];
	for (i = 0; i < this->nfuncs; i++) {				//进行了nfuncs个hash
		hashes[i] = (h1 + i * h2) % this->counts_per_func + i * this->counts_per_func;
	}
}
void ComposableCountingrBloomFilter::randMatrix()
{
	srand((unsigned)time(NULL));
	for (int i = 0; i < this->g; ++i)
		this->a[i] = i + 1;
	int index, temp, rest = this->g;			//全局变量不能乱定义，只能在开头定义一次，如果在代码中定义的话就会覆盖掉
	for (int j = 0; j <this->m; j++){			//随机矩阵的生成		是个g行m列的
		for (int i = 0; i < this->g; ++i) {
			index = rand() % rest;
			this->matrix[j][i] = this->a[index];
			temp = this->a[index];
			this->a[index] = this->a[rest - 1];
			this->a[rest - 1] = temp;
			rest--;
		}
		rest = this->g;
	}
}

