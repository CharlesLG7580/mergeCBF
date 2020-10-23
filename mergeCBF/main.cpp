#include<iostream>
using namespace std;
#include"ccbf.h"
#include"time.h"
int main(int argc, const char * argv[]) {
	ComposableCountingrBloomFilter bloom1(100000, 0.15);
	/*ComposableCountingrBloomFilter bloom2(100000, 0.05);
	ComposableCountingrBloomFilter bloom3(100000, 0.05);
	ComposableCountingrBloomFilter bloom4(100000, 0.05);
	ComposableCountingrBloomFilter bloom5(100000, 0.05);
	ComposableCountingrBloomFilter bloom6(100000, 0.05);*/


	int length = 100000;
	int in = 0;
	int falsein = 0;

	cout << "CCBF---" << length << endl;
	clock_t startTime, endTime, totalTime = 0;
	startTime = clock();
	for (int i = 0; i < length; i++)
	{
		bloom1.insertItem(i);
	}


	bloom1.update_orbarr();				//添加完以后要更新
	endTime = clock();
	cout << "Add time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;


	totalTime = totalTime + (endTime - startTime);

	startTime = clock();
	for (int i = 0; i < length; i++){
		bloom1.queryItem(i);
	}
	endTime = clock();
	cout << "Query time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	totalTime = totalTime + (endTime - startTime);

	

	startTime = clock();
	for (int i = 0; i < length; i++)
	{
		bloom1.deleteItem(i);
	}
	bloom1.update_orbarr();
	endTime = clock();
	cout << "delete time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	totalTime = totalTime + (endTime - startTime);
	
	for (int i = length; i < length * 2; i++)
	{
		if (bloom1.queryItem(i))
			falsein++;
	}
	cout << "The totalTime  is:" << (double)totalTime / CLOCKS_PER_SEC << "s" << endl;		//输出时间

	
	
	cout << "falsein:" << falsein << endl;

	
	int num_bloom = 5;
	//for (int i = 0; i <= length/2; i++)
	//{
	//	int m = rand() % 50000;
	//	//cout << m<<" ";
	//	bloom2.insertItem(m);
	//	bloom3.insertItem(m);
	//	//bloom4.insertItem(i);
	//	//bloom5.insertItem(i);
	//	//bloom6.insertItem(i);
	//}
	//
	//startTime = clock();
	//bloom2.mergeBloom(bloom3);
	////bloom2.mergeBloom(bloom4);
	////bloom2.mergeBloom(bloom5);
	////bloom2.mergeBloom(bloom6);

	//endTime = clock();
	//cout << "合并内容" <<num_bloom << "个：50000个 " << endl;
	//cout << "merege time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	system("pause");
	return 0;
}


