#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "packet.h"
#include "serializer.h"
#include "../queue/queue.h"

/*** Helper Function Prototypes ******************************************/

int cmp_strings(void *a, void *b);
char *strdup(char *s);

/*** Functions ***********************************************************/

packet_t *new_empty_packet() 
{
	packet_t *packet = NULL;
	packet = malloc(sizeof(packet_t));

	packet->code = -1;

	packet->name = NULL; 
	packet->name_len = 0;

	packet->data = NULL;
	packet->data_len = 0;

	packet->to = NULL;
	packet->to_len =  0;

	packet->users = NULL;
	packet->list_len = 0;
	packet->list_size = 0;

	return packet;
}

packet_t *new_packet(int code, char *name, char *data, char*to)
{
	packet_t *packet = NULL;
	packet = malloc(sizeof(packet_t));

	packet->code = code;

	if (name) {
		packet->name = name; 
		packet->name_len = strlen(name);
	} else {
		packet->name = NULL;
		packet->name_len = 0;
	}

	if (data) {
		packet->data = data;
		packet->data_len = strlen(data);
	} else {
		packet->data = NULL;
		packet->data_len = 0;
	}

	if (to) {
		packet->to = to;
		packet->to_len =  strlen(to);
	} else {
		packet->to = NULL;
		packet->to_len =  0;
	}

	packet->users = NULL;
	packet->list_len = 0;
	packet->list_size = 0;

	return packet;
}

void free_packet(void *p)
{
	packet_t *packet = (packet_t *)p;
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
	free(p);
}

void set_user_list(packet_t *p, queue_t *users) 
{
	node_t *n = NULL;
	queue_t *cusers = NULL;
	if (p->users) {
		free_queue(p->users);
		p->users = NULL;
	}

	init_queue(&cusers, cmp_strings, free);

	p->list_len = get_node_count(users);
	p->list_size = 0;
	for (n = users->head; n; n = n->next) {
		p->list_size += sizeof(int);
		if (n->data) {
			p->list_size += strlen((char *)n->data) * 2;
			insert_node(cusers, strdup((char *)n->data));
		} else {
			fprintf(stderr, "fault in queue nodes!!\n");
		}
	}
	p->users = cusers;
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
	char sbuffer[4];
	int *iptr = (int *)sbuffer;

	buffer = serialize(packet, &size);
	
	*iptr = htonl(size);

	write_bytes = write(fd, sbuffer, 4);
	for (i = 0; i < size;) {
		write_bytes = write(fd, (buffer + i), size - i);
		i += write_bytes;
	}
	free(buffer);
}

packet_t *receive_packet(int fd) 
{
	/* to be continued... */
	packet_t *packet = NULL;
	int size = 0;
	int r = 0;
	int i = 0;
	char sizebuffer[sizeof(int)];
	char *b = (char *)sizebuffer;
	int *intp;

	r = read(fd, (void *)b, sizeof(int));

	if (r == 0) {
#ifdef PDEBUG
		printf("0 disconnect*************\n");
		printf("Does this ever happen??*************\n");
#endif
		close(fd);
		return NULL;
	} else if (r == -1) {
#ifdef PDEBUG
		printf("-1 disconnect*************\n");
#endif
		close(fd);
		return NULL;
	}


	intp = (int *)b;
	size = ntohl(*intp);
	
	if (size <= 0) {
#ifdef PDEBUG
		printf("this is objectively weird. Inside receive_packet\n");
#endif
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
		printf("Read %d bytes of %d\n", r, size);
		i += r;
		if (r == -1) {
			/*
			fprintf(stderr, "Potentially weird!\n");
			*/
			/*
			close(fd);
			*/
			printf("read -1\n");
			break;
		} else if (r == 0) {
			printf("read 0\n");
		}
	}

	packet = deserialize(b);
	free(b);

	return packet;
	
}

/*** Helper Functions ****************************************************/

int cmp_strings(void *a, void *b)
{
	return strcmp((char *)a, (char *)b);
}

char *strdup(char *s)
{
	char *c = malloc(strlen(s) + 1);
	int i = 0;
	int j = 0;

	while((c[i++] = s[j++]));

	return c;
}
