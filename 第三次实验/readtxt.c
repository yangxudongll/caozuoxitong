#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#define num 10

//本来此结构应该包含山linux/sem.h中,但是一旦
//包含linux/sem.h就会出现许多重定义的问题
union semun {
	int val; 
	struct semid_ds *buf;
	unsigned short *array; 
	struct seminfo *__buf; 
};
void P(int semid,int index){
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0; //操作标记：0或IPC_NOWAIT等
	semop(semid,&sem,1); //1:表示执行命令的个数
	return;
}

void V(int semid,int index){
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op =  1;
	sem.sem_flg = 0;
	semop(semid,&sem,1);
	return;
}
char (*buf)[100];

key_t key=1000;
//缓冲区大小
const int size=100; 
//信号灯集
int semid=2000;

int shmid;

char (*buf)[100];

int in=0;   //读写指针

int *flag;        //判断文件读完写完标志


int main()
{
	//获取key的共享内存组
	shmid=shmget(key,sizeof(char[size])*num,0);
	//建立数组形式的环形缓冲	
	buf=shmat(shmid,0,SHM_R|SHM_W);

	//创建两个int大小的共享区,用于存放文件每次读取到的大小和读取次数,方便判断文件是否结束
	int shm;	
	shm=shmget(3000,sizeof(int)*2,0);
	flag=shmat(shm,0,SHM_R|SHM_W);
	//获取key的信号灯
	semid=semget(2000,2,0);
	//信号灯赋初值
/*	union semun a1;
	a1.val=num;
	semctl(semid,0,SETVAL,a1);//表示设置信号灯集semid中0号信号灯的值为1
	a1.val=0;
	semctl(semid,1,SETVAL,a1);*/

	//打开源文件文件
	FILE* fp=fopen("input.txt","r");
	int fsize;
	int i=1;
	while(1)
	{
		//信号灯P操作
		P(semid,0);
		printf("第%d次写入缓冲区\n",i);
		//取文件中数据
		//memset(buf[in],'\0',sizeof(buf[in]));
		fsize=fread(buf[in],sizeof(char),size,fp);
		//移动环形缓冲区指针
		in=(in+1)%num;
		//信号灯V操作
		V(semid,1);
		//if(文件结束)break;
		flag[0]=fsize;
		flag[1]++;      //读取次数
		if(fsize!=size)  //如果某次读到的字数少于100,说明文件结束了
		{
			fclose(fp);
			break;
			printf("结束阅读\n");
		}
		i++;
	}
	exit(0);
}