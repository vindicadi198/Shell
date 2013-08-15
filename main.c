#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int
main(int argc,char *argv[])
{
	char *name=argv[0];
	int cpid=fork();
	if(cpid==0){
		for(int i=0;i<10;i++)
		{	
			printf("The child pid is %d\n",getpid());
			sleep(1);
		}
		printf("Child exiting\n");
		exit(0);
	}
	else {
		printf("Child created with pid %d\n",cpid);
		printf("Waiting for child \n");
		wait(NULL);
		printf("The child exiting\n");
		exit(0);
        
	}
	printf("parent exiting\n");
}
