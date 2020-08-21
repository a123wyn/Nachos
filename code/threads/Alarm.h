#include "copyright.h" 
#include "list.h" 
#include "synch.h"
 
 
class Alarm { 
	public: 
    	Alarm(); 
    	~Alarm(); 
    	// Alarm *alarm;
		void Pause(int sleeptime); // 设定闹钟睡眠sleeptime这么多时间，属于原子操作
		void Wake(); // 用于闹钟唤醒，先取出队列中醒来时刻最小的线程，若已到达其醒来时刻则将该线程放入就绪态
		int num;
		// int getnum(); // 获取当前等待闹钟的线程数
		//void TightLoop(int which); // 为了防止出现线程闹钟等待唤醒且暂时没有可运行的线程导致的nachos退出，我们需要创建一个调整紧密循环的线程
	private: 
		List *pending;
};

