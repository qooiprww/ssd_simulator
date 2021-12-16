#include <iostream>
#include <fstream>
#include <string>
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

char parse_blktrace_line(ifstream *blktrace_file, int *stream_id, long long *start_sec, long long *num_sec) {
    string blktrace_line;
    char *ptr;
    long long lpn, len;
    int count;

    if ((*blktrace_file).is_open()) {
        while (getline(*blktrace_file, blktrace_line)) {
            cout << blktrace_line << endl;
        }
    }
//    getline(*blktrace_file, blktrace_line);
    cout << blktrace_line << endl;

//    if (feof(fp)){
//        printf("END!\n");
//        return -1;
//    }
//
//    // TODO: write length 인지, discard option 인지하는 기능
//    if((ptr = strchr(str, 'W')))
//    {
//        new = strtok(ptr, " ");
//        count = 0;
//        while(new != NULL ) {
//            if(count == 1) {
//                sscanf(new, "%lld", &lpn);
//            } else if (count == 3) {
//                sscanf(new, "%lld", &len);
//            }
//            new = strtok(NULL, " ");
//            count ++;
//        }
//
//
//        if((lpn+len)*SECTOR_SIZE/PAGE_SIZE < LOGICAL_PAGE) {
//            *start_LPN = lpn*SECTOR_SIZE/PAGE_SIZE;
//            *length = len*SECTOR_SIZE/PAGE_SIZE;
//        }
//        else {
//            printf("[ERROR] (%s, %d) lpn range\n", __func__, __LINE__);
//            return -1;
//        }
//
//        return 1;
//    } else {
//        printf("%s\n", str);
//        return 0;
//    }
//    return 0;
    return 'c';
}

int main() {
    int op_count, stream_id, i ;
    long long start_LPN, length;
    char op_code;

//    logFile[0] = 0;
//    statFile[0] = 0;
//    logFile[0] = 0;
//    if( initConf(argc, argv)< 0)
//        return 0;
    ifstream blktrace_file;
    blktrace_file.open("input.out");

//    config_init();
//    FTL_init();
//    stat_init();

//    if ( (blktrace_file = fopen(logFile, "r")) == 0 ) {
//        printf("Open File Fail \n");
//        exit(1);
//    }
    string ans;
    getline(blktrace_file, ans);
    cout << ans << endl;
    op_code = parse_blktrace_line(&blktrace_file, &stream_id, &start_LPN, &length);
    cout << op_code << endl;
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
    blktrace_file.close();
    return 0;
}
