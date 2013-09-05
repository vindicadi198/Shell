#define _POSIX_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<signal.h>
#include<setjmp.h>
#include<fcntl.h>
#include<ctype.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<stdbool.h>
#include<errno.h>
#define STDIN	0
#define STDOUT	1
#define STDERR	2

pid_t pid_arr[50]; /* Array to hold all running(foreground) pids */
pid_t pid_arr_bg[50]; /* Array to hold all running(background) pids */
int pid_arr_index=0,pid_arr_bg_index=0; /* index of last entry in corresponding
										   array */

sigjmp_buf ctrlc_buf; /*control jump point after handling events */
void init_sh();
bool error_check(char *,int);
void freeHistory();
void sigint_handler(int signum); /* function to handle SIGINT events */
void sigtstp_handler(int signum); /* function to handle SIGINT events */

int main(int argc,char * argv[],char **envp){
	init_sh(); /* initialise the shell and environment variables */
	struct sigaction sigint; /* SIGINT handler setup */
	sigint.sa_handler = sigint_handler;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags=0;
	sigaction(SIGINT, &sigint, NULL); /* register the signal handler */
	struct sigaction sigtstp; /* SIGTSTP handler setup */
	sigtstp.sa_handler = sigtstp_handler;
	sigemptyset(&sigtstp.sa_mask);
	sigtstp.sa_flags=0;
	sigaction(SIGTSTP, &sigtstp, NULL); /* register the signal handler */
	int num_pipes=0; /*variable to store the number of pipe symbols in a line*/
	char **tmp; /* temporary string array for parsing */
	char *line; /* string of entered command */
	while ( sigsetjmp( ctrlc_buf, 1 ) != 0 ); /* return point of signal handler*/
	while(1){
		fflush(stdin);
		fflush(stdout);
		line=readline("group1$"); /* function to get entire line from console*/
		if(line && *line)
			add_history(line); /* add entered command into history */
		else
			continue; /* prompt again if nothing entered */
		int line_length=strlen(line);
		bool syntax_err=error_check(line,line_length);
		if(syntax_err==1){
			printf("Error in entered command\n");
			continue;
		}
		bool notBackground=1; /* flag for background process */
		if(line[line_length-1]=='&'){
			notBackground=0;
			line[line_length-1]='\0';
		}
		tmp= (char**)malloc(100*sizeof(char*));
		char *tok=strtok(line,"|"); /* tokenize string with | symbol */
		while(tok!=NULL){
			tmp[num_pipes]=tok;
			tok=strtok(NULL,"|");
			num_pipes++;
		}
		tmp[num_pipes]=NULL;
		num_pipes--; /* number of pipes is one less than number of commands */
		if(strstr(tmp[0],"cd")){ /* implement cd in shell */
			char **cd=(char **)malloc(50*sizeof(char*));
			char *tok_cd=strtok(tmp[0]," "); /*tokenize the command */
			int itr_cd=0;
			while(tok_cd!=NULL){
				cd[itr_cd]=tok_cd;
				tok_cd=strtok(NULL," ");
				itr_cd++;
			}
			cd[itr_cd]=NULL;
			if(cd[1]==NULL){
				if(chdir(getenv("HOME"))<0)
					printf("%s\n",strerror(errno));
			}else{
				if(chdir(cd[1])<0)
					printf("%s\n",strerror(errno));
			}
			free(cd);
			free(tmp);
			continue;
		}else if(!strcmp(tmp[0],"bg")){ /*send stopped job to background */
			if(pid_arr_bg_index!=0){	
				for(int i=0;i<pid_arr_bg_index;i++){
					kill(pid_arr_bg[i],SIGCONT);
				}
			}
			pid_arr_bg_index=0;
			free(tmp);
			continue;
		}else if(!strcmp(tmp[0],"fg")){ /*send stopped job to foreground */
			if(pid_arr_bg_index!=0){
				for(int i=0;i<pid_arr_bg_index;i++){
					kill(pid_arr_bg[i],SIGCONT);
					pid_arr[i]=pid_arr_bg[i];
					pid_arr_index++;
				}
				int wait_ret=1;
				for(int i=0;i<pid_arr_bg_index;i++){
					wait_ret=waitpid(pid_arr_bg[i],0,0); /* wait for job*/
				}
				pid_arr_index=0;
				pid_arr_bg_index=0;
			}
			free(tmp);
			continue;
		}else if(!strcmp(tmp[0],"exit")){ /* exit command exits the shell */
			free(tmp);
			write_history(NULL);
			freeHistory();
			exit(0);
		}
		int fd[2*num_pipes];
		int i=0;
		for(;i<num_pipes+1;i++){
			if(i!=num_pipes)
				pipe(fd+(2*i)); /* creating pipes for commands */
			pid_t pid=fork(); /* fork new child */
			if(pid==0){ /* child process */
				if(notBackground==0)
					if(setpgid(0,0)!=0){ /*change group ID to prevent SIGINT */
						printf("setpgid error\n");
						exit(0);
					}
				char **cmd=(char **)malloc(50*sizeof(char*));
				char *tok1=strtok(tmp[i]," "); /*tokenize the command */
				int itr=0;
				while(tok1!=NULL){
					cmd[itr]=tok1;
					tok1=strtok(NULL," ");
					itr++;
				}
				cmd[itr]=NULL;
				int i1=(2*i)-2,i2=(2*i)-1,i3=(2*i),i4=(2*i)+1; /* indexes in 
																  array for pipes */
				int iterator = 0;
				int flag = 0;
				int filed;
				for(iterator = 0; iterator < itr; iterator++) /*
							handling file redirections */
				{
				    if(strcmp(cmd[iterator],">") == 0)
				    {
						if(cmd[iterator+1]==NULL){
							printf("No file name specified. Aborting\n");
							exit(1);
						}
				        filed = open(cmd[iterator+1], O_CREAT|O_RDWR|O_TRUNC,0644);
				        if(filed < 0){
				            fprintf(stderr,"Error in opening file \n");
				        }
				        fd[i3] = filed;
				        fd[i4] = filed;
				        cmd[iterator] = NULL;
				        flag = 1;
				    }
				    else if(strcmp(cmd[iterator],"<") == 0)
				    {
				        if(cmd[iterator+1]==NULL){
							printf("No file name specified. Aborting\n");
							exit(1);
						}
						filed = open(cmd[iterator+1], O_RDWR,0644);
				        if(filed < 0){
				            fprintf(stderr,"Error in opening file \n");
				        }
				        cmd[iterator] = NULL;
				        flag = 2;
				    }
				}
				if(i==0){ /* for first command */
					 if(flag == 2){ /* input file redcrection priority */
				        dup2(filed,0);
				        close(filed);
				        flag = 0;
				    }
				    if(num_pipes > 0 || flag == 1){ /* output redirection */
				        dup2(fd[i4],1);
					    close(fd[i3]);
					    close(fd[i4]);
				    }
				}else if(i!=num_pipes){ /* for all commands except last */
					if(flag == 2){ /* input file redcrection priority */
				        dup2(filed,0);
				        close(filed);
				        flag = 0;
				    }else{
				        dup2(fd[i1],0); /*input from previous pipe */
				    }
					dup2(fd[i4],1);
					close(fd[i1]);
					close(fd[i2]);
					close(fd[i3]);
					close(fd[i4]);
				}else{ /* for last command */
					close(0);
					if(flag == 1)	{ /* output redirection to file */
						dup2(filed,1);
						dup2(fd[i1],0);
					}else if(flag == 2){ /* input redirection from file */
						dup2(filed,0);
						close(filed);
						flag = 0;
					}else{
						dup2(fd[i1],0); /* input from pipe */
					}
					close(fd[i1]);
					close(fd[i2]);
				}
				execvp(cmd[0],cmd); /* execute command */
				fprintf(stderr,"error!!!%s %s\n",tmp[i],strerror(errno));
				exit(0);
			}
			else{
				if(notBackground){ /* put pid in correct array */
					pid_arr[pid_arr_index]=pid;
					pid_arr_index++;
				}else{
					pid_arr_bg[pid_arr_bg_index]=pid;
					pid_arr_bg_index++;
				}
				if(i!=0){
					close(fd[(2*i-2)]); /* closing ends of the pipe in parent */
					close(fd[(2*i-1)]);
				}
			}
		}
		while(notBackground){ /* wait for all non-background processes */
			int n=wait(NULL);
			if(n==-1)
				break;
		}
		pid_arr_index=0; /*reset variables to default for next loop */
		num_pipes=0;
		free(tmp); /* free temporary string memory */
		fflush(stdin);
		fflush(stdout);
	}
	return 0;
}
void init_sh(){
	chdir(getenv("HOME")); /* change directory to $HOME */
	char path[200];
	FILE* profile = fopen("profile","r");
	if(profile!=NULL){
		fscanf(profile, "%s", path);	
		setenv("PATH", path, 1); /* set PATH variable */
	}
	read_history(NULL); /* read previous history from file */
}
bool error_check(char * str,int size){
	for(int i=0;i<size;i++){
		if(i!=size-1 && str[i]=='&')
			return true;
	}
	return false;
}
void freeHistory(){
	int length = history_length;
	HIST_ENTRY * toBeFreed;
	for(int i = length-1;i >=0;i--){
		toBeFreed = remove_history(i);
		free(toBeFreed);
	}	
}
void sigint_handler(int signum){
	for(int i=0;i<pid_arr_index;i++){
		kill(SIGTERM,pid_arr[i]); /* send SIGTERM to all running processes */
	}
	siglongjmp(ctrlc_buf, 1); /* jump to set point in main */
}
void sigtstp_handler(int signum){
	for(int i=0;i<pid_arr_index;i++){ /* move running process to background array */
		pid_arr_bg[i]=pid_arr[i];
		pid_arr_bg_index++;
	}
	pid_arr_index=0; /* reset foreground array index */
}
