#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
 //cat /etc/passwd | cut -d : -f 1 
#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
  
// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")
 

char *trim(char *string){
    char *ptr = NULL;
    while (*string == ' ') string++;    
    ptr = string + strlen(string) - 1;
    while (*ptr == ' '){ *ptr = '\0' ; ptr--; } ;
    return string;
}


char *lerLinha(void)
{
   int i = 0;
   char *buffer = malloc(sizeof(char) * 1024);
   int c;
   
   while(1)
   {
   	c = getchar();
   	
   	if(c == '\n'){
   		buffer[i] = '\0';
   	 	return buffer;
   	}else{
   		buffer[i] = c;
   	}
   	i++;   
   }   
}
  
// Function where the system command is executed
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork(); 
  
    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL); 
        return;
    }
}

void execSeparated(char** parsed) {

    const char s[2] = ";";
    char *token;
	char *parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
	
	token = strtok(parsed, s);
   while( token != NULL ) {
	   
	   processString(token, parsedArgs, parsedArgsPiped);
       execArgs(parsedArgs);
        token = strtok(NULL, s);
   }

}
  
// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2]; 
    pid_t p1, p2;
  
    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }
  
    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
  
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();
  
        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }
  
        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(pipefd        [1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            wait(NULL);
        }
    }
}
 
  
// function for finding pipe
int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }
  
    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}
  
// function for parsing command words
void parseSpace(char* str, char** parsed)
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
  
int processString(char* str, char** parsed, char** parsedpipe)

{
    char separador = ';';
    char* strpiped[2];
    int piped = 0;
    char *ret;

     if(strchr(str, separador) != NULL){
         return piped + 3;
     }
 

    piped = parsePipe(str, strpiped);

    if (piped) {

        parseSpace(strpiped[0], parsed);

        parseSpace(strpiped[1], parsedpipe);

    } else {
        parseSpace(str, parsed);

    }

    return 1 + piped;

}
  
int main()
{
    char *inputString, *parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    int execFlag = 0;
  
    while (1) {

        printf(">>>");
        inputString = lerLinha();
        // process
        execFlag = processString(inputString, parsedArgs, parsedArgsPiped);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
        // 2 if it is including a pipe.
  
        // execute
        if (execFlag == 1)
            execArgs(parsedArgs);
  
        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);
        
        if (execFlag == 3)
            execSeparated(inputString);
    }
    return 0;
}