#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "users.h"
#include "server_listener.h"
#include "server_speaker.h"

int main (void) 
{
	pthread_t listen_thread;
	pthread_t speak_thread;
	users_t *users = NULL;
	server_speaker_t *speaker;
	server_listener_t *listener;
	int *ports;
	char line[100];

	ports = malloc(sizeof(int));
	ports[0] = 8002;

	printf("Starting up server\n");

	users = new_users();
	speaker = new_server_speaker(users);
	listener = new_server_listener(ports, users, speaker);

	pthread_create(&listen_thread, NULL, listener_run, (void *)listener);
	pthread_create(&speak_thread, NULL, speaker_run, (void *)speaker);

	printf("Server running\n");

	while(TRUE) {
		if (!scanf("%s", line)) {
		;		
		}
		if (strcmp(line, "quit") == 0) {
			break;
		} else if(strcmp(line, "quit") == 0) {
			printf("Server running\n");
		}
	}

	listener_stop(listener);
	pthread_join(listen_thread, NULL);
	pthread_join(speak_thread, NULL);
	return 0;
}
