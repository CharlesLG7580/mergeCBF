#include <iostream>
#include "countingbloomfilter.h"
#include <string>
#include"time.h"
using namespace std;

int main()
{
    const double error_rate = 0.05;
    const int capacity = 1000000;
    int in = 0;
    int falsein = 0;
	cout << "CBF---1000000" << endl;
    CountingBloomFilter cbf(capacity,error_rate);
	clock_t startTime, endTime,totalTime=0;
	startTime = clock();
    for(int i = 0 ; i < capacity ; i++)
    {
        cbf.Add(to_string(i));
    }
	endTime = clock();
	cout << "ADD time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	totalTime = totalTime + (endTime - startTime);

	startTime = clock();
    for(int i = 0 ; i < capacity ; i++)
    {
        if(cbf.Check(to_string(i)))
            in++;
    }
	endTime = clock();
	cout << "Query time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	totalTime = totalTime + (endTime - startTime);

	startTime = clock();
    for(int i = 0 ; i < capacity/2 ; i++)
    {
        cbf.Remove(to_string(i));
    }
	endTime = clock();
	cout << "Delete time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	totalTime = totalTime + (endTime - startTime);

	for (int i = 0; i < capacity; i++)
	{
		if (cbf.Check(to_string(i)))
			in++;
	}
	for (int i = capacity; i < capacity * 2; i++)
	{
		if (cbf.Check(to_string(i)))
			falsein++;
	}

   /* cout << "total:" << capacity << "   error_rate:" <<  error_rate << endl;
    cout << "in:" << in << "  false in:" << falsein <<endl;
    cout << "false in rate:" << falsein/double(capacity) <<endl;*/
	cout << "The totalTime is:" << (double)totalTime / CLOCKS_PER_SEC << "s" << endl;		//Êä³öÊ±¼ä
	cout << "falsein:" << falsein << endl;
	system("pause");
    return 0;
}

