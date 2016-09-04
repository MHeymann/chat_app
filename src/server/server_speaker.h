#ifndef SERVER_SPEAKER_H
#define SERVER_SPEAKER_H

#include <semaphore.h>
#include "../queue/queue.h"
#include "../packet/packet.h"
#include "users.h"

#define TRUE	1
#define FALSE	0

typedef struct speaker {
	users_t *users;
	sem_t *queue_sem;
	pthread_mutex_t *queue_lock;
	queue_t *q;
	int run_status;
	pthread_mutex_t *status_lock;
} server_speaker_t;

server_speaker_t *new_server_speaker(users_t *users);

void server_speaker_free(server_speaker_t *speaker);

void add_packet_to_queue(server_speaker_t *speaker, packet_t *packet);

void push_user_list(server_speaker_t *speaker); 

void broadcast(server_speaker_t *speaker, packet_t *packet);

void *speaker_run(void *speaker);

void speaker_stop(server_speaker_t *speaker);

int speaker_running(server_speaker_t *speaker);

#endif
