#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include "ftl.hpp"
#include <unistd.h>

using namespace std;
//int config_init(int argc, char* argv[]) {
//    int i ;
//    int param_opt, op;
//
//    LOGICAL_FLASH_SIZE = 0;
//    streamNum = 1;
//    op = 7;
//
//    // option get
//    while ((param_opt = getopt(argc, argv, "s:f:o:r:m:l:")) != -1){
//        switch(param_opt)
//        {
//            case 's':
//                sscanf(optarg,"%lld", &LOGICAL_FLASH_SIZE);
//                LOGICAL_FLASH_SIZE *= GB;
//                break;
//            case 'f':
//                strncpy(logFile, optarg, strlen(optarg));
//                break;
//            case 'o':
//                sscanf(optarg, "%d", &op);
//                break;
//            case 'r':
//                strncpy(statFile, optarg, strlen(optarg));
//                break;
//            case 'm':
//                sscanf(optarg, "%d", &streamNum);
//                break;
//        }
//    }
//    if (logFile[0] == 0){
//        printf("Please input log file name: -f [logFilename]\n");
//        return -1;
//    }
//
//    if(LOGICAL_FLASH_SIZE == 0 ){
//        printf("Please input flash size (opt: -s [x GB])\n");
//        return -1;
//    }
//
//    LOGICAL_PAGE = LOGICAL_FLASH_SIZE / PAGE_SIZE;
//
//
//    OP_REGION = LOGICAL_FLASH_SIZE * op / 100;
//    FLASH_SIZE = LOGICAL_FLASH_SIZE + OP_REGION;
//    BLOCKS_PER_FLASH = FLASH_SIZE / BLOCK_SIZE;
//    PAGES_PER_FLASH = FLASH_SIZE / PAGE_SIZE;
//
//    return 0;
//}

char parse_blktrace_line(ifstream *input_stream, int *cpu_id, long long *start_page, long long *num_page) {
    string RWBS;
    char op_code;
    string line, action;
    regex cpu_id_regex("^(?:\\s+)?(?:\\S+\\s+){1}(\\d+)");
    regex action_regex("^(?:\\s+)?(?:\\S+\\s+){5}(\\w+)");
    regex RWBS_regex("^(?:\\s+)?(?:\\S+\\s+){6}(\\w+)");
    regex start_sec_regex("^(?:\\s+)?(?:\\S+\\s+){7}(\\d+)");
    regex num_sec_regex("^(?:\\s+)?(?:\\S+\\s+){9}(\\d+)");
    cmatch regex_result;
    for(int line_cnt = 0; line_cnt < BLKTRACE_PARSING_THRESHOLD; line_cnt++){
        if (getline(*input_stream, line)){
            regex_search(line.c_str(), regex_result, cpu_id_regex);
            *cpu_id = stoi(regex_result[1]);
            regex_search(line.c_str(), regex_result, RWBS_regex);
            RWBS = regex_result[1];
            op_code = RWBS[0];
            regex_search(line.c_str(), regex_result, start_sec_regex);
            *start_page = stoll(regex_result[1])*SECTOR_SIZE/PAGE_SIZE;
            try{
                if(*start_page > LOGICAL_PAGE) {
//                    cout << *start_page << "   "  << LOGICAL_PAGE << "     " << PAGES_PER_FLASH << endl;
                    throw string("[ERROR] parse_blktrace_line: page number exceeds total logical flash size!");
                }
            }
            catch (string err_message) {
                cout << err_message << endl;
                exit(0);
            }
            regex_search(line.c_str(), regex_result, num_sec_regex);
            *num_page = stoll(regex_result[1])*SECTOR_SIZE/PAGE_SIZE;
            regex_search(line.c_str(), regex_result, action_regex);
            action = regex_result[1];
            if(action == "D"){
                break;
            }
        }
        else{
            printf("End of the file!\n");
        }
    }
    return op_code;
}



void print_stat_summary() {
    printf("\nread: %lld\t\twrite: %lld\n", total_stat.read_cnt, total_stat.write_cnt);
    printf("gc: %lld\tcopyback: %lld\n\n", total_stat.gc_cnt, total_stat.copyback_cnt);
    if(total_stat.write_cnt!=0)
        printf("WAF: %lf\n", (double)(total_stat.write_cnt+total_stat.copyback_cnt)/total_stat.write_cnt);
}

int main() {

    int cpu_id = 0;
    long long start_page, num_page;
    char op_code;

    // TODO: hanlde exception for file input stream
    ifstream input_stream("input3.out"); // blktrace output file
    ftl_init();

//    logFile[0] = 0;
//    statFile[0] = 0;
//    logFile[0] = 0;
//    if( initConf(argc, argv)< 0)
//        return 0;
    
//    config_init();
//    FTL_init();
//    stat_init();
    
//
//    printConf();
//
//
    while (input_stream.peek() != EOF) {

        op_code = parse_blktrace_line(&input_stream, &cpu_id, &start_page, &num_page);
        switch(op_code) {
            case 'W':
                for(int page_offset = 0; page_offset < num_page;  page_offset++)
                    ftl_write(start_page + page_offset, 0);
                break;
            case 'D':
                for(int page_offset = 0; page_offset < num_page;  page_offset++)
                    ftl_discard(start_page + page_offset, 0);
                break;
            case 'R':
                for(int page_offset = 0; page_offset < num_page;  page_offset++)
                    ftl_read(start_page + page_offset, 0);
                break;
            default:
                break;
        }

    }

    input_stream.close();

    print_stat_summary();

    ftl_close();
    
    return 0;
}
