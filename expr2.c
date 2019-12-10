#include <stdio.h>
#include <pthread.h>   //这个不是Linux默认的库,编译时需要加上-lpthread
#include <stdlib.h>
#include <sys/sem.h>

//本来此结构应该包含山linux/sem.h中,但是一旦
//包含linux/sem.h就会出现许多重定义的问题
union semun {
	int val; 
	struct semid_ds *buf;
	unsigned short *array; 
	struct seminfo *__buf; 
};
//P，V操作
void P(int semid,int index);
void V(int semid,int index);
//信号灯，线程句柄定义
int semid;
pthread_t p1,p2;
//线程执行函数定义
void *subp1();
void *subp2();

//线程的共享变量
int count=0;

int main()
{
	//创建信号灯集，个数为2,权限标志为IPC_CREAT|0666
	semid=semget(0,2,IPC_CREAT|0666);
	union semun a1;
	a1.val=1;
	semctl(semid,0,SETVAL,a1);//表示设置信号灯集semid中0号信号灯的值为1
	a1.val=0;
	semctl(semid,1,SETVAL,a1);//表示设置信号灯集semid中1号信号灯的值为0

	//创建两个线程
	pthread_create(&p1,NULL,subp1,NULL);
	pthread_create(&p2,NULL,subp2,NULL);

	//等待两个线程结束
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);

	//IPC_RMID表示将信号灯从内存中删除
	semctl(semid,0,IPC_RMID);

	return 0;
}

void P(int semid,int index)
{
	struct sembuf sem;
	sem.sem_num=index;  //哪一个信号灯,index
	sem.sem_op=-1;      //操作
	sem.sem_flg=0;      //操作标志,一般置位0即可
	semop(semid,&sem,1);
	return;
}
void V(int semid,int index)
{
	struct sembuf sem;
	sem.sem_num=index;
	sem.sem_op=1;
	sem.sem_flg=0;
	semop(semid,&sem,1);//1表示执行命令的个数
	return;
}
void *subp1()
{
	for(int i=0;i<=100;i++)
	{
		P(semid,0);
		count=count+i;
		V(semid,1);
	}
}
void *subp2()
{
	for(int i=0;i<=100;i++)
	{
		P(semid,1);
		printf("sum=%d\n", count);
		V(semid,0);
	}
}