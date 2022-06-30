#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h> 

#define ALPHANUMERIC_SIZE 36
#define STDOUT_FD 1

int M = 0;
const char ALPHANUMERIC_CHARS[ALPHANUMERIC_SIZE] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Invalid number of arguments\n");
		exit(1);	
	}
	
	M = atoi(argv[1]);
	
	if(M < 0)
	{
		fprintf(stderr, "Invalid value for M\n");
		exit(1);
	}
	
	srand(time(NULL)); 
	
	int bytesWritten;
	char randomAlphanumeric;
	
	for(bytesWritten = 0; bytesWritten < M; )
	{
		randomAlphanumeric = ALPHANUMERIC_CHARS[rand() % ALPHANUMERIC_SIZE];
		bytesWritten = bytesWritten + write(STDOUT_FD, &randomAlphanumeric, 1);
	}

	
	return 0;
}
