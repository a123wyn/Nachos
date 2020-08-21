#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "dllist.h"
#include "Table.h"
#include "BoundedBuffer.h"
#include "EventBarrier.h"
#include "Elevator.h"
int testnum = 1;    //测试类型
int sumThread = 2;  //线程数量
int typenum = 0;    //双向链表/buffer/table切换进程的时刻
int initsize = 32;  //Buffer/Table公用
//测试部分
DLList *testlist; 
Lock *dllLock;
Condition *dllnotFull;
Condition *dllnotEmpty;
//Buffer测试部分
BoundedBuffer *boundedBuffer;

//Table测试部分
Table *table;

//事件栅栏测试部分
EventBarrier *barrier; 
Alarm *alarm; 
Building *building;

extern void InsertItems(int which, int n, int a[],DLList *testlist);
extern void RemoveItems(int which, int n, int a[],DLList *testlist);
//----------------------------------------------------------------------
// SimpleThread
//  Loop 5 times, yielding the CPU to another ready thread 
//  each iteration.
//
//  "which" is simply a number identifying the thread, for debugging
//  purposes.
//----------------------------------------------------------------------

void SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
    printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
//  Set up a ping-pong between two threads, by forking a thread 
//  to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

void DLLtestThread(int which)//which为进程号
{
    int a[5];
    for (int i = 0; i < 2; i++) {
        //printf("I am in thread %d\n", which);
        InsertItems(which, 2, a, testlist);
        RemoveItems(which, 2, a, testlist);
    }
}

void OurThreadTest()
{
    dllLock = new Lock("lock");
    dllnotEmpty = new Condition("dll is not empty");
    dllnotFull = new Condition("dll is not full");
    DEBUG('t', "Entering MyThreadTest");
    for(int i = 1; i < sumThread; i++){
        Thread *t = new Thread("forked thread");
        t->Fork(DLLtestThread, i);//创建子进程双向链表Pi
    }
    testlist = new DLList(typenum,0);//创建链表并指明发生错误的类型
    DLLtestThread(0);//调用P0的子进程
}

void TableTest(int which)
{
    int index;
    int addr;
    for (int i = 0; i < 4; i++)
    {
        switch (which % 3)
        {
        case 0:
            addr = Random() % 1000 + 1;
            index = table->Alloc((void *)addr);
            if(index == -1)
                printf("Alloc Error!\n");
            else
                printf("Thread %d Alloc_index: %d addr: %d\n", which, index, addr);
            break;
        case 1:
            index = Random() % initsize;
            addr = (int)table->Get(index);
            if(addr == 0)
            {
                printf("Get Error! Get_index: %d\n", index);
            }
            else
                printf("Thread %d Get_index: %d addr: %d\n", which, index, addr);
            break;
        case 2:
            index = Random() % initsize;
            addr = (int)table->Release(index);
            if(addr == 0)
            {
                printf("Release Error! Release_index: %d\n", index);
            }
            else
                printf("Thread %d Release_index: %d\n", which, index);
            break;
        }
        currentThread->Yield();
    }
}

void TableThreadTest()
{
    DEBUG('t', "Entering TableThreadTest: Table");
    table = new Table(initsize);
    for (int i = 1; i < sumThread; i++)
    {
        char *threadName = new char[8];
        sprintf(threadName, "%d", i);
        Thread *t = new Thread(threadName);
        t->Fork(TableTest, i);
    }
    TableTest(0);//调用P0的子进程
}

void BoundedBufferTest(int which)
{
    char *data = new char[1024];
    int sz;
    for (int i = 0; i < 4; i++)
    {
        switch (which > 0)
        {
        case 0: // 消费者消费1个
            sz = 1;
            //printf("Thread %d want to read - size: %d\n", which, sz);
            boundedBuffer->Read((void *)data, sz);
            break;
        case 1: // 生产者生产3个
            sz = 3;
            //printf("Thread %d want to write - size: %d\n", which, sz);
            boundedBuffer->Write((void *)data, sz);
            break;
        }
        currentThread->Yield();
    }
}

void BufferThreadTest()
{
    DEBUG('t', "Entering BufferThreadTest: BoundedBuffer");
    boundedBuffer = new BoundedBuffer(initsize);
    for (int i = 0; i < sumThread; i++)
    {
        char *threadName = new char[8];
        sprintf(threadName, "%d", i);
        Thread *t = new Thread(threadName);
        t->Fork(BoundedBufferTest, i);
    }
    //BoundedBufferTest(0);//调用P0的子进程
}

void EventBarrierTest(int which) 
{ 
    if(which==0) // P0为负责控制栅栏的进程
    { 
        currentThread->Yield(); // 一开始P0需要切换线程，因为P0需要最后执行而不是一开始
        printf("Thread %d is singal\n", which); 
        barrier->Signal(); // 控制栅栏的P0负责唤醒所有阻塞进程
    }
    else 
    {
        printf("Thread %d is waiting\n", which); 
        barrier->Wait(); // 线程等待同步，其他进程先到达者等待
        printf("Thread %d is complete\n", which); 
        barrier->Complete(); // 线程唤醒同步，待Wait操作后需再阻塞于complete，直至全部进程均从wait中被唤醒后
    }
    printf("Thread %d is going\n", which); 
}

