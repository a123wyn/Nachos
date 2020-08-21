#include "synch.h"
#include "system.h"
#include "BoundedBuffer.h"

BoundedBuffer::BoundedBuffer(int size)
{
	// 初始化
	this->maxsize = size;
	head = 0;
	tail = 0;
	n = 0;
	buffer = new char[size]();
	lock = new Lock("buffer's lock");
	bufferempty = new Condition("buffer is empty");
	bufferfull = new Condition("buffer is full");
}

BoundedBuffer::~BoundedBuffer()
{
	delete[] buffer;
	delete lock;
	delete bufferempty;
	delete bufferfull;
}
     
void BoundedBuffer::Read(void *data, int size) // 消费者
{
	lock->Acquire(); // 获得锁
	for (int i = 0; i < size; i++) // 循环读取size大小数据
	{
		while (n == 0) // 如果当前缓冲区为空，则在条件变量上阻塞
			bufferempty->Wait(lock);
		((char *)data)[i] = buffer[head]; // 从head处读取到data中
		head = (head + 1) % maxsize; // head往后移，data过后也会往后移
		n--; // 当前数据-1
		bufferfull->Signal(lock); // 读取后唤醒一个生产者
		printf("Thread %s Read - head: %d tail: %d n: %d\n", currentThread->getName(), head, tail, n);
	}
	lock->Release(); // 释放锁
}
     
void BoundedBuffer::Write(void *data, int size) // 生产者
{
	lock->Acquire(); // 获得锁
	for (int i = 0; i < size; i++) // 循环写入size大小数据
	{
		while (n == maxsize) // 如果当前缓冲区已经满了，则在相应条件变量上阻塞
			bufferfull->Wait(lock);
		buffer[tail] = ((char *)data)[i]; // 缓冲区末尾追加
		tail = (tail + 1) % maxsize; // tail往后移，data过后也会往后移
		n++; // 当前数据+1
		bufferempty->Signal(lock); // 写入后唤醒消费者
		printf("Thread %s Write - head: %d tail: %d n: %d\n", currentThread->getName(), head, tail, n);
	}
	lock->Release(); // 释放锁
}