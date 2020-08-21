/*

Table implements a simple fixed-size table, a common kernel data
structure.  The table consists of "size" entries, each of which
holds a pointer to an object.  Each object in the table can be
named by an index in the range [0..size-1], corresponding to its
position in the table.  Table::Alloc allocates a free entry,
stores an object pointer in it, and returns its index.  The
object pointer can be retrieved by passing its index to Table::Get.
An entry is released by passing its index to Table::Release.

Table knows nothing about the objects it indexes.  In particular,
it is the responsibility of the caller to delete each object when
it is no longer needed (some time after the table entry is released).

It is also the caller's responsibility to correctly handle the types
of the objects stored in the Table.  The object pointer in each Table
entry is untyped (void*).  It is necessary to cast an object pointer
to a (void *) before storing it in the table, and to cast it back to
its correct type (e.g., (Process *)) after retrieving it with Get.
A more sophisticated solution would use parameterized types.aux

In later assignments, the Table class may be used to implement internal
operating system tables of processes, threads, memory page frames, open
files, etc.

*/

#include "synch.h"

class Table {
   public:
     Table(int size); // 创建一个大小为size的表结构
     
     ~Table();

     int Alloc(void *object); // 为对象分配一个表槽，返回已分配条目的索引，若无可用表槽则返回-1
   
     void *Get(int index); // 从索引处表槽中检索对象，如果没有分配则返回NULL
   
     void *Release(int index); // 释放一个索引处表槽
   private:
     void **data; // 表槽区
     int size; // 表槽最大容量
     int n; // 当前表内槽个数
     Lock *lock; // 锁
};

