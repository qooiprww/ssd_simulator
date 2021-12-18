#include <iostream>

using namespace std;


typedef struct _FREE_BLOCKS{
    int cnt;
    int head;
    int tail;
}FREE_BLOCKS;

FREE_BLOCKS *free_blocks;

free_blocks = new FREE_BLOCKS[5];

int fun () {
    
    
    free_blocks[6].cnt = 4;
    cout << "SF" << endl;
    
    
//    int x = 0;
    
//    try {
//        if (cpu_id <= 3)
//            throw (cpu_id);
//    }
//    catch (int cpu_id) {
//        cout << "[ERROR] cpu_id(" << cpu_id << ") is out of bound!\n";
//        exit(0);
//    }
    
//    try {
//        if (x == 0)
//            throw string("[ERROR] Couldn't find any free block!\n");
//    } catch(string str) {
//        cout << str << endl;
//        printf("[ERROR] (%s, %d) no free block'\n", __func__, __LINE__);
//        exit(0);
//    }
    
    
    cout << "hi" << endl;
    
    return 1;
}


int main () {
   
    cout << fun() << endl;
}
