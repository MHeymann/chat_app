/*
 * This lookuptable is specifically for the implementing a* with the aim
 * of solving an n-puzzle.
 *
 * It uses a generic hashtable, based on what Willem Bester made us use in
 * Computer Science 244.  
 *
 * “When the hurlyburly's done,
 * When the battle's lost and won.”
 *				― William Shakespeare, Macbeth
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "hashtable.h"
#include "fd_hashset.h"

/*** Lookuptable struct Description **************************************/

typedef struct fd_hashset {
	hashtable_p ht;
} fd_hashset_t;

/*** Helper Function Prototypes ******************************************/

unsigned long hash_fd(void *key, unsigned int size);
int cmp_fd_strings(void *a, void *b);
void fd_val2str(void *key, void *val, char *buffer);
void fd_dud_free(void *);
char *fd_strdup(char *s);
void *copy_fd_key(void *a);
int cmp_fds(void *a, void *b);

/*** Functions ***********************************************************/

/**
 * Allocate the datastructures of a fd_hashset and initialise the parameters.
 *
 * @param[in][out] ht a pointer to a pointer of the newly created fd_hashset.
 */
void fd_hashset_init_defaults(fd_hashset_ptr *hs)
{
	fd_hashset_init(hs, -1, -1);
}	

/**
 * Allocate the datastructures of a fd_hashset and initialise the parameters.
 *
 * @param[in][out] ht		a pointer to a pointer of the newly created 
 *							fd_hashset.
 * @param[in] init_delta	The power to raise two by to get the initial
 *							size of the underlying hashtable.
 * @param[in] delta_diff	The integer number to increment the delta
 *							value by when resizing the underlying 
 *							hashtable.
 */
void fd_hashset_init(fd_hashset_ptr *hs, int init_delta, int delta_diff)
{
	hashtable_p ht = NULL;
	fd_hashset_ptr hset = NULL;

	hset = malloc(sizeof(fd_hashset_t));
	if (!hset) {
		fprintf(stderr, "Error allocating memory for lookuptable.\n");
		*hs = NULL;
		return;
	}

	ht = ht_init(0.75f, init_delta, delta_diff, hash_fd, cmp_fds);
	if (!ht) {
		fprintf(stderr, "Error initializing hashtable.\n");
		free(hset);
		*hs = NULL;
		return;
	}

	hset->ht = ht;

	*hs = hset;
}

void fd_hashset_update(fd_hashset_ptr hs, int fdkey, char *svalue)
{
	long fdl;
	char *scopy = fd_strdup(svalue);
	if (!scopy) {
		fprintf(stderr, "failed to copy string\n");
	}

	fdl = fdkey;
	ht_update(hs->ht, (void *)fdl, (void *)scopy, free);
}

/**
 * Insert a string instance into the fd_hashset.
 *
 * @param[in] hs A pointer to the fd_hashset into which to insert.
 *
 * @param[in] skey A pointer to the string to insert.
 *
 * @return SUCCESS(1) if the game was inserted successfully and
 * FAIL(0) if not.
 */
int fd_hashset_insert(fd_hashset_ptr hs, int fdkey, char *svalue)
{
	int insert_status;
	char *scopy = fd_strdup(svalue);
	long fdl;
	if (!scopy) {
		fprintf(stderr, "failed to copy string\n");
	}

	fdl = fdkey;
	printf("about to insert fd into table\n");
	insert_status = ht_insert(hs->ht, (void *)fdl, (void *)scopy);
	printf("inserted fd into table\n");
	if (insert_status) {
		switch (insert_status) {
			case 1:
				fprintf(stderr, 
						"Memory error when inserting into hashtable\n");
				break;
			case KEY_PRESENT_IN_TABLE:
				/*just means it is present.*/
#ifdef DEBUGHS
				fprintf(stdout, "already in lookuptable\n");
#endif
				break;
			default:
				fprintf(stderr, 
						"This is weird in fd_hashset_insert table's switch statement: %d\n", 
						insert_status);
				break;
		}
		free(scopy);
		return FAIL;
	} else {
		return SUCCESS;
	}
}

