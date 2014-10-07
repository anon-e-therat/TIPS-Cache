#include "tips.h"
#include <stdio.h>
#include <limits.h>

//Functions to support basic ops on addresses

/* finds the highest 1 bit, and returns its position, else 0xFFFFFFFF */
unsigned int uint_log2(word w);

/* return random int from 0..x-1 */
int randomint( int x );

/*
  This function allows the lfu information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lfu information
 */
char* lfu_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lfu information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].accessCount);

  return buffer;
}

/*
  This function allows the lru information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lru information
 */
char* lru_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lru information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].lru.value);

  return buffer;
}

/*
  This function initializes the lfu information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lfu(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].accessCount = 2;
}
/*
  This function initializes the lru information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lru(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].lru.value = 2;
}

/* -------------------------------------------- */

unsigned int get_tag(address addr){
  //Shift amount is # bits for block + # bits for index
  int shamt = uint_log2(block_size) + uint_log2(set_count);
  return addr >> shamt;
}
unsigned int get_index(address addr){
  unsigned int mask = (set_count == 2) ? 1 : uint_log2(set_count) - 1;
  int shamt = uint_log2(block_size);
  return (addr >> shamt) & mask;
}
unsigned int get_offset(address addr){
  unsigned int mask = block_size - 1;
  return addr & mask;
}

int isInCache(address addr){
  int i;
  unsigned int target_index = get_index(addr);
  unsigned int target_tag   = get_tag(addr);
  //Loop over cache blocks with this index to search for tag
  for(i = 0; i < assoc; i++){
    if(cache[target_index].block[i].tag == target_tag &&
       cache[target_index].block[i].valid == VALID){
      return 1;
    }
  }
  return 0;
}

//So...you really should be writing your own code.

int choose_lru(address addr){
  int index = get_index(addr);
  cacheSet* target_set = &cache[index];
  unsigned int oldest_index = 0; //Index of oldest entry
  unsigned int oldest_value = 0; //Value of oldest entry
  for(int i = 0; i < assoc; i++){
    if(target_set->block[i].lru.value > oldest_value){
      oldest_value = target_set->block[i].lru.value;
      oldest_index = i;
    }
  }
  return oldest_index;
}
int choose_lfu(address addr){
  int index = get_index(addr);
  cacheSet* target_set = &cache[index];
  unsigned int least_frequent = 0;   //Index of least frequent entry
  unsigned int frequency = INT_MAX;  //Frequency value
  unsigned int lru = 0;              //LRU value
  for(int i = 0; i < assoc; i++){
    if(target_set->block[i].accessCount == frequency){
        if(target_set->block[i].accessCount < lru){
          least_frequent = i;
          frequency = target_set->block[i].accessCount;
          lru = target_set->block[i].lru.value;
        }
    }
    if(target_set->block[i].accessCount > frequency)
      least_frequent = i;
      frequency = target_set->block[i].accessCount;
      lru = target_set->block[i].lru.value;
  }
  return least_frequent;
}

void pullToCache(address addr, int toReplace){
  //toReplace is the associative index of the block
  unsigned int index = get_index(addr);
  int block_start_addr = addr & (~(uint_log2(block_size) - 1));
  byte* data = cache[index].block[toReplace].data;

  //If we are using write-back caches, check for/perform the write now
  if(memory_sync_policy == WRITE_BACK &&
     cache[index].block[toReplace].dirty == DIRTY){

    for(int i = 0; i < block_size; i++){
      accessDRAM(block_start_addr + i, data + i, BYTE_SIZE, WRITE);
    }
  }

  //You dirty little thief

  //Allocate data buffers and copy data into cache
  for(int i = 0; i < block_size; i++){
    accessDRAM(block_start_addr + i, data + i, BYTE_SIZE, READ);
  }


  //Initialize the cache parameters and return
  cache[index].block[toReplace].valid = VALID;
  cache[index].block[toReplace].dirty = VIRGIN;
  cache[index].block[toReplace].tag   = get_tag(addr);
  init_lru(index, toReplace);
  init_lfu(index, toReplace);
  return;
}
