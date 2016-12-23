#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "users.h"
#include "server_listener.h"
#include "server_speaker.h"


char ch = '\0';

void read_line(FILE *f, char *line);
void get_args(int argc, char *argv[], int **ports, int *port_count);

int main(int argc, char *argv[]) 
{
	pthread_t listen_thread;
	pthread_t speak_thread;
	users_t *users = NULL;
	server_speaker_t *speaker;
	server_listener_t *listener;
	int *ports;
	char line[100];
	int port_count = 0;

	get_args(argc, argv, &ports, &port_count);	

	printf("Starting up server\n");

	users = new_users();
	speaker = new_server_speaker(users);
	listener = new_server_listener(ports, port_count, users, speaker);

	pthread_create(&listen_thread, NULL, listener_run, (void *)listener);
	pthread_create(&speak_thread, NULL, speaker_run, (void *)speaker);

	printf("Server running\n");

	while(TRUE) {
		/*
		if (!scanf("%s", line)) {
			printf("Something weird in scanf\n");		
		}
		*/
		read_line(stdin, line);
		if (strcmp(line, "quit") == 0) {
			break;
		} else if(strcmp(line, "exit") == 0) {
			break;
		} else if(strcmp(line, "status") == 0) {
			printf("Server running\n");
		} else {
			if (ch == EOF) {
				printf("exit\n");
				break;
			} else {
				printf("Didn't get that!\n");
				printf("you said %s\n", line);
			}
		}
	}

	listener_stop(listener);
	speaker_stop(speaker);
	printf("Joining speaker\n");
	pthread_join(speak_thread, NULL);
	printf("Joined speaker\n");
	printf("Joining listener\n");
	pthread_join(listen_thread, NULL);
	printf("Joined listener\n");

	free_users(users);
	users = NULL;
	server_listener_free(listener);
	listener = NULL;
	server_speaker_free(speaker);
	speaker = NULL;

	free(ports);

	return 0;
}

void read_line(FILE *f, char *line)
{
	int i;
	for (i = 0; TRUE; i++) {
		ch = (char)fgetc(f);
		switch (ch) {
			case EOF:
				/* fall through */
			case '\n':
				line[i] = '\0';
				return;
			default:
				line[i] = ch;
		}
	}
}


void get_args(int argc, char *argv[], int **ports, int *port_count)
{
	int i = 0;
	char *endptr = NULL;
	if (argc <= 1) {
		*port_count = 1;
		*ports = malloc(sizeof(int));
		(*ports)[0] = 8002;
		printf("defaulting to port %d", (*ports)[0]);
	} else {
		*port_count = argc - 1;
		*ports = malloc(sizeof(int) * *port_count);
		for (i = 0; i < *port_count; i++) {
			printf("%d %s\n", i, argv[i + 1]);
			(*ports)[i] = strtol(argv[i + 1], &endptr, 10);
			printf("%d set for binding\n", (*ports)[i]);
			if (argv[i + 1] == endptr) {
				printf("please provide valid integers as arguments\n");
				printf("you erroneously provided '%s' as argument %d\n",
						argv[i + 1], i + 1);
				exit(1);
			}
		}
	}
}
