// C Program to design a shell in Linux
#include<iostream>
#include<stdio.h>
#include<string.h>
#include<cstring>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXCOM 1000
#define MAXLIST 100

using namespace std;

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

// Greeting shell during startup
void init_shell()
{
	clear();
	char* username = getenv("USERNAME");
	char* hostname = getenv("HOSTNAME");
	printf("\n\n\nUSER is: @%s @ %s", username,hostname);
	printf("\n");
	sleep(2);
	clear();
}

// Function to take input
int takeInput(char* str)
{
	char* buf=NULL;
  //ssize_t bufsize = 0; // have getline allocate a buffer for us
//  getline(&buf, &bufsize, stdin);
  buf = readline("\n% ");
	if (strlen(buf) != 0) {
		add_history(buf);
		strcpy(str, buf);
		return 0;
   	}
  else
		return 1;

}

// Function to print Current Directory.
void printDir()
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
//	printf("\nDir: %s", cwd);
	cout<<"\nDir: "<<cwd;
}


// function for finding pipe and redirection
int parsePipeAndRedirection(char* str, char** strspecial)
{
	int i;
  int com=0;
  for(i=0;i<strlen(str);i++){
    if((str[i]-'|')==0)
     {  com=1; break;}
    if((str[i]-'<')==0)
     {  com=2; break;}
    if((str[i]-'>')==0)
     { if((str[i+1]-'>')==0)
        {com=3; break;}
       else
        {com=4; break;}
     }
   }

  if(com==1) {
	 for (i = 0; i < 2; i++) {
		strspecial[i] = strsep(&str, "|");
		if (strspecial[i] == NULL)
			break;
	   }
   }

  if(com==2) {
 	 for (i = 0; i < 2; i++) {
 		strspecial[i] = strsep(&str, "<");
 		if (strspecial[i] == NULL)
 			break;
 	   }
    }

  if(com==3) {
  	 for (i = 0; i < 2; i++) {
  		strspecial[i] = strsep(&str, ">>");
  		if (strspecial[i] == NULL)
  			break;
  	   }
     }

 if(com==4) {
   	for (i = 0; i < 2; i++) {
   		strspecial[i] = strsep(&str, ">");
   		if (strspecial[i] == NULL)
   			break;
   	   }
  }
return com;
}

// function for parsing command words
void inputParser(char* str, char** parsed)
{
	int i;

	for (i = 0; i < MAXLIST; i++) {
		parsed[i] = strsep(&str, " ");

		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
}

int processString(char* str, char** parsed1, char** parsed2)
{
 char* strspecial[2];
	int comm = 0;

	comm = parsePipeAndRedirection(str, strspecial);

	if (comm) {
		inputParser(strspecial[0], parsed1);
		inputParser(strspecial[1], parsed2);
   }
  else {
  	inputParser(str, parsed1);
	}
	return comm;
}

// Function where the system command is executed
void execArgs(char** parsed)
{
	// Forking a child
	pid_t pid = fork();

	if (pid == -1) {
		printf("\nFailed forking child..");
		return;
	}
  else if (pid == 0) {
		if (execvp(parsed[0], parsed) < 0)
			printf("\nCould not execute command..");
		exit(0);
	}
  else {
		// waiting for child to terminate
		wait(NULL);
		return;
	}
}

//Function where pipe command is executed
int pipeEx(char** parsed1, char** parsed2){
     pid_t pid1, pid2;
     int pipefd[2];
     // The two commands we'll execute.  In this simple example, we will pipe
     // the output of `ls` into `wc`, and count the number of lines present.
    // char *argv1[] = {"ls", "-l", "-h", NULL};
    // char *argv2[] = {"wc", "-l", NULL};
    // char **c1=argv1;
    // char **c2=argv2;
     // Create a pipe.
     pipe(pipefd);
     // Create our first process.
     pid1 = fork();
     if (pid1 == 0) {
        // Hook stdout up to the write end of the pipe and close the read end of
        // the pipe which is no longer needed by this process.
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        // Exec `ls -l -h`.  If the exec fails, notify the user and exit.  Note
        // that the execvp variant first searches the $PATH before calling execve.
        execvp(parsed1[0], parsed1);
        perror("exec");
        return 1;
     }
     // Create our second process.
     pid2 = fork();
     if (pid2 == 0) {
        // Hook stdin up to the read end of the pipe and close the write end of
        // the pipe which is no longer needed by this process.
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        // Similarly, exec `wc -l`.
        execvp(parsed2[0], parsed2);
        perror("exec");
        return 1;
     }
     // Close both ends of the pipe.  The respective read/write ends of the pipe
     // persist in the two processes created above (and happen to be tying stdout
     // of the first processes to stdin of the second).
     close(pipefd[0]);
     close(pipefd[1]);
     // Wait for everything to finish and exit.
     waitpid(pid1,0,0);
     waitpid(pid2,0,0);
     return 0;
  }

int redirectionEx(char** parsed1, char** parsed2,int f){
  char **c1=parsed1;
  char **c2=parsed2;
  pid_t p;
	//cout<<c1[0]<<endl;
  //cout<<c2[0]<<endl;
	//>
  if(f==4) {
      //close(1); // Release fd no - 1
   int fd= open( c2[0], O_RDWR | O_CREAT , 0754 ); // Open a file with fd no = 1
        // Child process
   p=fork();
			if (p == 0) {
				dup2(fd,1);
        execvp(c1[0],c1); // By default, the program writes to stdout (fd no - 1). ie, in this case, the file
      close(fd);  }

	 waitpid(p,0,0);
	 return 0;
        }

  //<
   if(f==2) {
      close(0);//Release fd - 0
    int fd=  open( c2[0],O_RDONLY); //Open file with fd - 0
       //Child process
    p=fork();
		if (p == 0)
        execvp(c1[0],c1); // By default, program reads from stdin. ie, fd - 0
    close(fd);
		waitpid(p,0,0);
      }

   // >>
    if(f==3) {
	    //	close(1);//Release fd - 1
    int fd= open( c2[0], O_RDWR | O_CREAT | O_APPEND , 0754 ); //Open file with fd - 1
		//Child process
		p=fork();
		if (p == 0) {
		    dup2(fd,1);
				execvp(c1[0],c1); // By default, the program writes to stdout (fd no - 1). ie, in this case, the file
	      close(fd);
			  }

	 waitpid(p,0,0);
	 return 0;
		}

  return 0;
}


int main()
{
	char inputString[MAXCOM];
	char *parsedArgs1[MAXLIST] , *parsedArgs2[MAXLIST];
	int execFlag = 0;
	init_shell();

	while (1) {
		// print shell line
		printDir();

		if (takeInput(inputString))
			continue;

		execFlag = processString(inputString, parsedArgs1, parsedArgs2);

  // printf("\n##%d\n",execFlag);

		if (execFlag == 0)
			execArgs(parsedArgs1);

		if (execFlag == 1)
			pipeEx(parsedArgs1, parsedArgs2);

    if (execFlag == 2 || execFlag == 3 || execFlag == 4)
  		redirectionEx(parsedArgs1, parsedArgs2,execFlag);
	}
	return 0;
}
