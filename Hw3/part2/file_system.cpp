#include <iostream>
#include <fstream>
#include <cstring>

#include "file_system.h"

using namespace std;

#define INODE_COUNT 256

#define INODE_DIRECT_BLOCK_COUNT 15
#define INODE_INDIRECT_BLOCK_COUNT 3

FileSystemCreator::FileSystemCreator(
    char* _disk_name, long long _file_system_size, long long _block_size, 
    long long _i_node_count, long long _directory_count)
    : file_system_name(_disk_name)
{

    super_block.file_system_size = _file_system_size;
    super_block.block_size = _block_size;
    super_block.inode_count = _i_node_count;
    super_block.inode_size = sizeof(INode);

    set_size_settings();

    std::cout << "Created FileSystem" << std::endl;
}


FileSystemCreator::~FileSystemCreator(){
    std::cout << "Destroyed FileSystemCore" << std::endl;
}


void FileSystemCreator::set_size_settings(){

    long long total_size = 0;

    total_size += sizeof(SuperBlock);

    super_block.inode_bitmap_start = total_size;
    total_size += sizeof(long long) * super_block.inode_count;
    total_size += sizeof(long long) * super_block.inode_count;

    calculate_block_count(total_size);

    super_block.block_bitmap_start = total_size;
    total_size += sizeof(long long) * super_block.block_count;
    total_size += sizeof(long long) * super_block.block_count;

    super_block.root_start = total_size;
    total_size += sizeof(FileAttribute);

    super_block.inode_start = total_size;
    total_size += sizeof(INode) * super_block.inode_count;

    super_block.block_start = total_size;
    total_size += super_block.block_size * super_block.block_count;

    std::cout << "Setting size settings" << std::endl;
}


void FileSystemCreator::calculate_block_count(long long used){

    std::cout << "Calculating block count" << std::endl;

    used += sizeof(INode) * super_block.inode_count;
    used += sizeof(FileAttribute);

    long long rest = MAX_FILE_SYSTEM_SIZE - used;
    long long block_needed_size = super_block.block_size + (2 * sizeof(long long));

    super_block.block_count = rest / block_needed_size;
    
}


void FileSystemCreator::init_root_inode(){

    inodes[0].file_attribute.type = root_block.type;
    inodes[0].file_attribute.name[0] = root_block.name[0];
    inodes[0].file_attribute.name[1] = root_block.name[1];
    inodes[0].file_attribute.size = root_block.size;
    inodes[0].file_attribute.last_modified = root_block.last_modified;

    for(int i=1; i<INODE_DIRECT_BLOCK_COUNT; i++){
        inodes[0].direct_data_block[i] = -1;
    }
    for(int i=0; i<INODE_INDIRECT_BLOCK_COUNT; i++){
        inodes[0].indirect_data_block[i] = -1;
    }

}


void FileSystemCreator::init_inode(){

    inodes = new INode[super_block.inode_count];

    init_root_inode();

    for(int i = 1; i < super_block.inode_count; i++){

        strcpy(inodes[i].file_attribute.name, "\0");
        inodes[i].file_attribute.size = 0;
        inodes[i].file_attribute.type = 0;
        memset((void *)&inodes->file_attribute.last_modified, 0, sizeof(time_t));

        for(int j=0; j<INODE_DIRECT_BLOCK_COUNT; j++){
            inodes[i].direct_data_block[j] = -1;
        }
        for(int j=0; j<INODE_INDIRECT_BLOCK_COUNT; j++){
            inodes[i].indirect_data_block[j] = -1;
        }
    }   

}

