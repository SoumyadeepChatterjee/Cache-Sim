#include <iostream>
#include "sim.h"
#include <cstdlib>
#include <cmath>
#include <string>
#include <queue>
#include <bitset>
#include <sstream>
#include <stdio.h>

using namespace std;

Cache::Cache(uint32_t BLOCKSIZE, uint32_t CACHESIZE, uint32_t ASSOC, uint32_t LEVEL/*, uint32_t PREF_N, uint32_t PREF_M*/) {
	//set members
	blocksize = BLOCKSIZE;
	cachesize = CACHESIZE;
	assoc = ASSOC;
	level = LEVEL;
	/*
	pref_n = PREF_N;
	pref_m = PREF_M;
	*/
	//NO L2
	this->next_level_pointer = NULL;

	//this-> prefetcher = NULL;
	
	//calculate Cache params for building
	num_sets = (uint32_t)cachesize / (assoc * blocksize);
	num_indexbits = (uint32_t)log2(num_sets);
	num_blockoffsetbits = log2(blocksize);
	num_tagbits = 32 - (num_indexbits + num_blockoffsetbits);


	//build the empty Cache
	CACHE = new block_metadata * [num_sets]; //rows
	for (uint32_t i = 0; i < num_sets; i++) {
		CACHE[i] = new block_metadata[assoc];//ways
		//now make sure you can access/set
		for (uint32_t j = 0; j < assoc; j++) {
			CACHE[i][j].index = i;
			CACHE[i][j].valid_bit = false;
			CACHE[i][j].dirty = false;
			CACHE[i][j].LRU = j;//LRU(0)->MRU, or build in LRU to MRU order? 
			//cout << CACHE[i][j].index << " " << CACHE[i][j].tag << " " << CACHE[i][j].LRU << endl;
		}
	}
	/*
	block_metadata** stream_buffer = NULL;
	if (pref_n != 0) {
		//build empty stream buffer, assign blockaddresses to it later
		//same kind of 2d m*n array
		//prefetcher should store blockaddresses
		//build streambuffer queue f pref_m prefetcher_metadata objects
		for (uint32_t i = 0; i < pref_m; i++) {
			stream_buffer = new block_metadata* [pref_m];
			for (uint32_t j = 0; j < pref_m; j++) {
				stream_buffer[i][j].LRU = j; //LRU(0) is MRU
				stream_buffer[i][j].index = i;
			}
		}
	}
	*/
}


//need to do hex -> binary -> split into tag, index, blockoffset ACCORDING to cache's level and parameters
void Cache::convert_input_address(uint32_t hex_addr) {

	//cout << hex << hex_addr << endl;//correct values being passed in
	//Logic: left shift hex_addr by remaining number of bits to get ABCD0000....
	//Next, right shift that by same number of bit to get 00000000...ABCD
	//Finally, profit
	input_tag = hex_addr >> (32 - num_tagbits);
	//cout << hex<<"TAG " <<input_tag << endl;

	input_index = (hex_addr >> num_blockoffsetbits) & ((1 << num_indexbits) - 1);
	//cout <<hex<<"INDEX "<<input_index << endl;

	uint32_t temp = (hex_addr << (32 - num_blockoffsetbits));
	input_blockoffset = temp >> (32 - num_blockoffsetbits);
	//cout << hex<<"BLOCKOFFSET " << input_blockoffset << endl;
}


//Just go through all blocks in the search set to determine if any tag matches, return true if yes (HIT)
bool Cache::is_hit(uint32_t search_index, uint32_t search_tag) { //uint32* way) {
	for (uint32_t i = 0; i < assoc; i++) {
		//printf("input_index=%d, input_tag=%d, i=%d\n", input_index, input_tag, i);
		//printf("valid_bit: %d\n", CACHE[input_index][i].valid_bit);
		//printf("end---\n");
		//cout << i << endl;
		if ((CACHE[search_index][i].tag == input_tag) && CACHE[search_index][i].valid_bit == 1) {
			//*way = i;
			return true; //SEARCH HIT	
		}
	}
	//SEARCH MISS
	return false;
}
//Exception thrown: read access violation.
//this->CACHE was 0x111011101110111.

//returns the way location of the block where tag matches
uint32_t Cache::search_cache(uint32_t input_index, uint32_t input_tag) {
	uint32_t hit_way = 0;//fix direct mapped bug
	for (uint32_t i = 0; i < assoc; i++) {
		if (CACHE[input_index][i].valid_bit) {
			if (CACHE[input_index][i].tag == input_tag) {
				hit_way = i;
				break;
			}
		}
	}
	return hit_way;
}

