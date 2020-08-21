#include "copyright.h" 
#include "Alarm.h" 
#include "system.h" 

static void AlarmHandler(int arg) // 当中断来时调用该函数，该函数专门用来处理中断，参数是自身，会调用自身Wake函数进行闹钟唤醒
{
	Alarm *p = (Alarm *)arg; p->Wake();
}

Alarm::Alarm()
{
	// 初始化
    pending = new List();
	num=0;
}

Alarm::~Alarm()
{
    while (!pending->IsEmpty()) 
	delete (PendingInterrupt *)pending->Remove(); 
    delete pending; 
}

// int Alarm::getnum() // 获取当前等待闹钟的线程数
// {
// 	return num;
// }

void TightLoop(int addrnum) // 为了防止出现线程闹钟等待唤醒且暂时没有可运行的线程导致的nachos退出，我们需要创建一个调整紧密循环的线程
{
	int *addr=(int *)addrnum; // 传入num的地址，实时取出num的值
	while(*addr) // 反复检查当前有几个线程等待闹钟，若为0则退出，若不为0则切换线程
	{
		currentThread->Yield(); // 只要还有闹钟在等待，就应该执行这个；若就绪态无线程则不会成功切换，所以会一直让nachos以为还有东西在运行
	}
	currentThread->Finish();
}

void Alarm::Pause(int sleeptime) // 设定闹钟睡眠sleeptime这么多时间，将当前线程以及醒来时刻放入未决队列中，属于原子操作
{
	int waketime;
	IntStatus oldLevel = interrupt->SetLevel(IntOff); // 关中断
	num++; // 闹钟未决队列个数++
	if(num==1) // 未决队列为空，为了防止出现线程闹钟等待唤醒且暂时没有可运行的线程导致的nachos退出的情况
	{
		Thread *t = new Thread("thread");
		t->Fork(TightLoop,(int)&num);
	}
	waketime = stats->totalTicks + sleeptime; // 醒来的时刻=当前+睡的时间
	pending->SortedInsert((void *)currentThread, waketime);	// 将当前线程以及醒来时刻放入队列中
	interrupt->Schedule(AlarmHandler, (int) this, waketime-stats->totalTicks, TimerInt); // 这里表示加入一个中断处理函数到等待处理中断队列中，AlarmHandler，参数是自己
	currentThread->Sleep(); // 线程进入睡眠
	(void) interrupt->SetLevel(oldLevel); // 开中断
}

void Alarm::Wake() // 用于闹钟唤醒，先取出队列中醒来时刻最小的线程，若已到达其醒来时刻则将该线程放入就绪态
{
	int waketime;
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // 关中断
    Thread *t = (Thread *)pending->SortedRemove(&waketime);	// 按索引取出醒来时刻最小的线程，索引即为醒来时刻，赋值到变量waketime
	if (t == NULL)	// 未决中断队列中没有线程
		return ;	 
    if ( waketime <= stats->totalTicks&&t!=NULL) // 若取出的该线程的醒来时刻小于等于当前滴答声，即说明该线程需被唤醒  
	{
		scheduler->ReadyToRun(t); // 加入就绪态
		num--; // 闹钟未决队列个数--
    }
	else // 若还未到醒来时刻
	{ 
		pending->SortedInsert((void *)t, waketime); // 原样重新插入到未决队列中
		interrupt->Schedule(AlarmHandler, (int) this, waketime-stats->totalTicks, TimerInt); // 这里表示加入一个中断处理函数到等待处理中断队列中，AlarmHandler，参数是自己
	}
    (void) interrupt->SetLevel(oldLevel); // 开中断
}