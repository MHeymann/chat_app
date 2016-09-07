#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat_client.h"

/*** Struct Definition ***************************************************/

/*
typedef struct client {
	pthread_t *listen_thread;
	client_speaker_t *speaker;
	client_listener_t *listener;
	int connected_status;
	pthread_mutex_t *connection_mutex;
	char *username;
	char *hostname;
	int hostport;
} chat_client_t;
*/

/*** Helper Function Prototypes ******************************************/

char *client_strdup(char *s);
void broken_connection(chat_client_t *client);
void read_line(FILE *f, char *line);

char ch = '\0';

/*** Main Function *******************************************************/

int main (int argc, char *argv[]) 
{
	chat_client_t *client = NULL;
	char line[1024];
	char *localhost = "localhost";
	char *username = NULL;
	char *password = NULL;
	char *hostname = NULL;
	char *message = NULL;
	char *to = NULL;
	char *endptr;
	int host_port;
	int ret;
	int i;

	printf("%s", argv[0]);
	for (i = 1; i < argc; i++) {
		printf(" %s", argv[i]);
	}
	printf("\n");


	if ((client = new_client()) == NULL) {
		fprintf(stderr, "Failed to create client\n");
		exit(1);
	}
ESTABLISH_CONNECTION:
	while (client->connected_status == FALSE) {
		printf("Please provide a host address for the server: ");
		if (0 == scanf("%s", line)) {
			fprintf(stderr, "problem reading username\n");
			continue;
		}
		if (!strcmp(line, localhost)) {
			hostname = client_strdup("127.0.0.1");
		} else {
			hostname = client_strdup(line);
		}

		printf("Please provide the port on which the server is listening: ");
GIVE_PORT:
		if (0 == scanf("%s", line)) {
			printf("problem getting port, please try again: ");
			goto GIVE_PORT;
		}
		host_port = strtol(line, &endptr, 10);
		if (endptr == line) {
			printf("Invalid port provided, please try again: ");
			goto GIVE_PORT;
		}
	
GIVE_UNAME:
		printf("Please provide a username: ");
		if (0 == scanf("%s", line)) {
			fprintf(stderr, "problem reading username\n");
			goto GIVE_UNAME;
		}
		username = client_strdup(line);
	
		printf("Please provide a password: ");
		if (0 == scanf("%s", line)) {
			fprintf(stderr, "problem reading password\n");
		}
		password = client_strdup(line);
	
		
		printf("About to connect to %s:%d\n", hostname, host_port);
		printf("Login details\n");
		printf("\tusername: %s\n", username);
		printf("\tpassword: %s\n", password);

		client->username = username;
		client->hostname = hostname;
		client->hostport = host_port;

	
		if ((client->speaker = new_client_speaker(username, hostname, host_port)) == NULL) {
			fprintf(stderr, "Failed to create speaker\n");
			free_chat_client(client);
			/* TODO: set memory address values to 0 */
			free(password);
			password = NULL;
			exit(2);
		}
		if (!speaker_login(client->speaker, password)) {
			fprintf(stderr, "Failed to login\n");
			free_chat_client(client);
			client = new_client();
			/* TODO: set memory address values to 0 */
			free(password);
			password = NULL;
			/*
			exit(3);
			*/
		} else {
			client->connected_status = TRUE;
			/* TODO: set memory address values to 0 */
			free(password);
			password = NULL;
		}
	} /* while connecting */

	if ((client->listener = new_client_listener(client->speaker->sd, client, username)) == NULL) {
		fprintf(stderr, "Failed to create listener\n");
		free_chat_client(client);
		exit(4);
	}

	pthread_create(client->listen_thread, NULL, run_client_listener, (void *)client->listener);
	
	printf("Please type 'help<enter>' for available options\n");
	while(TRUE) {
		printf(">> ");
		read_line(stdin, line);
		ret = 1;
		/*
		ret = scanf("%s", line);
		*/
		if (ret != 1) {
			printf("some problem scanning for input\n");
		} else if (strcmp(line, "help") == 0) {
			printf("The following commands are supported:\n");
			printf("\t\x1b[1;34msend\x1b[0m: \t\tSend a message\n");
			printf("\t\x1b[1;34mbroadcast\x1b[0m: \tBroadcast a message to all users\n");
			printf("\t\x1b[1;34mecho\x1b[0m: \t\tEcho a message to yourself\n");
			printf("\t\x1b[1;34mexit\x1b[0m: \t\tShut down this chat client\n");
			printf("\t\x1b[1;34mquit\x1b[0m: \t\tSee exit\n");
		} else if (strcmp(line, "send") == 0) {
			printf("Type the user name of your recipient: \n");
			printf(">> ");
			read_line(stdin, line);
			/*
			ret = scanf("%s", line);
			if (!ret) {
				printf("some error entering recipient name\n");
				continue;
			}
			*/
			to = client_strdup(line);
			if (!to) {
				printf("some error copying recipient name\n");
				continue;
			}
			printf("Type the message to be sent: \n");
			printf(">> ");
			read_line(stdin, line);
			/*
			ret = scanf("%s", line);
			if (!ret) {
				printf("some error entering message\n");
				free(to);
				to = NULL;
				continue;
			}
			*/
			message = client_strdup(line);
			if (!message) {
				printf("some error copying message\n");
				free(to);
				to = NULL;
				continue;
			}
			if (send_string(client->speaker, message, to)) {
				printf("%s to %s : %s\n", client->username, to, message);
			} else {
				printf("Some error sending message\n");
			}
			free(to);
			to = NULL;
			free(message);
			message = NULL;
		} else if (strcmp(line, "broadcast") == 0) {
			printf("Type the message to be broadcast: \n");
			printf(">> ");
			read_line(stdin, line);
			/*
			ret = scanf("%s", line);
			if (!ret) {
				printf("some error entering message\n");
				continue;
			}
			*/
			message = client_strdup(line);
			if (!message) {
				printf("some error copying message\n");
				continue;
			}

			if (broadcast_string(client->speaker, message)) {
			} else {
				printf("Some error broadcasting message\n");
			}

			free(message);
			message = NULL;
		} else if (strcmp(line, "echo") == 0) {
			printf("Type the message to be echoed: \n");
			printf(">> ");
			read_line(stdin, line);
			/*
			ret = scanf("%s", line);
			if (!ret) {
				printf("some error entering message\n");
				continue;
			}
			*/
			message = client_strdup(line);
			if (!message) {
				printf("some error copying message\n");
				continue;
			}

			if (echo_string(client->speaker, message)) {
			} else {
				printf("Some error echoing message\n");
			}

			free(message);
			message = NULL;
		} else if (strcmp(line, "listusers") == 0) {
			get_online_names(client->speaker);
		} else if (strcmp(line, "logout") == 0) {
			printf("Stopping listener\n");
			stop_listener(client->listener);
			printf("Joining listen thread\n");
			pthread_join(*(client->listen_thread), NULL);
			printf("Notifying server\n");
			speaker_logoff(client->speaker);
			free_chat_client(client);
			client = new_client();
			goto ESTABLISH_CONNECTION;
		} else if ((strcmp(line, "exit") == 0) || 
			(strcmp(line, "quit") == 0)) {
			break;
		} else {
			printf("Please type 'help<enter>' for available options\n");
		}
	}

	speaker_logoff(client->speaker);
	printf("Stopping listener\n");
	stop_listener(client->listener);
	printf("Joining listen thread\n");
	pthread_join(*(client->listen_thread), NULL);
	free_chat_client(client);

	return 0;
}