void EventThreadTest()
{
    DEBUG('t', "Entering EventThreadTest"); 
    barrier=new EventBarrier();
    for(int j = 0; j < 2;j++) 
    { 
        for(int i = 1; i < sumThread;i++) 
        {
            Thread *t = new Thread("forked thread"); 
            t->Fork(EventBarrierTest,i); 
        }
        EventBarrierTest(0); 
    } 
}

void AlarmTest(int which) 
{
    switch(which)
    {
        case 0:
            printf("Thread %d is pausing 1000 Tick\n", which);
            alarm->Pause(1000);
            break;
        case 1:
            printf("Thread %d is pausing 500 Tick\n", which);
            alarm->Pause(500);
            break;
        case 2:
            printf("Thread %d is pausing 800 Tick\n", which);
            alarm->Pause(800);
            break;
        case 3:
            printf("Thread %d is pausing 900 Tick\n", which);
            alarm->Pause(900);
            break;
        case 4:
            printf("Thread %d is pausing 500 Tick\n", which);
            alarm->Pause(500);
            break;
        default:
            printf("Thread %d is pausing 1000 Tick\n", which);
            alarm->Pause(1000);
            break;
    }
    printf("Thread %d wake\n", which);
    //printf("thread %d is ok now=%d\n", which,stats->totalTicks);
}

void AlarmThreadTest() 
{ 
    DEBUG('t', "Entering AlarmThreadTest"); 
    alarm=new Alarm();
    for(int i = 1; i < sumThread;i++)
    { 
         Thread *t = new Thread("forked thread"); 
         t->Fork(AlarmTest,i);
    }
    AlarmTest(0);
}

void rider(int id, int srcFloor, int dstFloor) // 乘客从第几层到第几层
{ 
    Elevator *e;
    if (srcFloor == dstFloor) 
        return; 
    do 
    {
        //printf("Rider %d travelling from %d to %d\n",id,srcFloor,dstFloor); 
        if (srcFloor < dstFloor) // 电梯往上的方向
        {
            //printf("Rider %d CallUp(%d)\n", id, srcFloor); 
            building->CallUp(srcFloor); // 起始地址按按钮
            //printf("Rider %d AwaitUp(%d)\n", id, srcFloor); 
            e = building->AwaitUp(srcFloor); // 乘客在起始地址等待，阻塞了后切换线程，只有执行eb[srcFloor]->Signal才能将此线程唤醒
        }
        else
        {
            //printf("Rider %d CallDown(%d)\n", id, srcFloor); 
            building->CallDown(srcFloor);
            //printf("Rider %d AwaitDown(%d)\n", id, srcFloor); 
            e = building->AwaitDown(srcFloor);
        }
        //currentThread->Yield(); // 切换为下一个乘客
    }while (!e->Enter(dstFloor)); // 满人了则继续等，否则退出循环
    printf("Rider %d Enter()\n", id); 
    e->RequestFloor(dstFloor); // 该乘客目的地事件栅栏阻塞，按钮亮起，待电梯到达
    printf("Rider %d Exit()\n", id); 
    e->Exit(); 
}

void ElevatorTest(int which) 
{
    // 每个线程
    printf("rider %d is %d to %d\n", which,((which*3) %10)+1,((which*5)%10)+1);
    rider(which,((which*3)%10)+1,((which*5)%10)+1);
    // printf("rider %d is %d to %d\n", which,((which*5)%10)+1,((which*7)%10)+1); 
    // rider(which,((which*5)%10)+1,((which*7)%10)+1);
}

void ElevatorThreadTest() 
{ 
    DEBUG('t', "Entering ElevatorThreadTest"); 
    building=new Building("build",10,1);
    for(int i=1;i<=sumThread;i++) 
    {
         Thread *t = new Thread("forked thread"); 
         t->Fork(ElevatorTest,i); 
    }
    currentThread->Yield();
    building->Begin(); // 电梯开始运作
}

void ThreadTest()
{
    switch (testnum) {
    case 1:
       ThreadTest1();
       break;
    case 2:
        OurThreadTest();//测试双向链表进程
        break;
    case 3:
        TableThreadTest();//测试线程安全的表结构
        break;
    case 4:
        BufferThreadTest();//测试大小受限的缓冲区
        break;
    case 5:
        EventThreadTest();//事件栅栏
        break;
    case 6:
        AlarmThreadTest();//闹钟
        break;
    case 7:
        ElevatorThreadTest();//电梯
        break;
    default:
       printf("No test specified.\n");
    break;
    }
}

