#include "dllist.h"
#include "system.h"
#include <stdio.h>
#include "copyright.h"
#include "synch.h"

extern Lock *dllLock;
extern Condition *dllnotEmpty;
extern Condition *dllnotFull;

DLLElement::DLLElement(void *itemPtr, int sortKey)//第一个参数表示待插入元素，第二个为索引值，初始化一个链表元素类型
{
     item = itemPtr;
     key = sortKey;
     next = NULL;
	 prev = NULL;
}

DLList::DLList()
{
    first = last = NULL;
    type = 0;
}

DLList::DLList(int typenum,int count1)
{
    first = last = NULL;
    type = typenum;
    count = count1;
}

DLList::~DLList()
{
    while (Remove(NULL) != NULL)//删除
	;
}

void DLList::Append(void *item)//往链表尾部插入元素
{
    DLLElement *element = new DLLElement(item, 0);

    if (IsEmpty()) {//链表为空
        first = element;
        last = element;
    } else {//插入尾部，个数+1
        int maxkey = last->key;
        element->key = maxkey + 1;
        element->prev = last;
        last->next = element;
        last = element;
    }
}

void DLList::Prepend(void *item)
{
    DLLElement *element = new DLLElement(item, 0);

    if (IsEmpty()) {//链表为空
        first = element;
        last = element;
    } else {//头部插入
        int minkey = first->key;
        element->key = minkey - 1;
        first->prev = element;
        element->next = first;
        first = element;
    }
}

void *DLList::Remove(int *keyPtr)
{
    void* thing = NULL;
    if(first != NULL){
        thing = first->item;
        keyPtr = &(first->key);
        first = first->next;
    }
    return thing;
}

bool DLList::IsEmpty()
{
    if ((first == NULL) && (last == NULL)){
        return true;//链表为空返回true
	}else if(first == NULL){//头部为NULL
		//printf("  DLL has NO head but a rear!\n");
	}else if(last == NULL){//尾部为NULL
		//printf("  DLL has a head but NO rear!\n");
	}else{//头尾都不为NULL
		//printf("I have both head and rear\n");
	}
     return false;//否则为false
}

void DLList::preDisp()//顺序打印
{
	printf("   pretraversal of the list is :\n  ");
	DLLElement *ptr = first;
	while(ptr != NULL){
		printf(" %d ", ptr->key);
		ptr = ptr->next;
	}
	printf("\n");
}

void DLList::backDisp()//逆序打印
{
	printf("   posttraversal of the list is :\n  ");
	DLLElement *ptr = last;
	while(ptr != NULL){
		printf(" %d ", ptr->key);
		ptr = ptr->prev;
	}
	printf("\n");
}

int DLList::TotalElt()//统计元素个数
{
	int sum = 0;
	DLLElement *ptr = first;
	while(ptr != NULL){
		sum++;
		ptr = ptr->next;
	}
	return sum;
}

void DLList::SortedInsert(void *item, int sortKey,int which)//item为待插入元素值，sortkey为索引
{
    dllLock->Acquire();
    while (count==3)
        dllnotFull->Wait(dllLock);
    DLLElement *element = new DLLElement(item, sortKey);
    DLLElement *ptr;
    if (IsEmpty()) 
    {//链表为空的插入
        first = element;
        last = element;
    } 
    else if (sortKey < first->key) 
    {//插入头部
        first->prev = element;
        element->next = first;
		if(type==1)
            currentThread->Yield();//错误1：插入元素到头部的时候，在设置first指针前切换进程，当另一个进程把元素插入到头部时会出错
        first = element;
    } 
    else 
    {
        for (ptr = first; ptr->next != NULL; ptr = ptr->next) 
        {
            if (sortKey < ptr->next->key)//如果当前索引值小于下一个的索引值
            {
				ptr->next->prev = element;
				element->next = ptr->next;//先插在下一个元素的前面
				element->prev = ptr;
				ptr->next = element;
                count++;
				printf("Thread %d finish inserting key = %d.\n", which,sortKey);
                dllnotEmpty->Signal(dllLock);
                dllLock->Release();
				return;
			}
		}
		element->prev = last;
		last->next = element;//插入尾部
		if(type==2)
            currentThread->Yield();	//错误2：插入元素到尾部的时候，在设置last指针前切换进程，当另一个进程把元素插入到尾部时会出错
		last = element;
    }
    count++;
	printf("Thread %d finish inserting key = %d.\n", which,sortKey);
    dllnotEmpty->Signal(dllLock);
    dllLock->Release();
    currentThread->Yield();
}

void *DLList::SortedRemove(int sortKey,int which)//item为待插入元素值，sortkey为索引
{
    dllLock->Acquire();
    while (count==0)
        dllnotEmpty->Wait(dllLock);

    DLLElement *temp, *ptr;
    for(ptr = first; ptr != NULL; ptr = ptr->next){
        if(ptr->key == sortKey){//找到与索引值一样的项
            break;
        }
    }
    if(ptr == NULL){//没有这个项
		printf("  There is no such item %d.\n", sortKey);
        dllnotFull->Signal(dllLock);
        dllLock->Release();
		return NULL;
	}
    if(ptr == first){
		first = first->next;
		if(first != NULL)
			first->prev = NULL;
		else{	//删完该元素就成空链表
			if(type==3)
                currentThread->Yield(); //错误3：remove未完成时切换进程：在删除最后一个元素时last没有指向NULL
			last = NULL;
		}
	}else if(ptr == last){
        last = ptr->prev;
		if(last != NULL)
			last->next = NULL;
		else{	//删完该元素就成空链表，实际上这条应该不会发生
			if(type==3)
                currentThread->Yield(); //错误3：remove未完成时切换进程：在删除最后一个元素时first没有指向NULL,造成头部缺失
			first = NULL;
		}
    }else{
        temp = ptr->prev;
		temp->next = ptr->next;
        if(type==4)
		  currentThread->Yield();//错误4：删除元素时，还没让ptr->next的前驱指向temp：当下次需要用到ptr->next地方的时候会出错
        ptr->next->prev = temp;
    }
    count--;
	printf("Thread %d finish removing key = %d.\n", which,sortKey);
    dllnotFull->Signal(dllLock);
    dllLock->Release();
    currentThread->Yield();
    return ptr->item;
}

