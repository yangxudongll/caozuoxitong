#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#define num 10       //10个缓冲区

//本来此结构应该包含山linux/sem.h中,但是一旦
//包含linux/sem.h就会出现许多重定义的问题
union semun {
	int val; 
	struct semid_ds *buf;
	unsigned short *array; 
	struct seminfo *__buf; 
};

void readBuf();
void writeBuf();

int child1;
int child2;

//共享存储区名字
key_t key=1000;
//缓冲区大小
const int size=100; 
//信号灯集
int semid;

int shmid;

char (*buf)[100];

int in=0,out=0;   //读写指针

int *flag;        //判断文件读完写完标志

//P、V操作的定义
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

void readBuf()
{

	//打开源文件文件
	FILE* fp=fopen("input.txt","r");
	int fsize;
	flag[1]=0;
	while(1)
	{
		//信号灯P操作
		P(semid,0);
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
		}
	}
	exit(0);
}
void writeBuf()
{
	int fsize;
	//创建目标文件
	FILE* fw=fopen("output.txt","wb");

	int i=1;
	while(1)
	{

		//信号灯P操作
		P(semid,1);
		//写入文件
		if(i<flag[1] || flag[0]==size)
			fwrite(buf[out],sizeof(char),size,fw);
		else                                              //如果已经读到了最后一个缓冲区,即文件的最后一部分
			fwrite(buf[out],sizeof(char),flag[0],fw);
		//移动环形缓冲区指针
		out=(out+1)%num;
		//信号灯V操作
		V(semid,0);
		//if(数据结束)break;
		if(i==flag[1] && flag[0]!=size)
		{
			fclose(fw);	//关闭文件
			break;
		}
		i++;
	}
	exit(0);
}
int main()
{
	//创建key的共享内存组
	shmid=shmget(key,sizeof(char[size])*num,IPC_CREAT|0666);
	buf=shmat(shmid,0,SHM_R|SHM_W);
	int shm;
	//创建两个int大小的共享区,用于存放文件每次读取到的大小和读取次数,方便判断文件是否结束
	shm=shmget(3000,sizeof(int)*2,IPC_CREAT|0666);
	flag=shmat(shm,0,SHM_R|SHM_W);
	flag[0]=0;
	flag[1]=0;
	//创建Key的信号灯
	semid=semget(2000,2,IPC_CREAT|0666);
	//信号灯赋初值
	union semun a1;
	a1.val=num;
	semctl(semid,0,SETVAL,a1);//表示设置信号灯集semid中0号信号灯的值为1
	a1.val=0;
	semctl(semid,1,SETVAL,a1);
	//创建两个子进程
	child1=fork();
	if(child1==0)   //子进程1
	{
		execv("./readtxt",NULL);
		//readBuf();
	}
	else    //父进程
	{
		child2=fork();
		if(child2==0)		//子进程2
		{
			execv("./writetxt",NULL);
			//writeBuf();
		}
	}
	
	//等待子进程结束
	waitpid(child1,NULL,0);
	waitpid(child2,NULL,0);
	printf("抄写工作结束\n");
	//删除信号灯
	//IPC_RMID表示将信号灯从内存中删除
/*	semctl(semid,0,IPC_RMID);
	
	//删除共享内存组
	shmdt(buf);
	shmdt(flag);
	shmctl(shmid,IPC_RMID,0);
	shmctl(shm,IPC_RMID,0);*/
	return 0;
}
