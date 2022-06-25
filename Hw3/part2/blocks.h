#ifndef BLOCKS_H_
#define BLOCKS_H_

#include <ctime>
using namespace std;

#define INODE_COUNT 256

#define MAX_FILE_NAME_SIZE 32

#define INODE_DIRECT_BLOCK_COUNT 15
#define INODE_INDIRECT_BLOCK_COUNT 3


typedef struct SuperBlock{

    public:
        long long file_system_size;

        long long block_bitmap_start;
        long long inode_bitmap_start;

        long long root_start;

        long long block_count;
        long long block_size;
        long long block_start;

        long long inode_count;
        long long inode_size;
        long long inode_start;

}SuperBlock;


typedef struct BitMap{

    public:
        long long* block_in_use;
        long long* free_blocks;
        
}BitMap;


typedef struct FileAttribute{

    public:
        char name[MAX_FILE_NAME_SIZE];
        long long size;
        long long type;
        long long i_node_index;

        time_t last_modified;

}FileAttribute;


typedef struct INode{
    
    public:
        FileAttribute file_attribute;

        long long direct_data_block[INODE_DIRECT_BLOCK_COUNT];
        long long indirect_data_block[INODE_INDIRECT_BLOCK_COUNT];

}INode;


typedef struct Block{

    public:
        char *data; 

}Block;


#endif // !BLOCKS_H_
