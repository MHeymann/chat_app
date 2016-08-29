#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include "serializer.h"
#include "packet.h"

#ifdef DEBUG
#include <string.h>
#endif

int read_int_from_buffer(char *bytes, int *global_index);
char *read_string_from_buffer(char *bytes, int *global_index, int length);
int cmp(void *a, void *b);

/*** Functions ***********************************************************/

char *serialize(packet_t *packet, int *psize) 
{
	int global_index = 0;
	int i = 0;
	int len;
	int *iptr = NULL;
	int size = 0;
	char *buffer = NULL;
	char *string = NULL;
	node_t *n = NULL;

	size += sizeof(int);
#ifdef DEBUG
	size += strlen(packet->name);
	size += strlen(packet->data);
	size += strlen(packet->to);
	size += get_node_count(packet->users);
	if (get_node_count(packet->users)) {
		for (n = packet->users->head; n; n = n->next) {
			size += sizeof(int);
			if (n->data) {
				size += sizeof((char *)n->data);
			} else {
				fprintf(stderr, "fault in queue nodes!!\n");
			}
		}
	}
#else
	size += packet->name_len;
	size += packet->data_len;
	size += packet->to_len;
	size += packet->list_len;
	size += packet->list_size;
#endif

	/* TODO: reevaluate the correctness here */
	*psize = size/* + sizeof(int)*/;

	buffer = malloc(size + sizeof(int));
	iptr = (int *)(buffer + global_index);
	*iptr = htonl(size);
	global_index += sizeof(int);
	

#ifdef DEBUG
	if ((packet->name) && (packet->name_len != (int)strlen(packet->name))) {
		fprintf(stderr, "fault with length parameters of packet. fixing but find issue\n");
		packet->name_len = strlen(packet->name);
	}
#endif
	if (packet->name) {
		iptr = (int *)(buffer + global_index);
		*iptr = htonl(packet->name_len);
		global_index += sizeof(int);
		string = packet->name;
		for (i = 0; i < packet->name_len; i++) {
			buffer[global_index + i] = string[i];
		}
		global_index += i;
	} else {
		iptr = (int *)(buffer + global_index);
		*iptr = htonl(0);
		global_index += sizeof(int);
	}

#ifdef DEBUG
	if ((packet->data) && (packet->data_len != (int)strlen(packet->data))) {
		fprintf(stderr, "fault with length parameters of packet. fixing but find issue\n");
		packet->data_len = strlen(packet->data);
	}
#endif
	if (packet->data) {
		iptr = (int *)(buffer + global_index);
		*iptr = htonl(packet->data_len);
		global_index += sizeof(int);
		string = packet->data;
		for (i = 0; i < packet->data_len; i++) {
			buffer[global_index + i] = string[i];
		}
		global_index += i;
	} else {
		iptr = (int *)(buffer + global_index);
		*iptr = htonl(0);
		global_index += sizeof(int);
	}

#ifdef DEBUG
	if ((packet->to) && (packet->to_len != (int)strlen(packet->to))) {
		fprintf(stderr, "fault with length parameters of packet. fixing but find issue\n");
		packet->to_len = strlen(packet->to);
	}
#endif
	if (packet->to) {
		iptr = (int *)(buffer + global_index);
		*iptr = htonl(packet->to_len);
		global_index += sizeof(int);
		string = packet->to;
		for (i = 0; i < packet->to_len; i++) {
			buffer[global_index + i] = string[i];
		}
		global_index += i;
	} else {
		iptr = (int *)(buffer + global_index);
		*iptr = htonl(0);
		global_index += sizeof(int);
	}

#ifdef DEBUG
	if ((packet->users) && (packet->list_len != get_node_count(packet->users))) {
		fprintf(stderr, "fault with length parameters of packet. fixing but find issue\n");
		packet->list_len = get_node_count(packet->users);
	}
#endif
	if (packet->users) {
		iptr = (int *)(buffer + global_index);
		*iptr = htonl(packet->list_len);
		global_index += sizeof(int);
		for (n = packet->users->head; n; n = n->next) {
			iptr = (int *)(buffer + global_index);
			len = (int)strlen((char *)n->data);
			*iptr = htonl(len);
			global_index += sizeof(int);
			string = n->data;
			for (i = 0; i < len; i++) {
				buffer[global_index + i] = string[i];
			}
			global_index += i;
		}
	} else {
		iptr = (int *)(buffer + global_index);
		*iptr = htonl(0);
		global_index += sizeof(int);
	}

	return buffer;
}

packet_t *deserialize(char *bytes)
{
	packet_t *packet;
	int global_index = 0;
	int i			 = 0;
	int slen		 = 0;
	char *string	 = NULL;

	int name_len	= 0;
	int data_len	= 0;
	int to_len		= 0;
	int list_len	= 0;
	int list_size	= 0;
	
	int code   = -1;
	char *name = NULL;
	char *data = NULL;
	char *to   = NULL;
	queue_t *users = NULL;

	code = read_int_from_buffer(bytes, &global_index);

	name_len = read_int_from_buffer(bytes, &global_index);
	name = read_string_from_buffer(bytes, &global_index, name_len);

	data_len = read_int_from_buffer(bytes, &global_index);
	data = read_string_from_buffer(bytes, &global_index, data_len);

	to_len = read_int_from_buffer(bytes, &global_index);
	to = read_string_from_buffer(bytes, &global_index, to_len);

	list_len = read_int_from_buffer(bytes, &global_index);
	if (list_len) {
		init_queue(&users, cmp, free);
	}
	for (i = 0; i < list_len; i++) {
		slen = read_int_from_buffer(bytes, &global_index);
		list_size += slen + sizeof(int);
		string = read_string_from_buffer(bytes, &global_index, slen);
		insert_node(users, string);
	}

	packet = new_empty_packet();
	packet->code = code;

	packet->name_len = name_len;
	packet->name = name;

	packet->data_len = data_len;
	packet->data = data;

	packet->to_len = to_len;
	packet->to = to;

	packet->list_len = list_len;
	packet->list_size = list_size;
	packet->users = users;

	return packet;
}

int read_int_from_buffer(char *bytes, int *global_index) 
{
	int *iptr = (int *)(bytes + *global_index);
	int read_val;

	read_val = ntohl(*iptr);
	*global_index += (int)(sizeof(int));

	return read_val;
}

char *read_string_from_buffer(char *bytes, int *global_index, int length)
{
	int i = 0;
	char *string = NULL;

	string = malloc(length + 1);

	for (i = 0; i < length; i++) {
		string[i] = bytes[*global_index + i];
	}

	string[length] = '\0';
	*global_index += length;
	return string;
}

int cmp(void *a, void *b) 
{
	if (a == b) {
		return 0;
	} else {
		return 0;
	}
}
