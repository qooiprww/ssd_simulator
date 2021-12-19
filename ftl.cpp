#include "ftl.hpp"
#include <string>
#include <iostream>
#include <set>

using namespace std;

// long long LOGICAL_FLASH_SIZE;
// long long OP_REGION;
// long long LOGICAL_PAGE;
// long long FLASH_SIZE;

// long long BLOCKS_PER_FLASH;
// long long PAGES_PER_FLASH;

int cpu_num = 1;

STATISTICS total_stat;
STATISTICS *cpu_stat;
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

    cpu_stat = new STATISTICS [CPU_MAX];
    for (int i=0 ; i < cpu_num; i++) {
        cpu_stat[i].read_cnt = 0;
        cpu_stat[i].write_cnt = 0;
        cpu_stat[i].copyback_cnt = 0;
        cpu_stat[i].gc_cnt = 0;
    }
}

void ftl_init() {
    init_stat();
    
    free_blocks.cnt = BLOCKS_PER_FLASH;
    free_blocks.head = 0;
    free_blocks.tail = BLOCKS_PER_FLASH - 1;
    

    logical_map = new LOGICAL_MAP[LOGICAL_PAGE];
    physical_map = new PHYSICAL_MAP[PAGES_PER_FLASH];
    block_map = new BLOCK_MAP[BLOCKS_PER_FLASH];

    current_state = new CURRENT_STATE[cpu_num];

        
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
    delete[] cpu_stat;
    delete[] logical_map;
    delete[] physical_map;
    delete[] block_map;
    delete[] current_state;
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
    int cpu_id = block_map[block_num].cpu_id;

    int start_page = block_num * PAGES_PER_BLOCK;
    for (int cur_page = start_page; cur_page < start_page + PAGES_PER_BLOCK; ++cur_page)
        if (physical_map[cur_page].is_valid) {
            try {
                if (current_state[cpu_id].page >= PAGES_PER_BLOCK) { // Reached the end of the current block!
                    int fetched_block = fetch_free_block(cpu_id);
                    // TODO: move exceptions: throw string("[ERROR] ftl_gc:ftl_copybakc: reached end of the block, and there's no free block left!");
                    current_state[cpu_id].block = fetched_block;
                    current_state[cpu_id].page = 0;
                }
            } catch (string err_message) {
                cout << err_message << endl;
                exit(0);
            }

            int page = physical_map[cur_page].num;
            
            // Free old pysical page
            physical_map[cur_page].num = -1;
            physical_map[cur_page].is_valid = 0;

            // Copyback
            int new_phys_num = current_state[cpu_id].block * PAGES_PER_BLOCK + current_state[cpu_id].page;
            logical_map[page].num = new_phys_num;
            physical_map[new_phys_num].num = page;
            physical_map[new_phys_num].is_valid = 1;
        
            // Update stats
            total_stat.copyback_cnt++;
            cpu_stat[cpu_id].copyback_cnt++;
            current_state[cpu_id].page++;
        }
}

int choose_gc_block() {
    // Trying to choose a block for gc to free it

    int gc_block = -1;
    int max_invalid_cnt = -2; // TODO: change -2. If statement is strict.
    
    set<int> current_blocks;
    for (int i = 0; i < cpu_num; i++)
        current_blocks.insert(current_state[i].block);

    // Find a block with max invalid pages
    for (int i = 0; i < BLOCKS_PER_FLASH; i++) { // TODO: It's better to add dirty field
        if (current_blocks.count(i))
            continue;
        if (block_map[i].invalid_cnt > max_invalid_cnt) {
            max_invalid_cnt = block_map[i].invalid_cnt;
            gc_block = i;
        }
    }

    try {
        if (max_invalid_cnt > PAGES_PER_BLOCK)
            throw string("[ERROR] Invalid_cnt out of bound!\n");
    } catch (string err_message) {
        cout << err_message << endl;
    }

    int gc_cpu = block_map[gc_block].cpu_id;
    try {
        if(gc_cpu < 0 || cpu_num <= gc_cpu)
            throw string("[ERROR] Invalid cpu_id for gc_block!\n");
    } catch (string err_message) {
        cout << err_message << endl;
    }

    return gc_block;
}


int ftl_gc() {
    
    // Find the proper block for gc
    int gc_block = choose_gc_block();
    
    // Copyback the valid pages of gc_block and mark the old physical pages as stale
    ftl_copyback(gc_block);
    
    // Erase and free gc_block and add it to free_blocks linked list
    ftl_erase(gc_block);


    total_stat.gc_cnt++;
    int gc_cpu = block_map[gc_block].cpu_id;
    cpu_stat[gc_cpu].gc_cnt++; // TODO: what is cpu_stat[gc_cpu].gc_cnt?
    
    return gc_block;
}


int search_free_block(int cpu_id) {
    
//    if (cpu_num == 1) {
//        if (free_blocks.cnt <= 1) {
//            current_state[cpu_id].block = fetch_free_block(cpu_id);
//            current_state[cpu_id].page = 0;
//
//            ftl_gc();
//            return -1;
//        }
//    } else {
//        while (free_blocks.cnt <= 1) {
//            int ret = M_GC_stream(); // return code may represent error or gc victim stream number
//            if(ret < 0)    return -2;
//            if (ret == streamID)
//                return -1;
//        }

    if (free_blocks.cnt <= 1) {
        current_state[cpu_id].block = fetch_free_block(cpu_id);
        current_state[cpu_id].page = 0;

        ftl_gc();
        return -1;
    }
//    }
    
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
    cpu_stat[cpu_id].write_cnt++;
    
    
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


void ftl_discard(int page_num, int cpu_id) {
    int phys_num;

    total_stat.discard_cnt++;
    cpu_stat[cpu_id].discard_cnt++;
    phys_num = logical_map[page_num].num;
    physical_map[phys_num].num = -1;
    physical_map[phys_num].is_valid = 0;

    int block_num = phys_num / PAGES_PER_BLOCK;
    block_map[block_num].invalid_cnt++;
}

void ftl_read (int page_num, int cpu_id) {
    total_stat.read_cnt++;
    cpu_stat[cpu_id].read_cnt++;
}