void FileSystemCreator::init_root_block(){

    root_block.i_node_index = 0;
    strcpy(root_block.name, ".");
    root_block.size = 2 * sizeof(FileAttribute);
    root_block.type = -1;
    root_block.last_modified = time(NULL);

}

    
void FileSystemCreator::export_file_system(){

    std::cout << "Exporting FileSystem" << std::endl;

    std::ofstream fd;
    fd.open(file_system_name, std::ios::out | std::ios::binary);

    //Super block space
    fd.write((char*)&super_block, sizeof(SuperBlock));

    char *buffer = new char[super_block.inode_count * sizeof(long long)];
    long long *vals_use = new long long[super_block.inode_count];
    long long *vals_free = new long long[super_block.inode_count];

    for(long long i = 0; i < super_block.inode_count; i++)
        vals_use[i] = 0;

    for(long long i = 0; i < super_block.inode_count; i++)
        vals_free[i] = 1;

    vals_use[0] = 1;
    vals_free[0] = 0;

    //Inode bitmap space
    memcpy(buffer, vals_free, super_block.inode_count * sizeof(long long));
    fd.write(buffer, super_block.inode_count * sizeof(long long));

    memcpy(buffer, vals_use, super_block.inode_count * sizeof(long long));
    fd.write(buffer, super_block.inode_count * sizeof(long long));

    delete[] buffer;
    delete[] vals_free;
    delete[] vals_use;

    buffer = new char[super_block.block_count * sizeof(long long)];
    vals_free = new long long[super_block.block_count];
    vals_use = new long long[super_block.block_count];

    for(long long i = 0; i < super_block.block_count; i++)
        vals_use[i] = 0;
    
    for(long long i = 0; i < super_block.block_count; i++)
        vals_free[i] = 1;

    vals_use[0] = 1;
    vals_free[0] = 0;

    //Block bitmap space
    memcpy(buffer, vals_free, super_block.block_count * sizeof(long long));
    fd.write(buffer, super_block.block_count * sizeof(long long));

    memcpy(buffer, vals_use, super_block.block_count * sizeof(long long));
    fd.write(buffer, super_block.block_count * sizeof(long long));

    delete[] buffer;
    delete[] vals_free;
    delete[] vals_use;

    //Root space
    init_root_block();
    fd.write((char*)&root_block, sizeof(FileAttribute));

    //Inode space
    init_inode();
    for(long long i = 0; i < super_block.inode_count; i++){
        fd.write((char*)&inodes[i], sizeof(INode));
    }

    delete[] inodes;

    buffer = new char[super_block.block_size];
    memset(buffer, 0, super_block.block_size);

    FileAttribute root_1[2];

    root_1[0].type = root_block.type;
    root_1[0].size = root_block.size;
    root_1[0].last_modified = root_block.last_modified;
    strcpy(root_1[0].name, ".");
    root_1[0].i_node_index = root_block.i_node_index;

    root_1[1].type = root_block.type;
    root_1[1].size = root_block.size;
    root_1[1].last_modified = root_block.last_modified;
    strcpy(root_1[1].name, "..");
    root_1[1].i_node_index = root_block.i_node_index;

    memcpy(buffer, &root_1, sizeof(FileAttribute) * 2);
    
    fd.write(buffer, super_block.block_size);

    memset(buffer, 0, super_block.block_size);
    // Block space
    for(long long i=1; i<super_block.block_count; ++i){
        fd.write(buffer, super_block.block_size);
    }

    delete[] buffer;

    fd.close();

}



void FileSystemCreator::print_super_block(){

    std::cout << "Printing SuperBlock" << std::endl;

    std::cout << "SuperBlock File Size:    " << super_block.file_system_size << std::endl;
 
    std::cout << "SuperBlock Inode Bitmap Start:   " << super_block.inode_bitmap_start << std::endl;
    std::cout << "SuperBlock Block Bitmap Start:   " << super_block.block_bitmap_start << std::endl;

    std::cout << "Root Block Start:  " << super_block.root_start << std::endl;

    std::cout << "SuperBlock Inode Count:  " << super_block.inode_count << std::endl;
    std::cout << "SuperBlock Inode Size:   " << super_block.inode_size << std::endl;
    std::cout << "SuperBlock Inode Start:   " << super_block.inode_start << std::endl;

    std::cout << "SuperBlock Block Count:    " << super_block.block_count << std::endl;
    std::cout << "SuperBlock Block Size:   " << super_block.block_size << std::endl;
    std::cout << "SuperBlock Block Start:   " << super_block.block_start << std::endl;

    std::cout << std::endl;
}

