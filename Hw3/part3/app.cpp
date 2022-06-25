#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include "file_system_manager.h"

int main(int argc, char* argv[]){


    if(argc < 3){
        cout << "Usage: ./fileSystemOper <fileSystem.data> <operation> <parameters>" << endl;
        return -1;
    }    

    char* param1;
    char* param2; 

    char* disk_name = argv[1];
    char* operation = argv[2];
    
    if(argc > 3){
        param1 = argv[3];
    }
    if(argc > 4){
        param2 = argv[4];
    }

    FileSystemManager* fsm = new FileSystemManager(disk_name);

    fsm->open_in_disk();
    fsm->open_out_disk();
    fsm->read_super_block();
    fsm->read_inode_bitmap();
    fsm->read_block_bitmap();
    fsm->read_root_block();

    if(strcmp(operation, "dir") == 0 && argc == 4){
        fsm->list_directory(param1);
    }
    else if(strcmp(operation, "mkdir") == 0 && argc == 4){
        fsm->create_directory(param1);
    }
    else if(strcmp(operation, "rmdir") == 0 && argc == 4){
        fsm->remove_directory(param1);
    }
    else if(strcmp(operation, "dumpe2fs") == 0 && argc == 3){
        fsm->dumpe2fs();
    }
    else if(strcmp(operation, "write") == 0 && argc == 5){
        fsm->write(param1, param2);
    }
    else if(strcmp(operation, "read") == 0 && argc == 5){
        fsm->read(param1, param2);
    }
    else if(strcmp(operation, "del") == 0 && argc == 4){
        fsm->delete_file(param1);
    }
    else{
        cout << "Invalid operation" << endl;
    }

    fsm->close_in_disk();
    fsm->close_out_disk();

    delete fsm;

    return 0;
}
