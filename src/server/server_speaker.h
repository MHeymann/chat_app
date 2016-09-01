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
} server_speaker_t;

server_speaker_t *new_server_speaker(users_t *users);

void add_packet_to_queue(server_speaker_t *speaker, packet_t *packet);

void push_user_list(server_speaker_t *speaker); 

void broadcast(server_speaker_t *speaker, packet_t *packet);

void *speaker_run(void *speaker);

#endif
