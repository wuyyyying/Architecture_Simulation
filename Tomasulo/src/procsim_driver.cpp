#include <cstdio>
#include <cinttypes>
#include <cstdlib>
#include <cstring>

#include <getopt.h>
#include <unistd.h>

#include "procsim.hpp"

FILE* inFile = stdin;

// Print help message and exit
static void print_help_and_exit(void)
{
    printf("procsim [OPTIONS]\n");
    printf("  -f F\t\tNumber of instructions to fetch\n");
    printf("  -r R\t\tNumber of entries in reservation station of each type\n");
    printf("  -k k0\t\tNumber of k0 FUs\n");
    printf("  -l k1\t\tNumber of k1 FUs\n");
    printf("  -m k2\t\tNumber of k2 FUs\n");
    printf("  -g G\t\tlog2 number of entries in BTB\n");
    printf("  -c C\t\tlog2 number of bytes in data cache\n");
    printf("  -i traces/file.trace\n");
    printf("  -h\t\tThis helpful output\n");
    exit(EXIT_SUCCESS);
}

// Print configuration
static void print_config(proc_conf_t *config)
{
    printf("Processor Settings\n");
    printf("F:  %" PRIu64 "\n", config->f);
    printf("k0: %" PRIu64 "\n", config->k0);
    printf("k1: %" PRIu64 "\n", config->k1);
    printf("k2: %" PRIu64 "\n", config->k2);
    printf("R:  %" PRIu64 "\n", config->r);
    printf("G:  %" PRIu64 "\n", config->g);
    printf("C:  %" PRIu64 "\n", config->c);
    printf("\n");
}

// Print statistics at the end of the simulation
static void print_statistics(proc_stats_t* stats) {
    printf("\n\n");
    printf("Processor stats:\n");
    printf("Total instructions retired:     %lu\n", stats->instructions_retired);
    printf("Total Branch instructions:      %lu\n", stats->branch_instructions);
    printf("Correctly predicted branches:   %lu\n", stats->correctly_predicted);
    printf("Branch prediction accuracy:     %f\n", stats->branch_prediction_accuracy);
    printf("Total load instructions:        %lu\n", stats->load_instructions);
    printf("Total store instructions:       %lu\n", stats->store_instructions);
    printf("Total cache misses              %lu\n", stats->cache_misses);
    printf("Data cache miss rate:           %f\n", stats->cache_miss_rate);
    printf("Average Dispatch Queue size:    %f\n", stats->average_disp_queue_size);
    printf("Maximum Dispatch Queue size:    %lu\n", stats->max_disp_queue_size);
    printf("Average instructions retired:   %f\n", stats->average_instructions_retired);
    printf("Final Cycle count:              %lu\n", stats->cycle_count);
}

/* Function to read instruction from the input trace. Populates the inst struct
 *
 * returns true if an instruction was read successfully. Returns false at end of trace
 *
 */
bool read_instruction(inst_t* inst)
{
    int ret;

    if (inst == NULL) {
        fprintf(stderr, "Fetch requires a valid pointer to populate\n");
        return false;
    }

    // Don't modify this line. Instruction fetch might break otherwise!
    ret = fscanf(inFile, "%" PRIx64 " %d %d %d %d %" PRIx64 " %" PRIx64 " %d\n", &inst->inst_addr, &inst->opcode, &inst->dest_reg,
                    &inst->src_reg[0], &inst->src_reg[1], &inst->ld_st_addr, &inst->br_target, (int *) &inst->br_taken);

    if (ret != 8) { // Check if something went really wrong
        if (!feof(inFile)) { // Check if end of file has been reached
            fprintf(stderr, "Something went wrong and we could not parse the instruction\n");
        }
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    int opt;

    // Default configuration -- don't change
    proc_conf_t default_conf = {.f = DEFAULT_F, .k0 = DEFAULT_K0, .k1 = DEFAULT_K1, .k2 = DEFAULT_K2,
                            .r = DEFAULT_R, .g = DEFAULT_G, .c = DEFAULT_C};

    while(-1 != (opt = getopt(argc, argv, "f:k:l:m:r:g:c:i:h"))) {
        switch(opt) {
        case 'f':
            default_conf.f = atoi(optarg);
            break;
        case 'k':
            default_conf.k0 = atoi(optarg);
            break;
        case 'l':
            default_conf.k1 = atoi(optarg);
            break;
        case 'm':
            default_conf.k2 = atoi(optarg);
            break;
        case 'r':
            default_conf.r = atoi(optarg);
            break;
        case 'g':
            default_conf.g = atoi(optarg);
            break;
        case 'c':
            default_conf.c = atoi(optarg);
            break;
        case 'i':
            inFile = fopen(optarg, "r");
            if (inFile == NULL) {
                fprintf(stderr, "Failed to open %s for reading\n", optarg);
                print_help_and_exit();
            }
            break;
        case 'h':
        default:
            print_help_and_exit();
            break;
        }
    }

    print_config(&default_conf); // Print run configuration

    setup_proc(&default_conf); // Setup the processor

    proc_stats_t stats;
    memset(&stats, 0, sizeof(proc_stats_t));

    run_proc(&stats); // Run the processor

    complete_proc(&stats); // Finalize statistics and perform cleanup

    fclose(inFile); // release file descriptor memory

    print_statistics(&stats);

    return 0;
}
