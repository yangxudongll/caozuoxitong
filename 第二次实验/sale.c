#include <stdio.h>
#include <pthread.h>   //这个不是Linux默认的库,编译时需要加上-lpthread
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

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
pthread_t p1,p2,p3;   //3个线程一起卖票
//线程执行函数定义
void *subp1();
void *subp2();
void *subp3();

//线程的共享变量,假设有200张票,已售为0
int sum=100;
int count1=0,count2=0,count3=0;
int rand1;
int main()
{
	//创建3个信号灯
	semid=semget(0,3,IPC_CREAT|0666);

	union semun a1;
	a1.val=1;
	semctl(semid,0,SETVAL,a1);//表示设置信号灯集semid中0号信号灯的值为1
	a1.val=0;
	semctl(semid,1,SETVAL,a1);//表示设置信号灯集semid中1号信号灯的值为0
	a1.val=0;
	semctl(semid,2,SETVAL,a1);//表示设置信号灯集semid中2号信号灯的值为0

	srand(time(0));
		//创建三个线程
	pthread_create(&p1,NULL,subp1,NULL);
	pthread_create(&p2,NULL,subp2,NULL);
	pthread_create(&p3,NULL,subp3,NULL);

	//等待三个线程结束
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	pthread_join(p3,NULL);

	printf("售票员1售出%d张\n",count1);
	printf("售票员2售出%d张\n",count2);
	printf("售票员3售出%d张\n",count3);

	//IPC_RMID表示将信号灯从内存中删除
	semctl(semid,0,IPC_RMID);
}

void *subp1()
{

	while(sum>0)
	{
		P(semid,0);
		rand1=(rand())%5;
		if(rand1>sum)
		{
			printf("售票员1售出第%d张\n",sum);
			sum=0;			
		}
		else
		{
			if(rand1>0)
			{
				printf("售票员1售出第%d张\n",rand1);
				sum=sum-rand1;			
			}

		}
		count1=count1+rand1;
		V(semid,1);
	}
}

void *subp2()
{

	while(sum>0)
	{
		P(semid,1);
		rand1=(rand())%5;
		if(rand1>sum)
		{
			printf("售票员2售出第%d张\n",sum);
			sum=0;			
		}
		else
		{
			if(rand1>0)
			{
			printf("售票员2售出第%d张\n",rand1);
			sum=sum-rand1;				
			}

		}
		count2=count2+rand1;
		V(semid,2);
	}
}

void *subp3()
{

	while(sum>0)
	{
		P(semid,2);
		rand1=(rand())%5;
		if(rand1>sum)
		{
			printf("售票员3售出第%d张\n",sum);
			sum=0;			
		}
		else
		{
			if(rand1>0){
				printf("售票员3售出第%d张\n",rand1);
				sum=sum-rand1;				
			}

		}
		count3=count3+rand1;
		V(semid,0);
	}
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