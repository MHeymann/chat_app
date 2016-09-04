#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "users.h"
#include "../hashset/string_hashset.h"
#include "../hashset/fd_hashset.h"
#include "../queue/queue.h"

/*
typedef struct users {
	string_hashset_ptr names;
	fd_hashset_ptr sockets;
	pthread_mutex_t *hs_protect;
} users_t;
*/

users_t *new_users()
{
	users_t *users = NULL;
	string_hashset_ptr names = NULL;
	fd_hashset_ptr sockets = NULL;

	users = malloc(sizeof(users_t));

	if (!users) {
		perror("Memory error\n");
		return NULL;
	}
	fd_hashset_init_defaults(&sockets);
	string_hashset_init_defaults(&names);
	users->names = names;
	users->sockets = sockets;

	users->hs_protect = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(users->hs_protect, NULL);

	return users;
}

void free_users(users_t *users)
{
	if (!users) {
		return;
	}
	if (users->names) {
		free_string_hashset(users->names);
		users->names = NULL;
	}
	if (users->sockets) {
		free_fd_hashset(users->sockets);
		users->sockets = NULL;
	}
	if (users->hs_protect) {
		pthread_mutex_destroy(users->hs_protect);
		free(users->hs_protect);
		users->hs_protect = NULL;
	}
	free(users);
}

/* remember to free this queue appropriately */
queue_t *get_names(users_t *users)
{
	queue_t *q = NULL;

	pthread_mutex_lock(users->hs_protect);
	q = shs_get_keys(users->names);
	pthread_mutex_unlock(users->hs_protect);

	return q;
}

/* remember to free this queue appropriately */
queue_t *get_fds(users_t *users)
{
	queue_t *q = NULL;

	pthread_mutex_lock(users->hs_protect);
	q = fdhs_get_keys(users->sockets);
	pthread_mutex_unlock(users->hs_protect);

	return q;
}



void users_send_packet(users_t *users, packet_t *packet)
{
	int fd = 0;

	pthread_mutex_lock(users->hs_protect);
	fd = name_get_fd(users->names, packet->to);
	if (!fd) {
		pthread_mutex_unlock(users->hs_protect);
		fprintf(stderr, "Failed to send message in users.c!!!\n");
		return;
	}

	send_packet(packet, fd);
	/* TODO: maybe let send packet return and put this in an if statement */

	pthread_mutex_unlock(users->hs_protect);

}

void remove_channel(users_t *users, int fd)
{
	char *name = NULL;
	pthread_mutex_lock(users->hs_protect);

	name = fd_get_name(users->sockets, fd);
	if (name) {
		string_hashset_remove(users->names, name);
	} else {
		fprintf(stderr, "this is weird when removing fd\n");
		pthread_mutex_unlock(users->hs_protect);
		return;
	}
	fd_hashset_remove(users->sockets, fd);

	printf("User '%s' went offline, %d still online\n", name, 
			string_hashset_content_count(users->names));

	free(name);
	pthread_mutex_unlock(users->hs_protect);
}

void remove_name(users_t *users, char *name)
{
	int fd;

	pthread_mutex_lock(users->hs_protect);

	fd = name_get_fd(users->names, name);
	if (fd) {
		fd_hashset_remove(users->sockets, fd);
	} else {
		fprintf(stderr, "this is weird when removing name\n");
		pthread_mutex_unlock(users->hs_protect);
		return;
	}
	/*
	*/
	string_hashset_remove(users->names, name);

	printf("User '%s' went offline, %d still online\n", name, 
			string_hashset_content_count(users->names));

	pthread_mutex_unlock(users->hs_protect);
}

int add_connection(users_t *users,int fd)
{
	pthread_mutex_lock(users->hs_protect);

	if (!fd_hashset_insert(users->sockets, fd, "")) {
		printf("failed to insert into socket list");
		return 0;
	}

	pthread_mutex_unlock(users->hs_protect);
	return 1;
}

int login_connection(users_t *users, int fd, char *name)
{
	pthread_mutex_lock(users->hs_protect);

	printf("insertion\n");
	if (!string_hashset_insert(users->names, name, fd)) {
		printf("failed to insert into name list\n");
		pthread_mutex_unlock(users->hs_protect);
		return 0;
	}
	printf("update\n");
	fd_hashset_update(users->sockets, fd, name);

	pthread_mutex_unlock(users->hs_protect);
	return 1;
}

