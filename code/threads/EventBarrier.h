#include "synch.h"

class EventBarrier { 
  	public: 
    	EventBarrier(); 
    	~EventBarrier(); 
		void Wait(); // 线程等待同步，需要同步的线程调用，调用后等待直到被唤醒，若栅栏是SIGNALED则直接返回
		void Signal(); // 由控制栅栏的线程调用，唤醒当前等待的所有线程（广播），自己陷入等待直到线程全部被唤醒，然后重置栅栏状态为UNSIGNALED 
		void Complete(); // 线程唤醒同步，从Wait被唤醒后调用Complete，陷入等待直到所有线程被唤醒
		int Waiters(); // 返回当前等待的线程数量
   	private: 
    	int status; // 当前栅栏状态
    	int waitnum; // 等待线程数量
   		Lock* lock1; // lock1与signal搭配，线程等待同步
		Condition * signal;
		Lock* lock2; // lock2与complete搭配，线程唤醒同步
		Condition * complete; 
};

