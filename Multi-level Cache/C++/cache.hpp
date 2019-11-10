/**
 * @file cache.hpp
 * @brief Header for the CS{4/6}290 / ECE{4/6}100 Spring 2019 Project 1 stats
 *
 * Header file for the cache simulator containing a bunch of struct definitions,
 * constants, defaults, etc. Don't modify any code in this file!
 *
 * @author Anirudh Jain
 */

#ifndef CACHE_H
#define CACHE_H

#include <cstdint>

// Default configuration -- Don't modify
static const uint64_t DEFAULT_c = 15;
static const uint64_t DEFAULT_C = 18;
static const uint64_t DEFAULT_s = 4;
static const uint64_t DEFAULT_S = 3;
static const uint64_t DEFAULT_b = 6;
static const uint64_t DEFAULT_v = 8;
static const uint64_t DEFAULT_k = 3; // Prefetch stride

// Constants -- Don't modify
static const char READ = 'R';
static const char WRITE = 'W';
static const uint8_t TRUE = 1;
static const uint8_t FALSE = 0;

// Access time constants
static const double HIT_TIME_L1_BASE = 2.0;
static const double HIT_TIME_L2_BASE = 8.0;
static const double ADJUSTMENT_FACTOR_L1 = 0.2; // Increase in hit time due to set associativity
static const double ADJUSTMENT_FACTOR_L2 = 0.4;
static const double HIT_TIME_MEM = 80.0;

// Struct for keeping the cache hierarchy parameters
struct cache_config_t {
    uint64_t c;
    uint64_t C;
    uint64_t s;
    uint64_t S;
    uint64_t b; // We assume that both the caches have the exact same block size
    uint64_t v;
    uint64_t k;

    // Constructor with default values -- Don't modify
    cache_config_t() :  c(DEFAULT_c), C(DEFAULT_C), s(DEFAULT_s), S(DEFAULT_S),
                        b(DEFAULT_b), v(DEFAULT_v), k(DEFAULT_k) {}
};

// Struct for keeping track of hit-miss statistics
struct cache_stats_t {
    uint64_t num_accesses;                  // total accesses
    uint64_t num_accesses_writes;           // total write accesses
    uint64_t num_accesses_reads;            // total read accesses

    uint64_t num_misses_l1;                 // total misses in just the L1
    uint64_t num_misses_reads_l1;           // total misses in just the L1
    uint64_t num_misses_writes_l1;          // total misses in just the L1

    uint64_t num_hits_vc;                   // total victim cache hits
    uint64_t num_misses_vc;                 // total victim cache misses
    uint64_t num_misses_reads_vc;           // total read misses in the VC
    uint64_t num_misses_writes_vc;          // total write misses in the VC

    uint64_t num_misses_l2;                 // total misses in just the L2
    uint64_t num_misses_reads_l2;           // total misses in just the L2
    uint64_t num_misses_writes_l2;          // total misses in just the L2

    uint64_t num_write_backs;               // total writebacks
    uint64_t num_bytes_transferred;         // total data transferred on memory bus

    uint64_t num_prefetches;                // total number of prefetches
    uint64_t num_useful_prefetches;         // total number of useful prefetches

    double hit_time_l1;                     // L1 hit time
    double hit_time_l2;                     // L2 hit time
    double hit_time_mem;                    // Memory hit time
    double miss_rate_l1;                    // L1 miss rate
    double miss_rate_vc;                    // VC miss rate
    double miss_rate_l2;                    // L2 miss rate
    double avg_access_time;                 // average access time per access

};

// Visible functions
void cache_init(struct cache_config_t *conf);
void cache_access(uint64_t addr, char rw, struct cache_stats_t *stats);
void cache_cleanup(struct cache_stats_t *stats);

#endif // CACHE_H
