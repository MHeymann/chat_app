#ifndef CLIENT_LISTENER_H
#define CLIENT_LISTENER_H

typedef struct client_listener {
	int sd;
	void *chat_client;
	char *name;
	int running;
	pthread_mutex_t *listen_mutex;
} client_listener_t;

client_listener_t *new_client_listener(int sd, void *client, char *name);

void free_client_listener(client_listener_t *listener);

void *run_client_listener(void *client);

void stop_listener(client_listener_t *listener);

#endif
