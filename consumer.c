#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h> 

#define STDIN_FD 0

int M = 0;

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

    int bytesRead;
    char input;


    for(bytesRead = 0; bytesRead < M; )
    {
        bytesRead = bytesRead + read(STDIN_FD, &input , 1);
    }


    return 0;
}
