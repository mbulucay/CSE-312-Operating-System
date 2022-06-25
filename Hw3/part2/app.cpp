#include <iostream>
#include <fstream>

#include "file_system.h"

using namespace std;

const long long KB = 1024;
const long long MB = 1024 * KB;
const long long GB = 1024 * MB;


int main(int argc, char* argv[]){

    if(argc != 3){
        cout << "Usage: ./makeFileSystem <disk_name> <block_size>" << endl;
        return -1;
    }

    long long block_size = atoi(argv[1]) * KB;
    if(block_size <= 0){
        cout << "Block size must be greater than 0" << endl;
        return -1;
    }

    char* disk_name = argv[2];
    
    FileSystemCreator fs(disk_name, 16 * MB, block_size, 500, 50);
    fs.export_file_system();
    fs.print_super_block();

    return 0;
}
