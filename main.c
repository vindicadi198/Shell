#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#define STDIN	0
#define STDOUT	1
#define STDERR	2


int main(int argc,char * argv[],char **envp){
	chdir("/Users/adityakamath/Documents/iith/github/assgn1");
	int num_pipes=0;
	char **tmp; 
	int pid_arr[50];
	int pid_arr_index=0;
	char *line=(char*)malloc(4096*sizeof(char));
	size_t size=4096;
	while(1){
		
		printf("group1$");
		fflush(stdin);
		fflush(stdout);
		//gets(line);
		int n=read(0,line,size);
		if(n==0) continue;
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
				pid_arr[pid_arr_index]=pid;
				pid_arr_index++;
				if(i!=0){
				//	if(i!=num_pipes)
				//		close(fd[(2*i)]);
				//	else
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

