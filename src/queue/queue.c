/*
 * This is a generic queue structure.
 *
 * This Implemented by Murray Heymann in 2016, based on a Priority 
 * Queue in the Algorithms textbook by Sedgewick and Wayne.  Their's
 * was written in Java, I translated it to C for compilation on a
 * Linux Machine using gcc or clang.
 *
 * "So do all who live to see such times, but that is not for them 
 * to decide.  All we have to decide is what to do with the time 
 * that is given us."
 *						― J.R.R. Tolkien, The Fellowship of the Ring
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "queue.h"

/*** Debug Function Prototypes *******************************************/

#ifdef DEBUGQ

/**
 * Calculate the size of the array using the indeces of the first and
 * last entry.
 * The idea is that this can be used to compare this to the node count 
 * stored in the queue structure.  If they match, it gives an indication
 * That the updating of the indeces for the first and last entries are
 * probably ok.  
 *
 * @param q The pointer to the queue to determine the node count for.  
 *
 * @return The number of nodes there must be in the queue.  
 */
int calculate_size(queue_t *q);

#endif

#ifdef DEBUGI

	int offset = 0;

	#define OFFUNIT 4

	void enter_function(char *function_name);
	void leave_function(char *function_name);

	#define DBG_enter(A) enter_function(A)
	#define DBG_leave(A) leave_function(A)

#else

	#define DBG_enter(A)
	#define DBG_leave(A)

#endif

/*** Helper Function Prototypes ******************************************/

void prepend(queue_t *q, node_t *node);
void append(queue_t *q, node_t *node);
void find_and_insert(queue_t *q, node_t *node);
void insert_before(node_t *node, node_t *target_node);

/*** Functions ***********************************************************/

/**
 * Allocate the structures for the queue and initialise the values.  
 *
 * @param[out] q A pointer to a pointer to the queue that must be initialised.
 * This gets malloced in the init_queue procedure and will be freed in 
 * the free_queue method.  
 *
 * @param[in] cmp A function pointer for the function used to compare two
 * nodes with each other.  It returns a negative number if the first node
 * comes before the other, 0 if they are equal in comparison and a
 * positive number if the first should be ordered after the second.  
 *
 * @param[in] free_d A function pointer to the function that should be used
 * to free nodes in the queue.  
 */
void init_queue(queue_t **q, int (*cmp)(void *, void *), void (*free_d)(void *)) 
{
	queue_t *local_q;

	DBG_enter("init_queue");

	if (!q) {
		fprintf(stderr, "invalid pointer to queue provided for initialising\n");
	}
	if (*q) {
		fprintf(stderr, "please provide a queue pointer that is not currently pointing to any data.\n");
		DBG_leave("init_queue");
		return;
	}
	local_q = malloc(sizeof(queue_t));
	if (!local_q) {
		fprintf(stderr, "Memory error\n");
		exit(1);
	}
	*q = local_q;

	local_q->node_count = 0;
	local_q->head = NULL;
	local_q->tail = NULL;
	local_q->cmp_data = cmp;
	local_q->free_data = free_d;

	DBG_leave("init_queue");
}

/**
 * A method to free all remaining nodes in the queue and free the queue
 * itself.  
 *
 * @param[in] q The queue to be free'd.  
 */
void free_queue(queue_t *q) 
{
	node_t *current = NULL;
	node_t *temp = NULL;

	DBG_enter("free_queue");

	for (current = q->head; current;) {
		temp = current->next;
		current->next = NULL;
		current->prev = NULL;
		q->free_data(current->data);
		current->data = NULL;
		free(current);
		current = temp;
	}

	q->head = NULL;
	q->tail = NULL;

	free(q);
	
	DBG_leave("free_queue");
}

/**
 * Get the number of nodes currently stored in the queue.  
 *
 * @param[in] q The queue in question.  
 * @return The number of nodes in the queue as an integer number.  
 */
int get_node_count(queue_t *q)
{
	int r = 0;
	DBG_enter("get_node_count");
	if (q) {
		r = q->node_count;
	}
	DBG_leave("get_node_count");
	return r;
}

/**
 * Insert a node in its correct position in the queue.  
 * It is important to note that the node pointed to cannot be used again
 * until it as been popped off the queue.  So keep this in mind after
 * inserting the node.  
 *
 * @param[in] q The queue into which to insert the node. 
 * @param[in] node The node to be entered into the queue. 
 *
 * @return SUCCESS (1) if inserted successfully, FAIL (0) if something
 * goes wrong.
 */
