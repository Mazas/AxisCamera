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
#define SERVER_ADDRESS "192.168.20.249"

void take_image(int peer_socket)
{
	media_frame *frame;
	media_stream *stream;
	void *data;
	unsigned long long totalbytes = 0;
	char sizeString[256];

	int remainingData=0;
	int sent_bytes;
	
	stream = capture_open_stream(IMAGE_JPEG, "resolution=352x288&fps=10");
	frame = capture_get_frame(stream);

	syslog(LOG_INFO,"Frame captured");
	
	totalbytes = capture_frame_size(frame);
	data = capture_frame_data(frame);

	sprintf(sizeString,"%llu\n",totalbytes);
	syslog(LOG_INFO,"Total bytes: %s",sizeString);
	
	
	char dataBuffer[totalbytes];
	memcpy(dataBuffer, data, totalbytes);

	send(peer_socket, sizeString , sizeof(sizeString),0);
		// sending data
	sent_bytes = send(peer_socket,dataBuffer,totalbytes,0);
	syslog(LOG_INFO,"Send %d and remaining: %d Total size: %llu",sent_bytes, remainingData,totalbytes);

	//send(peer_socket, data, sizeof data,0);

	capture_frame_free(frame);
	capture_close_stream(stream);

}
void start_stream(int peer_socket)
{
	media_frame *frame;
	media_stream *stream;
	void *data;
	unsigned long long totalbytes = 0;
	char sizeString[256];
	char response[32];
	int remainingData=0;
	int sent_bytes;

	stream = capture_open_stream(IMAGE_JPEG, "resolution=352x288&fps=10");
	syslog(LOG_INFO,"Stream started");
	while (1)
	{	
		totalbytes = 0;
		frame = capture_get_frame(stream);
		syslog(LOG_INFO,"Frame captured");
	
		totalbytes += capture_frame_size(frame);
		data = capture_frame_data(frame);
	
		sprintf(sizeString,"%llu\n",totalbytes);
		syslog(LOG_INFO,"Total bytes: %s",sizeString);
		
		
//	char dataBuffer[totalbytes];
//	sprintf(dataBuffer,"%d",data);

		send(peer_socket, sizeString , sizeof(sizeString),0);
		// sending data
		sent_bytes = send(peer_socket,data,totalbytes,0);
		syslog(LOG_INFO,"Send %d and remaining: %d Total size: %llu",sent_bytes, remainingData,totalbytes);
		
		//if(recv(peer_socket, command,BUFSIZ,0)>0)

	//send(peer_socket, data, sizeof data,0);

		capture_frame_free(frame);
		if ((recv(peer_socket, response,32,0)>0)&&strcmp(response,"OK"))
		{
			syslog(LOG_INFO,"Client responded: %s",response);
			break;
		}
	}
	capture_close_stream(stream);
}




int main(int argc, char **argv)
{
	int server_socket;
	int peer_socket;
	socklen_t sock_len;
	struct sockaddr_in server_addr;
	struct sockaddr_in peer_addr;
	char command[BUFSIZ];

	/* Create server socket */
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
	{
		fprintf(stderr, "Error creating socket --> %s", strerror(errno));

		exit(EXIT_FAILURE);
	}

	/* Zeroing server_addr struct */
	memset(&server_addr, 0, sizeof(server_addr));
	/* Construct server_addr struct */
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_ADDRESS, &(server_addr.sin_addr));
	server_addr.sin_port = htons(PORT_NUMBER);

	/* Bind */
	if ((bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1)
	{
		fprintf(stderr, "Error on bind --> %s", strerror(errno));

		exit(EXIT_FAILURE);
	}

	/* Listening to incoming connections */
	if ((listen(server_socket, 5)) == -1)
	{
		fprintf(stderr, "Error on listen --> %s", strerror(errno));

		exit(EXIT_FAILURE);
	}

	sock_len = sizeof(struct sockaddr_in);
	/* Accepting incoming peers */
	peer_socket = accept(server_socket, (struct sockaddr *)&peer_addr, &sock_len);
	if (peer_socket == -1)
	{
		fprintf(stderr, "Error on accept --> %s", strerror(errno));

		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "Accept peer --> %s\n", inet_ntoa(peer_addr.sin_addr));
	
	while(1)
	{
		memset(&command, 0, sizeof command);
		if(recv(peer_socket, command,BUFSIZ,0)>0){
		printf("Received: %s",command);
		if (!strcmp(command,"send"))
		{
			openlog("Logs", LOG_PID, LOG_USER);
			syslog(LOG_INFO,"Send compare succesfull");
			take_image(peer_socket);
			syslog(LOG_INFO,"Image taken");
		}else if(!strcmp(command,"stream"))
		{
			openlog("Logs", LOG_PID, LOG_USER);
			syslog(LOG_INFO,"stream compare succesfull");
			start_stream(peer_socket);
			syslog(LOG_INFO,"Stream ended");
		}else
		{
			syslog(LOG_INFO,"Send compare unsuccesfull. Message received: %s", command);
			send(peer_socket,command,255,0);	
		}
		}
	}
	closelog();

	close(peer_socket);
	close(server_socket);

	return 0;
}
