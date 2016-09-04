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
#include <stdio.h>
#include <stdlib.h>

#include "hashtable.h"
#include "../queue/queue.h"

#define INITIAL_DELTA 23
#define INITIAL_DIFF 5
#define BUFFER_SIZE 1024

/*** Some Structs ********************************************************/

typedef struct ht_entry *ht_entry_p;
typedef struct ht_entry {
	void *key;
	void *val;
	ht_entry_p next_ptr;
} ht_entry_t;

typedef struct hashtable {
	ht_entry_p *table;
	unsigned int size;
	unsigned int num_entries;
	float loadfactor;
	unsigned int delta_index;
	int delta_diff;
	unsigned long (*hash)(void *, unsigned int);
	int (*cmp)(void *, void *);
} hashtable_t;

/*** Delta Table *********************************************************/

/*
 * This table is used to resize the hashtable to a large prime number.
 */
static unsigned int delta[] = { 0, 0, 1, 1, 3, 1, 3, 1, 5, 3, 3, 9, 3, 1, 
	3, 19, 15, 1, 5, 1, 3, 9, 3, 15, 3, 39, 5, 39, 57, 3, 35, 1 };

#define DELTA_SIZE (sizeof(delta) / sizeof(int))

/*** Helper Function Prototype *******************************************/

unsigned int calculate_table_size(hashtable_p ht);
ht_entry_p *alloc_table(hashtable_p ht);
void resize(hashtable_p ht);
int ht_insert_entry(hashtable_p ht, ht_entry_t *entry);

/*** Functions ***********************************************************/

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
		int (*cmp)(void *a, void *b))
{
	hashtable_p ht = NULL;
	ht = malloc(sizeof(hashtable_t));
	if (!ht) {
		fprintf(stderr, "Memory error in ht_init\n");
		return NULL;
	}
	/*If I'm here, table was allocated successfully */
	ht->table = NULL;	
	ht->size = -1;
	ht->num_entries = 0;
	ht->loadfactor = factor;
	ht->hash = hash;
	ht->cmp = cmp;

	if(init_delta < 0) {
		ht->delta_index = INITIAL_DELTA;
	} else {
		ht->delta_index = init_delta;
	}
	if (delta_diff < 0) {
		ht->delta_diff = INITIAL_DIFF;
	} else {
		ht->delta_diff = delta_diff;
	}


	ht->size = calculate_table_size(ht);
	ht->table = alloc_table(ht);
	if (ht->table == NULL) {
		return NULL;
	}

	return ht;
}

void ht_update(hashtable_p ht, void *key, void *value, void (*val_free)(void *)) 
{
	unsigned int hash = -1;
	ht_entry_p entry = NULL;

	hash = ht->hash(key, ht->size);

	for (entry = ht->table[hash]; entry; entry = entry->next_ptr) {
		if (ht->cmp(entry->key, key) == 0) {
			break;
		}
	}
	if (!entry) {
		return;
	}
	
	if (entry->val) {
		val_free(entry->val);
	}
	entry->val = value;
}

/**
 * Attempt to insert a key value pair.
 *
 * @param[in] ht	The table into which to attempt to put the pair.  
 * @param[in] key	The unique key, to be used for hashing and comparison
 *					with other entries in the table.  
 * @param[in] The	value of the pair to be put in the table.
 *
 * @return EXIT_SUCCESS (0) if successfully put in the table, 1 if memory
 * problems occur and KEY_PRESENT_IN_TABLE (-1) if the key is already in
 * the table. 
 */
int ht_insert(hashtable_p ht, void *key, void *value)
{
	unsigned int hash = -1;
	ht_entry_p entry = NULL;
	float loadfactor = 0.0;

	hash = ht->hash(key, ht->size);

	for (entry = ht->table[hash]; entry; entry = entry->next_ptr) {
		if (ht->cmp(entry->key, key) == 0) {
			break;
		}
	}
	if (entry) {
		return KEY_PRESENT_IN_TABLE;
	}

	entry = malloc(sizeof(ht_entry_t));

	if (!entry) {
		return 1;
	}

	entry->key = key;
	entry->val = value;

	entry->next_ptr = ht->table[hash];
	ht->table[hash] = entry;
	ht->num_entries++;

	loadfactor = (0.0f + ht->num_entries) / ht->size;
	if (loadfactor > ht->loadfactor) {
		printf("resize: %d, %f\n", ht->delta_index, loadfactor);
		resize(ht);
		loadfactor = (0.0f + ht->num_entries) / ht->size;
		printf("after resize: %d, %f\n", ht->delta_index, loadfactor);
	}
	
	return EXIT_SUCCESS;
}

