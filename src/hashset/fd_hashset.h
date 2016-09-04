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
#ifndef FD_HASHSET_H
#define FD_HASHSET_H

#include "../queue/queue.h"


/*** Macros **************************************************************/

#define TRUE	1
#define SUCCESS	1
#define FALSE	0
#define FAIL	0
#define HT_ERROR 5

/*** Type Definitions ****************************************************/

typedef struct fd_hashset *fd_hashset_ptr;

/*** Function Prototypes *************************************************/

/**
 * Allocate the datastructures of a fd_hashset and initialise the parameters.
 *
 * @param[in][out] ht a pointer to a pointer of the newly created fd_hashset.
 */
void fd_hashset_init_defaults(fd_hashset_ptr *hs);


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
void fd_hashset_init(fd_hashset_ptr *hs, int init_delta, int delta_diff);

/**
 * Insert a game instance into the fd_hashset.
 *
 * @param[in] hs   A pointer to the fd_hashset into which to insert.
 *
 * @return SUCCESS(1) if the game was inserted successfully and
 * FAIL(0) if not.
 */
int fd_hashset_insert(fd_hashset_ptr hs, int fdkey, char *svalue);

void fd_hashset_update(fd_hashset_ptr hs, int fdkey, char *svalue);

void fd_hashset_remove(fd_hashset_ptr hs, int key);

char *fd_get_name(fd_hashset_ptr shs, int fd);

/**
 * Get a count of the total number of items in the fd_hashset.
 *
 * @param[in] hs A pointer to the fd_hashset for which to get a count.
 *
 * @return An integer of the total number of items in the fd_hashset.
 */
int fd_hashset_content_count(fd_hashset_ptr hs);

/**
 *	Print the underlying hashtable of the fd_hashset. 
 *
 *	@param[in] hs A pointer to the fd_hashset to print.  
 */
void print_fd_hashset(fd_hashset_ptr hs);

/**
 * Free the values in the fd_hashset and the fd_hashset itself. 
 *
 * @param[in] hs The fd_hashset to be free'd.  
 */
void free_fd_hashset(fd_hashset_ptr hs);

queue_t *fdhs_get_keys(fd_hashset_ptr s_hs);

#endif
