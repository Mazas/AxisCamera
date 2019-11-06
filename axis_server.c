/* Server code */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include <syslog.h>
//Axis library
//#include <capture.h>

#define PORT_NUMBER 1025
#define SERVER_ADDRESS "127.0.0.1"
#define MAX_THREADS 5

void *connection_handler(void *);

void *send_image(int sock){
	FILE *fs;
	fs = fopen("image.jpeg","rb");
   	if(fs == NULL)
   	{
    	printf("Error!");   
    	exit(1);             
   	}
	unsigned long long totalbytes = 0;
	printf("File opened.\n");
    
    fseek(fs, 0L, SEEK_END);
    totalbytes = ftell(fs);
    rewind(fs);
	
	char dataBuffer[totalbytes];
    memset(dataBuffer,0,sizeof dataBuffer);
	fread(dataBuffer,1,totalbytes,fs);
	printf("File copied to buffer.\n");
    puts(dataBuffer);

	char size [128];
	sprintf(size, "%llu\n",totalbytes);
    printf("Sending file size %s",size );
    char buff[128];
    memset(buff,0,128);
	send(sock , size , strlen(size), 0);
    read(sock,buff, 128);
    puts(buff);

	// sending data
	printf("Started sending\n");
	send(sock, dataBuffer, totalbytes, 0);
    memset(buff,0,128);
    read(sock,buff,128);

	printf("File sent.\n");
   	fclose(fs);
}
// void *take_image(struct arg_struct arguments)
// {
// 	media_frame *frame;
// 	media_stream *stream;
// 	void *data;
// 	unsigned long long totalbytes = 0;
// 	char sizeString[256];
// 	//char resolution = "resolution=352x288&fps=10";
// 	stream = capture_open_stream(IMAGE_JPEG, arguments.buffer);
// 	int sent_bytes;

// 	frame = capture_get_frame(stream);

// 	totalbytes = capture_frame_size(frame);
// 	data = capture_frame_data(frame);

// 	sprintf(sizeString, "%llu\n", totalbytes);
// 	syslog(LOG_INFO, "Total bytes: %s", sizeString);

// 	char dataBuffer[totalbytes];
// 	memcpy(dataBuffer, data, totalbytes);

// 	// sending data

// 	sent_bytes = sendto(&arguments.server_socket, dataBuffer, totalbytes, 0, (struct sockaddr_in *)&arguments.client_socket, arguments.client_socket_lenght);
// 	if (sent_bytes < 0)
// 	{
// 		syslog(LOG_INFO, "ERROR in send_image");
// 	}
// 	capture_frame_free(frame);
// 	memset(&sizeString, 0, 256);
// 	memset(&dataBuffer, 0, totalbytes);
// 	capture_close_stream(stream);
// }

int main(int argc, char **argv)
{
  int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT_NUMBER );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
        close(client_sock);
     
    return 0;
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
		if (strstr(client_message, "resolution="))
		{
			puts(client_message);
			send_image(sock);
		}
		//clear the message buffer
		memset(client_message, 0, 2000);
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
	return 0;
}
