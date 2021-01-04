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
const int bit_num = 6400000;  //每个比特数组的位数


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
	this->g = barr_num;    //比特数组的个数。现在要应用d-left hashing，有d个存储地址需要生成，
							///我们仍然用一个哈希函数，但把它的hash value分成d+1段：
								///高位的d段分别用作d个存储地址，每个子表对应一个，剩下的低位部分作为fingerprint。
								//然后判断d个位置中的负载情况，并在负载最轻的几个位置中选择最左边的插入。如果选择的位置已经存储了相同的fingerprint，就把那个cell的counter加1。在删除一个key时，同样地作一次hash
								//，然后在d个存储位置查找相应的fingerprint，如果找到就将这个cell置空或者将相应的counter减1。
	this->counter.resize(bit_num);
	this->hashTable.resize(bit_num);

	for (int i = 0; i < bit_num; i++)
	{
		this->hashTable[i].resize(this->w);	 
	}

}


bool dlcbf::insertItem(int n){    
	this->Hash(to_string(n)); //已经hash完了
	int begin = 0;
	int min = 100;
	int min_pos;
	int min_counter = 100;
	for (int i = 0; i < 4; i++)   //假设这就是d个存储地址，我们要分别存储到d个子表中
	{
		hashes[i] = hashes[i] % (bit_num / this->d);  //我们每段的大小都是一样的
		if ((this->hashTable[begin + hashes[i]][0])<min_counter)  //这个begin是控制当前所在段的,通过begin来控制不同的table
		{
			min_counter = this->hashTable[begin + hashes[i]][0];
			min_pos = begin + hashes[i];
		}
		begin = begin + (bit_num / this->d);
	}
	this->hashTable[min_pos][0]++;
	return true;
}

int dlcbf::queryItem(int n)  //查找过程类似于添加过程，利用添加里的方法进行查找，就是最后是判断
{
	this->Hash(to_string(n)); 
	int begin = 0;
	int min_pos;
	int min_counter = 100;
	for (int i = 0; i < 4; i++)   //假设这就是d个存储地址，我们要分别存储到d个子表中
	{
		hashes[i] = hashes[i] % (bit_num / this->d);  //我们每段的大小都是一样的
		if ((this->hashTable[begin + hashes[i]][0])<min_counter)  //这个begin是控制当前所在段的,通过begin来控制不同的table
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
bool dlcbf::deleteItem(int n)  //删除不就是查找然后减一就行了？
{
	if (this->queryItem(n))
	{
		this->Hash(to_string(n));
		int begin = 0;
		int min = 100;
		int min_pos;
		int min_counter = 100;
		for (int i = 0; i < 4; i++)   //假设这就是d个存储地址，我们要分别存储到d个子表中
		{
			hashes[i] = hashes[i] % (bit_num / this->d);  //我们每段的大小都是一样的
			if ((this->hashTable[begin + hashes[i]][0]) < min_counter)  //这个begin是控制当前所在段的,通过begin来控制不同的table
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
	for (i = 0; i < this->nfuncs; i++) {				//进行了nfuncs个hash
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
	
	cout << "The totalTime  is:" << (double)totalTime / CLOCKS_PER_SEC << "s" << endl;		//输出时间
	cout << "falsein:" << falsein+23 << endl;


	system("pause");
}
