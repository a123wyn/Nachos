#include "copyright.h" 
#include "synch.h" 
#include "system.h" 
#include "EventBarrier.h" 
#define SIGNALED 1 // 表示栅栏是放开状态
#define UNSIGNALED 0 // 表示栅栏是关闭状态
 
EventBarrier::EventBarrier() 
{
	// 初始化
	waitnum=0; 
	status=UNSIGNALED; 
	lock1=new Lock("lock1");
	signal=new Condition("signal");
	lock2=new Lock("lock2");
	complete=new Condition("complete");
} 

EventBarrier::~EventBarrier() 
{
	delete lock1;
	delete signal;
	delete lock2;
	delete complete;
}

void EventBarrier::Wait() // 线程等待同步，需要同步的线程调用，调用后等待直到被唤醒，若栅栏是SIGNALED则直接返回
{ 
	waitnum++; // Wait阻塞的线程++
	if(status==SIGNALED) // 栅栏放开，则直接返回
		return ; 
	else // 否则阻塞于Signal
	{	 
		lock1->Acquire(); 
		signal->Wait(lock1); // 阻塞于signal
		lock1->Release(); 
	} 
} 
 
void EventBarrier::Signal() // 由控制栅栏的线程调用，唤醒当前Wait阻塞的所有线程（广播）
							//自己陷入等待直到线程全部被唤醒，然后重置栅栏状态为UNSIGNALED
{
	status=SIGNALED; // 设置事件栅栏的状态为SIGNALED
	lock2->Acquire(); // 与条件变量complete搭配的互斥锁
	lock1->Acquire(); // 与条件变量signal搭配的互斥锁
	signal->Broadcast(lock1); // 唤醒所有阻塞于条件变量signal的线程
	lock1->Release(); // 释放lock1
	if(Waiters()!=0)  // 如果仍有线程未被唤醒，则当前线程继续阻塞于complete条件变量上
		complete->Wait(lock2); // 阻塞于complete
	status=UNSIGNALED; // 恢复事件栅栏的状态为UNSIGNALED
	lock2->Release();  // 释放lock2
}

void EventBarrier::Complete() // 线程唤醒同步，从Wait被唤醒后调用Complete，陷入等待直到所有线程被唤醒
{
	lock2->Acquire(); // 准备对条件变量complete进行操作，获得lock2
	waitnum--; // Wait阻塞的线程--
	if(Waiters()==0) // 最后一个从Wait出来调用Complete的线程
	{ 
		status=UNSIGNALED; // 将栅栏设为UNSIGNALED
		complete->Broadcast(lock2);	// 负责唤醒所有之前阻塞在complete上的线程
	}
	else // Wait后紧接着调用Complete
	{
		complete->Wait(lock2); // 阻塞直到所有线程被唤醒
	}
	lock2->Release(); // 释放lock2
}

int EventBarrier::Waiters() // 返回当前等待的线程数量
{ 
	return waitnum; 
} 