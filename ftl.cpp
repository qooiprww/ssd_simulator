#include "ftl.hpp"
#include <string>
#include <iostream>

using namespace std;

long long LOGICAL_FLASH_SIZE = 1;
long long OP_REGION ;
long long LOGICAL_PAGE ;
long long FLASH_SIZE;

long long BLOCKS_PER_FLASH = 1;
long long PAGES_PER_FLASH = 1024;
int cpu_num = 1;

STATISTICS total_stat;
STATISTICS cpu_stat [CPU_MAX];
LOGICAL_MAP *logical_map;
PHYSICAL_MAP *physical_map;
BLOCK_MAP *block_map;
CURRENT_STATE *current_state;
FREE_BLOCKS free_blocks;

void init_stat() {

    total_stat.read_cnt = 0;
    total_stat.write_cnt = 0;
    total_stat.discard_cnt = 0;
    total_stat.copyback_cnt = 0;
    total_stat.gc_cnt = 0;

//    for (i=0 ; i<streamNum ; i++) {
//        streamStat[i].read = 0;
//        streamStat[i].write = 0;
//        streamStat[i].block.copyback = 0;
//        streamStat[i].block.gcCnt = 0;
//        streamStat[i].bef_write = 0;
//        streamStat[i].bef_block.copyback = 0;
//        streamStat[i].bef_block.gcCnt = 0;
//    }
}

void ftl_init() {
    init_stat();
    
    free_blocks.cnt = BLOCKS_PER_FLASH;
    free_blocks.head = 0;
    free_blocks.tail = BLOCKS_PER_FLASH - 1;
    

    logical_map = (LOGICAL_MAP*) malloc(sizeof(LOGICAL_MAP) * LOGICAL_PAGE);
    physical_map = (PHYSICAL_MAP*) malloc(sizeof(PHYSICAL_MAP) * PAGES_PER_FLASH);
    block_map = (BLOCK_MAP*) malloc(sizeof(BLOCK_MAP) * BLOCKS_PER_FLASH);

    current_state = (CURRENT_STATE*) malloc(sizeof(CURRENT_STATE) * cpu_num);
        
    for(long long i = 0; i < PAGES_PER_FLASH; i++) {
        if(i < LOGICAL_PAGE){
            logical_map[i].num = -1;
        }
        if(i < BLOCKS_PER_FLASH) {
            block_map[i].invalid_cnt = -1;
            block_map[i].next_block = i + 1;
            block_map[i].erase_cnt = 0;
            block_map[i].cpu_id = -1;
        }
        physical_map[i].num = -1;
        physical_map[i].is_valid = 0;
    }
    block_map[BLOCKS_PER_FLASH - 1].next_block = -1;

 
    for (long long i = 0; i < cpu_num; i++) {
        current_state[i].block = -1;
        current_state[i].page = PAGES_PER_BLOCK;
    }
}


void ftl_close() {
    free(current_state);
    free(physical_map);
    free(logical_map);
    free(block_map);
}


void ftl_erase (int block_num) {
    block_map[free_blocks.tail].next_block = block_num;
    free_blocks.tail = block_num;
    free_blocks.cnt++;
    block_map[free_blocks.tail].next_block = -1;
    
    block_map[block_num].erase_cnt++;
    block_map[block_num].invalid_cnt = -1; // TODO: Why isn't it 0?
    block_map[block_num].cpu_id = -1;
    
    try {
        if(free_blocks.head == -1){
            free_blocks.head = block_num;
            if(free_blocks.head != free_blocks.tail) {
                throw string("[ERROR] ftl_erase: free_blocks' linked list has an issue!");
            }
        }
    } catch (string err_message) {
        cout << err_message << endl;
        exit(0);
    }
}

void ftl_copyback (int block_num) {
    int start_page = block_num * PAGES_PER_BLOCK;
    
    for (int cur_page = start_page; cur_page < start_page + PAGES_PER_BLOCK; ++cur_page) {
        if (physical_map[cur_page].is_valid) {
            
            try {
                if (current_state[0].page >= PAGES_PER_BLOCK) { // Reached the end of the current block!
                    // TODO: check if there's any free block available!
                    throw string("[ERROR] ftl_gc:ftl_copybakc: reached end of the block!");
                }
            } catch (string err_message) {
                cout << err_message << endl;
                exit(0);
            }

            int page = physical_map[cur_page].num;
            
            // Update old pysical page's status
            physical_map[cur_page].num = -1;
            physical_map[cur_page].is_valid = 0;

            // Copyback
            int new_phys_num = current_state[0].block * PAGES_PER_BLOCK + current_state[0].page;
            logical_map[page].num = new_phys_num;
            physical_map[new_phys_num].num = page;
            physical_map[new_phys_num].is_valid = 1;
        
            // Update stats
            total_stat.copyback_cnt++;
            current_state[0].page++;
        }
    }

    // TODO: update cpu_stat
}


