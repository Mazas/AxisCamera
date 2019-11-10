/* Server code */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include <math.h>
//Axis library
#include <capture.h>

#define PORT_NUMBER 1025
#define SERVER_ADDRESS "127.0.0.1"
#define MAX_THREADS 5

void *connection_handler(void *);

void *send_image(int sock, signed char xor_key)
{
    char buff[256];
    FILE *fs;
    fs = fopen("image.jpeg", "rb");
    if (fs == NULL)
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
    memset(dataBuffer, 0, sizeof dataBuffer);
    fread(dataBuffer, 1, totalbytes, fs);
    printf("File copied to buffer.\n");
    puts(dataBuffer);

    // send key
    char key[16];
    sprintf(key, "%d\n", xor_key);
    printf("Sending key %s", key);
    memset(buff, 0, 256);
    send(sock, key, strlen(key), 0);
    read(sock, buff, 256);
    puts(buff);

    for (int i = 0; i < totalbytes; i++)
    {
        dataBuffer[i] = dataBuffer[i] ^ xor_key;
    }

    char size[256];
    sprintf(size, "%llu\n", totalbytes);
    printf("Sending file size %s", size);
    memset(buff, 0, 256);
    send(sock, size, strlen(size), 0);
    read(sock, buff, 256);
    puts(buff);

    // sending data
    printf("Started sending\n");
    send(sock, dataBuffer, totalbytes, 0);
    memset(buff, 0, 256);
    read(sock, buff, 256);

    printf("File sent.\n");

    fclose(fs);
}
void *take_image(int sock, char request[], signed char xor_key)
{
    media_frame *frame;
    media_stream *stream;
    char buff[256];
    void *data;
    unsigned long long totalbytes = 0;
    //char resolution = "resolution=352x288&fps=10";
    stream = capture_open_stream(IMAGE_JPEG, request);
    openlog("Axis_Server", LOG_PID, LOG_USER);

    frame = capture_get_frame(stream);

    totalbytes = capture_frame_size(frame);
    data = capture_frame_data(frame);

    char dataBuffer[totalbytes];
    memset(dataBuffer, 0, sizeof dataBuffer);
    memcpy(dataBuffer, data, totalbytes);
    syslog(LOG_INFO, "File copied to buffer.\n");
    puts(dataBuffer);

    char size[256];
    sprintf(size, "%llu\n", totalbytes);
    syslog(LOG_INFO, "Sending file size %s", size);

    memset(buff, 0, 256);
    send(sock, size, strlen(size), 0);
    read(sock, buff, 256);
    syslog(LOG_INFO, buff);

    // encrypt using xor
    for (int i = 0; i < totalbytes; i++)
    {
        dataBuffer[i] = dataBuffer[i] ^ xor_key;
    }

    // sending data
    syslog(LOG_INFO, "Started sending\n");
    send(sock, dataBuffer, totalbytes, 0);
    memset(buff, 0, 256);
    read(sock, buff, 256);

    capture_frame_free(frame);
    memset(size, 0, 256);
    memset(dataBuffer, 0, totalbytes);
    capture_close_stream(stream);
    closelog();
}

void exchange_keys(int sock, signed char xor_key, char message[])
{
    char buff[256];
    char pex[256];
    char pmod[256];
	long long pexd;
	long long pmodd;
    memset(pex,0,256);
    memset(pmod,0,256);

    sprintf(pex, "%s",message);
	pexd = atoll(pex);
    sprintf(buff, "%lli\n",pexd);

    send(sock, buff, strlen(buff), 0);
    read(sock, pmod, 256);
	pmodd = atoll(pmod);

	// here is something wrong

    double e = pow(double(xor_key),double(pexd));
    double encrypted_xor = fmod(e, double(pmodd));

    // send xor key
    char key[256];
    sprintf(key, "%lf\n", encrypted_xor);
    syslog(LOG_INFO,"Sending key %s", key);
    memset(buff, 0, 256);
    send(sock, key, strlen(key), 0);
    read(sock, buff, 256);
    puts(buff);
}

int main(int argc, char **argv)
{
	openlog("Axis_Server", LOG_PID, LOG_USER);
    int socket_desc, client_sock, c;
    struct sockaddr_in server, client;

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NUMBER);

    //Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    pthread_t thread_id;

    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        puts("Connection accepted");

        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join(thread_id, NULL);
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
    int sock = *(int *)socket_desc;
    int read_size;
    char *message, client_message[2000];
    signed char xor_key = rand() % 256;

    //Receive a message from client
    while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
    {
        if (strstr(client_message, "resolution="))
        {
            puts(client_message);
            take_image(sock, client_message, xor_key);
            //send_image(sock, xor_key);
        }
        else
        {
            exchange_keys(sock, xor_key, client_message);
        }

        //clear the message buffer
        memset(client_message, 0, 2000);
    }

    if (read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("recv failed");
    }
    return 0;
}
