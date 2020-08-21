#include "dllist.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void InsertItems(int which, int n, int a[],DLList *testlist)
{//向双向链表插入随机元素的函数，which表示进程号，n表示插入次数，a[]存储的是插入值的数组
	int key;
	srand((unsigned)time(NULL));
    for(int i = 0; i < n; i++)
    {
		key = rand() % 100;
		a[i] = key + which;//随机数
		printf("Thread %d try to insert key = %d.\n", which,a[i]);
		testlist->SortedInsert((void*)a[i], a[i],which);//将随机值插入链表中
		//printf("  Now DLL has %d elements.\n", testlist->TotalElt());//打印现在链表的元素总数
		// testlist->preDisp();//顺序打印
		// testlist->backDisp();//逆序打印
	}
}

void RemoveItems(int which, int n, int a[],DLList *testlist)
{//向双向链表删除元素的函数，which表示进程号，n表示删除次数，a[]表示插入值数组
	int key;
	for(int i = 0; i < n; i++){
		key = a[i];//将插入值再依次从链表中删除
		printf("Thread %d try to remove key = %d.\n", which,a[i]);
		testlist->SortedRemove(key,which);//删除指定key值
		//printf("  Now DLL has %d elements.\n", testlist->TotalElt());//打印现在链表的元素总数
		// testlist->preDisp();//顺序打印
		// testlist->backDisp();//逆序打印
	}

}