////////
// (@VIC) greedy victim selection algorithm
int victimGreedy() {
    // TODO: refactor this
    long long i;
    int max, res, secondMax, flag;
    
    res = 0;
    max = block_map[0].invalid_cnt;
    secondMax = max;

    // select most invalidated block
    for (i=1 ; i<BLOCKS_PER_FLASH ; i++) {
        if (block_map[i].invalid_cnt > max) {
            max = block_map[i].invalid_cnt;
            res = i;
        }
    }

    for(i= 0 ; i< cpu_num ; i++)
        if(res == current_state[i].block) {
//            printf("[NOTE] (%s, %d) : select update block as a victim (%d %lld)\n", __func__, __LINE__,res, i);
        }

//    verify.maxInvalidity = max;
    if (max > PAGES_PER_BLOCK){
//        printf("[ERROR] (%s, %d) : max invalidity too high\n", __func__, __LINE__);
        //printCount();
    }

    return res;
}



int ftl_gc() {
    
    // Find the proper block for gc
    int gc_block = victimGreedy();
    
    // Copyback the valid pages of gc_block and mark the old physical pages as stale
    ftl_copyback(gc_block);
    
    // Erase and free gc_block and add it to free_blocks linked list
    ftl_erase(gc_block);

    total_stat.gc_cnt++;
    // TODO: add cpu_id
    
    return gc_block;
}


int fetch_free_block(int cpu_id) {

    try {
        if (free_blocks.cnt == 0)
            throw string("[ERROR] Couldn't find any free block!\n");
    } catch(string err_message) {
        cout << err_message << endl;
        exit(0);
    }
    
    // Here we point to the next consecutive free block!
    int free_block_num = free_blocks.head;
    free_blocks.head = block_map[free_block_num].next_block;
    free_blocks.cnt--;
    
    // Update new free block's status
    block_map[free_block_num].cpu_id = cpu_id;
    
    
    
    // TODO: move this to GC
    // block_map[free_block_num].invalid_cnt = 0;

    return free_block_num;
}


// this function is for searching free block from free block list
// if there is free block less than 1, perform GC
int search_free_block(int cpu_id) {
    // TODO: hanlde multi-cpu case!
    
    if (cpu_num == 1) {
        if (free_blocks.cnt <= 1) {
            current_state[cpu_id].block = fetch_free_block(cpu_id);
            current_state[cpu_id].page = 0;

            ftl_gc();
            return -1;
        }
    } else {
        // TODO: bluh
//        while (freeMeta.freeBlock <= 1) {
//            int ret = M_GC_stream(); // return code may represent error or gc victim stream number
//            if(ret < 0)    return -2;
//            if (ret == streamID)
//                return -1;
//        }
    }
    
    current_state[cpu_id].block = fetch_free_block(cpu_id);
    current_state[cpu_id].page = 0;

//    if (free_blocks.head == -1) {
//        printf("[ERROR] (%s, %d)\n", __func__, __LINE__);
//        return -2;
//    }

    return 1;
}



int ftl_write(int page_num, int cpu_id) {
    
    try {
        if (cpu_num <= cpu_id)
            throw(cpu_id);
    }
    catch (int cpu_id) {
        cout << "[ERROR] cpu_id(" << cpu_id << ") is out of bound!\n";
        exit(0);
    }

    total_stat.write_cnt++;
//    cpu_stat[cpu_id].write_cnt++;
    
    
    if (logical_map[page_num].num != -1) {
        int old_phys_num = logical_map[page_num].num; // Find the physical page assigned to the page
        physical_map[old_phys_num].num = -1; // Break the link between the physical page & the page
        physical_map[old_phys_num].is_valid = 0; // Change the status of the physical page to invalid

        int block_num = old_phys_num / PAGES_PER_BLOCK;
        block_map[block_num].invalid_cnt++; // Update coressponding block's meta-data
    }

    // Write New data to curBlock, curPage
    // if update block is full, get new free block to write
    if(current_state[cpu_id].page == PAGES_PER_BLOCK){
        //get new free block
        int freeIndex = search_free_block(cpu_id);
        
        if (freeIndex == -2){
            printf("[ERROR] search free block failed \n");
            return -2;
        }
        // if freeIndex == -1 - GC is done so that new updateBlock is already allocated in GC step.
    }

    // write data to new physical address
    int new_phys_num = current_state[cpu_id].block * PAGES_PER_BLOCK + current_state[cpu_id].page;
    logical_map[page_num].num = new_phys_num;
    physical_map[new_phys_num].num = page_num;
    physical_map[new_phys_num].is_valid = 1;

    current_state[cpu_id].page++;
    return 1;
}
