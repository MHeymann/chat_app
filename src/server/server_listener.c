#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> /* inet_addr */
#include <unistd.h> /* write */
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

/*** Macros **************************************************************/

#define BUFFER_SIZE		2048
#define DEFAULT_PORT	8080
#define TRUE			1
#define FALSE			0
/* TODO: less hardcoded with hashtable */
#define MAX_CLIENTS		(int) sizeof(long)
	

/*
pthread_t *threads = NULL;
int thread_count = -1;
int running = TRUE;
pthread_mutex_t *thread_lock = NULL;
pthread_mutex_t *running_lock = NULL;
*/
int port;

/*** Function Prototypes *************************************************/

/*
void *connection_handler(void *socket_desc);
void *terminal_manager(void *some_pointer);
int init_globals();
void free_globals();
*/

/*** Main Routine ********************************************************/

int main(int argc, char *argv[])
{
	int master_socket = -1, new_socket = -1, i = 0;
	int valread = -1, sd = 0;
	int client_socket[MAX_CLIENTS];
	int opt = TRUE;
	int addrlen = -1;
	int activity;
	int max_sd;
	struct sockaddr_in address;
	char *message = NULL;
	unsigned int write_bytes;
	char *endptr;
	char buffer[BUFFER_SIZE];
	fd_set readfds;

	port = DEFAULT_PORT;

	if (argc > 1) {
		port = strtol(argv[1], &endptr, 10);
		if (endptr == argv[1]) {
			printf("Please provide a valid port number.\n");
			printf("Defaulting to port %d\n", DEFAULT_PORT);
			port = DEFAULT_PORT;
		}
	}

	printf("Port %d selected\n", port);

	/*
	if (!init_globals()) {
		free_globals();
		exit(1);
	}
	*/

	for (i = 0; i < MAX_CLIENTS; i++) {
		client_socket[i] = 0;
	}
	;

	/* Create master socket */
	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
		perror("socket failed\n");
		exit(EXIT_FAILURE);
	}

	/* good habit, not necessary: set master socket to allow multiple
	 * connections/
	 */
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
		perror("setsockopt\n");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	/* Bind */
	if (bind(master_socket, (struct sockaddr *)&address, 
				sizeof(address)) < 0) {
		perror("bind failed\n");
		return 2;
	}
	printf("Bind to %d done\n", port);

	/* listen */
	if (listen(master_socket, 3) < 0) {
		perror("listen\n");
		exit(EXIT_FAILURE);
	}


	/* Accept incoming connections */
	printf("Waiting for incoming connections...\n");
	addrlen = sizeof(address);
	while (TRUE) {
		/*
	while ((new_socket = accept(socket_desc, (struct sockaddr*)&client,
					(socklen_t *)&addrlen))) {
					*/
		/* clear the socket set */
		FD_ZERO(&readfds);

		/* add master for listening */
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		/* all sockets currently opened added to set */
		for (i = 0; i < MAX_CLIENTS; i++) {
			sd = client_socket[i];

			if (sd > 0) {
				FD_SET(sd, &readfds);
			}

			if (sd > max_sd) {
				max_sd = sd;
			}
		}

		/* ret	 = select(nfds, readfds, writefds, exceptfds, timeout); */
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR)) {
			printf("select error\n");
		}

		/* master socket, => incoming connection */
		if (FD_ISSET(master_socket, &readfds)) {
			if ((new_socket = accept(master_socket, 
							(struct sockaddr *)&address,
							(socklen_t *)&addrlen)) < 0) {
				perror("accept\n");
				exit(EXIT_FAILURE);
			}

			/* Inform user of socket number - useful in send 
			 * and receive actions */
			printf("New connection: \nsocket fd: \t%d\nip: \t%s\nport:\t%d\n",
					new_socket, inet_ntoa(address.sin_addr), 
					ntohs(address.sin_port));
			/* Reply to the client */
			message = "Hi! You are now connected and being assigned a handler\n";
			write_bytes = write(new_socket, message, strlen(message));
			if (write_bytes != strlen(message)) {
				fprintf(stderr, "failed to write");
				perror("send\n");
			}
			printf("Sent Welcome message\n");

			/* add to array */
			for (i = 0; i < MAX_CLIENTS; i++) {
				if (!client_socket[i]) {
					client_socket[i] = new_socket;
					printf("Socket added to array at index %d\n", i);
					break;
				}
			}
		}

		/* IO on other sockets */

		for (i = 0; i < MAX_CLIENTS; i++) {
			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds)) {
				/* was it for closing or receiving a message? */
				if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
					/* disconnection, display details */
					getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
					printf("Host disconnected: \nip %s, port %d\n", 
							inet_ntoa(address.sin_addr), ntohs(address.sin_port));

					/* close the socket and mard as 0 */
					close(sd);
					client_socket[i] = 0;
				} else {
					/* I am the echo in the cave */
					buffer[valread] = '\0';
					send(sd, buffer, strlen(buffer), 0);
				}
			}
		}
	}


	/*
		new_sockptr = malloc(sizeof(int));
		*new_sockptr = new_socket;
		
		pthread_mutex_lock(thread_lock);
		if (pthread_create(&threads[thread_count], NULL, connection_handler, 
					(void *) new_sockptr) < 0) {
			fprintf(stderr, "could not create a new thread\n");
			return 3;
		}
		thread_count++;
		pthread_mutex_unlock(thread_lock);

		printf("handler assigned\n");
	}

	free(threads);	

	if (new_socket < 0) {
		fprintf(stderr, "accept failed\n");
		return 4;
	}


	free_globals();
	*/

	return 0;
}

