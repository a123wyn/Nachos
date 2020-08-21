#include "copyright.h"
#include "synch.h"
#include "system.h"

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;   //用于调试
    value = initialValue;   //信号量值
    queue = new List;   //等待队列
}

Semaphore::~Semaphore()
{
    delete queue;
}

void Semaphore::P() //信号量--，阻塞直到信号量大于0
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // 关中断
    while (value == 0) // 信号量为0，进行睡眠
    {
        queue->Append((void *)currentThread);   // 进入阻塞队列
        currentThread->Sleep(); // 线程睡眠
    }
    value--;                    // 信号量--
    (void) interrupt->SetLevel(oldLevel);   // 开中断
}

void Semaphore::V() //信号量++，如果有必要则唤醒一个线程
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // 关中断
    thread = (Thread *)queue->Remove(); // 从等待队列中唤醒一个线程
    if (thread != NULL)
       scheduler->ReadyToRun(thread); // 唤醒的线程加入就绪态，按照mesa语义
    value++;                    // 信号量++ 
    (void) interrupt->SetLevel(oldLevel); // 开中断
}

Lock::Lock(char* debugName) {
    name = debugName;    // 用于调试
    LockSem = new Semaphore(debugName, 1); // 锁其实就是一个二元信号量，创建一个信号量，初始化为1
    owner = NULL; // 初始时，任何线程都没有拥有该锁
}

Lock::~Lock() {
    delete LockSem;
}

void Lock::Acquire() {
    ASSERT(!isHeldByCurrentThread()); // 若当前线程没有拥有锁，则尝试获得锁
    LockSem->P();   // 转化为信号量的P操作
    owner = currentThread;  // 将当前线程设置为锁的owner变量
}

void Lock::Release() {
    ASSERT(isHeldByCurrentThread()); // 如果当前线程有锁则释放
    LockSem->V();   // 转化为信号量的V操作
    owner = NULL;   // 此时没有线程拥有它
}

bool Lock::isHeldByCurrentThread(){     // 当前线程是否拥有该锁，拥有则返回1，否则返回0
    return owner == currentThread;
}

Condition::Condition(char* debugName) { // 条件变量
    name = debugName;   // 用于调试
    numWaiting = 0; // 等待的线程数
    ConSem = new Semaphore(debugName, 0);   // 条件变量用信号量实现，初始值设为0，这样达成阻塞的目的
}

Condition::~Condition() {
    delete ConSem;
}

//等待条件变量
void Condition::Wait(Lock* conditionLock) { 
    ASSERT(conditionLock->isHeldByCurrentThread());     // 若不持有锁则直接结束进程，说明发生了明显错误需要终止
    numWaiting++;   // 等待线程+1
    conditionLock->Release();   // 先释放当前持有的锁
    ConSem->P();    // 条件变量执行P操作，若value=0则睡眠
    conditionLock->Acquire();   // 被唤醒后重新尝试获得锁
}

//唤醒一个等待该条件变量的线程
void Condition::Signal(Lock* conditionLock) {   //signal与V操作的区别在于：signal不存在累加，如果没有因等待该条件变量而被阻塞的线程，则什么也不做
    if(numWaiting > 0){ // 如果当前有等待该条件变量的线程
        ASSERT(conditionLock->isHeldByCurrentThread());// 若不持有锁则直接结束进程
        ConSem->V();    // 条件变量执行V操作，唤醒一个等待线程
        numWaiting--;   // 等待线程数-1
    }
}

//唤醒所有等待该条件变量的线程
void Condition::Broadcast(Lock* conditionLock) {
    while(numWaiting > 0){  // 如果当前有等待该条件变量的线程（唤醒）
        ASSERT(conditionLock->isHeldByCurrentThread());// 若不持有锁则直接结束进程
        ConSem->V();    // 如果当前有等待该条件变量的线程（唤醒）
        numWaiting--;   // 等待线程数-1
    }
}