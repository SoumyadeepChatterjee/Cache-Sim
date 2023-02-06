#ifndef SIM_CACHE_H
#define SIM_CACHE_H
#include <inttypes.h>
#include <string>
#include <queue>
typedef struct cache_params_t {
    uint32_t BLOCKSIZE;
    uint32_t L1_SIZE;
    uint32_t L1_ASSOC;
    uint32_t L2_SIZE;
    uint32_t L2_ASSOC;
    uint32_t PREF_N; // number of stream buffers in L1, or L2 (if L2 exists)
    uint32_t PREF_M; // number of memory blocks in each stream buffer unit
} cache_params_t;

// must have everything for L1, L2
// NOTE: include prefetch stuff here?
typedef struct block_metadata {
    bool valid_bit;
    bool dirty; 
    uint32_t tag;
    uint32_t index;
    uint32_t block_offset;
    uint32_t LRU;
}block_metadata;


typedef struct measurements {
    uint32_t reads=0;
    uint32_t read_misses=0;
    uint32_t writes=0;
    uint32_t write_misses=0;
    uint32_t writebacks=0;
    uint32_t memory_accesses=0;
    uint32_t prefetches=0;
}measurements;

class Cache {
public:
    block_metadata** CACHE;
    // block_metadata curr_block; - Used simple assignment instead
    Cache* next_level_pointer;
    /*
    block_metadata** stream_buffer; //////// TODO: ADD PREF SPECIFIC FUNCTIONS
    uint32_t search_buffer(uint32_t);
    bool hit_in_buffer(uint32_t);
    void fill_buffer(uint32_t);
    void print_buffer();
    void update_bufferLRU(uint32_t);
    */
    // Constructor
    //Cache() {};
    Cache(uint32_t BLOCKSIZE, uint32_t CACHESIZE, uint32_t ASSOC, uint32_t LEVEL/*, uint32_t PREF_N, uint32_t PREF_M*/); // BLOCKSIZE, CACHESIZE, ASSOC, LEVEL)

    void read_Cache(uint32_t read_addr);
    void write_Cache(uint32_t write_addr);
    void print_Cache();

    //uint32_t return_empty_space_in_set(uint32_t input_index);

    uint32_t return_victim_full_address(uint32_t index_victim, uint32_t tag_victim);
    //uint32_t find_empty_LRU_way(uint32_t empty_lru_search_index);//gives out LRU(empty way) in a set, used only to ensure correct allocation
    
    //void print_measurements(); - needed if measurements are parametrized into classObj.instance)
    measurements stats;

    // Init params set for CacheOps
    // Should probably be private, unless translate or write needs them

    // Calc of what the content tags ARE should be done from ADDR

    // Read in addr params, to convert
    uint32_t input_tag=0;
    uint32_t input_index=0;
    uint32_t input_blockoffset = 0;
    uint32_t input_toCache_way = 0;

    uint32_t num_sets;
    uint32_t num_indexbits;
    uint32_t num_blockoffsetbits;
    uint32_t num_tagbits;
    uint32_t blocksize;
    uint32_t cachesize;
    uint32_t assoc;
    uint32_t level;
    /*
    uint32_t pref_n;
    uint32_t pref_m;
    */
private:
    


    void convert_input_address(uint32_t hex_addr); // converts uint32_t to search params according to cache level, params
    //searches for match; if HIT - returns HIT.way, if MISS - returns -1
    uint32_t search_cache(uint32_t input_index, uint32_t input_tag);
    bool is_hit(uint32_t search_index, uint32_t search_tag);
    uint32_t findLRU(uint32_t index_to_update); // if set is full, finds LRU block for eviction
    void updateLRU(uint32_t input_index, uint32_t input_toCache_way); // updates LRU for a given index and way
    //bool find_space_in_set(uint32_t index_to_search); // checks if a set is full
};

#endif
