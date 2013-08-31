#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<signal.h>
#include<setjmp.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<stdbool.h>
#define STDIN	0
#define STDOUT	1
#define STDERR	2
typedef struct{
	pid_t pid;
	bool isBackground;
	bool isStopped;
}pid_s;

pid_s pid_arr[50];

void signal_callback_handler(int signum)
sigjmp_buf ctrlc_buf;
void sigint_handler(int signum);

int main(int argc,char * argv[],char **envp){

	struct sigaction sigact;
	sigact.sa_handler = signal_callback_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags=0;
	sigaction(SIGINT, &sigact, NULL);
	chdir("/Users/adityakamath/Documents/iith/github/assgn1");
	int num_pipes=0;
	char **tmp; 
	int pid_arr[50];
	int pid_arr_index=0;
	char *line;
	while ( sigsetjmp( ctrlc_buf, 1 ) != 0 );
	while(1){
		
		//printf("group1$");
		fflush(stdin);
		fflush(stdout);
		line=readline("group1$");
		if(line && *line)
			add_history(line);
		else
			continue;
		tmp= (char**)malloc(100*sizeof(char*));
		char *tok=strtok(line,"|");
		while(tok!=NULL){
			tmp[num_pipes]=tok;
			tok=strtok(NULL,"|");
			num_pipes++;
		}
		tmp[num_pipes]=NULL;
		num_pipes--;
		int fd[2*num_pipes];
		int i=0;
		printf("num of pipes %d\n",num_pipes);
		for(;i<num_pipes+1;i++){
			if(i!=num_pipes)
				pipe(fd+(2*i));
			int pid=fork();
			if(pid==0){
				
				char **cmd=(char **)malloc(50*sizeof(char*));
				char *tok1=strtok(tmp[i]," ");
				int itr=0;
				while(tok1!=NULL){
					cmd[itr]=tok1;
					tok1=strtok(NULL," ");
					itr++;
				}
				cmd[itr]=NULL;
				int i1=(2*i)-2,i2=(2*i)-1,i3=(2*i),i4=(2*i)+1;
				if(i==0){
					dup2(fd[i4],1);
					close(fd[i3]);
					close(fd[i4]);
				}else if(i!=num_pipes){
					dup2(fd[i1],0);
					dup2(fd[i4],1);

					close(fd[i1]);
					close(fd[i2]);
					close(fd[i3]);
					close(fd[i4]);
				}else{
					close(0);
					dup2(fd[i1],0);
					close(fd[i1]);
					close(fd[i2]);
				}
				execvp(cmd[0],cmd);
				fprintf(stderr,"error!!!%s %d\n",tmp[i],i);
				exit(0);
			}
			else{
				pid_arr[pid_arr_index].pid=pid;
				pid_arr[pid_arr_index].isBackground=false;
				pid_arr[pid_arr_ndex].isStopped=false;
				pid_arr_index++;
				if(i!=0){
					close(fd[(2*i-2)]);
					close(fd[(2*i-1)]);
				}
			}
		}
		while(1){
			int n=wait(NULL);
			if(n==-1)
				break;
		}
		pid_arr_index=0;
		num_pipes=0;
		free(tmp);
		fflush(stdin);
		fflush(stdout);
	}
	return 0;
}
void signint_handler(int signum)
{
	for(int i=0;i<pid_arr_index;i++){
		kill(SIGTERM,pid_arr[i]);
	}
	siglongjmp(ctrlc_buf, 1);
}

