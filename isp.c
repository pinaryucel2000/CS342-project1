#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/wait.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

// The maximum combined length of both the file name and path name
#define PATH_MAX 4096

// The limit for the length of a command line
#define COMMAND_LINE_MAX 1000

#define BLUE "\033[0;34m"
#define BLACK "\033[0m"
#define SHELL_NAME "isp$"
#define ARGS_MAX 10
#define clearTerminal() printf("\e[1;1H\e[2J")
#define READ_END 0
#define WRITE_END 1
#define STDIN_FD 0
#define STDOUT_FD 1

int N = 10;
int mode = 1;

// Function declerations
void run();
void readInput(char* input);
void printWorkingDirectory();
void handleInput(char* input);
void parseInput(char* input, char*** args1, char*** args2);
void tokenizeInput(char* input, char*** args);
void handleCommand(char** args);
void handleCompositeCommand1(char** args1, char** args2);
void handleCompositeCommand2(char** args1, char** args2);
unsigned long getCurrentTime();

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Invalid number of arguments\n");
		exit(1);	
	}
	
	N = atoi(argv[1]);
	
	if(N < 1 || N > 4096)
	{
		fprintf(stderr, "Invalid value for N\n");
		exit(1);
	}
	
	mode = atoi(argv[2]);
	
	if(mode != 1 && mode != 2)
	{
		fprintf(stderr, "Invalid value for mode\n");
		exit(1);
	}
	
	run();
	
	return 0;
}

void run()
{
	clearTerminal();
	char input[COMMAND_LINE_MAX]; 
	
	while(1)
	{
		readInput(input);
		handleInput(input);
	}
}

void readInput(char* input)
{
	char inputBuffer[COMMAND_LINE_MAX]; 
	
	printWorkingDirectory();
	printf("%s ", SHELL_NAME);
	fgets(inputBuffer, COMMAND_LINE_MAX, stdin);
	
	// Remove the new line character from the input
	inputBuffer[strlen(inputBuffer) - 1] = '\0';
	
	strcpy(input, inputBuffer);
}

void printWorkingDirectory()
{
	char pathName[PATH_MAX];
	getcwd(pathName, sizeof(pathName));
	printf(BLUE);
	printf("%s", pathName);
	printf(BLACK);
}

void handleInput(char* input)
{
	char** args1;
	char** args2;
	
	unsigned long start;
	unsigned long end;
	 
	parseInput(input, &args1, &args2);
	
	if(args2 == NULL)
	{
		handleCommand(args1);
	}
	else if(mode == 1)
	{
		start = getCurrentTime();
		handleCompositeCommand1(args1, args2);
		end = getCurrentTime();
		
		//printf("\nTime taken to handle the command is %ld micro seconds\n", end - start);
	}
	else
	{
		start = getCurrentTime();
		handleCompositeCommand2(args1, args2);
		end = getCurrentTime();
		
		//printf("\nTime taken to handle the command is %ld micro seconds\n", end - start);
	}
	
	free(args1);
	free(args2);
}

void parseInput(char* input, char*** args1, char*** args2)
{
	char* args1Buffer = strtok (input, "|");
	char* args2Buffer = strtok (NULL, "|");
		
	tokenizeInput(args1Buffer, args1);
	tokenizeInput(args2Buffer, args2);	
}


void tokenizeInput(char* input, char*** args)
{
	if(input == NULL)
	{
		*args = NULL;
		return;
	}
	
	char* argsBuffer[ARGS_MAX];
	int i = 0;
	char* token = strtok(input, " ");
	
	while( token != NULL && i < 9) 
	{
		argsBuffer[i++] = token;
		token = strtok(NULL, " ");
	}
	
	argsBuffer[i] = NULL;
	*args = (char**) malloc(sizeof(char*) * (i + 1));  	
  	int j = 0;
  	
  	for(j = 0; j <= i; j++)
  	{
  		(*args)[j] = argsBuffer[j];
  	}
}

void handleCommand(char** args)
{
	pid_t  n; // stores process id
	n = fork();
	
	if (n < 0) 
	{
		fprintf(stderr, "Fork Failed");
		exit(1);
	}
	else if (n == 0) // child process
	{ 
		if(execvp(args[0], args) < 0)
		{
			fprintf(stderr, "Command execution failed.\n");
                	exit(1);
		}
		
		exit(0);
	}
	else // parent process 
	{ 
		wait(NULL);
	}
}

