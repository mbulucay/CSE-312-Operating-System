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
        long long file_system_size;         // File system size in bytes

        long long block_bitmap_start;       // Block bitmap start in bytes
        long long inode_bitmap_start;       // Inode bitmap start in bytes

        long long root_start;               // Root start in bytes

        long long block_count;              // Block count
        long long block_size;               // Block size in bytes
        long long block_start;              // Block start in bytes

        long long inode_count;              // Inode count
        long long inode_size;               // Inode size in bytes
        long long inode_start;              // Inode start in bytes

}SuperBlock;


typedef struct BitMap{

    public:
        long long* block_in_use;            // Block in use bitmap
        long long* free_blocks;             // Free blocks bitmap
        
}BitMap;


typedef struct FileAttribute{

    public:
        char name[MAX_FILE_NAME_SIZE];                                  // File name
        long long size;                                                 // File size in bytes
        long long type;                                                 // File type
        long long i_node_index;                                         // Inode index

        time_t last_modified;                                           // Last modified time

}FileAttribute;


typedef struct INode{
    
    public:
        FileAttribute file_attribute;                                   // File attribute in INode

        long long direct_data_block[INODE_DIRECT_BLOCK_COUNT];          // Direct acces data block
        long long indirect_data_block[INODE_INDIRECT_BLOCK_COUNT];      // Indirect acces data block

}INode;


typedef struct Block{

    public:
        char *data;                // Block data

}Block;


#endif // !BLOCKS_H_
