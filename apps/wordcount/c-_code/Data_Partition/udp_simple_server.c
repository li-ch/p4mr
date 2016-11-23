/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


#define P4_UDP_PORT 30001
#define P4_SERVER_IP "127.0.0.1"
#define BUFF_MAX_SIZE 256

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd;
     socklen_t clilen;
     char buffer[BUFF_MAX_SIZE];
     struct sockaddr_in serv_addr, cli_addr;
     int num_bytes;

     sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
     if (sockfd < 0) 
        error("ERROR opening socket");

     memset((char *) &serv_addr,0, sizeof(serv_addr));
     
     serv_addr.sin_family = AF_INET;
     inet_pton(AF_INET, P4_SERVER_IP, &(serv_addr.sin_addr)); 
     serv_addr.sin_port = htons(P4_UDP_PORT);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     //listen(sockfd,5);


     clilen = sizeof(cli_addr);

     while(1) // just keep reading and do nothing else
     {
       bzero(buffer,256);
      
       if((num_bytes = recvfrom(sockfd, buffer, BUFF_MAX_SIZE-1, 0, 
         (struct sockaddr *) &cli_addr, &clilen)) == -1)
       {
         perror("recevfrom");
         exit(-1);
       }
       printf("P4_server has received a packet from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
       printf("Data: %s\n", buffer);

     }
     
     close(sockfd);
     return 0; 
}
