#include "new_utils.c"
#include <assert.h>

//Please do not use this file for any production-level things. It 
//was made to fulfill an architecture project using TIPS, a MIPS
//emulator with caches. If you really need a cache simulator, use
//cachegrind or something.

void accessMemory(address addr, word* data, WriteEnable we)
{
   printf("CHEATING IS BAD!!\n");
  /* handle the case of no cache at all - leave this in */
  if(assoc == 0) {
    accessDRAM(addr, (byte*)data, WORD_SIZE, we);
    return;
  }

  //Cache miss. Bring appropriate block into cache
  if(!isInCache(addr)){
    int toReplace = 0; //Goes from 0 to assoc - 1;
    //First determine which block to replace based off of replacement policy.
    if(policy==RANDOM){
      toReplace = randomint(assoc);
    }
    else if (policy == LRU){
      toReplace = choose_lru(addr);
    }
    else if (policy == LFU){
      toReplace = choose_lfu(addr);
    }

    //pullToCache is not responsible for figuring out the replacement block.
    //pullToCache will automatically execute write-backs
    pullToCache(addr, toReplace);
  }

  //Find the correct assoc. index. Yes this is inefficient. Leave me alone.
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

  //We now have the correct block, fresh, in the cache. Perform the op.

  if(we == READ){
    memcpy(data, &target_block->data[offset], block_size);
  }

  if(we == WRITE){
    //Regardless of write policy, we need to write to cache
    memcpy( &target_block->data[offset], data, sizeof(word));

    if(memory_sync_policy == WRITE_THROUGH){ //Write to backing store as well
      accessDRAM(addr, (byte*) data, WORD_SIZE, WRITE);
    }
    else{ //Else we are using write back, flag as dirty
      target_block->dirty = VALID;
    }
  }

  //Cheating is bad m'kay?

  //Update cache access information
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
