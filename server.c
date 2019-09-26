#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/wait.h>
#include <signal.h>
#define MYPORT 1025 // the port users will be connecting to
#define BACKLOG 10 // how many pending connections queue will hold
#define MAXDATASIZE 512 // max number of bytes we can get at once

void sigchld_handler(int s)
{
	while(wait(NULL) > 0);
}
int main(void)
{
	int numbytes;
	char buf[MAXDATASIZE];
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct sockaddr_in my_addr; // my address information
	struct sockaddr_in their_addr; // connector.s address information
	int sin_size;
	struct sigaction sa;
	int yes=1;


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
	perror("socket");
	exit(1);
	}
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}
	my_addr.sin_family = AF_INET; // host byte order
	my_addr.sin_port = htons(MYPORT); // short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(my_addr.sin_zero), 0, 8); // zero the rest of the struct


	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}
	while(1)  // main accept() loop
	{
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr,&sin_size)) == -1)
		{
			//perror("accept");
			continue;
		}
		printf("Received request from Client: %s:%d\n",
		inet_ntoa(their_addr.sin_addr),MYPORT);
		if (!fork()) 
		{ // this is the child process
			close(sockfd); // child doesn.t need the listener

			//lear buffer just in case
			memset(buf,0,sizeof(buf));
			//read shit
			if ((numbytes=recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) 
			{
				perror("recv");
				exit(1);
			}
			printf("Received: %s\n",buf);

			//stream the file
			if (strcmp(buf,"send\r\n"))
			{
				FILE *fp;
				unsigned char bytes[512];

				fp = fopen("image", "rb");

				//get file size
				fseek(fp, 0, SEEK_END); // seek to end of file
				int size = ftell(fp);
				// send file size
				send(new_fd,size,sizeof(int),0);

				while (!feof(fp))
				{
					fread(bytes, 1,512,fp);
					send(new_fd, bytes, 512,0);
				}
				fclose(fp);

			}

			// sent shit
			if (send(new_fd, buf, MAXDATASIZE-1, 0) == -1)
			perror("send");
			close(new_fd);
			exit(0);
		}
	close(new_fd); // parent doesn.t need this
	}
	return 0;
}











