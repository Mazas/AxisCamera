/* Server code */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <syslog.h>

//Axis library
#include <capture.h>

#define PORT_NUMBER 1025
#define SERVER_ADDRESS "192.168.20.252"

void take_image(int peer_socket, struct sockaddr_in clientaddr, int clientlen, char **resolution)
{
	media_frame *frame;
	media_stream *stream;
	void *data;
	unsigned long long totalbytes = 0;
	char sizeString[256];
	//char resolution = "resolution=352x288&fps=10";
	stream = capture_open_stream(IMAGE_JPEG, resolution);
	int sent_bytes;

	frame = capture_get_frame(stream);

	totalbytes = capture_frame_size(frame);
	data = capture_frame_data(frame);

	sprintf(sizeString, "%llu\n", totalbytes);
	syslog(LOG_INFO, "Total bytes: %s", sizeString);

	char dataBuffer[totalbytes];
	memcpy(dataBuffer, data, totalbytes);

	// sending data

	sent_bytes = sendto(peer_socket, dataBuffer, totalbytes, 0, (struct sockaddr *)&clientaddr, clientlen);
	if (sent_bytes < 0)
	{
		syslog(LOG_INFO, "ERROR in send_image");
	}
	capture_frame_free(frame);
	memset(&sizeString, 0, 256);
	memset(&dataBuffer, 0, totalbytes);
	capture_close_stream(stream);
}

int main(int argc, char **argv)
{
	int sockfd;					   /* socket */
	int clientlen;				   /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	char buf[BUFSIZ];			   /* message buf */
	char *hostaddrp;			   /* dotted decimal host addr string */
	int optval;					   /* flag value for setsockopt */
	int n;						   /* message byte size */
	/* 
   * socket: create the parent socket 
   */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		syslog(LOG_INFO, "ERROR opening socket");
	}
	/* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

	/*
   * build the server's Internet address
   */
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)PORT_NUMBER);

	/* 
   * bind: associate the parent socket with a port 
   */
	if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		syslog(LOG_INFO, "ERROR on binding");
	}
	/* 
   * main loop: wait for a datagram, then echo it
   */
	clientlen = sizeof(clientaddr);
	while (1)
	{

		/*
     * recvfrom: receive a UDP datagram from a client
     */
		memset(&buf, 0, sizeof buf);
		n = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *)&clientaddr, &clientlen);
		if (n < 0)
		{
			syslog(LOG_INFO, "ERROR in recvfrom");
		}
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL)
		{
			syslog(LOG_INFO, "ERROR on inet_ntoa\n");
		}
		if (strstr(buf, "resolution="))
		{
			syslog(LOG_INFO, "%s", buf);
			take_image(sockfd, clientaddr, clientlen, buf);
		}
	}
	return 0;
}
