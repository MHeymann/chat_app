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
#ifndef QUEUE_H
#define QUEUE_H

/*** Macros **************************************************************/

#define SUCCESS 1
#define FAIL 0

/*** Typedefinitions *****************************************************/

typedef struct node {
	void *data;
	struct node *prev;
	struct node *next;
} node_t;

/*
 * The struct for storing the queue and information about the queue. 
 */
typedef struct queue {
	int node_count;
	node_t *head;
	node_t *tail;
	int (*cmp_data)(void *, void *);
	void (*free_data)(void *);
} queue_t;

/*** Function Prototypes *************************************************/

/**
 * Allocate the structures for the queue and initialise the values.  
 *
 * @param q		A pointer to a pointer to the queue that must be 
 *				initialised. This gets malloced in the init_queue procedure
 *				and will be freed in the free_queue method.  
 * @param cmp	A function pointer for the function used to compare two 
 *				nodes with each other.  It returns a negative number if the
 *				first node comes before the other, 0 if they are equal in
 *				comparison and a positive number if the first should be 
 *				ordered after the second.  
 * @param free_n A function pointer to the function that should be used
 *				to free nodes in the queue.  
 */
void init_queue(queue_t **q, int (*cmp)(void *, void *), 
		void (*free_n)(void *));

/**
 * A method to free all remaining nodes in the queue and free the queue
 * itself.  
 *
 * @param q The queue to be free'd.  
 */
void free_queue(queue_t *q);

/**
 * Get the number of nodes currently stored in the queue.  
 *
 * @param q The queue in question.  
 * @return The number of nodes in the queue as an integer number.  
 */
int get_node_count(queue_t *q);

/**
 * Insert a node in its correct position in the queue.  
 * It is important to note that the node pointed to cannot be used again
 * until it as been popped off the queue.  So keep this in mind after
 * inserting the node.  
 *
 * @param q		The queue into which to insert the node. 
 * @param node	The node to be entered into the queue. 
 *
 * @return SUCCESS (1) if inserted successfully, FAIL (0) if something
 * goes wrong.
 */
int insert_node(queue_t *q, void *node);

/**
 * Pop the first item in the queue.  
 *
 * @param q The queue from which to pop.  
 */
void *pop_first(queue_t *q);

/**
 * Free all nodes in the queue and reset the values in the queue.  
 *
 * @param q The queue in quesion.  
 */
void empty_queue(queue_t *q);

/**
 * Throw out all nodes in the queue without freeing and reset 
 * the values in the queue.  
 *
 * @param q The queue in quesion.  
 */
void expell_queue_contents(queue_t *q);

/**
 * Print all the items in the queue in order.
 * Note that this does not actually pop the items in the queue, it just 
 * prints them.  
 * 
 * @param q The queue to be printed.  
 *
 * @param print_n A function pointer to be used to print the nodes 
 * themselves in a sensible manner.  
 */
void print_queue(queue_t *q, void (*print_n)(void *n));

/**
 * Print the weights of the items in the queue in order.
 * Note that this does not actually pop the items in the queue, it just 
 * prints them.  
 * 
 * @param q		The queue to be printed.  
 *
 * @param get_weight A function pointer to be used to print the nodes 
 *				themselves in a sensible manner.  
 */
void print_weights(queue_t *q, int (*get_weight)(void *n));


#endif