void ht_remove(hashtable_p ht, void *key, void (*freekey)(void *), void (*freeval)(void *))
{
	unsigned int hash = -1;
	ht_entry_p entry = NULL;
	ht_entry_p temp = NULL;

	hash = ht->hash(key, ht->size);

	if (ht->table[hash] == NULL) {
		printf("Not found for removal\n");
		return;
	} else if (ht->cmp((ht->table[hash])->key, key) == 0) {
		temp = ht->table[hash];
		ht->table[hash] = (ht->table[hash])->next_ptr;
		temp->next_ptr = NULL;
		freekey(temp->key);
		temp->key = NULL;
		freeval(temp->val);
		temp->val = NULL;
		free(temp);
		ht->num_entries--;
		return;
	}
	for (entry = ht->table[hash]; entry->next_ptr; entry = entry->next_ptr) {
		if (ht->cmp(entry->next_ptr->key, key) == 0) {
			ht->num_entries--;
			break;
		}
	}

	if (entry->next_ptr) {
		temp = entry->next_ptr;
		entry->next_ptr = entry->next_ptr->next_ptr;
		temp->next_ptr = NULL;
		freekey(temp->key);
		temp->key = NULL;
		freeval(temp->val);
		temp->val = NULL;
		free(temp);
	}
}

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
int ht_force_insert(hashtable_p ht, void *key, void *value)
{
	unsigned int hash = -1;
	ht_entry_p entry = NULL;
	float loadfactor = 0.0;

	hash = ht->hash(key, ht->size);

	entry = malloc(sizeof(ht_entry_t));

	if (!entry) {
		return 1;
	}

	entry->key = key;
	entry->val = value;

	entry->next_ptr = ht->table[hash];
	ht->table[hash] = entry;
	ht->num_entries++;

	loadfactor = (0.0f + ht->num_entries) / ht->size;
	if (loadfactor > ht->loadfactor) {
		printf("resize: %d, %f\n", ht->delta_index, loadfactor);
		resize(ht);
		loadfactor = (0.0f + ht->num_entries) / ht->size;
		printf("after resize: %d, %f\n", ht->delta_index, loadfactor);
	}
	
	return EXIT_SUCCESS;
}

/** 
 * Get the total item count in the hashtable.
 *
 * @param ht A pointer to the hashtable in question.
 *
 * @return An integer of the total item count in the hashtable. 
 */
int ht_item_count(hashtable_p ht)
{
	return ht->num_entries;
}



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
int ht_lookup(hashtable_p ht, void *key, void **value)
{
	int compare;
	ht_entry_p entry = NULL;
	unsigned int hash = ht->hash(key, ht->size);
	for (entry = ht->table[hash]; entry; entry = entry->next_ptr) {
		compare = ht->cmp(entry->key, key);
		if (compare == 0) {
			/* found it! */
			*value = entry->val;
			return SUCCESS;
		}
	}

	return FAIL;
}

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
		void (*freeval)(void *v))
{
	unsigned int i;
	ht_entry_p entry = NULL;
	ht_entry_p temp = NULL;

	for (i = 0; i < ht->size; i++) {
		for (entry = ht->table[i]; entry;) {
			freekey(entry->key);
			freeval(entry->val);
			temp = entry;
			entry = entry->next_ptr;
			temp->next_ptr = NULL;
			free(temp);
		}
	}

	free(ht->table);
	free(ht);
}

/**
 * Print the hashtable using the provided function pointers. 
 *
 * @param[in] ht			The hashtable to print.
 * @param[in] val_to_str	A function to get a string representation
 *							of the key value pair with.
 */
void print_ht(hashtable_p ht, void (*val_to_str)(void *key, void *val,
			char *buffer))
{
	unsigned int i;
	ht_entry_p entry = NULL;
	char b[BUFFER_SIZE];

	for (i = 0; i < ht->size; i++) {
		printf("socket[%3i]", i);
		for (entry = ht->table[i]; entry; entry = entry->next_ptr) {
			val_to_str(entry->key, entry->val, b);
			printf("--> %s", b);
		}
		printf("--> NULL\n");
	}
}

/**
 * Print the hashtable using the provided function pointers. 
 *
 * @param[in] ht			The hashtable to print.
 * @param[in] val_to_str	A function to get a string representation
 *							of the key value pair with.
 */
void print_ht_entries(hashtable_p ht, void (*val_to_str)(void *key, void *val,
			char *buffer))
{
	unsigned int i;
	ht_entry_p entry = NULL;
	char b[BUFFER_SIZE];

	for (i = 0; i < ht->size; i++) {
		for (entry = ht->table[i]; entry; entry = entry->next_ptr) {
			val_to_str(entry->key, entry->val, b);
			printf("%s\n", b);
		}
	}
}


