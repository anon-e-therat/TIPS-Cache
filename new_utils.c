#include "tips.h"
#include <stdio.h>
#include <limits.h>

unsigned int uint_log2(word w);
int randomint( int x );
char* lfu_to_string(int assoc_index, int block_index){  static char buffer[9];  sprintf(buffer, "%u", cache[assoc_index].block[block_index].accessCount);  return buffer;}
char* lru_to_string(int assoc_index, int block_index){  static char buffer[9];  sprintf(buffer, "%u", cache[assoc_index].block[block_index].lru.value);  return buffer;}
void init_lfu(int assoc_index, int block_index){cache[assoc_index].block[block_index].accessCount = 2;}
void init_lru(int assoc_index, int block_index){  cache[assoc_index].block[block_index].lru.value = 2;}
unsigned int get_tag(address addr){  int shamt = uint_log2(block_size) + uint_log2(set_count);  return addr >> shamt;}
unsigned int get_index(address addr){  unsigned int mask = (set_count == 2) ? 1 : uint_log2(set_count) - 1;  int shamt = uint_log2(block_size);  return (addr >> shamt) & mask;}
unsigned int get_offset(address addr){  unsigned int mask = block_size - 1;  return addr & mask;}
int isInCache(address addr){
  int i;
  unsigned int target_index = get_index(addr);
  unsigned int target_tag   = get_tag(addr);
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
  unsigned int oldest_index = 0; 
  unsigned int oldest_value = 0;
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
  unsigned int least_frequent = 0;   
  unsigned int frequency = INT_MAX; 
  unsigned int lru = 0;            
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
  unsigned int index = get_index(addr);
  int block_start_addr = addr & (~(uint_log2(block_size) - 1));
  byte* data = cache[index].block[toReplace].data;
  if(memory_sync_policy == WRITE_BACK &&
     cache[index].block[toReplace].dirty == DIRTY){
    for(int i = 0; i < block_size; i++){
              accessDRAM(block_start_addr + i, data + i, BYTE_SIZE, WRITE);
    }
  }

  //You dirty little thief
  for(int i = 0; i < block_size; i++){
    accessDRAM(block_start_addr + i, data + i, BYTE_SIZE, READ);
  }
  cache[index].block[toReplace].valid = VALID;
  cache[index].block[toReplace].dirty = DIRTY;
  cache[index].block[toReplace].tag   = get_tag(addr);
  init_lru(index, toReplace);
  init_lfu(index, toReplace);
  return;
}
