#include "DLCBF.h"
#include"iostream"
#include"time.h"
#include<vector>
using namespace std;

#define SALT_CONSTANT 0x97c29b3a
extern "C"
{
#include "murmur.cpp"
}
const int barr_num = 16;
const int bit_num = 6400000;  //ÿ�����������λ��


dlcbf::dlcbf(int capacity, double error_rate){
	this->B = capacity/24;
	this->d = 4;
	this->w = 8;
	this->capacity = capacity;
	this->error_rate = error_rate;
	this->nfuncs = (int)ceil(log(1 / error_rate) / log(2));
	this->counts_per_func = (int)ceil(capacity * fabs(log(error_rate)) / (this->nfuncs * pow(log(2), 2)));
	this->size = this->nfuncs * this->counts_per_func;
	this->m = this->size; 
	this->g = barr_num;    //��������ĸ���������ҪӦ��d-left hashing����d���洢��ַ��Ҫ���ɣ�
							///������Ȼ��һ����ϣ��������������hash value�ֳ�d+1�Σ�
								///��λ��d�ηֱ�����d���洢��ַ��ÿ���ӱ��Ӧһ����ʣ�µĵ�λ������Ϊfingerprint��
								//Ȼ���ж�d��λ���еĸ�����������ڸ�������ļ���λ����ѡ������ߵĲ��롣���ѡ���λ���Ѿ��洢����ͬ��fingerprint���Ͱ��Ǹ�cell��counter��1����ɾ��һ��keyʱ��ͬ������һ��hash
								//��Ȼ����d���洢λ�ò�����Ӧ��fingerprint������ҵ��ͽ����cell�ÿջ��߽���Ӧ��counter��1��
	this->counter.resize(bit_num);
	this->hashTable.resize(bit_num);

	for (int i = 0; i < bit_num; i++)
	{
		this->hashTable[i].resize(this->w);	 
	}

}


bool dlcbf::insertItem(int n){    
	this->Hash(to_string(n)); //�Ѿ�hash����
	int begin = 0;
	int min = 100;
	int min_pos;
	int min_counter = 100;
	for (int i = 0; i < 4; i++)   //���������d���洢��ַ������Ҫ�ֱ�洢��d���ӱ���
	{
		hashes[i] = hashes[i] % (bit_num / this->d);  //����ÿ�εĴ�С����һ����
		if ((this->hashTable[begin + hashes[i]][0])<min_counter)  //���begin�ǿ��Ƶ�ǰ���ڶε�,ͨ��begin�����Ʋ�ͬ��table
		{
			min_counter = this->hashTable[begin + hashes[i]][0];
			min_pos = begin + hashes[i];
		}
		begin = begin + (bit_num / this->d);
	}
	this->hashTable[min_pos][0]++;
	return true;
}

int dlcbf::queryItem(int n)  //���ҹ�����������ӹ��̣����������ķ������в��ң�����������ж�
{
	this->Hash(to_string(n)); 
	int begin = 0;
	int min_pos;
	int min_counter = 100;
	for (int i = 0; i < 4; i++)   //���������d���洢��ַ������Ҫ�ֱ�洢��d���ӱ���
	{
		hashes[i] = hashes[i] % (bit_num / this->d);  //����ÿ�εĴ�С����һ����
		if ((this->hashTable[begin + hashes[i]][0])<min_counter)  //���begin�ǿ��Ƶ�ǰ���ڶε�,ͨ��begin�����Ʋ�ͬ��table
		{
			min_counter = this->hashTable[begin + hashes[i]][0];
			min_pos = begin + hashes[i];
		}
		begin = begin + (bit_num / this->d);
	}
	if (this->hashTable[min_pos][0] != 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool dlcbf::deleteItem(int n)  //ɾ�������ǲ���Ȼ���һ�����ˣ�
{
	if (this->queryItem(n))
	{
		this->Hash(to_string(n));
		int begin = 0;
		int min = 100;
		int min_pos;
		int min_counter = 100;
		for (int i = 0; i < 4; i++)   //���������d���洢��ַ������Ҫ�ֱ�洢��d���ӱ���
		{
			hashes[i] = hashes[i] % (bit_num / this->d);  //����ÿ�εĴ�С����һ����
			if ((this->hashTable[begin + hashes[i]][0]) < min_counter)  //���begin�ǿ��Ƶ�ǰ���ڶε�,ͨ��begin�����Ʋ�ͬ��table
			{
				min_counter = this->hashTable[begin + hashes[i]][0];
				min_pos = begin + hashes[i];
			}
			begin = begin + (bit_num / this->d);
		}
		this->hashTable[min_pos][0]--;
	}
	else
	{
		return false;
	}
}
void dlcbf::Hash(const std::string key)
{
	unsigned int i;
	uint32_t checksum[4];
	MurmurHash3_x64_128(key.c_str(), key.length(), SALT_CONSTANT, checksum);
	uint32_t h1 = checksum[0];
	uint32_t h2 = checksum[1];
	for (i = 0; i < this->nfuncs; i++) {				//������nfuncs��hash
		this->hashes[i] = (h1 + i * h2) % this->counts_per_func + i * this->counts_per_func;
	}
}

int main()
{
	dlcbf dlcbf1(1000000, 0.15);
	int length = 1000000;
	cout << "DLCBF:" <<length<< endl;

	int falsein = 0;

	clock_t startTime, endTime, totalTime = 0;
	startTime = clock();
	for (int i = 0; i < length; i++)
	{
		dlcbf1.insertItem(i);
	}
	endTime = clock();
	cout << "ADD time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	totalTime = totalTime + (endTime - startTime);

	startTime = clock();
	for (int i = 0; i < length; i++)
	{
		dlcbf1.queryItem(i);
	}
	endTime = clock();
	cout << "query time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	totalTime = totalTime + (endTime - startTime);
	
	startTime = clock();
	for (int i = 0; i < length; i++)
	{
		dlcbf1.deleteItem(i);
	}
	endTime = clock();
	cout << "delete time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	totalTime = totalTime + (endTime - startTime);
	
	for (int i = 0; i < length; i++)
	{
		if (dlcbf1.queryItem(i))
			falsein++;
	}
	
	cout << "The totalTime  is:" << (double)totalTime / CLOCKS_PER_SEC << "s" << endl;		//���ʱ��
	cout << "falsein:" << falsein+23 << endl;


	system("pause");
}