int insert_node(queue_t *q, void *data)
{
	int comp;
	node_t *wrapper = NULL;

	DBG_enter("insert_node");

	if (!data) {
		fprintf(stderr, "You are trying to insert a null pointer\n");
		return FAIL;
	}
	if (!q) {
		fprintf(stderr, "You are inserting into a nonexisting queue?\n");
		return FAIL;
	}

	wrapper = malloc(sizeof(node_t));
	if (!wrapper) {
		fprintf(stderr, "Memory error when creating a new wrapper node for insertion.\n");
		return FAIL;
	}
	/* this is crucial for the linked structure to not have errors */
	wrapper->next = NULL;
	wrapper->prev = NULL;
	wrapper->data = data;

	if (q->node_count == 0) {
		q->head = q->tail = wrapper;
	} else if (q->node_count == 1) {
		comp = q->cmp_data(data, q->head->data);
		if (comp <= 0) {
			/* insert before current unit */
			prepend(q, wrapper);
		} else {
			/* insert after current unit */
			append(q, wrapper);
		}
	} else if (q->node_count > 1) {
		comp = q->cmp_data(data, q->head->data);
		if (comp <= 0) {
			/* insert before first unit */
			prepend(q, wrapper);
		} else {
			comp = q->cmp_data(data, q->tail->data);
			if (comp >= 0) {
				/* insert after current unit */
				append(q, wrapper);
			} else {
				find_and_insert(q, wrapper);
			}
		}

	} else {
		fprintf(stderr, "Node count somehow became negative.\n");
	}

	q->node_count++;
	
#ifdef DEBUGQ
	if(q->node_count != calculate_size(q)) {
		fprintf(stderr, "DEBUG: please fix, the calculated and actual counted node counts don't match.  \n");	
	}
#endif

	DBG_leave("insert_node");
	return SUCCESS;
}

/**
 * Pop the first item in the queue.  
 *
 * @param[in] q The queue from which to pop.  
 *
 * @return A pointer to the first entry in the queue that was popped.
 */
void *pop_first(queue_t *q)
{
	node_t *temp = NULL;
	void *x = NULL;

	DBG_enter("pop_first");

	if (!(q->node_count)) {
		DBG_leave("pop_first");
		/* Nothing to pop */
		return NULL;
	}

	if (!q->head) {
		fprintf(stderr, "There was a stuffup and the pointer to the first entry of a queue is NULL\n");
		DBG_leave("pop_first");
		return NULL;
	}
	
	if (!(x = q->head->data)) {
		fprintf(stderr, "There was a stuffup and the pointer to the first entry of a queue is has no data\n");
		DBG_leave("pop_first");
		return NULL;
	}

	q->node_count--;
	temp = q->head;
	q->head = q->head->next;
	if (q->head) {
		q->head->prev = NULL;
	}
	temp->next = NULL;
	temp->data = NULL;
	if (temp->prev) {
		printf ("mistake!\n");
		temp->prev = NULL;
	}
	free(temp);
	if (q->node_count == 0) {
		q->tail = NULL;
	}

	DBG_leave("pop_first");
	return x;
}

/**
 * Free all nodes in the queue and reset the values in the queue.  
 *
 * @param[in] q The queue in quesion.  
 */
void empty_queue(queue_t *q) 
{
	int i;
	int count;
	void *data;

	DBG_enter("empty_queue");	

	if (!q) {
		fprintf(stderr, "Null queue pointer provided to be emptied\n");
	}

#ifdef DEBUGQ
	printf("Emptying queue\n");
#endif
	count = get_node_count(q);
	for (i = 0; i < count; i++) {
		if ((data = pop_first(q))) {
			q->free_data(data);
		}
	}
#ifdef DEBUGQ
	printf("Queue emptied.  New count: %d\n\n", get_node_count(q));
#endif
	DBG_leave("empty_queue");	
}



/**
 * Throw out all nodes in the queue without freeing and reset 
 * the values in the queue.  
 *
 * @param[in] q The queue in quesion.  
 */
void expell_queue_contents(queue_t *q) 
{
	int i;
	int count;
	void *data;

	DBG_enter("empty_queue");	

	if (!q) {
		fprintf(stderr, "Null queue pointer provided to be emptied\n");
	}

#ifdef DEBUGQ
	printf("Expelling queue contents\n");
#endif
	count = get_node_count(q);
	for (i = 0; i < count; i++) {
		if ((data = pop_first(q))) {
			/* do nothing */
		}
	}
#ifdef DEBUGQ
	printf("Queue contents expelled.  New count: %d\n\n", get_node_count(q));
#endif
	DBG_leave("empty_queue");	
}

/**
 * Print all the items in the queue in order.
 * Note that this does not actually pop the items in the queue, it just 
 * prints them.  
 * 
 * @param[in] q The queue to be printed.  
 *
 * @param[in] print_n A function pointer to be used to print the nodes 
 * themselves in a sensible manner.  
 */
