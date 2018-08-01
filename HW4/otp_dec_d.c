#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define BUFFERSIZE    80000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: otp_dec_d port\n");
        exit(1);
    }// Check usage & args
    int sockfd=0, findclientsocfd, portNumber, charsread,charskey,sendcheck;
    char keybuffer[BUFFERSIZE],readbuffer[BUFFERSIZE],sendbuffer[BUFFERSIZE];
    struct sockaddr_in serverinfo, clientinfo;
    socklen_t clientlen = sizeof(clientinfo);
    int pid,i;
    int readint,keyint,sendint;
    // server information
    portNumber = atoi(argv[1]);
    memset((char *)&serverinfo, '\0', sizeof(serverinfo)); // Clear
    serverinfo.sin_family = AF_INET;
    serverinfo.sin_port = htons(portNumber);
    serverinfo.sin_addr.s_addr = INADDR_ANY;
    
    // Set up the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create
    if (sockfd < 0) error("ERROR opening socket");
    
    // Enable the socket to begin listening
    if (bind(sockfd, (struct sockaddr *)&serverinfo, sizeof(serverinfo)) < 0) //socket to port
    error("ERROR on binding");
    listen(sockfd, 5);
    
    while(1){
        findclientsocfd = accept(sockfd, (struct sockaddr *)&clientinfo,    &clientlen); // Accept
        if (findclientsocfd < 0)
        {
            printf("Error: otp_dec_d unable to accept connection\n");
            continue;
        }
        pid=fork();
        if (pid < 0)
        {
            perror("otp_dec_c: error on fork\n");
        }
        // Get the message from the client and display it
        if(pid==0){
            memset(readbuffer, '\0', BUFFERSIZE);
            charsread = read(findclientsocfd, readbuffer, BUFFERSIZE);
            if (charsread < 0) error("ERROR reading txt from socket");
            sendcheck=send(findclientsocfd, "de", 2, 0);
            if (sendcheck < 0) error("otp_enc_d error to client");
            
            memset(keybuffer, 0, BUFFERSIZE);
            charskey= read(findclientsocfd, keybuffer, BUFFERSIZE);
            if (charsread < 0) error("ERROR reading key from socket");
            charsread-=1;
            //start
//            if(charsread>charskey){
//                printf("otp_dec_d error: key is too short\n");
//                exit(1);
//            }
            for( i=0;i<charsread;i++){
                //                printf("%c",readbuffer[i]);
                //                if((int)readbuffer[i]>90||((int) readbuffer[i] < 65 && (int) readbuffer[i] != 32)){
                //                    printf("otp_dec_d error: key contains bad characters\n");
                //                    exit(1);
                //
                //                }else
                if(readbuffer[i]==' '){
                    sendbuffer[i]=' ';
                }else{
                    readint=(int)readbuffer[i]-65;
                    keyint=(int)keybuffer[i]-65;
                    sendint=readint-keyint;
                    if(sendint<0) sendint+=26;
                    sendbuffer[i]=(char)(sendint+65);
                }
                
            }
            
            sendcheck=send(findclientsocfd,sendbuffer, charsread, 0);
            if (sendcheck < charsread) error("otp_enc_d error to client");
            
            close(findclientsocfd);
            close(sockfd);
            exit(0);
        }
        else close(findclientsocfd);

    }
    return 0;
}
