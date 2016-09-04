/*
 * This hashtable was very closely modelled after one given by Willem
 * Bester for our compiler project in Computer Science 244.  I do not
 * claim that this is my own work, although some names of functions
 * may differ, and some functions have been slightly adjusted to add
 * some minor funcionality.  Again, I do not claim this to be my own
 * work.
 *
 * “Of course it is happening inside your head, Harry, but why on earth 
 * should that mean that it is not real?”
 *					― J.K. Rowling, Harry Potter and the Deathly Hallows
 */
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "../queue/queue.h"

/*** Macros **************************************************************/

#define KEY_PRESENT_IN_TABLE   -1
#define SUCCESS					1
#define FAIL					0

/*** Type Definitions ****************************************************/

typedef struct hashtable *hashtable_p;

/*** Function Prototypes *************************************************/

/**
 * Initilise the structures required for the hashtable. 
 *
 * @param[in] factor	 The fraction at which the table must resize
 * @param[in] init_delta The initial power by which to raise 2 to get the
 *						 initial size of the hashtable.
 * @param[in] init_diff	 The increment by which to increase the delta
 *						 value when resizing the hashtable.
 * @param[in] hash		 The hashing function to use on keys.
 * @param[in] cmp		 The comparison function to use when comparing
 *						 keys.
 *
 * @return A pointer to a newly allocated hashtable
 */
hashtable_p ht_init(float factor, int init_delta, int delta_diff, 
		unsigned long (*hash)(void *key, unsigned int size), 
		int (*cmp)(void *a, void *b));

void ht_update(hashtable_p ht, void *key, void *value, void (*val_free)(void *));

/**
 * Attempt to insert a key value pair.
 *
 * @param[in] ht		The table into which to attempt to put the pair.  
 *
 * @param[in] key		The unique key, to be used for hashing and 
 *						comparison with other entries in the table.  
 * @param[in] value		Thevalue of the pair to be put in the table.
 *
 * @return EXIT_SUCCESS (0) if successfully put in the table, 1 if memory
 * problems occur and KEY_PRESENT_IN_TABLE (-1) if the key is already in
 * the table. 
 */
int ht_insert(hashtable_p ht, void *key, void *value);

void ht_remove(hashtable_p ht, void *key, void (*freekey)(void *), void (*freeval)(void *));

/**
 *  Force an insert of a key value pair.
 *
 * @param[in] ht	The table into which to attempt to put the pair.  
 * @param[in] key	The unique key, to be used for hashing and comparison
 *					with other entries in the table.  
 * @param[in] Value The	value of the pair to be put in the table.
 *
 * @return EXIT_SUCCESS (0) if successfully put in the table, 1 if memory
 * problems occur. 
 */
int ht_force_insert(hashtable_p ht, void *key, void *value);

/** 
 * Get the total item count in the hashtable.
 *
 * @param ht A pointer to the hashtable in question.
 *
 * @return An integer of the total item count in the hashtable. 
 */
int ht_item_count(hashtable_p ht);

/**
 * Check to see if a given key value pair is present in the table.
 *
 * @param[in]	ht		The hashtable to look in.  
 * @param[in]	key		The key to use to compare with entries in the 
 *						hashtable.
 * @param[out]	value	A double pointer, where the value of the pair is
 *						stored, if found.
 *
 * @return SUCCESS (1) if found and FAIL (0) if not. 
 */ 
int ht_lookup(hashtable_p ht, void *key, void **value);

/**
 * Free the entries of the hashtable using the provided function pointers,
 * after which the table itself is also free'd.
 *
 * @param[in] ht		The hashtable to free
 * @param[in] freekey	A function pointer to free keys in the table with.
 * @param[in] freeval	A function pointer to free values in the table 
 *						with.
 */
void ht_free(hashtable_p ht, void (*freekey)(void *key),
		void (*freeval)(void *v));

/**
 * Print the hashtable using the provided function pointers. 
 *
 * @param[in] ht			The hashtable to print.
 * @param[in] val_to_str	A function to get a string representation
 *							of the key value pair with.
 */
void print_ht(hashtable_p ht, void (*val_to_str)(void *key, void *val,
			char *buffer));

void print_ht_entries(hashtable_p ht, void (*val_to_str)(void *key,
			void *val, char *buffer));

queue_t *get_keys(hashtable_p ht, void *(*copy_key)(void *key), int (*cmp_k)(void *, void *), void (*free_k)(void *));
#endif
