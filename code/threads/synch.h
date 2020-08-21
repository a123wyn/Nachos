#ifndef SYNCH_H
#define SYNCH_H
#include "copyright.h"
#include "thread.h"
#include "list.h"

//信号量
class Semaphore {
  public:
    Semaphore(char* debugName, int initialValue);   //信号量构造函数
    ~Semaphore();                       //析构函数
    char* getName() { return name;}         //debugging
    
    void P();    // 等待线程
    void V();    // 唤醒线程
    
  private:
    char* name;        // debugging
    int value;         // 信号量的值，一直为非负数
    List *queue;       // 被阻塞的队列
};

//互斥锁
class Lock {
  public:
    Lock(char* debugName);          // 锁的构造函数，一开始是FREE
    ~Lock();                // 析构函数
    char* getName() { return name; }    // debugging

    void Acquire(); // 线程获得锁
    void Release(); // 线程释放锁

    bool isHeldByCurrentThread();   //测试当前线程是否拥有该锁

  private:
    char* name;   //名字
    enum value{ FREE = 0, BUSY = 1} value; //当前锁的状态
    List* queue; //创建队列
    Thread* owner;  //拥有lock的进程
};

//条件变量
class Condition {
  public:
    Condition(char* debugName);     // 条件变量构造函数
    ~Condition();           // 析构函数
    char* getName() { return (name); }  // debugging
    
    void Wait(Lock *conditionLock);     // 线程等待条件变量：1.释放该锁 2.进入睡眠状态 3.重新获得该锁
    void Signal(Lock *conditionLock);   // 唤醒一个等待该条件变量的线程
    void Broadcast(Lock *conditionLock);// 唤醒所有等待该条件变量的线程

  private:
    char* name; //名字
    int numWaiting; //阻塞等待的线程数
    List *queue; //创建队列
};
#endif
