#ifndef __CACHESIM_H
#define __CACHESIM_H
typedef unsigned long long addr_t;
typedef unsigned long long counter_t;

typedef struct set{ // define a set
    int top; // top is the head of the stack
    int stack[3000];
    int tag[3000];
    int valid[3000];
    int dirty[3000];
} set;

typedef struct cache{ // define a cache
    set sets[5000];
} cache;

void cachesim_init(int block_size, int cache_size, int ways);
void cachesim_access(addr_t physical_add, int write);
void update_stack2(int tagBits, int top, int indexBits);
int update_stack1(int tagBits, int top, int indexBits);
int hit(int tagBits, int indexBits);
void cachesim_print_stats(void);

#endif