//Returns the way location of the LRU block, to be used for CacheOps
uint32_t Cache::findLRU(uint32_t index_to_update) {
	uint32_t way_to_update = 0;
	if (assoc == 1) {
		return way_to_update;
	}
	//Iterate through all blocks in the index, return the block with the max LRU value
	//max LRU value is assoc - 1
	else {
		for (way_to_update = 0; way_to_update < assoc; way_to_update++) {
			if (CACHE[index_to_update][way_to_update].LRU == assoc - 1) {
				return way_to_update;
			}
		}
		return way_to_update;
	}
}

//Update the LRU in the index, and setting MRU block's LRU to 0 
void Cache::updateLRU(uint32_t input_index, uint32_t input_toCache_way) {
	//the maximum LRU value possible is (assoc-1)
	//MRU is 0
	//MRU is the block we passed in, so we need to set that to zero
	//Algorithm -> all LRUs that are less than this block need to be increased
	if (assoc == 1) {
		CACHE[input_index][input_toCache_way].LRU = 0;
		return;
	}
	else {
		for (uint32_t i = 0; i < assoc; i++) {
			if (i != input_toCache_way) {
				if (CACHE[input_index][i].LRU < CACHE[input_index][input_toCache_way].LRU) {
					CACHE[input_index][i].LRU++;
				}
			}
		}
		//Now set 0 to designate MRU as the block we just operated on
		CACHE[input_index][input_toCache_way].LRU = 0;
	}
}


//Returns the full hex addr (with random (zero?) Boffset vals; works because Boff will be same across all levels 
uint32_t Cache::return_victim_full_address(uint32_t index_victim, uint32_t tag_victim) {
	uint32_t victim_hex_addr;
	uint32_t temp_victimtag = tag_victim << (num_blockoffsetbits + num_indexbits);
	uint32_t temp_victimindex = index_victim << num_blockoffsetbits;
	victim_hex_addr = temp_victimtag | temp_victimindex;
	return victim_hex_addr;
}

