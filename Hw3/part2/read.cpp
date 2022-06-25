#include <iostream>
#include <cstring>
#include <fstream>
#include <ctime>
#include <unistd.h>

#include "file_system.h"

using namespace std;

// For testing Reading 

int main(){

    std::ifstream in;
    
    in.open("mySystem.data", std::ios::binary);
    if(!in.is_open()){
        cout << "Error opening file" << endl;
        return -1;
    }

    SuperBlock super_block;
    //Super Block
    in.read((char*)&super_block, sizeof(SuperBlock));

    // for(int i=0 ; i<super_block.directory_count ; i++){
    //     cout << "##################" << i << " " << endl;
    //     for(int j=0 ; j<10 ; j++){
    //         cout << directory[i].file_names[j] << "_";
    //         cout << "index: " << directory[i].inode_indexes[j] << "-";
    //         cout << "isdir: " << directory[i].is_dir[j] << " ======= ";
    //     }
    // }

    char* buffer = new char[sizeof(long long) * super_block.inode_count];
    long long* vals = new long long[super_block.inode_count];
    long long* vals2 = new long long[super_block.inode_count];
    
    cout << "Reading inode bitmap " << in.tellg() << endl;
    //Inode bitmap
    in.read(buffer, sizeof(long long) * super_block.inode_count);
    memcpy(vals, buffer, sizeof(long long) * super_block.inode_count);

    in.read(buffer, sizeof(long long) * super_block.inode_count);
    memcpy(vals2, buffer, sizeof(long long) * super_block.inode_count);
    
    cout << endl;

    // for(int i = 0; i < super_block.inode_count; i++){
    //     std::cout << "[" << i << "]: (" 
    //     << vals[i] << ", " << vals2[i] << ") - ";     

    //     // if(inode_bitmap.block_in_use[i] == 1){
    //     //     std::cout << "In use" << std::endl;
    //     // }else{
    //     //     std::cout << "Free" << std::endl;
    //     // }
    // }

    cout << endl << endl;

    delete[] buffer;
    delete[] vals;
    delete[] vals2;

    buffer = new char[sizeof(long long) * super_block.block_count];
    vals = new long long[super_block.block_count];
    vals2  = new long long[super_block.block_count];

    //Block bitmap
    in.read(buffer,  sizeof(long long)  * super_block.block_count);
    memcpy(vals, buffer, sizeof(long long) * super_block.block_count);

    in.read(buffer,  sizeof(long long)  * super_block.block_count);
    memcpy(vals2, buffer, sizeof(long long) * super_block.block_count);

    // cout << endl;

    // for(unsigned long long i = 0; i < super_block.block_count; i++)
    //     cout << "(" << i << ">" << vals[i] << "), ";

    // cout << endl << endl;

    // for(unsigned long long i = 0; i < super_block.block_count; i++)
    //     cout << "(" << i << ">" << vals2[i] << "), ";

    delete[] buffer;
    delete[] vals;
    delete[] vals2;

    //Root Block
    FileAttribute root_block;
    cout << endl;

    in.read((char*)&root_block, sizeof(FileAttribute));

    cout << "Reading root block" << in.tellg() << endl;
    cout << "Root block size: " << sizeof(FileAttribute) << endl;
    cout << "Inode_index: " << root_block.i_node_index << endl;
    cout << "Name: " << root_block.name << endl;
    cout << "Is_dir: " << root_block.type << endl;
    cout << "Size: " << root_block.size << endl;
    cout << endl;

    INode* inodes = new INode[super_block.inode_count];

    cout << "Reading inodes " << in.tellg() << endl;
    //Inodes
    for(int i = 0; i < super_block.inode_count; i++){
        in.read((char*)&inodes[i], sizeof(INode));
    }

    cout << endl;

    cout << "INode " << 0 << ": " << endl;
    cout << "Name: " << inodes[0].file_attribute.name << endl;
    cout << "Size: " << inodes[0].file_attribute.size << endl;
    cout << "Type: " << inodes[0].file_attribute.type << endl;
    cout << "Blocks: ";
    for(int j = 0; j < 10; j++){
        cout << "(" << j << ")" << inodes[0].direct_data_block[j] << ", ";
    }
    cout << endl;
    for(int j=0; j<3; ++j){
        cout << "(" << j << ") " << inodes[0].indirect_data_block[j] << ", ";
    }

    // for(unsigned long long i = 0; i < super_block.inode_count; i++){
    //     cout << "INode " << i << ": " << endl;
    //     cout << "Name: " << inodes[i].file_attribute.name << endl;
    //     cout << "Size: " << inodes[i].file_attribute.size << endl;
    //     cout << "Blocks: ";
    //     for(int j = 0; j < 10; j++){
    //         cout << "(" << j << ")" << inodes[i].direct_data_block[j] << ", ";
    //     }
    //     cout << endl;
    //     for(int j=0; j<3; ++j){
    //         cout << "(" << j << ") " << inodes[i].indirect_data_block[j] << ", ";
    //     }
    // }

    cout << endl << endl;

    buffer = new char[super_block.block_size];

            cout << "########### Reading block " << in.tellg() << endl;

    for(int i = 0; i < super_block.block_count; i++){

        in.read(buffer, super_block.block_size);

        if(i == 0){

            // cout << "Block size: " << buffer << endl;

            FileAttribute tmp[2];

            memcpy(&tmp, buffer, sizeof(FileAttribute) * 2);
    
            cout << "Root block: " << endl;
            cout << "Name: " << tmp[0].name << endl;
            cout << "Size: " << tmp[0].size << endl;
            cout << "Type: " << tmp[0].type << endl;
            cout << "I Node Index: " << tmp[0].i_node_index << endl;
            cout << "Last Mode: " << tmp[0].last_modified << endl;

            cout << endl;

            // memcpy(&tmp, buffer + sizeof(FileAttribute), sizeof(FileAttribute));
            cout << "Root block size: " << sizeof(FileAttribute) << endl;
            cout << "Inode_index: " << tmp[1].i_node_index << endl;
            cout << "Name: " << tmp[1].name << endl;
            cout << "Is_dir: " << tmp[1].type << endl;
            cout << "Size: " << tmp[1].size << endl;
            cout << "Last Mode: " << tmp[1].last_modified << endl;

        }
    
    }
    
    delete[] buffer;

    in.close();

    std::cout << "Printing SuperBlock" << std::endl;

    std::cout << "SuperBlock File Size:    " << super_block.file_system_size << std::endl;

    std::cout << "SuperBlock Inode Bitmap Start:   " << super_block.inode_bitmap_start << std::endl;
    std::cout << "SuperBlock Block Bitmap Start:   " << super_block.block_bitmap_start << std::endl;

    std::cout << "Root Block Start " << super_block.root_start << std::endl;

    std::cout << "SuperBlock Inode Count:  " << super_block.inode_count << std::endl;
    std::cout << "SuperBlock Inode Size:   " << super_block.inode_size << std::endl;
    std::cout << "SuperBlock Inode Start:   " << super_block.inode_start << std::endl;

    std::cout << "SuperBlock Block Count:    " << super_block.block_count << std::endl;
    std::cout << "SuperBlock Block Size:   " << super_block.block_size << std::endl;
    std::cout << "SuperBlock Block Start:   " << super_block.block_start << std::endl;

    std::cout << std::endl;


    return 0;
}