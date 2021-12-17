#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include "ftl.h"
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

string parse_blktrace_line(ifstream *input_stream, int *cpu_id, long long *start_sec, long long *num_sec) {
    
    string line, RWBS;
    regex cpu_id_regex("^(?:\\s+)?(?:\\S+\\s+){1}(\\d+)");
    regex RWBS_regex("^(?:\\s+)?(?:\\S+\\s+){6}(\\w+)");
    regex start_sec_regex("^(?:\\s+)?(?:\\S+\\s+){7}(\\d+)");
    regex num_sec_regex("^(?:\\s+)?(?:\\S+\\s+){9}(\\d+)");
    cmatch regex_result;

    if (getline(*input_stream, line)){
        regex_search(line.c_str(), regex_result, cpu_id_regex);
        *cpu_id = stoi(regex_result[1]);
        regex_search(line.c_str(), regex_result, RWBS_regex);
        RWBS = regex_result[1];
        regex_search(line.c_str(), regex_result, start_sec_regex);
        *start_sec = stoll(regex_result[1]);
        regex_search(line.c_str(), regex_result, num_sec_regex);
        *num_sec = stoll(regex_result[1]);
    }
    else
        printf("End of the file!\n");
    cout << *cpu_id << " " << RWBS << " "<< *start_sec << " "<< *num_sec << endl;


    return RWBS;
}

int main() {
    ftl_init();
//    ftl_write(100, 0);
//    ftl_close();
    
    
    
//    int cpu_id;
//    long long start_sec, num_sec;
//    string op_code;

//    logFile[0] = 0;
//    statFile[0] = 0;
//    logFile[0] = 0;
//    if( initConf(argc, argv)< 0)
//        return 0;
    
//    config_init();
//    FTL_init();
//    stat_init();
    
//    ifstream input_stream("C:\\Users\\peter\\Desktop\\csc2233\\final_project\\ssd_simulator\\input.out"); // blktrace output file
//// TODO: hanlde exception for file input stream
//
//    op_code = parse_blktrace_line(&input_stream, &cpu_id, &start_sec, &num_sec);
//    cout << op_code << endl;
//
//    input_stream.close();
    
    
//
//    printConf();
//
//
//    op_count = 0;
//    while (EOF) {
//
//        op_code = parse_blktrace_line(blktrace_file, &stream_id, &start_LPN, &length);
//
//        switch(op_code) {
//            case 'W':
//                FTL_write(start_LPN+i, 0);
//                break;
//            default:
//                break;
//        }
//
//        op_count ++;
//    }
//
//    fclose(blktrace_file);
//
//    print_stat_summary();
//
//    FTL_close();
    
    return 0;
}
