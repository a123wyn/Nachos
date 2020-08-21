#include "copyright.h" 
#include "synch.h" 
#include "system.h" 
#include "EventBarrier.h"    
#include "Elevator.h"    
 
 
 
Elevator::Elevator(char *debugName, int numFloors, int myID) // 电梯构造函数
{
	name=debugName; // 电梯名
	numf=numFloors; // 楼层数
	occupancy=0; // 电梯内人数
	updown=1; // 初始电梯向上移动
	currentfloor=1; // 当前楼层
	ID=myID;
	lock =new Lock("lock"); // 互斥锁 
	alarm1=new Alarm(); // 闹钟
	close=new EventBarrier();  // 事件栅栏
	for(int i=1;i<=numFloors;i++) 
	{ 
		eb[i]=new EventBarrier(); // eb是一个事件栅栏类数组
		dfloor[i]=0; // 未有楼层亮灯
	} 
}
 
Elevator::~Elevator() // 电梯析构
{ 
	for(int i=1;i<=numf;i++) 
	{ 
		delete eb[i]; 
	} 
} 
 
char * Elevator::getName() 
{ 
	return name; 
} 
    
     // elevator control interface: called by Elevator thread 
void Elevator::OpenDoors()
{ 
	//   signal exiters and enterers to action 
//	printf("open \t"); 
	eb[currentfloor]->Signal(); // 唤醒所有睡眠线程，控制栅栏打开（类似于打开电梯门）
//		printf("openend \t"); 
} 
void Elevator::CloseDoors()
{ 
	//   after exiters are out and enterers are in 
 
//	close->Signal(); 
 
} 
void Elevator::VisitFloor(int floor) // 指定楼层需停
{
	dfloor[floor]=1; // 标识该楼层
}
     // elevator rider interface (part 1): called by rider threads. 
int Elevator::Enter(int destinationFloor) // 进入电梯
{
	eb[currentfloor]->Complete(); // 跨越栅栏
	// if(occupancy>=3) // 电梯内人数大于等于3
	// {
	// 	printf("already 3 people in elevator\n"); 
	// 	return 0; 
	// }
	occupancy++; // 电梯里人数++
	return 1; 
}
void Elevator::Exit() // 出去电梯
{
	occupancy--; // 电梯人数--
	eb[currentfloor]->Complete(); // 跨越栅栏
//	close->Wait(); 
//	close->Complete(); 
	 
} 
void Elevator::RequestFloor(int floor)    // 告诉电梯要去往的指定楼层
{
	dfloor[floor]=1; // 标识该楼层
	eb[floor]->Wait(); // 该楼层门等待同步被唤醒
}
void Elevator::Wait(int floor)    // 乘客在该楼层等待
{
	eb[floor]->Wait(); // 该楼层等待
}
void Elevator::Move() // 电梯运动
{ 
	if(updown==1) // 往上
	{ 
		for(int i=currentfloor+1;i<=numf;i++) 
		{ 
			if(dfloor[i]==1) // 若该楼层为目的地
			{ 
				alarm1->Pause(200*(i-currentfloor)); // 停一段时间 
				currentfloor=i; // 当前楼层置为i
				printf("*********elevator arrived %d************\n",i); 
				dfloor[i]=0; // 灭灯
				OpenDoors(); // 打开门，阻塞在另一个条件变量上
				CloseDoors(); // 关门
			} 
		} 
		updown=0; // 到达最高层则往下
	}
	else if(updown==0) // 往下
	{
		for(int i=currentfloor-1;i>=1;i--) 
		{
			if(dfloor[i]==1) // 若该楼层为目的地
			{
				alarm1->Pause(200*(currentfloor-i)); // 停一段时间 
				currentfloor=i; // 当前楼层置为i
				printf("*********elevator arrived %d************\n",i); 
				dfloor[i]=0; // 灭灯
				OpenDoors(); // 开门
				CloseDoors(); // 关门		 
			} 
		} 
		updown=1; // 到达最底层则往上
	} 
} 

int Elevator::Used() // 返回当前电梯仍有目的地要到达
{
		for(int i=1;i<=numf;i++) 
		{ 
			if(dfloor[i]==1) 
			{ 
				return 1; 
			} 
		} 
		return 0; 
} 

Building::Building(char *debugname, int numFloors, int numElevators) // 建筑构造函数
{
	name=debugname; // 建筑名
	numf=numFloors; // 楼层数
	nume=numElevators; // 电梯数
	elevator=new Elevator("E1",numFloors,1); // 电梯
}
Building::~Building() 
{ 
	delete elevator; 
} 
char * Building::getName() 
{
	return name; 
} 			 
void Building::CallUp(int fromFloor) // 在起始楼层按按钮的动作 
{
	elevator->VisitFloor(fromFloor);
}
void Building::CallDown(int fromFloor) // 按按钮的动作
{
	elevator->VisitFloor(fromFloor); 
}//   ... down 
Elevator * Building::AwaitUp(int fromFloor) // 乘客在该楼层在等待
{
	elevator->Wait(fromFloor);  // 该楼层阻塞
	return elevator; 
} 
Elevator * Building::AwaitDown(int fromFloor) // 乘客在该楼层在等待
{ 
	elevator->Wait(fromFloor); 
	return elevator; 
} 
void Building::Begin() // 电梯开始运作
{
	while(elevator->Used()) // 若电梯仍有目的地到达则继续，一直循环检查
	{
		elevator->Move(); // 电梯运动
		currentThread->Yield(); // 切换线程
	}
}