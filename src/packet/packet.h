#ifndef PACKET_H
#define PACKET_H
#include "../queue/queue.h"

/*** struct description **************************************************/

typedef struct packet {
	int code;

	int name_len;
	char *name;

	int data_len;
	char *data;

	int to_len;
	char *to;

	int list_len;
	int list_size;
	queue_t *users;

} packet_t;

/*** Function Prototypes *************************************************/

packet_t *new_empty_packet();

packet_t *new_packet(int code, char *name, char *data, char*to);

void free_packet(void *p);

void set_user_list(packet_t *packet, queue_t *users);

int get_code(packet_t *packet);

void set_code(packet_t *packet, int code);

char *get_data(packet_t *packet);

void set_data(packet_t *packet, char *data);

void send_packet(packet_t *packet, int fd);

packet_t *receive_packet(int fd);

#endif
