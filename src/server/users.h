#ifndef USERS_H
#define USERS_H

#include <stdlib.h>
#include <pthread.h>

#include "../hashset/string_hashset.h"
#include "../hashset/fd_hashset.h"
#include "../queue/queue.h"
#include "../packet/packet.h"

typedef struct users {
	string_hashset_ptr names;
	fd_hashset_ptr sockets;
	pthread_mutex_t *hs_protect;
} users_t;

users_t *new_users();

void free_users(users_t *users);

queue_t *get_names(users_t *users);

queue_t *get_fds(users_t *users);

void users_send_packet(users_t *users, packet_t *packet);

void remove_channel(users_t *users, int fd);

void remove_name(users_t *users, char *name);

int add_connection(users_t *users, int fd);

int login_connection(users_t *users, int fd, char *name);

#endif
