#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <pthread.h>

#include "client_listener.h"
#include "client_speaker.h"
#include "../queue/queue.h"

/*** Macros **************************************************************/

#define TRUE			1
#define FALSE			0
#define DEFAULT_PORT	8002

/*** Struct Definition ***************************************************/

typedef struct client {
	pthread_t *listen_thread;
	client_speaker_t *speaker;
	client_listener_t *listener;
	int connected_status;
	pthread_mutex_t *connection_mutex;
	char *username;
	char *hostname;
	int hostport;
} chat_client_t;

/*** Function Headers ****************************************************/

chat_client_t *new_client();
/*
chat_client_t *new_client(char *host, int port, char *name);
*/

void free_chat_client(chat_client_t *client);

void client_append(chat_client_t *client, char *s);

void client_show_online_users(chat_client_t *client, queue_t *users_q);

#endif
