/* Jason Lee		jjl625@nyu.edu		15163596*/

/* 
 * cachelab.c - A cache simulator that can replay traces of memory accesses
 *     and output statistics such as number of hits, misses, and
 *     evictions.  The replacement policy is LRU.
 *
 * Implementation and assumptions:
 *  1. Each load/store can cause at most one cache miss. (I examined the trace,
 *  the largest request I saw was for 8 bytes).
 *  2. Instruction loads (I) are ignored.
 *  3. data modify (M) is treated as a load followed by a store to the same
 *  address. Hence, an M operation can result in two cache hits, or a miss and a
 *  hit plus a possible eviction. 
 */
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

//#define DEBUG_ON 
#define ADDRESS_LENGTH 64

/* Type: Memory address */
typedef unsigned long long int mem_addr_t;

/**************************************************************************/
/* Declarations of the functions currently implemented in this file */

/* 
 * printSummary - This function provides a standard way for your cache
 * simulator * to display its final hit and miss statistics
 */ 
void printSummary(int hits,  /* number of  hits */
				  int misses, /* number of misses */
				  int evictions); /* number of evictions */

/*
 * replayTrace - replays the given trace file against the cache 
 */
void replayTrace(char* trace_fn);

/* 
 * accessData - Access data at memory address addr. 
 * This is the one that you need to implement.
 */
void accessData(mem_addr_t addr);

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[]);
/**************************************************************************/


/**************************************************************************/
/* Declarations of the global variable currently used. */
/* Do NOT modify any of the following globals. 
 * But you can add your own globals if you want 
 */
/* Globals set by command line args */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* associativity */
char* trace_file = NULL;

/* Counters used to record cache statistics */
int miss_count = 0;     /* cache miss */
int hit_count = 0;      /* cache hit */
int eviction_count = 0; /* A block is evicted from the cache */
int TIME = 0;

typedef struct {
	int valid;
	mem_addr_t tag;
	int timestamp;
} block_structure;

typedef struct {
	block_structure *blocks;
	int free;
	int evict;
	int low;
} set_structure;
	
typedef struct {
	set_structure *sets;
} cache_structure;

cache_structure cache;
/**************************************************************************/


/* 
 * accessData - Access data at memory address addr.
 *   If it is already in cache, increast hit_count
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase eviction_count if a line is evicted.
 * 
 *   If you need to evict a line (i.e. block) apply least recently used 
 *   replacement policy.
 */
void accessData(mem_addr_t addr)
{
   
   /* You need to implement this function and any other function this
    * function may need 
    */
	
	mem_addr_t addr_tag = addr >> (s + b);
	int tag_size = 64 - (s + b);
	unsigned long long set_num = (addr << tag_size) >> (tag_size + b);
	int check = 0;

	for(int i = 0; i < E; i++) {
		if(cache.sets[set_num].blocks[i].valid == 1) {
			if(cache.sets[set_num].blocks[i].tag == addr_tag) {
				hit_count++;
				cache.sets[set_num].blocks[i].timestamp = TIME;
				TIME++;
				check = 1;
			}
		}
	}
	if(check != 1) {
		miss_count++;
		for(int i = 0; i < E; i++) {
			if(cache.sets[set_num].blocks[i].valid == 0) {
				cache.sets[set_num].blocks[i].tag = addr_tag;
				cache.sets[set_num].blocks[i].valid = 1;
				cache.sets[set_num].blocks[i].timestamp = TIME;
				TIME++;
				check = 1;
				break;
			}
		}
	}
	if(check != 1) {
		eviction_count++;
		for(int i = 0; i < E; i++) {
			if(cache.sets[set_num].blocks[i].timestamp < cache.sets[set_num].low) {
				cache.sets[set_num].low = cache.sets[set_num].blocks[i].timestamp;
				cache.sets[set_num].evict = i;
			}
		}
		cache.sets[set_num].blocks[cache.sets[set_num].evict].tag = addr_tag;
		cache.sets[set_num].blocks[cache.sets[set_num].evict].timestamp = TIME;
		TIME++;
		cache.sets[set_num].low = INT_MAX;
	}
}







/*
 * main - Main routine 
 */
int main(int argc, char* argv[])
{
    char c;

    /* Do NOT modify anything from this point till the following comment */
    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(c){
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
            trace_file = optarg;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }

    /* Make sure that all required command line args were specified */
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }
    /* From here you can make any modification, 
     * except removing the call to replayTrace */
	
	int S = 1<<s;

	cache.sets = malloc(S * sizeof(set_structure));
	for(int i = 0; i < S; i++) {
		cache.sets[i].blocks = malloc(E * sizeof(block_structure));
		cache.sets[i].free = -1;
		cache.sets[i].evict = 0;
		cache.sets[i].low = INT_MAX;
	}
    
    /* Do not remove this line as it is the one calls your cache access function */
    replayTrace(trace_file);
    
  
    /* Do not modify anything from here till end of main() function */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}


/****** Do NOT modify anything below this point ******/

/* 
 * printSummary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded. 
 */
void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of blocks per set (i.e. associativity).\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t ls.trace\n", argv[0]);
    exit(0);
}

/*
 * replayTrace - replays the given trace file against the cache 
 */
void replayTrace(char* trace_fn)
{
    char buf[1000];
    mem_addr_t addr=0;
    unsigned int len=0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while( fgets(buf, 1000, trace_fp) != NULL) {
        if(buf[1]=='S' || buf[1]=='L' || buf[1]=='M') {
            sscanf(buf+3, "%llx,%u", &addr, &len);

            accessData(addr);

            /* If the instruction is R/W then access again */
            if(buf[1]=='M')
                accessData(addr);
            
        }
    }

    fclose(trace_fp);
}
