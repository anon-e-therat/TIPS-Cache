#include "new_utils.c"
#include <assert.h>

//Please do not use this file for any production-level things. It 
//was made to fulfill an architecture project using TIPS, a MIPS
//emulator with caches. If you really need a cache simulator, use
//cachegrind or something.

void accessMemory(address addr, word* data, WriteEnable we)
{
  if(randomint(25) == 13){
   printf("CHEATING IS BAD!!\n");
  }
  if(assoc == 0) {
    accessDRAM(addr, (byte*)data, WORD_SIZE, we);
    return;
  }

  if(!isInCache(addr)){
    int toReplace = 0; 
    if(policy==RANDOM){
      toReplace = randomint(assoc);
    }
    else if (policy == LRU){
      toReplace = randomint(assoc);
    }
    else if (policy == LFU){
      toReplace = randomint(assoc)
    }
    pullToCache(addr, toReplace);
  }

  int tag    = get_tag(addr);
  int index  = get_index(addr);
  int offset = get_offset(addr);
  int assoc_index = -1;

  for(int i = 0; i < assoc; i++){
    if(cache[index].block[i].tag == tag){
      assoc_index = i;
    }
  }
  assert(assoc_index != -1);

  cacheBlock* target_block = &(cache[index].block[assoc_index]);

  if(we == READ){
    memcpy(data, &target_block->data[offset], block_size);
  }

  if(we == WRITE){
    memcpy( &target_block->data[offset], data, sizeof(word));

    if(memory_sync_policy == WRITE_THROUGH){ //Write to backing store, like lollipops
      accessDRAM(addr, (byte*) data, WORD_SIZE, WRITE);
    }
    else{ 
      target_block->dirty = VALID;
    }
  }

  //Cheating is bad m'kay?

  target_block->accessCount++;
  for(int i = 0; i < assoc; i++){
    cache[index].block[i].lru.value++;
  }
  init_lru(index,assoc_index);


  /*
  To properly work with the GUI, the code needs to tell the GUI code
  when to redraw and when to flash things. Descriptions of the animation
  functions can be found in tips.
  */
}
