#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
*/
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> /* write */
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
/*
#include <select.h>
*/


#include "client_speaker.h"
#include "../packet/packet.h"
#include "../packet/code.h"

/*
typedef struct client_speaker {
	char *name;
	char *hostname;
	int port;
	int sd;
} client_speaker_t;
*/

/*** Helper Function Prototypes ******************************************/

char *speaker_strdup(char *s);
int speaker_send_packet(client_speaker_t *speaker, packet_t *packet);
int connect_speaker(client_speaker_t *speaker);

/*** Functions ***********************************************************/

client_speaker_t *new_client_speaker(char *name, char *hostname, int port)
{
	client_speaker_t *speaker = NULL;

	speaker = malloc(sizeof(client_speaker_t));
	if (speaker) {
		speaker->name = speaker_strdup(name);
		speaker->hostname = speaker_strdup(hostname);
		speaker->port = port;
		speaker->sd = 0;
	} else {
		fprintf(stderr, "error allocating speaker\n");
	}

	return speaker;
}

void free_client_speaker(client_speaker_t *speaker)
{
	if (!speaker) {
		return;
	}
	if (speaker->name) {
		free(speaker->name);
		speaker->name = NULL;
	}
	if (speaker->hostname) {
		free(speaker->hostname);
		speaker->hostname = NULL;
	}
	if (speaker->port) {
		speaker->port = -1;
	}
	if (speaker->sd) {
		/*
		close(speaker->sd);
		*/
		speaker->sd = -1;
	}
	free(speaker);
}

void free_speaker(client_speaker_t *speaker)
{
	if (!speaker) {
		return;
	}
	if (speaker->name) {
		free(speaker->name);
		speaker->name = NULL;
	}
	if (speaker->hostname) {
		free(speaker->hostname);
		speaker->hostname = NULL;
	}
	speaker->port = 0;
	close(speaker->sd);
	speaker->sd = 0;

	free(speaker);
}

void get_online_names(client_speaker_t *speaker)
{
	packet_t *packet = new_packet(GET_ULIST, speaker_strdup(speaker->name), NULL, NULL);
	if (packet) {
		if (!speaker_send_packet(speaker, packet)) {
			fprintf(stderr, "Failed to send packet to request user list\n");
		}
	} else {
		fprintf(stderr, "could not make a packet to request users with\n");
	}
	free_packet(packet);
}

int send_string(client_speaker_t *speaker, char *s, char *to)
{
	packet_t *packet = new_packet(SEND, speaker_strdup(speaker->name), 
			speaker_strdup(s), speaker_strdup(to));
	if (packet) {
		if (!speaker_send_packet(speaker, packet)) {
			fprintf(stderr, "Failed to send message packet\n");
		} else {
			free_packet(packet);
			return TRUE;
		}
	} else {
		fprintf(stderr,
				"could not make a packet to send message with\n");
	}
	free_packet(packet);
	return FALSE;
}

int echo_string(client_speaker_t *speaker, char *s)
{
	packet_t *packet = new_packet(ECHO, speaker_strdup(speaker->name), 
			speaker_strdup(s), NULL);
	if (packet) {
		if (!speaker_send_packet(speaker, packet)) {
			fprintf(stderr, "Failed to send packet for echoing\n");
		} else {
			free_packet(packet);
			return TRUE;
		}
	} else {
		fprintf(stderr, "could not make a packet to echo message with\n");
	}
	free_packet(packet);
	return FALSE;
}

int broadcast_string(client_speaker_t *speaker, char *s)
{
	packet_t *packet = new_packet(BROADCAST, speaker_strdup(speaker->name), 
			speaker_strdup(s), NULL);
	if (packet) {
		if (!speaker_send_packet(speaker, packet)) {
			fprintf(stderr, "Failed to send packet to broadcast with\n");
		} else {
			free_packet(packet);
			return TRUE;
		}
	} else {
		fprintf(stderr, 
				"could not make a packet to broadcast message with\n");
	}
	free_packet(packet);
	return FALSE;
}

int get_speaker_sd(client_speaker_t *speaker)
{
	return speaker->sd;
}

int speaker_login(client_speaker_t *speaker, char *pw)
{
	packet_t *packet = NULL;
	fd_set readfds;
	int act;

	printf("Connecting\n");
	if (!connect_speaker(speaker)) {
		fprintf(stderr, "Failed to connect to server\n");
		return FALSE;
	}
	printf("Connected\n");

	packet = new_packet(LOGIN, speaker_strdup(speaker->name), 
			speaker_strdup(pw), NULL);

	printf("Sending log in packet\n");
	if (!speaker_send_packet(speaker, packet)) {
		printf("Failed to send login details\n");
		free_packet(packet);
		return FALSE;
	}
	printf("Sent packet\n");

	free_packet(packet);
	packet = NULL;

	FD_ZERO(&readfds);
	FD_SET(speaker->sd, &readfds);
	printf("Awaiting response\n");
	act = select(speaker->sd + 1, &readfds, NULL, NULL, NULL);
	printf("Getting response\n");

	if ((act < 0) && (errno != EINTR)) {
		printf("select error\n");
	}

	if (!FD_ISSET(speaker->sd, &readfds)) {
		printf(
		"After select with only one sd set, this should be impossible\n");
	}

	packet = receive_packet(speaker->sd);

	printf("Received response\n");
	
	if (packet == NULL)	{
		return FALSE;
	}
	if (packet->data == NULL) {
		return FALSE;
	}

	if (strcmp(packet->data, "accept") == 0) {
		free_packet(packet);
		return TRUE;
	} else {
		free_packet(packet);
		return FALSE;
	}
}

int speaker_logoff(client_speaker_t *speaker)
{
	packet_t *packet = new_packet(QUIT, speaker_strdup(speaker->name), 
			NULL, NULL);
	if (packet) {
		if (!speaker_send_packet(speaker, packet)) {
			fprintf(stderr,
					"Failed to send packet to notify server of exit\n");
		} else {
			free_packet(packet);
			return TRUE;
		}
	} else {
		fprintf(stderr,
				"could not make a packet to notify server of exit\n");
	}
	free_packet(packet);

	return FALSE;
}

/*** Helper Functions ****************************************************/

char *speaker_strdup(char *s)
{
	char *c = malloc(strlen(s) + sizeof(char));
	int i = 0;
	int j = 0;

	if (!c) {
		fprintf(stderr, "memory error in speaker_strdup\n");
	} else {
		while ((c[i++] = s[j++]));
	}
	return c;
}

int speaker_send_packet(client_speaker_t *speaker, packet_t *packet)
{
	send_packet(packet, speaker->sd);
	return TRUE;
}

int connect_speaker(client_speaker_t *speaker)
{
	int sockfd;
	struct sockaddr_in addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Problem creating socket\n");
		return FALSE;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(speaker->hostname);
	addr.sin_port = htons(speaker->port);

	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Problem connecting to server\n");
		return FALSE;
	} else {
		speaker->sd = sockfd;
		return TRUE;
	}
}
