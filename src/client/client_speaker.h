#ifndef CLIENT_SPEAKER_H
#define CLIENT_SPEAKER_H


#define TRUE	1
#define FALSE	0

typedef struct client_speaker {
	char *name;
	char *hostname;
	int port;
	int sd;
} client_speaker_t;


client_speaker_t *new_client_speaker(char *name, char *hostname, int port);

void free_client_speaker(client_speaker_t *speaker);

void get_online_names(client_speaker_t *speaker);

int send_string(client_speaker_t *speaker, char *s, char *to);

int echo_string(client_speaker_t *speaker, char *s);

int broadcast_string(client_speaker_t *speaker, char *s);

int get_speaker_sd(client_speaker_t *speaker);

int speaker_login(client_speaker_t *speaker, char *pw);

int speaker_logoff(client_speaker_t *speaker);

#endif