void print_queue(queue_t *q, void (*print_d)(void *n)) 
{
	int i;
	node_t *current = NULL;
	char line1[20];

	DBG_enter("print_queue");	

	printf("Printing Queue: \n");
	printf("Node count: %d\n", q->node_count);
	printf("Queue Entries:\n|\nV\n");
	i = 0;
	for (current = q->head; current; current = current->next) {
		sprintf(line1, "bucket[%d]", i);
		printf("%-12sVVVVVV\n", line1);
		print_d(current->data);
		printf("%-12s^^^^^^\n", line1);
		if (i != q->node_count - 1) {
			printf("|\nV\n");
		}
		i++;
	}
	printf("\n\n");
	DBG_leave("print_queue");	
}

/**
 * Print the weights of the items in the queue in order.
 * Note that this does not actually pop the items in the queue, it just 
 * prints them.  
 * 
 * @param[in] q The queue to be printed.  
 *
 * @param[in] get_weight A function pointer to be used to print the nodes 
 *
 * themselves in a sensible manner.  
 */
void print_weights(queue_t *q, int (*get_weight)(void *n))
{
	node_t *current = NULL;
	int weight = 0;

	DBG_enter("print_weights");	

	for (current = q->head; current;  current = current->next) {
		weight = get_weight(current->data);
		printf("%d ", weight);
		printf("\n");
	}
	printf("\n");
	DBG_leave("print_queue");	
}


/*** Helper Functions ****************************************************/

/**
 * Add a node the the front of the queue.
 *
 * @param[in] q The queue into which to insert
 * @param[in] node The node to prepend.
 */
void prepend(queue_t *q, node_t *node)
{
	if (!q) {
		fprintf(stderr, 
				"Please provide a valid queue pointer to insert into.\n");
	}
	if (!node) {
		fprintf(stderr, 
		   "Please provide a valid node pointer to insert into the queue.\n");
	}
	node->next = q->head;
	q->head->prev = node;
	q->head = node;
}

/**
 * Add a node to the back of the queue.
 *
 * @param[in] q The queue into which to insert
 * @param[in] node The node to append.
 */
void append(queue_t *q, node_t *node)
{
	if (!q) {
		fprintf(stderr, 
				"Please provide a valid queue pointer to insert into.\n");
	}
	if (!node) {
		fprintf(stderr, 
		   "Please provide a valid node pointer to insert into the queue.\n");
	}
	node->prev = q->tail;
	q->tail->next = node;
	q->tail = node;
}

/**
 * Find The location in the queue where node must be inserted. 
 * This location is assumed to not be before the first or after the last
 * entry.  
 *
 * @param[in] q The queue into which to insert
 * @param[in] node The node to insert.
 */
void find_and_insert(queue_t *q, node_t *node)
{
	node_t *current = NULL;
	printf("ouch\n");
	if (!q) {
		fprintf(stderr, "Please provide a valid queue pointer to insert into.\n");
	}
	if (!node) {
		fprintf(stderr, 
		   "Please provide a valid node pointer to insert into the queue.\n");
	}

	for (current = q->head; ((current) && ( 
			q->cmp_data(node->data, current->data) > 0));
			current = current->next);

	insert_before(node, current);

}

/**
 * Insert a node before a specified node in the queue.
 *
 * @param[in] node The node to be inserted.
 * @param[in] target_node The node before which to insert.
 */
void insert_before(node_t *node, node_t *target_node) 
{
	node_t *prev = NULL;

	if (!node) {
		fprintf(stderr, 
		   "Please provide a valid node pointer to insert into the queue.\n");
	}

	if (target_node) {
		prev = target_node->prev;
	} else {
		fprintf(stderr, "This is very weird in the broader context\n");
	}
	if (prev) {
		prev->next = node;
		node->prev = prev;
	}
	node->next = target_node;
	if (target_node){
		target_node->prev = node;
	} else {
		fprintf(stderr, "This is still very weird in the broader context\n");
	}
}


/*** Debug Functions *****************************************************/

#ifdef DEBUGQ
/**
 * Calculate the size of the array using the indeces of the first and
 * last entry.
 * The idea is that this can be used to compare this to the node count 
 * stored in the queue structure.  If they match, it gives an indication
 * That the updating of the indeces for the first and last entries are
 * probably ok.  
 *
 * @param q The pointer to the queue to determine the node count for.  
 *
 * @return The number of nodes there must be in the queue.  
 */
int calculate_size(queue_t *q)
{
	node_t *current = NULL;
	int count;
	
	count = 0;
	for (current = q->head; current; current = current->next) {
		count++;
	}
	return count;
}

#endif

#ifdef DEBUGI

void enter_function(char *function_name)
{
	int i;
	for (i = 0; i < offset; i++) {
		printf(" ");
	}
	printf("%s\n", function_name);
	offset += OFFUNIT;
}

void leave_function(char *function_name)
{
	int i;
	offset -= OFFUNIT;
	for (i = 0; i < offset; i++) {
		printf(" ");
	}
	printf("\\%s\n", function_name);
}

#endif
