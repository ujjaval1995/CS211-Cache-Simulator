#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include "first.h"



int main(int argc, char** argv)
{
	if(argc != 7)
	{
	  return 0;
	}
	int cache_size;
	int block_size;
	char* cache_policy;
	char* associativity;
	int prefetch_size;
	FILE* trace;
	int sets;
	int assoc;
	int  block_bits	;
	int  set_bits;
	char operation[5];
	unsigned long address;


	cache_size	= atoi(argv[1]);
	block_size	= atoi(argv[2]);
	cache_policy	= argv[3];
	associativity	= argv[4];
	prefetch_size	= atoi(argv[5]);
	trace	= fopen(argv[6],"r");

	if (strcmp(associativity,"direct") == 0)
	  {
	    sets = cache_size/block_size;
	    assoc = 1;
	  }
	else if (strcmp(associativity, "assoc") == 0)
	  {
	    sets = 1;
	    assoc = cache_size / block_size;
	  }
	else if (strncmp(associativity,"assoc:",6) == 0)
	  {
	    char* temp = strchr(associativity,':');
	    assoc = atoi(temp+1);
	    sets = cache_size / (block_size * assoc);
	  }
	else
	  {
	    return 0;
	  }
	if (prefetch_size < 0)
	  {
	    return 0;
	  }
	if (trace == NULL)
	  {
	    return 0;
	  }

	block_bits = log(block_size) / log(2);
	set_bits = log(sets) / log(2);

	unsigned long tag;
	int set_num;

	block **cacheNP = build_cache(sets, assoc);
	queue *arrNP = build_array(sets, assoc);
	block **cacheP = build_cache(sets, assoc);
	queue *arrP = build_array(sets, assoc);

	while (fscanf(trace, "%s %lx", operation, &address) != EOF && strcmp(operation, "#eof") != 0)
	  {
	    tag = address >> (block_bits + set_bits);
	    set_num = (address >> block_bits) % sets;

	    if (strcmp(operation,"W") == 0)
	      writesNP++;						// no-prefetch

	    if (search(cacheNP, assoc, tag, set_num, cache_policy, arrNP))			// search
	      {
		hitsNP++;
	      }

	    else		     							// miss
	      {
		insert(cacheNP, assoc, tag, set_num, arrNP);
		readsNP++;
		missesNP++;
	      }

	    if (strcmp(operation,"W") == 0)
	      writesP++;		                 				// prefetch

	    if (search(cacheP, assoc, tag, set_num, cache_policy, arrP))			// search
		hitsP++;

	    else			      						// miss
	      {
		insert(cacheP, assoc, tag, set_num, arrP);
		readsP++;
		missesP++;
		int i=0;
		while(i<prefetch_size)
		  {
		    address += block_size;
		    tag = address >> (block_bits + set_bits);
		    set_num = (address >> block_bits) % sets;

		    if (!search_2(cacheP, assoc, tag, set_num, arrP))
		      {
			insert(cacheP, assoc, tag, set_num, arrP);
			readsP++;
		      }
		    i++;
		  }
	      }
	  }

	printf("no-prefetch\n");
	printf("Memory reads: %d\n", readsNP);
	printf("Memory writes: %d\n", writesNP);
	printf("Cache hits: %d\n", hitsNP);
	printf("Cache misses: %d\n", missesNP);
	printf("with-prefetch\n");
	printf("Memory reads: %d\n", readsP);
	printf("Memory writes: %d\n", writesP);
	printf("Cache hits: %d\n", hitsP);
	printf("Cache misses: %d\n", missesP);

	return 0;
}

block ** build_cache(int sets, int assoc)
{
  block ** cache = malloc(sets*sizeof(block *));
  int i;
  for (i=0; i<sets; i++)
    {
      block * set = malloc(assoc* sizeof(block));
      cache[i] = set;
    }
  return cache;
}

queue* build_array(int sets, int assoc)
{
  queue * arr = malloc(sets*sizeof(queue));
  int i;
  for (i=0; i<sets; i++)
    {
      queue * r = malloc(sizeof(queue));
      r->head = NULL;
      r->tail = NULL;
      r->size = 0;
      r->capacity = assoc;
      arr[i] = *r;
    }
  return arr;
}


bool search(block ** cache, int assoc, unsigned long tag, int set_num, char* cache_policy, queue * arr)
{
  int j=0;
  while(j<assoc)
    {
      if (tag == cache[set_num][j].tag && cache[set_num][j].valid == 1)
	{
	  if (strcmp(cache_policy,"lru")==0)
	    mov_back(&arr[set_num], j);
	  return true;
	}
      j++;
    }
  return false;
}

bool search_2(block ** cache, int assoc, unsigned long tag, int set_num, queue * arr)
{
  int j=0;
  while(j<assoc)
    {
      if (tag == cache[set_num][j].tag && cache[set_num][j].valid == 1)
	{
	  return true;
	}
      j++;

    }
  return false;
}

void insert(block ** cache, int assoc, unsigned long tag, int set_num, queue * arr)
{
  unsigned long position;
  if (arr[set_num].size == arr[set_num].capacity)
    {
      position = dequeue(&arr[set_num]);
    }
  else
    {
      position = arr[set_num].size;
    }
  enqueue(&arr[set_num], position);
  cache[set_num][position].tag = tag;
  cache[set_num][position].valid = 1;
}

void mov_back(queue * r, unsigned long int data)
{
  node * ptr=r->head;
  while(ptr!=r->tail && ptr->data!=data)
	ptr=ptr->next;
  unsigned long temp = ptr->data;
  ptr->data = r->tail->data;
  r->tail->data = temp;
}

void enqueue(queue * r, int data)
{
  node * temp = malloc(sizeof(node));
  temp->data = data;
  if (r->size == 0)
    {
      r->head = temp;
      r->tail = temp;
    }
  else
    {
      r->tail->next = temp;
      r->tail = temp;
    }
  r->size++;
}

unsigned long dequeue(queue * r)
{
  if (r->size == 0) return 0;
  node * temp = r->head;
  unsigned long junk = temp->data;
  r->head = r->head->next;
  free(temp);
  r->size--;
  return junk;
}
