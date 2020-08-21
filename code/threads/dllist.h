#ifndef DLLIST_H
#define DLLIST_H

class DLLElement {//链表元素
	public:
	DLLElement( void *itemPtr, int sortKey );  //初始化一个链表元素类
	DLLElement *next;    //表示该元素下一个，没有则为null
	DLLElement *prev;    //表示该元素上一个，没有则为null
	int key;         //索引，用于排序
	void *item;      //链表元素值的指针
};

class DLList {//链表
	public:
	int count;   // 链表内元素个数
	int a[5];
	DLList();    // 创建链表
	DLList(int typenum,int count);
	~DLList();   //删除

	void Prepend(void *item);  //item插入链表头部
	void Append(void *item);   //item插入链表尾部
	void *Remove(int *keyPtr); //从头部删除，返回该删除元素key值

	bool IsEmpty();     //如果空则为true
	int TotalElt();		//返回链表元素数
	
	void SortedInsert(void *item, int sortKey,int which);//插入item，索引值是sortkey
	void *SortedRemove(int sortKey,int which);  // 删除指定key值的元素，如果不存在返回NULL
	
	void preDisp();//顺序打印item值
	void backDisp();//逆序打印item值

	private:
	int type;			 // 双向链表的错误类型
	DLLElement *first;   // 头指针
	DLLElement *last;    // 尾指针
};
#endif // DLLIST_H