/*** Functions ***********************************************************/

chat_client_t *new_client()
{
	chat_client_t *client = malloc(sizeof(chat_client_t));
	if (!client) {
		return NULL;
	} 

	/* init all as NULL or similar */
	client->connected_status = FALSE;
	/*
	client->speak_thread = NULL;
	*/
	client->listen_thread = NULL;
	client->speaker = NULL;
	client->listener = NULL;
	client->connection_mutex = NULL;
	client->username = NULL;
	client->hostname = NULL;
	client->hostport = DEFAULT_PORT;

	/*
	client->speak_thread = malloc(sizeof(pthread_t));
	*/
	client->listen_thread = malloc(sizeof(pthread_t));

	if (/*(!client->speak_thread) ||*/ (!client->listen_thread)) {
		free_chat_client(client);
		return NULL;
	}

	client->connection_mutex = malloc(sizeof(pthread_mutex_t));
	if (!client->connection_mutex) {
		free_chat_client(client);
		return NULL;
	}
	pthread_mutex_init(client->connection_mutex, NULL);

	return client;
}

void free_chat_client(chat_client_t *client)
{
	if (client->listen_thread) {
		/* must be joined */
		free(client->listen_thread);
		client->listen_thread = NULL;
	}
	/*
	if (client->speak_thread) {
		free(client->speak_thread);
		client->speak_thread = NULL;
	}
	*/
	if (client->speaker) {
		free_client_speaker(client->speaker);
		client->speaker = NULL;
	}
	if (client->listener) {
		free_client_listener(client->listener);
		client->listener = NULL;
	}
	if (client->connection_mutex) {
		pthread_mutex_destroy(client->connection_mutex);
		free(client->connection_mutex);
		client->connection_mutex = NULL;
	}
	if (client->username) {
		free(client->username);
		client->username = NULL;
	}
	if (client->hostname) {
		free(client->hostname);
		client->hostname = NULL;
	}

	free(client);
}

void client_append(chat_client_t *client, char *s)
{
	if (!client) {
		return;
	}
	printf("%s:: %s", client->username, s);
}

void client_show_online_users(chat_client_t *client, queue_t *users_q)
{
	node_t *node = NULL;
	printf("Online Users being shown to %s:\n", client->username);
	for (node = users_q->head; node; node = node->next) {
		printf("\t- %s\n", (char *)node->data);
	}
	printf("shown\n>> ");
}

/*** Helper Functions ****************************************************/

char *client_strdup(char *s)
{
	char *c = malloc(strlen(s) + 1);
	int i = 0;
	int j = 0;

	while ((c[i++] = s[j++]));

	return c;
}

void read_line(FILE *f, char *line)
{
	int i;
	for (i = 0; TRUE; i++) {
		ch = (char)fgetc(f);
		switch (ch) {
			case EOF:
				/* fall through */
				printf("EOF");
			case '\n':
				line[i] = '\0';
				return;
			default:
				line[i] = ch;
		}
	}
}
