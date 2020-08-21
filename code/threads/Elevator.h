#include "copyright.h" 
#include "synch.h" 
#include "system.h" 
#include "Alarm.h"  

class Elevator { 
  public: 
    Elevator(char *debugName, int numFloors, int myID);
    ~Elevator();
    char *getName();
     // elevator control interface: called by Elevator thread 
    void OpenDoors();                //   用事件栅栏来进行开门操作
    void CloseDoors();               //   用事件栅栏来进行关门操作
    void VisitFloor(int floor);      //   访问目的楼层
     // elevator rider interface (part 1): called by rider threads. 
    int Enter(int destinationFloor); //   进入电梯
    void Exit();                     //   退出电梯
    void RequestFloor(int floor);    //   指定目的地
    void Move();                     //   电梯移动
    void Wait(int floor);            //   
    int Used();                      //   返回当前电梯仍有目的地要到达
 
  private: 
    char *name;                 // 电梯名
    int numf;                   // 楼层数
    int ID; 
    int currentfloor;           // 当前楼层
    int occupancy;              // 电梯内人数
    int updown;                 // 方向
    int dfloor[20];             // 按钮亮灯状态
    EventBarrier * eb[20];      // 每层一个事件栅栏用于等待同步，该楼层亮灯则进行阻塞
    Alarm *alarm1;              // 闹钟
    EventBarrier *close;        // 事件栅栏
    Lock * lock;                // 互斥锁
};

class Building 
{ 
  public: 
    Building(char *debugname, int numFloors, int numElevators); //构造函数
    ~Building();
    char *getName();
    // elevator rider interface (part 2): called by rider threads 
    void CallUp(int fromFloor);      // 在起始楼层按按钮，准备往上
    void CallDown(int fromFloor);    // 在起始楼层按按钮，准备往下
    Elevator *AwaitUp(int fromFloor); // 乘客在该楼层在等待，准备往上
    Elevator *AwaitDown(int fromFloor); // 乘客在该楼层在等待，准备往下
    void Begin();  
  private: 
    char *name; 
    Elevator *elevator; 
    int numf; // 楼层数
    int nume; // 电梯数
   // insert your data structures here, if needed 
}; 

   // here's a sample portion of a rider thread body showing how we 
   // expect things to be called. 
   // 
   // void rider(int id, int srcFloor, int dstFloor) { 
   //    Elevator *e; 
   //   
   //    if (srcFloor == dstFloor) 
   //       return; 
   //   
   //    DEBUG('t',"Rider %d travelling from %d to %d\n",id,srcFloor,dstFloor); 
   //    do { 
   //       if (srcFloor < dstFloor) { 
   //          DEBUG('t', "Rider %d CallUp(%d)\n", id, srcFloor); 
   //          building->CallUp(srcFloor); 
   //          DEBUG('t', "Rider %d AwaitUp(%d)\n", id, srcFloor); 
   //          e = building->AwaitUp(srcFloor); 
   //       } else { 
   //          DEBUG('t', "Rider %d CallDown(%d)\n", id, srcFloor); 
   //          building->CallDown(srcFloor); 
   //          DEBUG('t', "Rider %d AwaitDown(%d)\n", id, srcFloor); 
   //          e = building->AwaitDown(srcFloor); 
   //       } 
   //       DEBUG('t', "Rider %d Enter()\n", id); 
   //    } while (!e->Enter()); // elevator might be full! 
   //   
   //    DEBUG('t', "Rider %d RequestFloor(%d)\n", id, dstFloor); 
   //    e->RequestFloor(dstFloor); // doesn't return until arrival 
   //    DEBUG('t', "Rider %d Exit()\n", id); 
   //    e->Exit(); 
   //    DEBUG('t', "Rider %d finished\n", id); 
   // } 

