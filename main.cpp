#include <iostream>

int config_init(int argc, char* argv[]) {
    int i ;
    int param_opt, op;

    LOGICAL_FLASH_SIZE = 0;
    streamNum = 1;
    op = 7;

    // option get
    while ((param_opt = getopt(argc, argv, "s:f:o:r:m:l:")) != -1){
        switch(param_opt)
        {
            case 's':
                sscanf(optarg,"%lld", &LOGICAL_FLASH_SIZE);
                LOGICAL_FLASH_SIZE *= GB;
                break;
            case 'f':
                strncpy(logFile, optarg, strlen(optarg));
                break;
            case 'o':
                sscanf(optarg, "%d", &op);
                break;
            case 'r':
                strncpy(statFile, optarg, strlen(optarg));
                break;
            case 'm':
                sscanf(optarg, "%d", &streamNum);
                break;
        }
    }
    if (logFile[0] == 0){
        printf("Please input log file name: -f [logFilename]\n");
        return -1;
    }

    if(LOGICAL_FLASH_SIZE == 0 ){
        printf("Please input flash size (opt: -s [x GB])\n");
        return -1;
    }

    LOGICAL_PAGE = LOGICAL_FLASH_SIZE / PAGE_SIZE;


    OP_REGION = LOGICAL_FLASH_SIZE * op / 100;
    FLASH_SIZE = LOGICAL_FLASH_SIZE + OP_REGION;
    BLOCKS_PER_FLASH = FLASH_SIZE / BLOCK_SIZE;
    PAGES_PER_FLASH = FLASH_SIZE / PAGE_SIZE;

    return 0;
}

int main() {
    FILE * blktrace_file;
    int op_code, op_count, stream_id, i ;
    long long start_LPN, length;

    logFile[0] = 0;
    statFile[0] = 0;
    logFile[0] = 0;
    if( initConf(argc, argv)< 0)
        return 0;

    config_init();
    FTL_init();
    stat_init();

    if ( (inputFp = fopen(logFile, "r")) == 0 ) {
        printf("Open File Fail \n");
        exit(1);
    }

    printConf();


    op_count = 0;
    while (EOF) {

        op_code = parse_blktrace_line(blktrace_file, &stream_id, &start_LPN, &length);

        switch(op_code) {
            case 'W':
                FTL_write(start_LPN+i, 0);
                break;
            default:
                break;
        }

        op_count ++;
    }

    fclose(inputFp);

    print_stat_summary();

    FTL_close();
    return 0;
}
