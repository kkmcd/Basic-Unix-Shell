/*
*************BASIC SHELL***************
***************************************
Abhishek Chakraborty(CS 1120)
Arani Bhattacharya(CS 1108)
Sayan Bandyapadhyay(CS 1111)
Sujoy Madhab Roy(CS 1124)
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include<fcntl.h>
#include <ctype.h>

#define MAX 100

typedef struct  
{
  char *name;
  int argc;
  char **argv;
} CommandLine;


//input string without any following space
char *input;

//environmental variables
char *current_user;
char *current_dir;
char *current_system;

CommandLine *command;
int amper;

//parse the input string, trim spaces
void string_parse()
{

	char* buffer = (char*)calloc(1024, sizeof(char));
	int count;
	char* input1;
	char* temp;
	char* ptr;  
	char* inptr;

	input1 = (char *)calloc(1024, sizeof(char));
	temp  = (char *)calloc(1024, sizeof(char));
    
	inptr = input1; 
	ptr = temp;
    
	while((*ptr ++ = getchar()) != '\n');   
		* -- ptr = '\0';               
	ptr = temp;
	while(isspace(*ptr)) 
		++ptr ; 
	while((*inptr++ = *ptr++) != '\0');   

	free(temp);

	command->argv = (char**)calloc(16, sizeof(char*));     
	command->name = (char*)calloc(16, sizeof(char));
	command->argv[0] = (char*)calloc(16, sizeof(char));
	command->argv[1] = (char*)calloc(16, sizeof(char));          
	command->argv[2] = (char*)calloc(16, sizeof(char));          
	command->argv[3] = (char*)calloc(16, sizeof(char));                    

	ptr = strtok(input1, " ");
	command->name = ptr;
	command->argv[0] = ptr;
	command->argc = 1;
	count = 1;
	ptr = strtok (NULL, " ");

	while (ptr != NULL) 
	{
		command->argv[count] = ptr;
		count++;
		ptr = strtok(NULL, " ");
	}
	command->argc=count;
}


//handle internal commands like cd, logout, exit
void internal_command_handler()
{

	int i,l,j;
	char *temp=(char *)malloc(MAX*sizeof(char));
	char *temp1=(char *)malloc(MAX*sizeof(char));

	if(!strcmp(input,"exit"))
	{
		free(input);
		free(current_dir);
		free(command);
		free(current_system);
		exit(0);
	}

	else if(!strcmp(input,"whoami"))
	{
		printf("%s\n",current_user);
	}
	
	else if(!strcmp(input,"logout"))
	{
		char temp2[]="pkill -KILL -u ";
		strcat(temp2,current_user);
		system(temp2);
	}

	else if(!strcmp(input,"pwd"))
	{
		printf("%s\n",current_dir);
	}

	else if(!strcmp(input,"cd"))
	{
		l=strlen(current_dir);
		for(i=0;i<l;i++)
			current_dir[i]='\0';
		current_dir[0]='/';
           	if (chdir(current_dir) != 0) {
           		printf("Error: Invalid directory.\n");
            }		

	}

	else if(!strcmp(input,"cd ..") || !strcmp(input,"cd.."))
	{
		if(!strcmp(current_dir,"/"))
			return;

		l=strlen(current_dir)-1;
		for(i=0;i<=l;i++)
			temp[i]=current_dir[i];
		for(i=0;i<=MAX;i++)
			current_dir[i]='\0';
		while(temp[l] != '/')
			l--;
		for(j=0;j<l;j++)
			current_dir[j]=temp[j];	
		if(strlen(current_dir) == 0)
			current_dir[0]='/';	
  
           	if (chdir(current_dir) != 0) {
           		printf("Error: Invalid directory.\n");
            }		
	}
	
	else
	{
		if((input[0] == 'c') && (input [1] == 'd'))
		{
	          	if (command->argc == 2) {
      	      	if (chdir(command->argv[1]) != 0) {
      	      		printf("Error: Invalid directory.\n");
      	      	}
      	    	} else {
      	      	printf("Error: Invalid use of cd command.\n");
      	    	}
			getcwd(current_dir,MAX);

		}
	}
	free(temp);
	free(temp1);
		
}


//handle external commands
void external_command_handler()
{
	int pid,status;

	if(command->argc == 1)
		command->argv[1]=(char  *) 0;
	pid = fork();

	if (pid < 0) {
		printf("Error in the fork process.\n");
		return;
	} else if (pid == 0) {

		execvp(command->name,command->argv);
	    	printf("Error: invalid command.\n");
		return;
	} 
	else {
		if(amper == 0)
			while (wait(&status) != pid);
	}
}

//run single level redirection command (<, >)
void redirect_command(int index,int fileid){

	char *filename;
	filename=(char *)malloc(MAX*sizeof(char));
	strcpy(filename,command->argv[index+1]);
	command->argv[index]=(char  *) 0;
	command->argv[index+1]=(char  *) 0;
	command->argc-=2;

 	int pid,status;
	if ((pid = fork()) < 0) {
    		printf("Error: Fork failed.\n");
    		return;
  	}
  	if (pid == 0) {
      int fd;
    	if (fileid == 0) {
      	fd = open(filename, O_RDONLY, 0600);
    	} else if (fileid == 1) {
      	fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    	}
      dup2(fd,fileid);
      close(fd);

    	execvp(command->name, command->argv);
    	printf("Error: execvp failed.\n");
   	return;    
  	} else {
		if(amper == 0)
	    		while (wait(&status) != pid);    
  	}

}

//run pipe commands ('|')
void piped_command(CommandLine* cmnd1, CommandLine* cmnd2)
{

	int pipefd[2],status,pid;

	if((pid=fork()) == 0)
	{
		pipe(pipefd);
		if(fork() == 0)
		{
			fclose(stdout);
			dup(pipefd[1]);
			close(pipefd[1]);
			close(pipefd[0]);			

		    	execvp(cmnd1->name, cmnd1->argv);
		    	printf("Error: execvp failed.\n");
		   	return;    

		}
		fclose(stdin);
		dup(pipefd[0]);
		close(pipefd[1]);
		close(pipefd[0]);	

	    	execvp(cmnd2->name, cmnd2->argv);
	    	printf("Error: execvp failed.\n");
	   	return;    
		
  	} else {
		if(amper == 0)
	    		while (wait(&status) != pid);    
  	}

}

//main program
int main(){

	int cntin,cntout,cntpipe,index,i;
	CommandLine* cmnd1 = (CommandLine*) malloc(sizeof(CommandLine));
	CommandLine* cmnd2 = (CommandLine*) malloc(sizeof(CommandLine));

	command = (CommandLine*) malloc(sizeof(CommandLine));

	input=(char *)malloc(MAX*sizeof(char));
	current_dir=(char *)malloc(MAX*sizeof(char));
	getcwd(current_dir,MAX);
	current_user=(char *)malloc(MAX*sizeof(char));
	current_user=getlogin();
	current_system=(char *)malloc(MAX*sizeof(char));
	gethostname(current_system,MAX);
	printf("\n");
	while(1)
	{
		cntin=0,cntout=0,cntpipe=0;
		amper=0;
		for(i=0;i<MAX;i++)
			input[i]='\0';
		printf("mySHELL~");
		printf("%s",current_user);
		printf("@");
		printf("%s",current_system);
		printf(":");
		getcwd(current_dir,MAX);
		printf("%s",current_dir);
		printf("> ");
		string_parse();

		if(command->name == NULL)
			continue;
		strcat(input,command->name);
		if(command->argc >= 2)
		{
			strcat(input," ");
			strcat(input,command->argv[1]);
		}
		if(command->argc >= 3)
		{
			strcat(input," ");
			strcat(input,command->argv[2]);
		}
		if(command->argc == 4)
		{
			strcat(input," ");
			strcat(input,command->argv[3]);
		}
		command->argv[command->argc]=(char  *) 0;

		
		if (strcmp(command->argv[0], ">") == 0  || strcmp(command->argv[0], "<") == 0  || strcmp(command->argv[0], "|") == 0  ||
			strcmp(command->argv[command->argc-1], ">") == 0  || strcmp(command->argv[command->argc-1], "<") == 0  || 
			strcmp(command->argv[command->argc-1], "|") == 0  )
			continue;

		for(i=1;i<command->argc-1;i++)
		{
					if (strcmp(command->argv[i], ">") == 0)
					{
						index=i;
						cntout++;
					} 
					if (strcmp(command->argv[i], "<") == 0)
					{
						index=i;
						cntin++;
					} 
					if (strcmp(command->argv[i], "|") == 0)
					{
						index=i;
						cntpipe++;
					} 
		}

		if((cntout+cntin+cntpipe) >=2)
		{
			printf("more than one level redirection not supported\n");
			continue;
		}
		
		if(!strcmp(command->argv[command->argc-1],"&"))
		{
			amper=1;
			puts(command->argv[command->argc-1]);
			command->argv[command->argc-1]=(char  *) 0;			
			command->argc--;
		}
			
		
		if(cntout == 1)
		{
			redirect_command(index,1);
			continue;
		}
		
		if(cntin == 1)
		{
			redirect_command(index,0);
			continue;
		}

		if(cntpipe == 1)
		{
			cmnd1->argv = (char**)calloc(16, sizeof(char*));     
			cmnd1->name = (char*)calloc(16, sizeof(char));
			cmnd1->argv[0] = (char*)calloc(16, sizeof(char));
			cmnd1->argv[1] = (char*)calloc(16, sizeof(char));          
			cmnd1->argv[2] = (char*)calloc(16, sizeof(char));          
			cmnd1->argv[3] = (char*)calloc(16, sizeof(char));                    
		
			cmnd2->argv = (char**)calloc(16, sizeof(char*));     
			cmnd2->name = (char*)calloc(16, sizeof(char));
			cmnd2->argv[0] = (char*)calloc(16, sizeof(char));
			cmnd2->argv[1] = (char*)calloc(16, sizeof(char));          
			cmnd2->argv[2] = (char*)calloc(16, sizeof(char));          
			cmnd2->argv[3] = (char*)calloc(16, sizeof(char));                    
			
			strcat(cmnd1->name,command->name);
			for(i=0;i<index;i++)
				strcat(cmnd1->argv[i],command->argv[i]);	
			cmnd1->argv[i]=(char  *) 0;

			strcat(cmnd2->name,command->argv[index+1]);			
			for(i=index+1;i<command->argc;i++)
				strcat(cmnd2->argv[i-index-1],command->argv[i]);				
			cmnd2->argv[i-index-1]=(char  *) 0;

			piped_command(cmnd1,cmnd2);
			continue;
		}

		if (strcmp(command->name, "exit") == 0  || strcmp(command->name, "logout")
		== 0 || strcmp(command->name, "cd") == 0 || strcmp(command->name, "cd..") == 0 ||
		 strcmp(command->name, "pwd") == 0 || strcmp(command->name, "whoami") == 0)
		{
			internal_command_handler();
		}
		else
		{
			external_command_handler();
		}

	}
}
