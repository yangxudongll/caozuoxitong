#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void myfunc1(int siginfo);
void myfunc2(int siginfo);
void did_nothing(int siginfo);

int child1,child2;
int fd[2];


int main()
{
	char sendbuf[20];
	char recvbuf[20];
	int count=1;

	printf("parent process be created\n");

	pipe(fd);

	signal(SIGINT,myfunc1);

	child1=fork();
	if(child1==0)
	{
		signal(SIGINT,did_nothing);
		signal(SIGUSR1,myfunc2);
		while(1)
		{
			sprintf(sendbuf,"I send you %d times",count++);
			write(fd[1],sendbuf,20);
			sleep(1);
		}
	}
	else
	{
		child2=fork();
		if(child2==0)
		{
			signal(SIGINT,did_nothing);
			signal(SIGUSR1,myfunc2);
			while(1)
			{
				read(fd[0],recvbuf,20);
				printf("%s\n",recvbuf);
				sleep(1);
			}
		}
	}
	waitpid(child1,NULL,0);
	waitpid(child2,NULL,0);
	printf("parent be killed by ctrl+c\n");
	return 0;
}	
void did_nothing(int siginfo)
{

}
void myfunc1(int siginfo)
{
	if(siginfo==SIGINT)
	{
		kill(child1,SIGUSR1);
		kill(child2,SIGUSR1);
	}
}
void myfunc2(int siginfo)
{
	close(fd[0]);
	close(fd[1]);
	if(child1==0 && siginfo==SIGUSR1)
	{
		printf("child1 be killed by parent\n");
		exit(0);
	}
	if(child2==0 && siginfo==SIGUSR1)
	{
		printf("child2 be killed by parent\n");
		exit(0);
	}
}
