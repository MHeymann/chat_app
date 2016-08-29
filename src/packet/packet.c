#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "packet.h"
#include "serializer.h"
#include "../queue/queue.h"


/*** Functions ***********************************************************/

packet_t *new_empty_packet() 
{
	packet_t *packet = NULL;
	packet = malloc(sizeof(packet_t));

	packet->code = -1;

	packet->name = NULL; 
	packet->name_len = -1;

	packet->data = NULL;
	packet->data_len = -1;

	packet->to = NULL;
	packet->to_len =  -1;

	packet->users = NULL;
	packet->list_len = -1;
	packet->list_size = -1;

	return packet;
}

packet_t *new_packet(int code, char *name, char *data, char*to)
{
	packet_t *packet = NULL;
	packet = malloc(sizeof(packet_t));

	packet->code = code;

	packet->name = name; 
	packet->name_len = strlen(name);

	packet->data = data;
	packet->data_len = strlen(data);

	packet->to = to;
	packet->to_len =  strlen(to);

	packet->users = NULL;
	packet->list_len = -1;
	packet->list_size = -1;

	return packet;
}

void free_packet(packet_t *packet)
{
	if (!packet) {
		fprintf(stderr, "Null pointer was given to free");
		return;
	}
	if (packet->name) {
		free(packet->name);
		packet->name = NULL;
		packet->name_len = -1;
	}
	if (packet->data) {
		free(packet->data);
		packet->data = NULL;
		packet->data_len = -1;
	}
	if (packet->to) {
		free(packet->to);
		packet->to = NULL;
		packet->to_len = -1;
	}
	if (packet->users) {
		free_queue(packet->users);
		packet->users = NULL;
		packet->list_len = -1;
		packet->list_size = -1;
	}
}

void set_user_list(packet_t *p, queue_t *users) 
{
	node_t *n = NULL;
	if (p->users) {
		free_queue(p->users);
		p->users = NULL;
	}
	p->users = users;
	p->list_len = get_node_count(users);
	p->list_size = 0;
	for (n = users->head; n; n = n->next) {
		p->list_size += sizeof(int);
		if (n->data) {
			p->list_size += strlen((char *)n->data);
		} else {
			fprintf(stderr, "fault in queue nodes!!\n");
		}
	}
	
}

int get_code(packet_t *p) 
{
	return p->code;
}

void set_code(packet_t *p, int code)
{
	p->code = code;
}

char *get_data(packet_t *p)
{
	return p->data;
}

void set_data(packet_t *p, char *data)
{
	if (p->data) {
		free(p->data);
		p->data = NULL;
	}
	p->data = data;
}

void send_packet(packet_t *packet, int fd)
{
	int size;
	int write_bytes;
	int i;
	char *buffer = NULL;

	buffer = serialize(packet, &size);

	for (i = 0; i < size;) {
		write_bytes = write(fd, (buffer + i), size - i);
		i += write_bytes;
	}
}

packet_t *receive_packet(int fd) 
{
	/* to be continued... */
	packet_t *packet = NULL;
	int size = 0;
	int r = 0;
	int i = 0;
	char sizebuffer[sizeof(int)];
	char *b = (void *)sizebuffer;
	int *intp;

	r = read(fd, (void *)b, sizeof(int));

	if (r == 0) {
		printf("Does this ever happen??*************\n");
		close(fd);
		return NULL;
	} else if (r == -1) {
		close(fd);
		return NULL;
	}


	intp = (int *)b;
	size = ntohl(*intp);
	
	if (size <= 0) {
		printf("this is objectively weird. Inside receive_packet\n");
		return NULL;
	}

	b = NULL;
	b = malloc(size);
	if (!b) {
		fprintf(stderr, "Failed to malloc a buffer in receive_packet\n");
		return NULL;
	}

	for (i = 0; i < size; i++) {
		r = read(fd, (b + i), size - i);
		i += r;
		if (r == -1) {
			fprintf(stderr, "Potentially weird!\n");
			close(fd);
		} else if (r == 0) {
			printf("read 0\n");
		}
	}

	packet = deserialize(b);

	return packet;
	
}
