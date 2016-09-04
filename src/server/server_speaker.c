#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../packet/code.h"
#include "server_speaker.h"
/*
typedef struct speaker {
	users_t *users;
	sem_t *queue_sem;
	pthread_mutex_t *queue_lock;
	queue_t *q;
} server_speaker_t;
*/

/*** Helper Function Prototypes ******************************************/

int cmp_dummy(void *a, void *b);
char *speak_strdup(char *s);
void speaker_go(server_speaker_t *speaker);

/*** Functions ***********************************************************/

server_speaker_t *new_server_speaker(users_t *users)
{
	server_speaker_t *speaker = NULL;
	queue_t *q = NULL;

	speaker = malloc(sizeof(server_speaker_t));
	if (!speaker) {
		fprintf(stderr, "failed to malloc server_speaker\n");
		return NULL;
	}
	speaker->users = users;
	speaker->queue_sem = malloc(sizeof(sem_t));
	if (!speaker->queue_sem) {
		fprintf(stderr, "failed to malloc queue sem for server_speaker\n");
		free(speaker);
		return NULL;
	}
	sem_init(speaker->queue_sem, 0, 0);

	speaker->queue_lock = malloc(sizeof(pthread_mutex_t));
	if (!speaker->queue_lock) {
		fprintf(stderr, "failed to malloc queue lock for server_speaker\n");
		sem_destroy(speaker->queue_sem);
		free(speaker->queue_sem);
		free(speaker);
		return NULL;
	}
	pthread_mutex_init(speaker->queue_lock, NULL);

	init_queue(&q, cmp_dummy, free_packet);
	speaker->q = q;

	speaker->run_status = TRUE;
	speaker->status_lock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(speaker->status_lock, NULL);

	return speaker;
}

void server_speaker_free(server_speaker_t *speaker)
{

	speaker->users = NULL;
	if (speaker->queue_sem) {
		sem_destroy(speaker->queue_sem);
		free(speaker->queue_sem);
		speaker->queue_sem = NULL;
	}
	if (speaker->queue_lock) {
		pthread_mutex_destroy(speaker->queue_lock);
		free(speaker->queue_lock);
		speaker->queue_lock = NULL;
	}
	if (speaker->q) {
		free_queue(speaker->q);
		speaker->q = NULL;
	}
	if (speaker->status_lock) {
		pthread_mutex_destroy(speaker->status_lock);
		free(speaker->status_lock);
		speaker->status_lock = NULL;
	}
	free(speaker);
}

void add_packet_to_queue(server_speaker_t *speaker, packet_t *packet)
{
	pthread_mutex_lock(speaker->queue_lock);
	insert_node(speaker->q, (void *)packet);
	pthread_mutex_unlock(speaker->queue_lock);

	sem_post(speaker->queue_sem);
}


void push_user_list(server_speaker_t *speaker)
{
	packet_t *packet = NULL;
	queue_t *names = NULL;
	node_t *n = NULL;

	names = get_names(speaker->users);

	for (n = names->head; n; n = n->next) {
		packet = NULL;
		packet = new_packet(GET_ULIST, speak_strdup((char *)n->data), NULL, speak_strdup((char *)n->data));
		set_user_list(packet, names);

		add_packet_to_queue(speaker, packet);
	}

	free_queue(names);
}


void broadcast(server_speaker_t *speaker, packet_t *packet)
{
	packet_t *copy;
	queue_t *names = get_names(speaker->users);
	node_t *n = NULL;

	printf("%s is broadcasting %s\n", packet->name, packet->data);
	for (n = names->head; n; n = n->next) {
		printf("%s to be added for broadcasting\n", (char *)n->data);
		copy = NULL;
		copy = new_packet(packet->code, speak_strdup(packet->name), 
				speak_strdup(packet->data), speak_strdup((char *)n->data));
		add_packet_to_queue(speaker, copy);
	}
}

void *speaker_run(void *s) 
{
	if (!s) {
		return NULL;
	} else {
		speaker_go((server_speaker_t *)s);
	}
	return NULL;
}

void speaker_stop(server_speaker_t *speaker)
{
	pthread_mutex_lock(speaker->status_lock);
	speaker->run_status = FALSE;
	pthread_mutex_unlock(speaker->status_lock);
	sem_post(speaker->queue_sem);
}

int speaker_running(server_speaker_t *speaker)
{
	int status;
	pthread_mutex_lock(speaker->status_lock);
	status = speaker->run_status;
	pthread_mutex_unlock(speaker->status_lock);
	return status;
}


/*** Helper Functions ****************************************************/

int cmp_dummy(void *a, void *b)
{
	if ((long)a == (long)b) {
		return 1;
	} else {
		return 1;
	}
}

char *speak_strdup(char *s)
{
	int i = 0, j = 0;
	char *c = malloc(strlen(s) + 1);
	while ((c[i++] = s[j++]));
	return c;
}

void speaker_go(server_speaker_t *speaker)
{
	packet_t *packet = NULL;
	queue_t *online_users = NULL;
	while(TRUE) {
		sem_wait(speaker->queue_sem);
		if (!speaker_running(speaker)) {
			break;
		}
		packet = NULL;

		pthread_mutex_lock(speaker->queue_lock);
		packet = (packet_t *)pop_first(speaker->q);
		pthread_mutex_unlock(speaker->queue_lock);

		if (packet->code == SEND) {
			printf("Sending message: %s -> %s %s\n", packet->name, packet->to, packet->data);
		} else if (packet->code == GET_ULIST) {
			online_users = NULL;
			if (packet->users == NULL) {
				online_users = get_names(speaker->users);
				set_user_list(packet, online_users);
				free_queue(online_users);
			}
			if (packet->to == NULL) {
				packet->to = speak_strdup(packet->name);
				packet->to_len = packet->name_len;
			}
			free(packet->name);
			packet->name = NULL;
			packet->name_len = 0;
			printf("Sending list of online users to %s\n", packet->to);
		}
		users_send_packet(speaker->users, packet);
		free_packet(packet);
		packet = NULL;
	}
}


