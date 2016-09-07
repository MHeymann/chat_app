#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h> /* write */
#include <errno.h>
#include <pthread.h>


#include "client_listener.h"
#include "client_speaker.h"
#include "chat_client.h"
#include "../packet/packet.h"
#include "../packet/code.h"

#define TRUE		1
#define FALSE		0

/*
typedef struct client_listener {
	int sd;
	void *chat_client;
	char *name;
	int running;
	pthread_mutex_t *listen_mutex;
} client_listener_t;
*/

/*** Helper Function Prototypes ******************************************/

void listener_go(client_listener_t *listener);
char *listen_strdup(char *s);
int listener_is_running(client_listener_t *listener);

/*** Functions ***********************************************************/

client_listener_t *new_client_listener(int sd, void *client, char *name)
{
	client_listener_t *listener = NULL;

	listener = malloc(sizeof(client_listener_t));
	if (!listener) {
		fprintf(stderr, "memory error\n");
	} else {
		listener->sd = sd;
		listener->chat_client = client;
		listener->name = listen_strdup(name);
		listener->running = TRUE;
		listener->listen_mutex = malloc(sizeof(client_listener_t));
		if (listener->listen_mutex) {
			pthread_mutex_init(listener->listen_mutex, NULL);
		} else {
			free_client_listener(listener);
			listener = NULL;
		}
	}

	return listener;
}

void free_client_listener(client_listener_t *listener)
{
	if (!listener) {
		return;
	}
	if (listener->sd) {
		/*
		close(listener->sd);
		*/
		listener->sd = 0;
	} 
	listener->chat_client = NULL;
	if (listener->name) {
		free(listener->name);
		listener->name = NULL;
	}
	listener->running = TRUE;
	if (listener->listen_mutex) {
		pthread_mutex_destroy(listener->listen_mutex);
		free(listener->listen_mutex);
		listener->listen_mutex = NULL;
	}
	free(listener);
}

void *run_client_listener(void *listener)
{
	if (!listener) {
		printf("Invalid pointer provided for running listener\n");
	} else {
		listener_go((client_listener_t *)listener);
	}
	return NULL;

}

void stop_listener(client_listener_t *listener)
{
	pthread_mutex_lock(listener->listen_mutex);
	listener->running = FALSE;
	pthread_mutex_unlock(listener->listen_mutex);
}

/*** Helper Functions ****************************************************/

void listener_go(client_listener_t *listener)
{
	int activity;
	fd_set readfds;
	struct timeval tv;
	packet_t *packet = NULL;
	char s[1024];
	printf("client listener starting up\n");

	while (listener_is_running(listener)) {
		FD_ZERO(&readfds);	

		FD_SET(listener->sd, &readfds);

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		activity = select(listener->sd + 1, &readfds, NULL, NULL, &tv);

		if ((activity < 0) && (errno != EINTR)) {
			printf("Select error\n");
		}
		if (activity == 0) {
			continue;
		}
		
		if (!FD_ISSET(listener->sd, &readfds)) {
			printf("This is very weird, only one sd was set\n");
			printf("but now after select it is not marked as set\n");
			continue;
		}

		packet = NULL;
		packet = receive_packet(listener->sd);
		if (!packet) {
			printf("server went offline\n");
			break;
		} else if(packet->code == SEND) {
			sprintf(s, "%s: %s\n", packet->name, packet->data);
			client_append((chat_client_t *)listener->chat_client, s);
		} else if(packet->code == ECHO) {
			sprintf(s, "YOU echoed: %s\n", packet->data);
			client_append((chat_client_t *)listener->chat_client, s);
		} else if(packet->code == BROADCAST) {
			if (strcmp(listener->name, packet->name) == 0) {
				sprintf(s, "YOU Broadcast: %s\n", packet->data);
			} else {
				sprintf(s, "%s Broadcast: %s\n", packet->name, packet->data);
			}
			client_append((chat_client_t *)listener->chat_client, s);
		} else if(packet->code == GET_ULIST) {
			printf("showing users\n");
			client_show_online_users((chat_client_t *)listener->chat_client, packet->users);
		} else {
			printf("The server did something unorthodox\n");
		}
		if (packet) {
			free_packet(packet);
			packet = NULL;
		}
	}
}

char *listen_strdup(char *s)
{
	char *c = malloc(strlen(s) + 1);
	int i = 0, j = 0;

	while ((c[i++] = s[j++]));

	return c;
}

int listener_is_running(client_listener_t *listener) 
{
	int ret_val;

	pthread_mutex_lock(listener->listen_mutex);
	ret_val = listener->running;
	pthread_mutex_unlock(listener->listen_mutex);
	
	return ret_val;
}
