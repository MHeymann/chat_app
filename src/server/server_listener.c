#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> /* inet_addr */
#include <unistd.h> /* write */
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

#include "server_listener.h"
#include "users.h"
#include "../packet/code.h"
#include "../hashset/fd_hashset.h"
#include "../hashset/string_hashset.h"

/*** Macros **************************************************************/

#define BUFFER_SIZE		2048
#define DEFAULT_PORT	8008
#define TRUE			1
#define FALSE			0
#define MAX_CLIENTS		(int) sizeof(long)

#define PORT_COUNT(A)	(sizeof(A) / sizeof(int))
	

/*** Helper Function Prototypes ******************************************/

void listener_go(server_listener_t *listener);
int check_user_password(char *name, char *pw);

/*** Functions ***********************************************************/

server_listener_t *new_server_listener(int *ports, users_t *users, 
		server_speaker_t *speaker)
{
	server_listener_t *listener = NULL;

	if (!users) {
		fprintf(stderr, "Error: no users_t * provided\n");
		return NULL;
	}
	if (!speaker) {
		fprintf(stderr, "Error: no speaker provided\n");
		return NULL;
	}

	if (!ports) {
		ports = malloc(sizeof(int));
		ports[0] = 8002;
		printf("Listener defaulting to port 8002\n");
	}

	listener = malloc(sizeof(server_listener_t));
	if (!listener) {
		fprintf(stderr, "error mallocing listener\n");
		return NULL;
	}
	listener->ports = ports;
	listener->run_status = TRUE;
	listener->status_lock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(listener->status_lock, NULL);
	listener->users = users;
	listener->speaker = speaker;

	return listener;
}

void server_listener_free(server_listener_t *listener)
{
	if (listener->ports) {
		free(listener->ports);
		listener->ports = NULL;
	}
	if (listener->status_lock) {
		pthread_mutex_destroy(listener->status_lock);
		free(listener->status_lock);
		listener->status_lock = NULL;
	}
	if (listener->users) {
		free_users(listener->users);
		listener->users = NULL;
	}
	if (listener->speaker) {
		/* this gets free'd in main */
		listener->speaker = NULL;
	}
}

void *listener_run(void *listener) 
{
	if (!listener) {
	} else {
		listener_go((server_listener_t *)listener);
	}
	return NULL;
}

void listener_stop(server_listener_t *listener)
{
	pthread_mutex_lock(listener->status_lock);
	listener->run_status = FALSE;
	pthread_mutex_unlock(listener->status_lock);
}

int listener_running(server_listener_t *listener)
{
	int status;
	pthread_mutex_lock(listener->status_lock);
	status = listener->run_status;
	pthread_mutex_unlock(listener->status_lock);
	return status;
}

/*** Helper Functions ****************************************************/

