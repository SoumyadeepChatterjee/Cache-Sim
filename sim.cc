#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cmath>
#include <vector>
#include <string>
#include <queue>
#include <iostream>  
using namespace std;

//16 1024 2 0 0 0 0 gcc_trace.txt
/*  "argc" holds the number of command-line arguments.
    "argv[]" holds the arguments themselves.

    Example:
    ./sim 32 8192 4 262144 8 3 10 gcc_trace.txt
    argc = 9
    argv[0] = "./sim"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/

int main(int argc, char* argv[]) {
    FILE* fp;			// File pointer.
    char* trace_file;		// This variable holds the trace file name.
    cache_params_t params{};	// Look at the sim.h header file for the definition of struct cache_params_t.
    char rw;			// This variable holds the request's type (read or write) obtained from the trace.
    uint32_t addr;		// This variable holds the request's address obtained from the trace.
    // The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.

// Exit with an error if the number of command-line arguments is incorrect.
    if (argc != 9) {
        printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
        exit(EXIT_FAILURE);
    }

    // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
    params.BLOCKSIZE = (uint32_t)atoi(argv[1]);
    params.L1_SIZE = (uint32_t)atoi(argv[2]);
    params.L1_ASSOC = (uint32_t)atoi(argv[3]);
    params.L2_SIZE = (uint32_t)atoi(argv[4]);
    params.L2_ASSOC = (uint32_t)atoi(argv[5]);
    params.PREF_N = (uint32_t)atoi(argv[6]);
    params.PREF_M = (uint32_t)atoi(argv[7]);
    trace_file = argv[8];

    // Open the trace file for reading.
    fp = fopen(trace_file, "r");
    if (fp == (FILE*)NULL) {
        // Exit with an error if file open failed.
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }//29
	
    //35
	Cache L1(params.BLOCKSIZE, params.L1_SIZE, params.L1_ASSOC, 1);//Call L1 no matter what
    if (params.L2_SIZE > 0) {
		Cache L2(params.BLOCKSIZE, params.L2_SIZE, params.L2_ASSOC, 2);
        L1.next_level_pointer = &L2;

        // Read requests from the trace file and echo them back (UPDATES READ AND WRITES FOR INITIAL REQUEST)
        while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {	// Stay in the loop if fscanf() successfully parsed two tokens as specified.
            if (rw == 'r') {
                L1.read_Cache(addr);
            }
            else if (rw == 'w') {
                L1.write_Cache(addr);
            }
        }

        // Print simulator configuration.
        printf("===== Simulator configuration =====\n");
        printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
        printf("L1_SIZE:    %u\n", params.L1_SIZE);
        printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
        printf("L2_SIZE:    %u\n", params.L2_SIZE);
        printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
        printf("PREF_N:     %u\n", params.PREF_N);
        printf("PREF_M:     %u\n", params.PREF_M);
        printf("trace_file: %s\n", trace_file);

        double L1_miss_rate = (double)(L1.stats.read_misses + L1.stats.write_misses) / (L1.stats.reads + L1.stats.writes);
        double L2_miss_rate = (double)(L2.stats.read_misses) / (L2.stats.reads);
        L1.stats.memory_accesses = (L2.stats.read_misses + L2.stats.read_misses + L2.stats.write_misses + L2.stats.writebacks + L2.stats.prefetches);
        uint32_t total_memory_traffic = L2.stats.memory_accesses;
        L1.print_Cache();
        cout << endl;
        L2.print_Cache();
		
        printf("\n\n===== Measurements =====\n");
        printf("a. L1 reads:                   %u\n", L1.stats.reads);
        printf("b. L1 read misses:             %u\n", L1.stats.read_misses);
        printf("c. L1 writes:                  %u\n", L1.stats.writes);
        printf("d. L1 write misses:            %u\n", L1.stats.write_misses);
        printf("e. L1 miss rate:               %.4f\n", L1_miss_rate);
        printf("f. L1 writebacks:              %u\n", L1.stats.writebacks);
        printf("g. L1 prefetches:              %u\n", L1.stats.prefetches);
        printf("h. L2 reads (demand):          %u\n", L2.stats.reads);
        printf("i. L2 read misses (demand):    %u\n", L2.stats.read_misses);
        printf("j. L2 reads (prefetch):        %u\n", 0);
        printf("k. L2 read misses (prefetch):  %u\n", 0);
        printf("l. L2 writes:                  %u\n", L2.stats.writes);
        printf("m. L2 write misses:            %u\n", L2.stats.write_misses);
        printf("n. L2 miss rate:               %.4f\n", L2_miss_rate);
        printf("o. L2 writebacks:              %u\n", L2.stats.writebacks);
        printf("p. L2 prefetches:              %u\n", L2.stats.prefetches);
        printf("q. memory traffic:             %u\n", total_memory_traffic);
    }
    else {

			// Read requests from the trace file and echo them back (UPDATES READ AND WRITES FOR INITIAL REQUEST)
			while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {	// Stay in the loop if fscanf() successfully parsed two tokens as specified.
				if (rw == 'r') {
					L1.read_Cache(addr);
				}
				else if (rw == 'w') {
					L1.write_Cache(addr);
				}
			}

			// Print simulator configuration.
			printf("===== Simulator configuration =====\n");
			printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
			printf("L1_SIZE:    %u\n", params.L1_SIZE);
			printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
			printf("L2_SIZE:    %u\n", params.L2_SIZE);
			printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
			printf("PREF_N:     %u\n", params.PREF_N);
			printf("PREF_M:     %u\n", params.PREF_M);
			printf("trace_file: %s\n", trace_file);

			double L1_miss_rate = (double)(L1.stats.read_misses + L1.stats.write_misses) / (L1.stats.reads + L1.stats.writes);
			L1.stats.memory_accesses = (L1.stats.read_misses + L1.stats.write_misses + L1.stats.writebacks + L1.stats.prefetches);
			uint32_t total_memory_traffic = L1.stats.memory_accesses;
			L1.print_Cache();
            
            printf("\n\n===== Measurements =====\n");
            printf("a. L1 reads:                   %u\n", L1.stats.reads);
            printf("b. L1 read misses:             %u\n", L1.stats.read_misses);
            printf("c. L1 writes:                  %u\n", L1.stats.writes);
            printf("d. L1 write misses:            %u\n", L1.stats.write_misses);
            printf("e. L1 miss rate:               %.4f\n", L1_miss_rate);
            printf("f. L1 writebacks:              %u\n", L1.stats.writebacks);
            printf("g. L1 prefetches:              %u\n", L1.stats.prefetches);
            printf("h. L2 reads (demand):          %u\n", 0);
            printf("i. L2 read misses (demand):    %u\n", 0);
            printf("j. L2 reads (prefetch):        %u\n", 0);
            printf("k. L2 read misses (prefetch):  %u\n", 0);
            printf("l. L2 writes:                  %u\n", 0);
            printf("m. L2 write misses:            %u\n", 0);
            printf("n. L2 miss rate:               %.4f\n",0.0000);
            printf("o. L2 writebacks:              %u\n",0);
            printf("p. L2 prefetches:              %u\n", 0);
            printf("q. memory traffic:             %u\n", total_memory_traffic);
			

    }
    return(0);
}

/*
printf("\n\n===== Measurements =====\n");
    printf("a. L1 reads:                   %u\n", L1.stats.reads);
    printf("b. L1 read misses:             %u\n", L1.stats.read_misses);
    printf("c. L1 writes:                  %u\n", L1.stats.writes);
    printf("d. L1 write misses:            %u\n", L1.stats.write_misses);
    printf("e. L1 miss rate:               %.4f\n", L1_miss_rate);
    printf("f. L1 writebacks:              %u\n", L1.stats.writebacks);
    printf("g. L1 prefetches:              %u\n", L1.stats.prefetches);
    printf("h. L2 reads (demand):          %u\n", L2.stats.reads);
    printf("i. L2 read misses (demand):    %u\n", L2.stats.read_misses);
    printf("j. L2 reads (prefetch):        %u\n", 0);
    printf("k. L2 read misses (prefetch):  %u\n", 0);
    printf("l. L2 writes:                  %u\n", L2.stats.writes);
    printf("m. L2 write misses:            %u\n", L2.stats.write_misses);
    printf("n. L2 miss rate:               %.4f\n", L2_miss_rate);
    printf("o. L2 writebacks:              %u\n", L2.stats.writebacks);
    printf("p. L2 prefetches:              %u\n", L2.stats.prefetches);
    printf("q. memory traffic:             %u\n", total_memory_traffic);
*/