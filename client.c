/* Client code */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT_NUMBER 1025
#define SERVER_ADDRESS "192.168.20.252"
#define FILENAME "received_file.jpeg"

void receive_file_from_server(int client_socket)
{
	int file_size;
	FILE *received_file;
	int remain_data = 0;
	char buffer[BUFSIZ];
	ssize_t len;
	/* Receiving file size */
	recv(client_socket, buffer, BUFSIZ, 0);
	file_size = atoi(buffer);
	fprintf(stdout, "\nFile size : %d\n", file_size);

	received_file = fopen(FILENAME, "wb");
	if (received_file == NULL)
	{
		fprintf(stderr, "Failed to open file --> %s\n", strerror(errno));

		exit(EXIT_FAILURE);
	}

	remain_data = file_size;

	while ((remain_data > 0) && ((len = recv(client_socket, buffer, BUFSIZ, 0)) > 0))
	{
		fwrite(buffer, sizeof(char), len, received_file);
		remain_data -= len;
		fprintf(stdout, "Receive %ld bytes and we hope :- %d bytes\n", len, remain_data);
	}
	fclose(received_file);
}

int main(int argc, char **argv)
{
	int client_socket;
	struct sockaddr_in remote_addr;
	char command[255];
	

	/* Zeroing remote_addr struct */
	memset(&remote_addr, 0, sizeof(remote_addr));

	/* Construct remote_addr struct */
	remote_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_ADDRESS, &(remote_addr.sin_addr));
	remote_addr.sin_port = htons(PORT_NUMBER);

	/* Create client socket */
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == -1)
	{
		fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));

		exit(EXIT_FAILURE);
	}

	/* Connect to the server */
	if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
	{
		fprintf(stderr, "Error on connect --> %s\n", strerror(errno));

		exit(EXIT_FAILURE);
	}
	printf("Connected.\nWrite a command: ");
	scanf("%s",command);

	send(client_socket, command,255,0);
	if (!strcmp(command,"send"))
	{
		receive_file_from_server(client_socket);

		//recv(client_socket,command,255,0);

	}else
	{
		recv(client_socket,command,255,0);
		printf("Received: %s\n",command);
	}

	close(client_socket);

	return 0;
}
