#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

/* Structure for cache line */
typedef struct line {
	int validbit;
	long tag;
	int data;
} line;

/* Structure for saving count */
struct count {
	int hitcnt;
	int misscnt;
	int evictcnt;
};

/* Structure for implementing LRU replacement (Use Linked List to record access history)
 * In the Linked List, the least-recently used tag is in the first Node, 
 * while the most-recently used tag is in the Last Node */
typedef struct Node *PtrToNode;
typedef PtrToNode List;
typedef PtrToNode Position;

struct Node {
	long Element;
	Position Next;
};

/* Return Previous Position of tag in L */
Position FindPrevious(long tag, List L){
	Position P;

	P = L;
	while(P->Next != NULL && P->Next->Element != tag)
		P = P->Next;
	return P;
}

/* Return Position of Last Node in L */
Position FindLast(List L){
	Position P;

	P = L;
	while(P->Next != NULL)
		P = P->Next;
	return P;
}

/* Find the Node of tag and Move it to the Last Node in L */
void UpdateList(long tag, List L){
        Position P1, P2, tmp;

	if(L->Next == NULL)
		printf("L(%p) is empty!!\n", L);
	
	P1 = FindPrevious(tag, L);
	P2 = FindLast(L);

	tmp = P1->Next;
	if(tmp->Next != NULL) {
	  P1->Next = tmp->Next;
	  P2->Next = tmp;
	  tmp->Next = NULL;
	}
}

/* Insert new Node of tag to the Last in L */
void AddList(long tag, List L){
	Position P1, tmpCell;

	P1 = FindLast(L);
	tmpCell = malloc(sizeof(struct Node));
	if(tmpCell == NULL)
		printf("Error: Out of space!");
	
	tmpCell->Element = tag;
	tmpCell->Next = NULL;
	P1->Next = tmpCell;

}

/* Delete the First Node in L and Return its tag */
long FindEvict(List L){
        Position P;
	long evict;

	P = L->Next;
        evict = P->Element;
	L->Next = P->Next;
	free(P);

	return evict;
}

/* Access cache with the given address (Simulating) */
void addraccess(int printopt, line **cache, unsigned setindex, unsigned long tagbits, unsigned E, struct count *cnt, List L)
{
   int j;
   long evict; 
   int cnt1 = 0;
   int cnt2 = 0;
   for(j = 0; j < E; j++) {
      /* A Hit occurs */
      if(cache[setindex][j].tag == tagbits && cache[setindex][j].validbit) {
	  cnt->hitcnt++;
	  UpdateList(tagbits, L);
	  if(printopt)
		  printf("hit ");
	  break;
      }
      cnt1++;
   }

   if(cnt1 == E) {
      /* A Miss occurs */
      cnt->misscnt++;
      if(printopt)
	      printf("miss ");
      
      for(j = 0; j < E; j++) {
	  /* There's a room for new line in the set */
	  if(cache[setindex][j].validbit == 0) {
		  cache[setindex][j].tag = tagbits;
		  cache[setindex][j].validbit = 1;
		  AddList(tagbits, L);
		  break;
	  }  
	  cnt2++;
      }
      
      if(cnt2 == E) {
	  /* There's no room for new line. An eviction is needed */
	  cnt->evictcnt++;
	  if(printopt)
		  printf("eviction ");

          evict = FindEvict(L);
	  for(j = 0; j < E; j++) {
		  if(cache[setindex][j].tag == evict) {
			  cache[setindex][j].tag = tagbits;
                          AddList(tagbits, L);
		          break;
		  }
          }
      
      }
  }
}   



int main(int argc, char *argv[])
{
    int printopt = 0;

    int opt;
    FILE *fp_trace;

    Position tmp1, tmp2;

    char oper[5];
    int size;

    unsigned s, E, b, setnum;

    unsigned long addr, tagbits;
    unsigned setmask, setindex;

    /* Parse command line arguments */
    while((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
	switch(opt) {
	    case 'v':
		    printopt = 1;
		    break;
	    case 's':
		    s = atoi(optarg);
		    break;
	    case 'E':
		    E = atoi(optarg);
		    break;
	    case 'b':
		    b = atoi(optarg);
		    break;
	    case 't':
		    if((fp_trace = fopen(optarg, "r")) == NULL) {
			    fprintf(stderr, "Error opening input file %s\n", optarg);
			    return -1;
		    }
		    break;
	    default:
		    break;
       }
    }

    setnum = 1 << s;
    setmask = (1 << s) + ~0;

    /* Allocate cnt for saving count */
    struct count *cnt = malloc(sizeof(struct count));
    cnt->hitcnt = 0; cnt->misscnt = 0; cnt->evictcnt = 0;

    /* Allocate Cache */
    line **cache = (line**)malloc(sizeof(line*) * setnum);
    for (int i = 0; i < setnum; i++) {
	    cache[i] = (line*)malloc(sizeof(line) * E);
	    for (int j = 0; j < E; j++){
		    cache[i][j].validbit = 0;
		    cache[i][j].tag = 0;
	    }
    }

    /* Allocate Linked List for LRU replacement */
    List *L = (List*)malloc(sizeof(List*) * setnum);
    for(int i = 0; i < setnum; i++) {
	   L[i] = malloc(sizeof(struct Node));
	   L[i]->Next = NULL;
    }
    
    /* Read data from the given file and Access the cache */
    while(fscanf(fp_trace, "%1s %lx ,%d", oper, &addr, &size) != EOF) {
       if(*oper == 'I'){
	     continue;
       }
       tagbits = addr >> (b + s);
       setindex = (addr >> b) & setmask;
       if(printopt)
	       printf("%s %lx,%d ", oper, addr, size);
       addraccess(printopt, cache, setindex, tagbits, E, cnt, L[setindex]);
       if (*oper == 'M') {
	       addraccess(printopt, cache, setindex, tagbits, E, cnt, L[setindex]);
       }
       if(printopt)
	       printf("\n");
    }

    /* Print out the result */
    printSummary(cnt->hitcnt, cnt->misscnt, cnt->evictcnt);

    /* Deallocate Cache, Linked List, cnt */
    for(int i = 0; i < setnum; i++) {
	    free(cache[i]);
	    tmp1 = L[i]->Next;
	    while(tmp1 != NULL) {
		    tmp2 = tmp1;
		    tmp1 = tmp1->Next;
		    free(tmp2);
	    }
	    free(L[i]);
    }
    free(cache);
    free(L);
    free(cnt);

    /* Close the file */
    fclose(fp_trace);

    return 0;
}
