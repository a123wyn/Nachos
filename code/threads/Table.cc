#include "Table.h"
#include "synch.h"
#include "system.h"

Table::Table(int size) // 创建一个大小为size的表结构
{
	this->size = size;
	data = (void **)new int[size](); // 表槽区
	n = 0; // 初始化表内数据为0
	lock = new Lock("Table's lock"); // 新建锁
}

Table::~Table()
{
	delete[] data; // 释放数据区
	delete lock; // 释放锁
}

int Table::Alloc(void *object) // 为对象分配一个表槽，返回已分配条目的索引，若无可用表槽则返回-1
{
	lock->Acquire(); // 尝试获得锁
	if (n == size || object == NULL) // 若数量已满或者插入为NULL则
	{
		lock->Release(); // 释放锁
		return -1; // 无可用槽
	}
	n++; // 表内槽数量加1
	for (int i = 0; i < size; i++) // 遍历
	{
		if (data[i] == NULL) // 如果有未被分配则分配
		{
			data[i] = object; // 分配成功
			lock->Release(); // 释放锁
			return i; // 返回槽的索引
		}
	}
}

void *Table::Get(int index) // 从索引处表槽中检索对象，如果没有分配则返回NULL
{
	ASSERT(0 <= index && index < size); // 索引发生错误则退出
	void *re;
	lock->Acquire(); // 获得锁
	re = data[index];// 指定索引的槽对象内容返回给re
	lock->Release(); // 释放锁
	return re; // 返回对象
}

void *Table::Release(int index) // 释放一个索引处表槽
{
	ASSERT(0 <= index && index < size); // 索引发生错误则退出
	void *re;
	lock->Acquire(); // 获得锁
	if (data[index] != NULL) // 若该索引下的槽不是空的
	{
		re = data[index];
		n--; // 表内槽数量减1
		data[index] = NULL; // 移除槽后设为NULL
		lock->Release(); // 释放锁
		return re; // 返回地址
	}
	else
	{
		re = data[index];
		lock->Release(); // 释放锁
		return re; // 返回空地址
	}
}
