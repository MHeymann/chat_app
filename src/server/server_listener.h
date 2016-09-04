#ifndef SERVER_LISTENER_H
#define SERVER_LISTENER_H

#include <pthread.h>
#include "server_speaker.h"
#include "users.h"

#define TRUE	1
#define FALSE	0

typedef struct listener {
	int *ports;
	int port_count;
	int run_status;
	pthread_mutex_t *status_lock;
	users_t *users;
	server_speaker_t *speaker;
} server_listener_t;

server_listener_t *new_server_listener(int *ports, int port_count, users_t *users, 
		server_speaker_t *speaker);

void server_listener_free(server_listener_t *listener);

void *listener_run(void *listener);

void listener_stop(server_listener_t *listener);

int listener_running(server_listener_t *listener);

#endif
