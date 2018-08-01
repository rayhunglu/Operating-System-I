#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

#define BUFFERSIZE    80000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
    int socketFD=0, charsWritten, charsRead;
    struct sockaddr_in serverinfo;
    struct hostent* serverHostInfo;
    char keybuffer[BUFFERSIZE],readbuffer[BUFFERSIZE],sendbuffer[BUFFERSIZE];
    int portNumber = atoi(argv[3]); //Get the port number
    if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); }
    //get file
    int fp,i;
    fp=open(argv[1],O_RDONLY);
    if(fp <0) {
        perror("Error opening file");
        exit(1);
    }
    int readlong=read(fp, readbuffer, BUFFERSIZE);
    close(fp);
    
    //get key
    fp=open(argv[2],O_RDONLY);
    if(fp <0) {
        perror("Error opening file");
        return(-1);
    }
    int keylong=read(fp, keybuffer, BUFFERSIZE);
    
    close(fp);
    
    if (keylong < readlong)
    {
        printf("Error: key '%s' is too short\n", argv[2]);
        exit(1);
    }
    
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create
    if (socketFD < 0) error("CLIENT: ERROR opening socket");

    memset(&serverinfo, '\0', sizeof(serverinfo)); // Clear
    serverHostInfo = gethostbyname("localhost");
    serverinfo.sin_family = AF_INET;
    serverinfo.sin_port = htons(portNumber);
    bcopy((char *)serverHostInfo->h_addr, (char *)&serverinfo.sin_addr.s_addr, serverHostInfo->h_length);

//    serverHostInfo = gethostbyname(argv[1]);
//
//    if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
//    memcpy((char*)&serverinfo.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
    

    if (connect(socketFD, (struct sockaddr*)&serverinfo, sizeof(serverinfo)) < 0) // Connect
        error("CLIENT: ERROR connecting");
    
    //start
//    memset(readbuffer, '\0', BUFFERSIZE); // Clear out the buffer array

    // Send message to server
    charsWritten = send(socketFD, readbuffer, readlong, 0);
    if (charsWritten < 0) error("CLIENT: ERROR writing file to socket");
    if (charsWritten < readlong) printf("CLIENT: WARNING: Not all data written to socket!\n");
    
    // Get return message from server
    memset(readbuffer,0, BUFFERSIZE); // Clear
    charsRead = recv(socketFD, readbuffer, BUFFERSIZE, 0);
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    
    // Send key to server
    charsWritten = send(socketFD, keybuffer, keylong, 0);
    if (charsWritten < 0) error("CLIENT: ERROR writing key to socket");
    if (charsWritten < keylong) printf("CLIENT: WARNING: Not all data written to socket!\n");
    //get enc_file
    memset(sendbuffer,0, BUFFERSIZE); // Clear
    charsRead = recv(socketFD, sendbuffer, BUFFERSIZE, 0);
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    
    //output enc file
    for ( i = 0; i < charsRead-1; i++)
    {
        if(sendbuffer[i]=='@'){
        printf(" ");
        }else{
        printf("%c", sendbuffer[i]);}
    }
    printf("\n");
    
    
    close(socketFD); // Close the socket
    return 0;
}
