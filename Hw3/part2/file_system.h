
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "blocks.h"

class FileSystemCreator{

    public:

        FileSystemCreator(char* _disk_name, long long _file_system_size, 
                    long long _block_size, long long _i_node_count, long long _directory_count);
        ~FileSystemCreator();

        /* File System Operations */

        void set_size_settings();
        void calculate_block_count(long long used);

        void export_file_system();

        void print_super_block();

        void init_root_inode();
        void init_root_block();

        void init_inode();

    private:   

        const long long MAX_FILE_SYSTEM_SIZE = 16000000;
        // const long long MAX_FILE_SYSTEM_SIZE = 16 * 1024 * 1024;

        char *file_system_name;

        SuperBlock super_block;
    
        FileAttribute root_block;

        BitMap free_block_space;
        BitMap free_inode_space;

        INode *inodes;

        long long root_inode;
        
};
        

#endif // !FILESYSTEM_H_

