#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <glib.h>
#include <glib-object.h>
#include <axsdk/axevent.h>
#include <syslog.h>
#include <stdio.h>
#include <time.h>

#define PORT_NUMBER 1025
#define SERVER_ADDRESS "192.168.20.245"

static void
subscription_callback(guint subscription, AXEvent *event, guint client_socket);

static guint
subscribe_to_motion_detection_event(AXEventHandler *event_handler, guint client_socket);

static void
subscription_callback(guint subscription, AXEvent *event, guint client_socket)
{
  char string[255];
  const AXEventKeyValueSet *key_value_set;
  gboolean active;
  time_t cur_time;
  struct tm *time_info;
  openlog("axis_client:Motion Callback", LOG_PID|LOG_CONS, LOG_USER);

  (void)subscription;

  key_value_set = ax_event_get_key_value_set(event);

  ax_event_key_value_set_get_boolean(key_value_set, "active", NULL, &active, NULL);

  time(&cur_time); 
  time_info = localtime (&cur_time);

  if (active) {
    syslog(LOG_INFO, "Motion detected");
    g_message("Motion detected: %s", asctime(time_info));
    //send that motion is detected  
    sprintf(string, "Motion detected: %s",asctime(time_info));
    send(client_socket, string,255,0);
    recv(client_socket,string,255,0);
  } else {
    syslog(LOG_INFO, "Motion not detected");
    g_message("Motion not detected: %s", asctime(time_info));
    //send that motion is not detected
    sprintf(string, "Motion not detected: %s",asctime(time_info));
    send(client_socket, string,255,0);
    recv(client_socket,string,255,0);
  }
  memset(&string, 0, sizeof string);
  closelog();

}

static guint
subscribe_to_motion_detection_event(AXEventHandler *event_handler, guint client_socket)
{
  AXEventKeyValueSet *key_value_set;
  guint subscription;

  key_value_set = ax_event_key_value_set_new();

  ax_event_key_value_set_add_key_values(key_value_set, NULL, 
  "topic0", "tns1", "RuleEngine", AX_VALUE_TYPE_STRING,
  "topic1", "tnsaxis", "VMD3", AX_VALUE_TYPE_STRING,	
  "active", NULL, NULL, AX_VALUE_TYPE_BOOL, NULL);

  ax_event_handler_subscribe(event_handler, key_value_set, &subscription, (AXSubscriptionCallback)subscription_callback, client_socket, NULL);

  ax_event_key_value_set_free(key_value_set);
  return subscription;
}

int main(void)
{
  guint client_socket;
  struct sockaddr_in remote_addr;
  GMainLoop *main_loop;
  AXEventHandler *event_handler;
  guint subscription;
  
  main_loop = g_main_loop_new(NULL, FALSE);

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
		syslog(LOG_INFO, "Error creating socket");

		exit(EXIT_FAILURE);
	}

	/* Connect to the server */
	if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
	{
		syslog(LOG_INFO, "Error on connect");

		exit(EXIT_FAILURE);
	}


  event_handler = ax_event_handler_new();
  openlog("axis_client", LOG_PID|LOG_CONS, LOG_USER);

  subscription = subscribe_to_motion_detection_event(event_handler, client_socket); 

  syslog(LOG_INFO, "Subscription completed. Entering the main loop...");

  g_main_loop_run(main_loop);

  char string[255];
  sprintf(string, "Disconnecting...");
  send(client_socket, string,255,0);
  
  ax_event_handler_unsubscribe(event_handler, subscription, NULL);

  ax_event_handler_free(event_handler);
  closelog();
  close(client_socket);

  return 0;
}