void fd_hashset_remove(fd_hashset_ptr hs, int key)
{
	long lkey = (long) key;
	ht_remove(hs->ht, (void *)lkey, fd_dud_free, free);
}

char *fd_get_name(fd_hashset_t *fdhs, int fd) 
{
	void *name = NULL;
	long fdl = fd;
	if (ht_lookup(fdhs->ht, (void *)fdl, &name)) {
		return fd_strdup((char *)name);
	} else {
		return NULL;
	}
}

/**
 * Get a count of the total number of items in the fd_hashset.
 *
 * @param[in] hs A pointer to the fd_hashset for which to get a count.
 *
 * @return An integer of the total number of items in the fd_hashset.
 */
int fd_hashset_content_count(fd_hashset_ptr hs)
{
	return ht_item_count(hs->ht);
}



/**
 *	Print the underlying hashtable of the fd_hashset. 
 *
 *	@param[in] hs A pointer to the fd_hashset to print.  
 */
void print_fd_hashset(fd_hashset_ptr hs)
{
	print_ht(hs->ht, fd_val2str);	
}

/**
 * Free the values in the fd_hashset and the fd_hashset itself. 
 *
 * @param[in] hs The fd_hashset to be free'd.  
 */
void free_fd_hashset(fd_hashset_ptr hs)
{
	/* TODO: close file descriptor? */
	ht_free(hs->ht, fd_dud_free, free);
	hs->ht = NULL;
	free(hs);
}

queue_t *fdhs_get_keys(fd_hashset_ptr fd_hs)
{
	return get_keys(fd_hs->ht, copy_fd_key, cmp_fds, fd_dud_free);
}

/*** Helper Functions ****************************************************/

void *copy_fd_key(void *a)
{
	long b = (long)a;
	return (void *)b;
}

int cmp_fds(void *a, void *b) 
{
	long A, B;
	int ret;
	A = (long)a;
	B = (long)b;
	ret = (int)(A - B);
	return ret;
}

/**
 * Get a string representation of a key value pair, to be put in buffer.
 *
 * @param[in] key The unique key of the pair, used to hash with.   
 *
 * @param[in] val The value of the pair.
 *
 * @param[out] buffer The char pointer where the string representation 
 * of the pair will be put.  
 */
void fd_val2str(void *key, void *val, char *buffer)
{
	char *name = (char *)val;
	long fd = (long) key;
	struct sockaddr_in address;
	socklen_t addrlen;

	getsockname((int)fd, (struct sockaddr *)&address, &addrlen);

	sprintf(buffer, "name: %s, socket fd: \t%ld, ip: \t%s, port:\t%d", name, fd, 
			inet_ntoa(address.sin_addr), 
			ntohs(address.sin_port));	
}


/** 
 * Just a dud function, as in our case, the key and the value are the
 * same thing.  
 * The stuff inside are simply to keep the warning flags off.  
 *
 * @param[in] key The key that doesn't need freeing. 
 */
void fd_dud_free(void *key) 
{
	if ((long)key & 0) {
		return;
	} else {
		return;
	}
}

/**
 * A hash function to be used when placing games in the hashtable.  
 *
 * @param[in] key The key to be used to hash with.  
 *
 * @param[in] size The size of the table into which to put the 
 * key value pair.  
 *
 * @return The hash value as an unsigned long. 
 */
unsigned long hash_fd(void *key, unsigned int size)
{
	long fd = (long)key;

	return (fd % size);
}

/**
 * A comparison function used to compare two string when trying 
 * to put a value into the hashtable. 
 *  @param[in] a A char pointer
 *
 *  @param[in] b A char pointer
 *
 *  @return An integer value, 0 if the two strings are identical
 *  less than 0 if a comes before b and more than 0 otherwise. 
 */
int cmp_fd_strings(void *a, void *b) 
{
	return strcmp((char *)a, (char *)b);
}


char *fd_strdup(char *s) {
	int i;
	int len = strlen(s);
	char *scopy = malloc(len + 1);
	for (i = 0; i < len; i++) {
		scopy[i] = s[i];
	}
	scopy[len] = '\0';
	return scopy;
}