void listener_go(server_listener_t *listener)
{
	int master_socket = -1, new_socket = -1, i = 0;
	int sd = 0;
	int opt = TRUE;
	int addrlen = -1;
	int activity;
	int max_sd;
	struct sockaddr_in address;
	char *message = NULL;
	unsigned int write_bytes;
	fd_set readfds;
	int port_count;
	queue_t *online_list2 = NULL;
	node_t *n = NULL;

	packet_t *packet = NULL;
	packet_t *p = NULL;

	port_count = PORT_COUNT(listener->ports);


	for (i = 0; i < port_count; i++) {
	/* Create master sockets */
		if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
			perror("socket failed\n");
			exit(EXIT_FAILURE);
		}
		printf("Port %d selected\n", listener->ports[i]);
		/* good habit, not necessary: set master socket to allow multiple
		 * connections/
		 */
		if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
			perror("setsockopt\n");
			exit(EXIT_FAILURE);
		}
	
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(listener->ports[i]);
	
		/* Bind */
		if (bind(master_socket, (struct sockaddr *)&address, 
					sizeof(address)) < 0) {
			perror("bind failed\n");
			exit(EXIT_FAILURE);
		}
		printf("Bind to %d done\n", listener->ports[i]);
	
		/* listen */
		if (listen(master_socket, 3) < 0) {
			perror("listen\n");
			exit(EXIT_FAILURE);
		}

		listener->ports[i] = master_socket;
	}


	/* Accept incoming connections */
	printf("Waiting for incoming connections...\n");
	addrlen = sizeof(address);
	while (TRUE) {
		/* clear the socket set */
		FD_ZERO(&readfds);

		max_sd = 0;
		for (i = 0; i < port_count; i++) {
		/* add masters for listening */
			FD_SET(listener->ports[i], &readfds);
			if (max_sd < listener->ports[i]) {
				max_sd = listener->ports[i];
			}
		}

		/* get a list of those currently online */
		/*
		online_list1 = get_names(listener->users);
		if (!online_list1) {
			fprintf(stderr, "failed to get names\n");
			exit(EXIT_FAILURE);
		}
		*/


		/* all sockets currently opened added to set */
		/*
		for (n = online_list1->head; n; n = n->next) {
			name_ptr = (char *)n->data;
			sd = name_get_fd(listener->users->names, name_ptr);
			if (sd <= 0) {
				printf("this shouldn't be possible when translating a name to an sd.\n");
				continue;
			}
			FD_SET(sd, &readfds);

			if (max_sd < sd) {
				max_sd = sd;
			}
		}
		*/

		online_list2 = get_fds(listener->users);
		for (n = online_list2->head; n; n = n->next) {
			sd = (int)((long)n->data);
			if (sd <= 0) {
				printf("this shouldn't be possible when translating a name to an sd.\n");
				continue;
			}
			FD_SET(sd, &readfds);

			if (max_sd < sd) {
				max_sd = sd;
			}
		}

		/* ret	 = select(nfds, readfds, writefds, exceptfds, timeout); */
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR)) {
			printf("select error\n");
		}

		for (i = 0; i < port_count; i++) {
			/* master socket, => incoming connection */
			if (FD_ISSET(listener->ports[i], &readfds)) {
				if ((new_socket = accept(listener->ports[i], 
								(struct sockaddr *)&address,
								(socklen_t *)&addrlen)) < 0) {
					perror("accept\n");
					exit(EXIT_FAILURE);
				}

				/* Inform user of socket number - useful in send 
				* and receive actions */
				printf("New connection: \nsocket fd: \t%d\nip: \t%s\nport:\t%d\n",
						new_socket, inet_ntoa(address.sin_addr), 
						ntohs(address.sin_port));
				/* Reply to the client */
				message = "Hi! You are now connected and being assigned a handler\n";
				write_bytes = write(new_socket, message, strlen(message));
				if (write_bytes != strlen(message)) {
					fprintf(stderr, "failed to write");
					perror("send\n");
				}
				printf("Sent Welcome message\n");

				/* add to users */
				add_connection(listener->users, new_socket);
			}
		}

		/* IO on other sockets */

		for (n = online_list2->head; n; n = n->next) {
			sd = (int)((long)n->data);
			if (sd <= 0) {
				printf("this shouldn't be possible when translating a name to an sd.\n");
				continue;
			}
			if (FD_ISSET(sd, &readfds)) {
				/* boom */
				packet = NULL;
				packet = receive_packet(sd);
				if (!packet) {
					remove_channel(listener->users, sd);
					push_user_list(listener->speaker);
					close(sd);
				} else if (packet->code == QUIT) {
					remove_channel(listener->users, sd);
					push_user_list(listener->speaker);
					close(sd);
				} else if (packet->code == SEND) {
					add_packet_to_queue(listener->speaker, packet);
				} else if (packet->code == ECHO) {
					send_packet(packet, sd);
				} else if (packet->code == BROADCAST) {
					broadcast(listener->speaker, packet);
				} else if (packet->code == LOGIN) {
					if ((check_user_password(packet->name, packet->data)) && 
							login_connection(listener->users, sd, packet->name)) {
						push_user_list(listener->speaker);
						p = new_packet(SEND, "admin", "accept", packet->name);
						send_packet(p, sd);
						p = NULL;
					} else {
						p = new_packet(SEND, "admin", "denial", packet->name);
						send_packet(p, sd);
						p = NULL;
						close(sd);
					}

				} else if (packet->code == GET_ULIST) {
					add_packet_to_queue(listener->speaker, packet);
				} else {
					fprintf(stderr, "Packet with code %d came.  this is weird\n", 
							packet->code);
				}
			}
		}
	}
}

int check_user_password(char *name, char *pw)
{
	if (name == pw) {
		return TRUE;
	} else {
		return TRUE;
	}
}

