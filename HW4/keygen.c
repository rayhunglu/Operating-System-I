#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>

#define LSH_TOK_BUFSIZE 512
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
    srand(time(0));
    char randLetter;
    int i,j;
    int num = atoi(argv[1]);
//    int num=argv[1];
    int keybuffer[num+1];
    for( i=0;i<num;i++){
//        keybuffer[i]=(rand()%26);
        j=rand()%26+65;
        randLetter=(char)j;
        //65+%26
        printf("%c", randLetter);
    }
    printf("\n");
    return 0;
}