// Normal mode
void handleCompositeCommand1(char** args1, char** args2)
{
	int fds[2];
	
	if(pipe(fds) < 0)
	{
	        fprintf(stderr, "Pipe failed.\n");
       	 exit(1);
	}
	
	pid_t pid1 = fork();
	
	if (pid1 < 0) 
	{
		fprintf(stderr, "First fork failed.\n");
		exit(1);
	}
	else if(pid1 > 0) // parent
	{	
		pid_t pid2 = fork();
		
		if (pid2 < 0) 
		{
			fprintf(stderr, "Second fork failed.\n");
			exit(1);
		}
		else if(pid2 > 0) // parent
		{
			close(fds[READ_END]);
			close(fds[WRITE_END]);
			wait(NULL);
			wait(NULL);
		}
		else if (pid2 == 0) // a child
		{
			close(fds[READ_END]);
			dup2(fds[WRITE_END], STDOUT_FD);
			
			if (execvp(args1[0], args1) < 0) 
			{
				fprintf(stderr, "Execution of the first command failed.\n");
				exit(1);
			}
			
			close(fds[WRITE_END]);
		}
	}
	else if(pid1 == 0) // another child
	{
	
		close(fds[WRITE_END]);
		dup2(fds[READ_END], STDIN_FD);
		
		if (execvp(args2[0], args2) < 0) 
		{
			fprintf(stderr, "Execution of the second command failed.\n");
			exit(1);
		}
		
		close(fds[READ_END]);
	}
}

// Tapped mode
void handleCompositeCommand2(char** args1, char** args2)
{
	int fds1[2];
	int fds2[2];
	
	// for statistics
	int characterCount = 0;
	int readCallCount = 0;
	int writeCallCount = 0;
	
	if(pipe(fds1) < 0 || pipe(fds2))
	{
		fprintf(stderr, "Pipe failed.\n");
		exit(1);
	}
	
	pid_t pid1 = fork();
	
	if (pid1 < 0) 
	{
		fprintf(stderr, "First fork failed.\n");
		exit(1);
	}
	else if(pid1 > 0) // parent
	{	
		pid_t pid2 = fork();
		
		if (pid2 < 0) 
		{
			fprintf(stderr, "Second fork failed.\n");
			exit(1);
		}
		else if(pid2 > 0) // parent
		{
			close(fds2[READ_END]);
			close(fds1[WRITE_END]);
	
			char buffer[N];
			int bytesRead = 0;
			int bytesWritten = 0;
			
			while (++readCallCount && ((bytesRead = read(fds1[READ_END], buffer, N)) > 0))
			{
				bytesWritten = write(fds2[WRITE_END], buffer, bytesRead);
				writeCallCount++;		
				characterCount = characterCount + bytesWritten;
		
          		}
          		
			close(fds1[READ_END]);
			close(fds2[WRITE_END]);
			
			wait(NULL);
			wait(NULL);
			
			//printf("\n\ncharacter-count: %d\n", characterCount);
			//printf("read-call-count: %d\n", readCallCount);
			//printf("write-count: %d\n", writeCallCount);
		}
		else if (pid2 == 0) // a child
		{
			close(fds2[READ_END]);
			close(fds2[WRITE_END]);
			close(fds1[READ_END]);
			dup2(fds1[WRITE_END], STDOUT_FD);
			
			if (execvp(args1[0], args1) < 0) 
			{
				fprintf(stderr, "Execution of the first command failed.\n");
				exit(1);
			}
			
			close(fds1[WRITE_END]);
		}
	}
	else if(pid1 == 0) // another child
	{
		close(fds1[READ_END]);
		close(fds1[WRITE_END]);
		close(fds2[WRITE_END]);
		dup2(fds2[READ_END], STDIN_FD);
		
		if (execvp(args2[0], args2) < 0) 
		{
			fprintf(stderr, "Execution of the second command failed.\n");
			exit(1);
		}
		
		close(fds2[READ_END]);
	}
}

// Returns the current time in microseconds
unsigned long getCurrentTime()
{
	struct timeval timeValue;
	gettimeofday(&timeValue, NULL);
	unsigned long currentTime = timeValue.tv_usec;
	currentTime = currentTime + timeValue.tv_sec * 1e6;
	return currentTime;
}
