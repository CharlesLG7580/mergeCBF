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
const int bit_num = 640000;  //ÿ�����������λ��


ComposableCountingrBloomFilter::ComposableCountingrBloomFilter(int capacity, double error_rate){
	this->capacity = capacity;
	this->error_rate = error_rate;
	this->nfuncs = (int)ceil(log(1 / error_rate) / log(2));
	this->counts_per_func = (int)ceil(capacity * fabs(log(error_rate)) / (this->nfuncs * pow(log(2), 2)));
	this->size = this->nfuncs * this->counts_per_func;
	this->m = this->size;
	this->g = barr_num;    //��������ĸ���
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
			result = result | this->barr[j][i];		//���л�����
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
		t = this->matrix[x][this->counter[x]] - 1;	//Ҫѡȡ������
		this->counter[x] += 1;	//����
		this->barr[t][x] = 1;
	}
	return true;
}
bool ComposableCountingrBloomFilter::queryItem(int n){
	this->Hash(to_string(n));
	unsigned int i;
	for (i = 0; i < nfuncs; i++){
		if (this->orBarr[(hashes[i])] == 0){			//ֱ�Ӳ鿴orBarr�Ķ�Ӧ��Ԫ�Ƿ�Ϊ1��orBarr��g��bit�����Ľ��
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
			x = hashes[i];					//���ϴε���ӵ�λ�ã�this->counter���¼hashֵx���ڼ�����
			pos = this->matrix[x][this->counter[x] - 1];
			this->barr[pos - 1][x] = 0;
		}
	}
	return true;
}

void ComposableCountingrBloomFilter::mergeBloom(ComposableCountingrBloomFilter bloom2)  //��˳��ϲ�������������
{

	//int size1, size2;
	//if (size1 + size2 >= n){		//�ж�һ�ºϲ��������Ƿ񳬹�����
	//	return false;
	//}
	for (int i = 0; i < g; i++){	//g�����������ÿһλ
		for (int j = 0; j < m; j++){
			this->barr[i][j] = bloom2.barr[i][j] | this->barr[i][j];		//ʮ��λ��ÿһλ����һ��
		}
	}

	for (int i = 0; i < m; i++)
	{
		this->orBarr[i] = this->orBarr[i] | bloom2.orBarr[i];
	}
	//����һ��orbarr
}

void ComposableCountingrBloomFilter::Hash(const std::string key)
{
	unsigned int i;
	uint32_t checksum[4];
	MurmurHash3_x64_128(key.c_str(), key.length(), SALT_CONSTANT, checksum);
	uint32_t h1 = checksum[0];
	uint32_t h2 = checksum[1];
	for (i = 0; i < this->nfuncs; i++) {				//������nfuncs��hash
		hashes[i] = (h1 + i * h2) % this->counts_per_func + i * this->counts_per_func;
	}
}
void ComposableCountingrBloomFilter::randMatrix()
{
	srand((unsigned)time(NULL));
	for (int i = 0; i < this->g; ++i)
		this->a[i] = i + 1;
	int index, temp, rest = this->g;			//ȫ�ֱ��������Ҷ��壬ֻ���ڿ�ͷ����һ�Σ�����ڴ����ж���Ļ��ͻḲ�ǵ�
	for (int j = 0; j <this->m; j++){			//������������		�Ǹ�g��m�е�
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

