#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include "serializer.h"
#include "packet.h"

#ifdef DEBUG
#endif

int read_int_from_buffer(char *buffer, int *global_index);
char *read_string_from_buffer(char *buffer, int *global_index, int length);
int cmp(void *a, void *b);

void write_int_to_buffer(char *buffer, int *global_index, int integer);
void write_string_to_buffer(char *buffer, int *global_index, int length, char *string);

/*** Functions ***********************************************************/

char *serialize(packet_t *packet, int *psize) 
{
	int global_index = 0;
	int len;
	int size = 0;
	char *buffer = NULL;
	node_t *n = NULL;

	size += sizeof(int);
#ifdef DEBUG
	size += sizeof(int);
	if (packet->name) {
		size += strlen(packet->name) * 2;
	}
	size += sizeof(int);
	if (packet->data) {
		size += strlen(packet->data) * 2;
	}
	size += sizeof(int);
	if (packet->to) {
		size += strlen(packet->to) * 2;
	}
	/*
	size += get_node_count(packet->users) * sizeof(int);
	*/
	size += sizeof(int);
	if (get_node_count(packet->users)) {
		for (n = packet->users->head; n; n = n->next) {
			size += sizeof(int);
			if (n->data) {
				size += strlen((char *)n->data) * 2;
			} else {
				fprintf(stderr, "fault in queue nodes!!\n");
			}
		}
	}
#else
	size += sizeof(int);
	size += packet->name_len * 2;
	size += sizeof(int);
	size += packet->data_len * 2;
	size += sizeof(int);
	size += packet->to_len * 2;
	size += sizeof(int);
	size += packet->list_size;
#endif

	/* TODO: reevaluate the correctness here */
	*psize = size/* + sizeof(int)*/;

	buffer = malloc(size + sizeof(int));
	write_int_to_buffer(buffer, &global_index, packet->code);
	/*
	iptr = (int *)(buffer + global_index);
	*iptr = htonl(size);
	global_index += sizeof(int);
	*/
	

#ifdef DEBUG
	if ((packet->name) && (packet->name_len != (int)strlen(packet->name))) {
		fprintf(stderr, "fault with length parameters of packet. fixing but find issue\n");
		packet->name_len = strlen(packet->name);
	}
#endif
	if (packet->name) {
		write_int_to_buffer(buffer, &global_index, packet->name_len);
		write_string_to_buffer(buffer, &global_index, packet->name_len, packet->name);
	} else {
		write_int_to_buffer(buffer, &global_index, 0);
	}

#ifdef DEBUG
	if ((packet->data) && (packet->data_len != (int)strlen(packet->data))) {
		fprintf(stderr, "fault with length parameters of packet. fixing but find issue\n");
		packet->data_len = strlen(packet->data);
	}
#endif
	if (packet->data) {
		write_int_to_buffer(buffer, &global_index, packet->data_len);
		write_string_to_buffer(buffer, &global_index, packet->data_len, packet->data);
	} else {
		write_int_to_buffer(buffer, &global_index, 0);
	}

#ifdef DEBUG
	if ((packet->to) && (packet->to_len != (int)strlen(packet->to))) {
		fprintf(stderr, "fault with length parameters of packet. fixing but find issue\n");
		packet->to_len = strlen(packet->to);
	}
#endif
	if (packet->to) {
		write_int_to_buffer(buffer, &global_index, packet->to_len);
		write_string_to_buffer(buffer, &global_index, packet->to_len, packet->to);
	} else {
		write_int_to_buffer(buffer, &global_index, 0);
	}

#ifdef DEBUG
	if ((packet->users) && (packet->list_len != get_node_count(packet->users))) {
		fprintf(stderr, "fault with length parameters of packet. fixing but find issue\n");
		packet->list_len = get_node_count(packet->users);
	}
#endif
	if (packet->users) {
		write_int_to_buffer(buffer, &global_index, packet->list_len);
		for (n = packet->users->head; n; n = n->next) {
			len = (int)strlen((char *)n->data);
			write_int_to_buffer(buffer, &global_index, len);
			write_string_to_buffer(buffer, &global_index, len, (char *)n->data);
		}
	} else {
		write_int_to_buffer(buffer, &global_index, 0);
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
	if (name_len) {
		name = read_string_from_buffer(bytes, &global_index, name_len);
	}

	data_len = read_int_from_buffer(bytes, &global_index);
	if (data_len) {
		data = read_string_from_buffer(bytes, &global_index, data_len);
	}

	to_len = read_int_from_buffer(bytes, &global_index);
	if (to_len) {
		to = read_string_from_buffer(bytes, &global_index, to_len);
	}

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
	short *temp = 0;;

	string = malloc(length + 1);

	for (i = 0; i < length; i++) {
		temp = (short *)&bytes[*global_index + (2 * i)];
		string[i] = (char) ntohs(*temp);
	}

	string[length] = '\0';
	*global_index += length * 2;
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

void write_int_to_buffer(char *buffer, int *global_index, int integer)
{
	int *iptr = NULL;
	iptr = (int *)(buffer + *global_index);
	*iptr = htonl(integer);
	*global_index += sizeof(int);
}


void write_string_to_buffer(char *buffer, int *global_index, int length, char *string)
{
	int i;
	short *ch;
	for (i = 0; i < length; i++) {
		ch = (short *)(buffer + *global_index + (2 * i));
		*ch = htons((short)string[i]);
		/*
		buffer[*global_index + (2 * i)] = ch;
		*/
	}
	*global_index += i * 2;
}
