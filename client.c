
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 1025 /*This is the port for the client server connection*/
#define MAXDATASIZE 512 // max number of bytes we can get at once

int main(int argc, char *argv[])
{
	int sockfd;
	char buf[MAXDATASIZE];
	struct sockaddr_in server; // connector.s address information
	struct hostent *hostIP; //placeholder for the IP address

	if (argc != 2)
	{
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}
	
	if ((hostIP=gethostbyname(argv[1])) == NULL) 
	{ // get the host info
		perror("gethostbyname");
		exit(1);
	}
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		exit(1);
	}
	server.sin_family = AF_INET; // host byte order
	server.sin_port = htons(PORT); // short, network byte order
	server.sin_addr = *((struct in_addr *)hostIP->h_addr);
	memset(&(server.sin_zero), 0, 8);
	puts("socket created");

	if (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
		perror("connect");
		exit(1);
	}
	puts("connected");

	//send shit
	printf("Write something: ");
	scanf("%s",buf);
	if (send(sockfd, buf, MAXDATASIZE-1, 0) == -1)
			perror("send");
	if (strcmp(buf,"send\r\n"))
	{
		// get file size
		puts("receiving file");
		int size;
		unsigned char bytes[512];
		recv(sockfd,size,sizeof(int),0);
		FILE *fp = fopen("received_file","wb");
		while (size>0)
		{
			recv(sockfd, bytes, 512, 0);
			printf("%d\t%s\n",size,bytes);
			fwrite(bytes, 1,512,fp);
			memset(bytes,0,sizeof(bytes));
			size=size-512;
		}

		
	}
	

	//clear buffer
	memset(buf, 0, sizeof(buf));
	// read shit back
	if ((recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
	{
		perror("recv");
		exit(1);
	}
	
	//printf("\n\nLocalhost: %s\n", inet_ntoa(*(struct in_addr *)hostIP->h_addr));
	printf("Local Port: %d\n", PORT);
	//printf("Remote Host: %s\n", inet_ntoa(their_addr.sin_addr));
	printf("Received data: %s\n",buf);
	close(sockfd);
	return 0;
}

