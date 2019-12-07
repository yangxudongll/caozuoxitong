#include<stdio.h>
#include<sys/types.h>
#include<linux/sem.h>
#include<pthread.h>
//P，V操作
void P(int semid,int index);
void V(int semid,int index);
//信号灯，线程句柄定义
int semid;
pthread_t p1,p2;
//线程执行函数定义
void *subp1();
void *subp2();

int main()
{
	//创建信号灯，赋值



	//创建两个线程


	//等待两个线程结束


	//删除信号灯
	return 0;
}