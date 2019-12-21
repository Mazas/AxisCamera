#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

#define PORT_NUMBER 1025
#define SERVER_ADDRESS "127.0.0.1"

void handleConnection(int socket)
{
	char string[13];
	time_t seconds;

	while (1)
	{
		seconds = time(NULL)*1000;
		memset(string, 0, sizeof(string));
		sprintf(string, "%ld\n", seconds);
		printf("%s",string);
		send(socket, string, 13, 0);
		recv(socket, string, 13, 0);
		sleep(rand() % 10);
	}
}

int main(void)
{
	int client_socket;
	struct sockaddr_in remote_addr;

	/* Zeroing remote_addr struct */
	memset(&remote_addr, 0, sizeof(remote_addr));

	puts("Constructing socket");
	/* Construct remote_addr struct */
	remote_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_ADDRESS, &(remote_addr.sin_addr));
	remote_addr.sin_port = htons(PORT_NUMBER);

	/* Create client socket */
	puts("Creating client socket");
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == -1)
	{
		printf("Error creating socket\n");

		exit(EXIT_FAILURE);
	}

	/* Connect to the server */
	puts("Connecting...");
	if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
	{
		printf("Error on connect\n");

		exit(EXIT_FAILURE);
	}

	char string[255];

	handleConnection(client_socket);
	sprintf(string, "Disconnecting...\n");
	send(client_socket, string, 255, 0);
	close(client_socket);

	return 0;
}
