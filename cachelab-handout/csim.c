#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include "cachelab.h"
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

typedef unsigned long long int MEMORY_ADDRESS;


//cache line struct
typedef struct  {
    char valid;
    MEMORY_ADDRESS tag;
    unsigned long long int lru;
} cache_line;

typedef cache_line* CACHE_SET; //set of lines that makes a CACHE_SET
typedef CACHE_SET* CACHE; //set of  CACHE_SETs that make the CACHE


//for init
int X = 0; //verbose
int s = 0; //set index bits     
int b = 0; //block offset bit count       
int E = 0; //cache lines per set
char* trace_file_loc = NULL;


int S; //set count
int B; //block size


//counters
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 1;


CACHE cache;
MEMORY_ADDRESS set_index_mask;


int pow2(int num) {
    return 1 << num;
}


void init() {
    int i, j;
    //malloc based on set count
    cache = (CACHE_SET*)malloc(sizeof(CACHE_SET) * S);
    for (i = 0; i < S; i++) {
        //iter through each set and malloc them cachelines E
        cache[i] = (cache_line*)malloc(sizeof(cache_line) * E);
        //init cache line
        for (j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru = 0;
        }
    }
    set_index_mask = (MEMORY_ADDRESS)(pow2(s) - 1);
}
 
void freeCache() {
    int i;
    for (i = 0; i < S; i++) {
        free(cache[i]);
    }
    free(cache);
}


void retrieve(MEMORY_ADDRESS addr) {
    int i;
    unsigned long long int eviction_lru = ULONG_MAX;
    unsigned int eviction_line = 0;
    MEMORY_ADDRESS set_index = (addr >> b) & set_index_mask;
    MEMORY_ADDRESS tag = addr >> (s + b);

    CACHE_SET cache_set = cache[set_index];

    for (i = 0; i < E; ++i) {
        if (cache_set[i].valid) {
            if (cache_set[i].tag == tag) {
                cache_set[i].lru = lru_counter++;
                hit_count++;
                return;
            }
        }
    }

    miss_count++;

    for (int i = 0; i < E; ++i) {
        if (eviction_lru > cache_set[i].lru) {
            eviction_line = i;
            eviction_lru = cache_set[i].lru;
        }
    }

    if (cache_set[eviction_line].valid) {
        eviction_count++;
    }

    cache_set[eviction_line].valid = 1;
    cache_set[eviction_line].tag = tag;
    cache_set[eviction_line].lru = lru_counter++;
}



void replayTrace(char* trace_fn) {
    FILE* trace_fp = fopen(trace_fn, "r");
    char trace_cmd;
    MEMORY_ADDRESS address;
    int size;

    while (fscanf(trace_fp, " %c %llx,%d", &trace_cmd, &address, &size) == 3) {
        switch(trace_cmd) {
            //load
            case 'L': 
                retrieve(address); 
                break;
            //store
            case 'S': 
                retrieve(address); 
                break;
            //modify
            case 'M': 
                retrieve(address); 
                retrieve(address); 
                break;
            default: 
                break;
        }
    }
    fclose(trace_fp);
}

int main(int argc, char* argv[]) {
    char c;
    while ((c = getopt(argc, argv, "E:b:s:t:v")) != -1) {
        switch (c) {
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
                trace_file_loc = optarg;
                break;
            case 'v':
                X = 1;
                break;
            default:
                printf("Examples:\n./csim -s 4 -E 1 -b 4 -t traces/yi.trace\nlinux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
                exit(1);
        }
    }

    if (s == 0 || E == 0 || b == 0 || trace_file_loc == NULL) {
        printf("Missing arg\n");
        printf("Examples:\n./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
        exit(1);
    }

    S = (unsigned int)pow2(s);
    B = (unsigned int)pow2(b);

    init();
    replayTrace(trace_file_loc);

    freeCache();

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}