#include "synch.h"

class BoundedBuffer {
   public:
     BoundedBuffer(int size); // 创建一个最大为maxsize的缓冲区
     
     ~BoundedBuffer();
     
     void Read(void *data, int size); // 从缓冲区读大小为size的数据
     
     void Write(void *data, int size); // 往缓冲区写大小为size的数据
   private:
     char *buffer; // 缓冲区
     int maxsize; //最大容量
     int head, tail, n; // 头部和尾部，以及当前缓冲区数据大小
     Lock *lock; // 互斥锁
     Condition *bufferempty; // 缓冲区空的条件变量
     Condition *bufferfull; // 缓冲区满的条件变量
};

