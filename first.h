#ifndef _first_h
#define _first_h

typedef struct block {
	// unsigned long data;
	unsigned long tag;
	int valid;
	// int dirty;
} block;

typedef struct node {
    unsigned long data;
    struct node * next;
} node;

typedef struct queue {
	node * head;
	node * tail;
	int size;
	int capacity;
} queue;

int readsNP = 0, writesNP = 0, hitsNP = 0, missesNP = 0;				
int readsP = 0, writesP = 0, hitsP = 0, missesP = 0;					

block ** build_cache(int sets, int assoc);
void free_cache(block ** cache, int sets);

bool search(block ** cache, int assoc, unsigned long tag, int set_num, char* cache_policy,queue * arr);
bool search_2(block ** cache, int assoc, unsigned long tag, int set_num, queue * arr);
void insert(block ** cache, int assoc, unsigned long tag, int set_num, queue * arr);

queue * build_array(int sets, int assoc);
void enqueue(queue * r, int data);
unsigned long dequeue(queue * r);
void mov_back(queue * r, unsigned long int data);

#endif
