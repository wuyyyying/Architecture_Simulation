/**
 * @file cache_driver.cpp
 * @brief Trace reader and driver for the CS{4/6}290 / ECE{4/6}100 Spring 2019 Project 1
 *
 * Project 1 trace reader and driver. Don't modify any code in this file!
 *
 * @author Anirudh Jain
 * @author Bradley Thwaites
 */

#include <getopt.h>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
// #include <unistd.h>

#include "cache.hpp"

static void print_err_usage(std::string err)
{
    std::cout << err << std::endl;
    // print usage
    std::cout << "./cachesim [OPTIONS] -i <tracename.trace>" << std::endl;
    std::cout << "    -c c     Total size of the L1 cache is 2^c bytes" << std::endl;
    std::cout << "    -s s     Number of blocks per set in the L1 cache is 2^s" << std::endl;
    std::cout << "    -b b     Block size in both cases is 2^b bytes" << std::endl;
    std::cout << "    -C C     Total size of the L2 cache is 2^C bytes" << std::endl;
    std::cout << "    -S S     Number of blocks per set in the L2 cache is 2^S" << std::endl;
    std::cout << "    -v v     Number of blocks in the victim cache is v" << std::endl;
    std::cout << "    -k k     Prefetch distance is k" << std::endl;
    std::exit(EXIT_FAILURE);
}

static void print_config(struct cache_config_t *conf)
{
    std::cout << "Cache Configuration" << std::endl;
    std::cout << "c = " << conf->c << std::endl;
    std::cout << "s = " << conf->s << std::endl;
    std::cout << "b = " << conf->b << std::endl;
    std::cout << "C = " << conf->C << std::endl;
    std::cout << "S = " << conf->S << std::endl;
    std::cout << "v = " << conf->v << std::endl;
    std::cout << "k = " << conf->k << std::endl;
}

static void print_stats(struct cache_stats_t *stats)
{
    std::cout << std::fixed; // Make sure that 6 significant digits are always displayed
    std::cout << std::endl << "HIT MISS STATISTICS" << std::endl;
    std::cout << "Total Number of accesses:       " << stats->num_accesses << std::endl;
    std::cout << "Total Number of reads:          " << stats->num_accesses_reads << std::endl;
    std::cout << "Total Number of writes:         " << stats->num_accesses_writes << std::endl;
    std::cout << "Number of L1 misses:            " << stats->num_misses_l1 << std::endl;
    std::cout << "Number of L1 read misses:       " << stats->num_misses_reads_l1 << std::endl;
    std::cout << "Number of L1 write misses:      " << stats->num_misses_writes_l1 << std::endl;
    std::cout << "Number of VC hits:              " << stats->num_hits_vc << std::endl;
    std::cout << "Number of VC misses:            " << stats->num_misses_vc << std::endl;
    std::cout << "Number of VC read misses:       " << stats->num_misses_reads_vc << std::endl;
    std::cout << "Number of VC write misses:      " << stats->num_misses_writes_vc << std::endl;
    std::cout << "Number of L2 misses:            " << stats->num_misses_l2 << std::endl;
    std::cout << "Number of L2 read misses:       " << stats->num_misses_reads_l2 << std::endl;
    std::cout << "Number of L2 write misses:      " << stats->num_misses_writes_l2 << std::endl;
    std::cout << "Number of write backs:          " << stats->num_write_backs << std::endl;
    std::cout << "Number of bytes transferred:    " << stats->num_bytes_transferred << std::endl;
    std::cout << "Number of blocks prefetched:    " << stats->num_prefetches << std::endl;
    std::cout << "Number of useful prefetches:    " << stats->num_useful_prefetches << std::endl;
    std::cout << "L1 hit time:                    " << std::setprecision(6) << stats->hit_time_l1 << std::endl;
    std::cout << "L2 hit time:                    " << std::setprecision(6) << stats->hit_time_l2 << std::endl;
    std::cout << "Memory hit time:                " << std::setprecision(6) << stats->hit_time_mem << std::endl;
    std::cout << "L1 miss rate:                   " << std::setprecision(6) << stats->miss_rate_l1 << std::endl;
    std::cout << "VC miss rate:                   " << std::setprecision(6) << stats->miss_rate_vc << std::endl;
    std::cout << "L2 miss rate:                   " << std::setprecision(6) << stats->miss_rate_l2 << std::endl;
    std::cout << "Average Access Time:            " << std::setprecision(6) << stats->avg_access_time << std::endl;
}

int main(int argc, char *const argv[])
{
    int opt;
    FILE *fin = stdin;

    struct cache_config_t DEFAULT_CONF;

    if (argc < 2) {
        print_err_usage("Input file argument not provided");
    }

    while (-1 != (opt = getopt(argc, argv, "c:C:b:B:s:S:i:I:v:V:k:K:h"))) {
        switch (opt) {
            case 'c':
                DEFAULT_CONF.c = (uint64_t) atoi(optarg);
                break;
            case 'C':
                DEFAULT_CONF.C = (uint64_t) atoi(optarg);
                break;
            case 'b':
            case 'B': // Just incase someone decides to pass 'B' for the block size
                DEFAULT_CONF.b = (uint64_t) atoi(optarg);
                break;
            case 's':
                DEFAULT_CONF.s = (uint64_t) atoi(optarg);
                break;
            case 'S':
                DEFAULT_CONF.S = (uint64_t) atoi(optarg);
                break;
            case 'v':
            case 'V':
                DEFAULT_CONF.v = (uint64_t) atoi(optarg);
                break;
            case 'k':
            case 'K':
                DEFAULT_CONF.k = (uint64_t) atoi(optarg);
                break;
            case 'i':
            case 'I':
                fin = fopen(optarg, "r");
                break;
            case 'h':
            default:
                print_err_usage("");
                break;
        }
    }

    print_config(&DEFAULT_CONF);

    // stats struct being used by the driver
    struct cache_stats_t stats;
    memset(&stats, 0, sizeof(struct cache_stats_t));

    // Set access times for each level of the memory hierarchy
    stats.hit_time_l1 = HIT_TIME_L1_BASE + ADJUSTMENT_FACTOR_L1 * (double) DEFAULT_CONF.s;
    stats.hit_time_l2 = HIT_TIME_L2_BASE + ADJUSTMENT_FACTOR_L2 * (double) DEFAULT_CONF.S;
    stats.hit_time_mem = HIT_TIME_MEM;

    // Call the init function only once
    cache_init(&DEFAULT_CONF);

    char rw;
    uint64_t addr;
    while (!feof(fin)) {
        // Don't change this line if you want this to work!
        int ret = fscanf(fin, "%" PRIx64 " %c\n", &addr, &rw);
        if (ret == 2) {
            // Perform accesses -- one at a time
            cache_access(addr, rw, &stats);
        }
    }
    fclose(fin);

    // Cleanup memory and perform any computations you might need to then print statistics
    cache_cleanup(&stats);
    print_stats(&stats);

    return 0;
}
