#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int
main(int argc,char *argv[],char **env)
{
	char *name=argv[0];
	char cmd[100];
	scanf("%s",cmd);
	printf("The command entered is %s\n",cmd);
	int cpid=fork();
	if(cpid==0){
		execve(cmd,argv,env);
		printf("Command finished\n");
		exit(0);
	}
	else{
		wait(NULL);
		printf("Child finished executing command\n");
	}
	return 0;
}