/*** Functions ***********************************************************/

/*
int init_globals() 
{
	if ((thread_lock = malloc(sizeof(pthread_mutex_t))) == NULL) {
		fprintf(stderr, "failed to initialize lock\n");
		return FALSE;
	}
	if (0 != pthread_mutex_init(thread_lock, NULL)) {
		return FALSE;
	}

	if ((running_lock = malloc(sizeof(pthread_mutex_t))) == NULL) {
		fprintf(stderr, "failed to initialize lock\n");
		return FALSE;
	}
	if (0 != pthread_mutex_init(running_lock, NULL)) {
		return FALSE;
	}

	threads = malloc(sizeof(pthread_t) * MAX_CLIENTS);
	if (!threads) {
		return FALSE;
	}
	thread_count = 0;
	return TRUE;
}
*/

/*
void free_globals()
{
	int i;

	if (threads) {
		for (i = 0; i < thread_count; i++) {
			pthread_join(threads[i], NULL);
		}
		free(threads);
		threads = NULL;
	}

	if (thread_lock) {
		pthread_mutex_destroy(thread_lock);
		free(thread_lock);
		thread_lock = NULL;
	}
	if (running_lock) {
		pthread_mutex_destroy(running_lock);
		free(running_lock);
		running_lock = NULL;
	}

}

*/

	/*
void *connection_handler(void *socket_desc)
{
	int sock = *(int *)socket_desc;
	int read_size = -1;
	char *message, client_message[BUFFER_SIZE];
	int write_bytes;
	*/

	/* Send message to the client */
	/*

	message = "Greetings! I am your Lord Connection Handler\n";
	write_bytes = write(sock, message, strlen(message));
	if (write_bytes == 0) {
		fprintf(stderr, "failed to write");
	}

	message = "Type a message and I will say the same: \n";
	write_bytes = write(sock, message, strlen(message));
	if (write_bytes == 0) {
		fprintf(stderr, "failed to write");
	}

	*/
	/* Receive messages from the client */
	/*
	while ((read_size = recv(sock, client_message, BUFFER_SIZE, 0)) > 0) {
		client_message[read_size] = '\0';
		write_bytes = write(sock, client_message, strlen(client_message) + 1);
		if (write_bytes == 0) {
			fprintf(stderr, "failed to write");
		}

	}

	if (read_size == 0) {
		printf("Disconnected client\n");
		fflush(stdout);
	} else if (read_size == -1) {
		fprintf(stderr, "Failed to receive\n");
	}

	free(socket_desc);

	return NULL;
}
*/


/*
void *terminal_manager(void *some_pointer) 
{
	long number = (long) some_pointer;
	if (number) {
		printf("%ld, please provide commands as text\n", number);
	}
	return NULL;
}
*/
