#include "copyright.h"
#include "synch.h"
#include "system.h"

//Mesa语义即为唤醒一个线程后该线程并没有马上执行而是等到当前线程继续执行完后再执行
Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName; //用于调试
    value = initialValue; //信号量值
    queue = new List; //等待队列
}

Semaphore::~Semaphore()
{
    delete queue;
}

void Semaphore::P() //信号量--，阻塞直到信号量大于0
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // 关中断
    while (value == 0) 
    {            // 信号量为0，进行睡眠
        queue->Append((void *)currentThread);   // 进入阻塞队列
        currentThread->Sleep(); //线程睡眠
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
    name = debugName;    // 名字
    value = FREE;        // 初始化锁为FREE
    queue = new List;    // 创建新队列
    owner = NULL;        // 初始时，任何线程都没有拥有该锁
}

Lock::~Lock() {            // 锁销毁
    delete queue;
}

void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // 关中断
    ASSERT(!isHeldByCurrentThread()); // 若当前线程没有拥有锁，则尝试获得锁
    while(value != FREE){     // 若锁被占用
        queue->Append((void *)currentThread);
        currentThread->Sleep();   // 当前线程睡眠
    }
    value = BUSY;                 // 获得锁，将锁置为busy
    owner = currentThread;        // 将当前线程设置为锁的
    (void) interrupt->SetLevel(oldLevel);   // 开中断
}

//Release: set lock to be FREE, waking up a thread waiting in Acquire if necessary
void Lock::Release() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // 关中断
    ASSERT(isHeldByCurrentThread()); //如果当前线程有锁则释放

    Thread *thread;
    thread = (Thread *)queue->Remove(); //等待队列中唤醒一个线程来拥有该锁
    if(thread != NULL){
        scheduler->ReadyToRun(thread); //将该线程转为就绪
    }
    value = FREE; //释放锁后，锁的状态时FREE
    owner = NULL; //此时没有线程拥有它
    
    (void) interrupt->SetLevel(oldLevel);   // 开中断
}

bool Lock::isHeldByCurrentThread(){     //当前线程是否拥有该锁，拥有则返回1，否则返回0
    return owner == currentThread;
}

Condition::Condition(char* debugName) { //条件变量
    name = debugName; //用于调试
    numWaiting = 0; //等待的线程数
    queue = new List; //创建等待队列
}

Condition::~Condition() { 
    delete queue;
}

//等待条件变量
void Condition::Wait(Lock* conditionLock) { 
    ASSERT(conditionLock->isHeldByCurrentThread()); // 若不持有锁则直接结束进程，说明发生了明显错误需要终止
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // 关中断
    numWaiting++; // 等待线程+1
    queue->Append((void *)currentThread);   // 进入等待队列
    conditionLock->Release();   // 先释放当前持有的锁
    currentThread->Sleep(); // 当前线程进入睡眠
    conditionLock->Acquire(); // 被唤醒后重新尝试获得锁
    (void) interrupt->SetLevel(oldLevel); // 开中断
}

//唤醒一个等待该条件变量的线程
void Condition::Signal(Lock* conditionLock) { 
    if(numWaiting > 0){ // 如果当前有等待该条件变量的线程
        IntStatus oldLevel = interrupt->SetLevel(IntOff); // 关中断
        ASSERT(conditionLock->isHeldByCurrentThread());// 若不持有锁则直接结束进程
        Thread *thread;
        thread = (Thread*)queue->Remove();  // 从等待队列中唤醒一个等待该条件变量的线程
        if(thread != NULL)
            scheduler->ReadyToRun(thread);  // 进入就绪态
        numWaiting--; // 等待线程数-1
        (void) interrupt->SetLevel(oldLevel); // 开中断
    }
}

//唤醒所有等待该条件变量的线程
void Condition::Broadcast(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // 关中断
    ASSERT(conditionLock->isHeldByCurrentThread());// 若不持有锁则直接结束进程
    Thread *thread;
    while(numWaiting > 0){ // 如果当前有等待该条件变量的线程（唤醒）
        thread = (Thread*)queue->Remove(); // 从等待队列中唤醒一个等待该条件变量的线程
        if(thread != NULL)
            scheduler->ReadyToRun(thread); // 进入就绪态
        numWaiting--; // 等待线程数-1
    }
    (void) interrupt->SetLevel(oldLevel); // 开中断
}
