#ifndef FTL_HPP
#define FTL_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BLKTRACE_PARSING_THRESHOLD 10 // Number of attempts for parse_blktrace_line to find a D action line

extern long long SECTOR_SIZE;
extern long long SECTOR_PER_PAGE;
extern long long PAGES_PER_BLOCK;
extern long long PAGE_SIZE;
extern long long BLOCK_SIZE;
extern long long LOGICAL_FLASH_SIZE;
extern long long OP_REGION;
extern long long LOGICAL_PAGE;
extern long long FLASH_SIZE;
extern long long BLOCKS_PER_FLASH;
extern long long PAGES_PER_FLASH;
extern int OP_PERCENTAGE;

void ftl_init();
void ftl_close();

int ftl_gc();
//int ftl_GC_stream();
int ftl_write(int page_num, int cpu_id);
void ftl_discard(int page_num, int cpu_id);
void ftl_read (int page_num, int cpu_id);


//-------------------------------

const int CPU_MAX = 4;

extern int cpu_num;


typedef struct _STATISTICS {
    long long read_cnt;
    long long write_cnt;
    long long discard_cnt;
    long long gc_cnt;
    long long copyback_cnt;
}STATISTICS;

extern STATISTICS total_stat;
extern STATISTICS *cpu_stat;


typedef struct _LOGICAL_MAP{ // This is a logical page table that stores physical page number
    int num;
}LOGICAL_MAP;

typedef struct _PHYSICAL_MAP{ // This is a physical page table that stores logical page number and status of the physical page
    int num;
    int is_valid; // TODO: Should handle erased status too!
}PHYSICAL_MAP;

extern LOGICAL_MAP *logical_map;
extern PHYSICAL_MAP *physical_map;


typedef struct _BLOCK_MAP{
    int invalid_cnt;
    int next_block;
    int erase_cnt;
    int cpu_id;
}BLOCK_MAP;

extern BLOCK_MAP *block_map;


typedef struct _CURRENT_STATE{
    int block;
    int page;
}CURRENT_STATE;

extern CURRENT_STATE *current_state;


typedef struct _FREE_BLOCKS{
    int cnt;
    int head;
    int tail;
}FREE_BLOCKS;

extern FREE_BLOCKS free_blocks;

#endif