//EVICTION BEFORE ALLOCATION
//Handles all reads, cascaded too
void Cache::read_Cache(uint32_t read_addr) {
	//need a function to extract block address from read_addr, store it in a uint32_t
	//uint32_t block_address;
	stats.reads++;
	//convert address to searchable parameters
	convert_input_address(read_addr);
	//search cache
	//uint32_t search_hit_way = search_cache(input_index, input_tag);
	//find victim block, either to update or evict
	uint32_t victim_index = findLRU(input_index);
	uint32_t victim_full_address;
	//HIT or MISS workflows
	//uint32_t way = 0;
	if (is_hit(input_index, input_tag)) { //&way)) {//if(uint32_t result_way = search_cache(input_index,input_tag) 
		uint32_t search_hit_way = search_cache(input_index, input_tag);
		victim_index = search_hit_way;
	//update_lru(way);
	}
	else {
		stats.read_misses++;
		//victim_way = find_LRU();
		//if Dirty, then eviction->writeback->readrequest needed
		if ((CACHE[input_index][victim_index].dirty) && (CACHE[input_index][victim_index].valid_bit)) {
			stats.writebacks++;
			if (next_level_pointer == NULL) {
				stats.memory_accesses++;
			}
			else {
				//add L2 logic here
				//cascade request to -> {victimblock.tag, victimblock.index,victim.blockaddress}
				//L2.write_cache(hex
				//the following should be victim_tag right
				uint32_t victim_tag = CACHE[input_index][victim_index].tag;
				uint32_t address_to_writeback = return_victim_full_address(input_index, victim_tag);
				next_level_pointer->write_Cache(address_to_writeback);
				
				
			}
			//Now clean, evict the block
			CACHE[input_index][victim_index].dirty = false;
			CACHE[input_index][victim_index].valid_bit = false;
			CACHE[input_index][victim_index].LRU = assoc - 1;
			CACHE[input_index][victim_index].tag = 0;
		}
		//if it's the last level, read is requested from memory, so only increase those stats
		if (next_level_pointer == NULL) {
			stats.memory_accesses++;
			CACHE[input_index][victim_index].valid_bit = 1;
			CACHE[input_index][victim_index].dirty = false;
			CACHE[input_index][victim_index].tag = input_tag;
			CACHE[input_index][victim_index].LRU = assoc - 1;//since we are allocating to a block that was the LRU
		}
		else {
			//CACHE[input_index][victimindex] = read(L2)
			//next_level_pointer->read_cache(read_addr);
			//CACHE[input_index][victim_nidex].valid = true;
			//CACHE[input_index][victim_nidex].tag = input_tag
			victim_full_address = return_victim_full_address(input_index, input_tag);
			next_level_pointer->read_Cache(victim_full_address);
			CACHE[input_index][victim_index].valid_bit = true;
			CACHE[input_index][victim_index].dirty = false;
			CACHE[input_index][victim_index].tag = input_tag;
			CACHE[input_index][victim_index].index = input_index;
			CACHE[input_index][victim_index].LRU = assoc - 1;
			
			CACHE[input_index][victim_index].tag = input_tag;//just set tag now, since we've "got" the block
			
		}
	}
	updateLRU(input_index, victim_index);
	/*
	for (int i = 0; i < assoc; i++) {
		cout << "set: " << i << CACHE[input_index][i].tag << " " << CACHE[input_index][i].LRU;
	}
	cout << endl;
	*/
}
//EVICTION BEFORE ALLOCATION
//Handles all writes, cascaded too
void Cache::write_Cache(uint32_t write_addr) {
	uint32_t current_address = write_addr;
	uint32_t victim_full_address;
	stats.writes++;
	convert_input_address(write_addr);
	//search cache
	//uint32_t search_hit_way = search_cache(input_index, input_tag);
	//find victim block, either to update or evict
	uint32_t victim_index = findLRU(input_index);
	//HIT or MISS workflows
		//WRITE HIT
	if (is_hit(input_index, input_tag)) {
		uint32_t search_hit_way = search_cache(input_index, input_tag);
		victim_index = search_hit_way;
	}
	else {
		//WRITE MISS
		stats.write_misses++;
		//if Dirty, then eviction->writeback->readrequest needed
		if ((CACHE[input_index][victim_index].dirty) && (CACHE[input_index][victim_index].valid_bit)) {
			stats.writebacks++;
			if (next_level_pointer == NULL) {
				stats.memory_accesses++;
			}
			else {
				//add L2 logic here
				uint32_t victim_tag = CACHE[input_index][victim_index].tag;
				uint32_t address_to_writeback = return_victim_full_address(input_index, victim_tag);
				next_level_pointer->write_Cache(address_to_writeback);
			}
			//END OF WRITEBACK_VICTIM_IF_NEEDED
			//Now, we clean the block in our cache
			CACHE[input_index][victim_index].dirty = false;
			CACHE[input_index][victim_index].valid_bit = false;
			CACHE[input_index][victim_index].LRU = assoc - 1;
			CACHE[input_index][victim_index].tag = 0;
		}
		//Now, issue read to next level
		//if it's the last level, read is requested from memory
		if (next_level_pointer == NULL) {
			stats.memory_accesses++;
			CACHE[input_index][victim_index].valid_bit = 1;
			CACHE[input_index][victim_index].dirty = false;
			CACHE[input_index][victim_index].tag = input_tag;
			CACHE[input_index][victim_index].LRU = assoc - 1;//since we are allocating to a block that was the LRU
		}
		else {
			victim_full_address = return_victim_full_address(input_index, input_tag);
			next_level_pointer->read_Cache(victim_full_address);
			CACHE[input_index][victim_index].valid_bit = true;
			CACHE[input_index][victim_index].dirty = false;
			CACHE[input_index][victim_index].tag = input_tag;
			CACHE[input_index][victim_index].index = input_index;
			CACHE[input_index][victim_index].LRU = assoc - 1;
		}
	}
	CACHE[input_index][victim_index].dirty = true;
	updateLRU(input_index, victim_index);
	/*
	for (int i = 0; i < assoc; i++) {
		cout << "set: " << i <<" " << CACHE[input_index][i].tag << " " << CACHE[input_index][i].LRU;
	}
	cout << endl;
	*/
}
/*
uint32_t Cache::search_buffer(uint32_t) {
	return 0;
}

bool Cache::hit_in_buffer(uint32_t) {
	return 0;
}

void Cache::fill_buffer(uint32_t) {

}

void Cache::update_bufferLRU(uint32_t) {

}

void Cache::print_buffer() {

}
*/

void Cache::print_Cache() {
	//cout << "I printed a cache" << endl;
	cout << endl;
	string temp;
	if (level == 1) { temp = "L1"; }
	else if (level == 2) { temp = "L2"; }
	cout << "===== " << temp << " contents =====";
	for (uint32_t i = 0; i < num_sets; i++) {
		printf("\nset%7d: ", i);
		for (uint32_t j = 0; j < assoc; j++) {
			for (uint32_t k = 0; k < assoc; k++) {
				if (CACHE[i][k].LRU == j) {
					//cout << hex << CACHE[i][k].tag << " ";
					printf("%8x ", CACHE[i][k].tag);
					if (CACHE[i][k].dirty)
						printf("D");//cout << "D\t";
					else
						//cout << " \t";
						printf(" ");
				}
			}
		}
	}
}

