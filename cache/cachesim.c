#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachesim.h"
#include <math.h>

int tag = 0, ind = 0, byte = 0; // address breakdown
int block_Size = 0, cache_Size = 0, way_S = 0; // global variables
cache c1; // cache object

counter_t accesses = 0, hits = 0, misses = 0, writebacks = 0, writes = 0, reads = 0, writeHit = 0, readHit = 0, writeMiss = 0, readMiss = 0; // all parameters about miss rate

void cachesim_init(int blocksize, int cachesize, int ways) {
    block_Size = blocksize;
    cache_Size = cachesize;
    way_S = ways;
    // calculate address breakdown
    byte = log(blocksize)/ log(2);
    ind = log(cachesize / blocksize / ways) / log(2);
    tag = 32 - ind - byte;
    int lines = cachesize / blocksize / ways;
    int i = 0;
    for (i = 0; i < lines; i++) { // c1 contains #lines sets
        c1.sets[i].top = -1;
        int j = 0;
        for (j = 0; j < ways; j++) { // each set contains a stack, tag, valid, dirty
            (c1.sets)[i].stack[j] = 0;
            (c1.sets)[i].tag[j] = 0;
            (c1.sets)[i].valid[j] = 0;
            (c1.sets)[i].dirty[j] = 0;
        }
    }
}

void cachesim_access(addr_t physical_addr, int write) {
    // breakdown address
    int byteBits = physical_addr & (1 << byte - 1);
    int indexBits = (physical_addr >> byte) & ((1 << ind) - 1) ;
    int tagBits = (physical_addr >> (ind + byte)) & ((1 << tag) - 1);
    accesses++;
    int flag = hit(tagBits, indexBits);
    if (flag != -1 && write == 1) { // write hit
        hits++;
        writes++;
        writeHit++;
        update_stack2(tagBits, (c1.sets)[indexBits].top, indexBits); // update stack for hit
        int i = 0;
        for (i = 0; i < way_S; i++) { // iterate through tags to find the matching one
            if ((c1.sets)[indexBits].tag[i] == tagBits) { // set dirty bit to 1
                (c1.sets)[indexBits].dirty[i] = 1;
                break;
            }
        }
    } else if (flag != -1 && write == 0) { // read hit
        hits++;
        reads++;
        readHit++;
        update_stack2(tagBits, (c1.sets)[indexBits].top, indexBits); // update stack for hit
	int i = 0;	
	for (i = 0; i < way_S; i++) { // set valid bit to 1
            if ((c1.sets)[indexBits].tag[i] == tagBits) {
                (c1.sets)[indexBits].valid[i] = 1;
                break;
            }
	}
    } else if (flag == -1 && write == 1) { // write miss
        misses++;
        writes++;
        writeMiss++;
        int blockNum = update_stack1(tagBits, (c1.sets)[indexBits].top, indexBits); // update stack for miss
        if (blockNum != -1) { // if stack is not full, set dirty, valid, update tag
            (c1.sets)[indexBits].dirty[blockNum] = 1;
            (c1.sets)[indexBits].valid[blockNum] = 1;
            (c1.sets)[indexBits].tag[blockNum] = tagBits;
        } else { // if stack is full
            int i = 0;
            for (i = 0; i < way_S; i++) { // iterate through tags to find an invalid line, and set, dirty, valid, update tag
                if ((c1.sets)[indexBits].valid[i] == 0) {
                    (c1.sets)[indexBits].dirty[i] = 1;
                    (c1.sets)[indexBits].valid[i] = 1;
                    (c1.sets)[indexBits].tag[i] = tagBits;
                    break;
                }
            }
        }
    } else if (flag == -1 && write == 0) { // read miss
        misses++;
        reads++;
        readMiss++;
        int blockNum = update_stack1(tagBits, (c1.sets)[indexBits].top, indexBits); // update stack for miss
        if (blockNum != -1) { // if stack is not full, set valid, update tag
            (c1.sets)[indexBits].valid[blockNum] = 1;
            (c1.sets)[indexBits].dirty[blockNum] = 0;
            (c1.sets)[indexBits].tag[blockNum] = tagBits;
        } else {
            int i = 0;
            for (i = 0; i < way_S; i++) { // iterate through tags to find an invalid line, and set valid, update tag
                if ((c1.sets)[indexBits].valid[i] == 0) {
                    (c1.sets)[indexBits].valid[i] = 1;
		    (c1.sets)[indexBits].dirty[i] = 0;
                    (c1.sets)[indexBits].tag[i] = tagBits;
                    break;
                }
            }
        }
    }
}

int update_stack1(int tagBits, int top, int indexBits) { // this function updates stack for miss
    int tagIndex = 0;
    if (top == way_S - 1) { // if stack is already full of tags
        int i = 0;
        for (i = 0; i < way_S; i++) {
            if ((c1.sets)[indexBits].tag[i] == (c1.sets)[indexBits].stack[0]) { // check which tag matches stack[0]
                if ((c1.sets)[indexBits].dirty[i] == 1) { // if it is dirty, write back
                    writebacks++;
                }
                tagIndex = i;
            }
        }
        int j = 0;
        for (j = 0; j < top; j++) { // evict stack[0]
            (c1.sets)[indexBits].stack[j] = (c1.sets)[indexBits].stack[j + 1];
        }
        (c1.sets)[indexBits].stack[top] = tagBits; // make the top of the stack tagBits
        return tagIndex;
    } else {
        (c1.sets)[indexBits].top++; // if stack is not full, increase top
	top++;        
	(c1.sets)[indexBits].stack[top] = tagBits; // just add tagBits to be the top of the stack
        return -1;
    }
}

int hit(int tagBits, int indexBits) { // this function measures whether there is a hit or miss
    int i = 0;
    for (i = 0; i < way_S; i++) {
        if ((c1.sets)[indexBits].tag[i] == tagBits && (c1.sets)[indexBits].valid[i] == 1) {
            return i; // return the tag that hits
        }
    }
    return -1; // return -1 if there is a miss
}

void update_stack2(int tagBits, int top, int indexBits) { // this function updates stack for hit
    int i = 0;
    for (i = 0; i < way_S; i++) {
        if ((c1.sets)[indexBits].stack[i] == tagBits) {
            int j = 0;
            for (j = i; j < top; j++) { // move the hit tag to the top
                (c1.sets)[indexBits].stack[j] = (c1.sets)[indexBits].stack[j + 1];
            }
            (c1.sets)[indexBits].stack[top] = tagBits; // make the top tag tagBits
            break;
        }
    }
}

void cachesim_print_stats() {
    printf("%llu, %llu, %llu, %llu", accesses, hits, misses, writebacks);
}