queue_t *get_keys(hashtable_t *ht, void *(*copy_key)(void *key), int (*cmp_k)(void *, void *),
		void (*free_k)(void *)) 
{
	unsigned int i;
	ht_entry_p entry;
	queue_t *q = NULL;

	init_queue(&q, cmp_k, free_k);

	for (i = 0; i < ht->size; i++) {
		for(entry = ht->table[i]; entry; entry = entry->next_ptr) {
			insert_node(q, copy_key(entry->key));
		}
	}
	return q;
}

/*** Helper Functions ****************************************************/

/** Calculate the size that the hashtable should be. 
 * The delta table is used to quickly get the largest prime less than or
 * equal to a given power of two. 
 *  @param[in] ht The hashtable to calculate the size for. 
 *
 * @return The size of the table as an unsigned int.
 */
unsigned int calculate_table_size(hashtable_p ht)
{
	unsigned int i = ht->delta_index;
	unsigned int size;
	if (i < DELTA_SIZE) {
		size = (1 << i) - delta[i];
		return size;
	} else {
		size = (1 << (DELTA_SIZE - 1)) - delta[DELTA_SIZE - 1];
		return size;
	}
}

/**
 * Allocate the table iteself of the entries of the hashtable.
 *
 * @param[in] ht The hashtable for which an entry table is required.
 *
 * @return A pointer to the table where entries will be stored.
 */
ht_entry_p *alloc_table(hashtable_p ht)
{
	unsigned int size = ht->size;
	ht_entry_p *p = malloc(size * sizeof(ht_entry_p));
	unsigned int i = 0;

	if (!p) {
		fprintf(stderr, "memory error alloc'ing table\n");
		return NULL;
	}
	for (i = 0; i < size; i++) {
		p[i] = NULL;
	}
	return p;
}

/**
 * More or less double the hashtable size by allocating a new table
 * and carrying the old table's entries over, freeing the old table
 * itself when done.
 *
 * @param[in] ht The table to be resized
 */
void resize(hashtable_p ht)
{
	ht_entry_p *old_table = ht->table, *new_table = NULL;
	ht_entry_p entry = NULL, temp = NULL;
	int i = 0, old_size = -1;
	
	ht->delta_index += ht->delta_diff;

	old_size = ht->size;
	ht->size = calculate_table_size(ht);

	new_table = alloc_table(ht);
	if (!new_table) {
		printf("memory stuffup?\n");
	}
	ht->table = new_table;
	ht->num_entries = 0;

	for (i = 0; i < old_size; i++) {
		for(entry = old_table[i]; entry;) {
			temp = entry;
			entry = entry->next_ptr;
			temp->next_ptr = NULL;
			if (ht_insert_entry(ht, temp)) {
				printf("expect valgrind error\n");
			}
			temp = NULL;
		}
	}

	free(old_table);
}

/**
 * Insert a ht_entry_t pointer into the hashtable.
 *
 * @param[in] ht The hashtable into which to put entry.
 *
 * @param[in] entry The entry to put into the hashtable.
 *
 * @return EXIT_SUCCESS (0) if successfully inserted,
 * 1 if one of the parameters are wrong and KEY_PRESENT_IN_TABLE (-1) if the entry is
 * already inside the table.
 */
int ht_insert_entry(hashtable_p ht, ht_entry_t *entry) 
{
	unsigned int hash = -1;
	float loadfactor = 0.0;

	/* test if parameters are present */
	if (!ht) {
		fprintf(stderr, "Please Provide a valid ht to insert into\n");
		return 1;
	}
	if (!entry) {
		fprintf(stderr, "Please Provide a valid entry to insert in ht\n");
		return 1;
	}

	/* hash the entry */
	hash = ht->hash(entry->key, ht->size);

	/* check if already present in table */
	/*
	for (temp = ht->table[hash]; temp; temp = temp->next_ptr) {
		if (ht->cmp(temp->key, entry->key) == 0) {
			break;
		}
	}
	if (temp) {
		printf("here\n");
		return KEY_PRESENT_IN_TABLE;
	}
	*/

	entry->next_ptr = ht->table[hash];
	ht->table[hash] = entry;
	ht->num_entries++;

	loadfactor = (0.0f + ht->num_entries) / ht->size;
	if (loadfactor > ht->loadfactor) {
		printf("resize: %d, %f\n", ht->delta_index, loadfactor);
		resize(ht);
		loadfactor = (0.0f + ht->num_entries) / ht->size;
		printf("after resize: %d, %f\n", ht->delta_index, loadfactor);
	}
	
	return EXIT_SUCCESS;
}


