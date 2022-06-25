#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>

#define INODE_COUNT 256

#define DIR_FILE_SIZE 10

#define INODE_DIRECT_BLOCK_COUNT 15
#define INODE_INDIRECT_BLOCK_COUNT 3

#define INODE_DIR -1
#define INODE_FILE 1

#include "file_system_manager.h"

using namespace std;


string readFileIntoString(const string& path) {

    ifstream input_file(path);

    if (!input_file.is_open()) {
        cerr << "Could not open the file - '"
             << path << "'" << endl;
        exit(EXIT_FAILURE);
    }

    return string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
}

void FileSystemManager::test(){

    std::cout << "FileSystemManager test" << std::endl;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File System Operations    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileSystemManager::FileSystemManager(std::string _disk_name)
    : disk_name(_disk_name)
{   
    std::cout << "Created FileSystemManager" << std::endl;
}


FileSystemManager::~FileSystemManager(){

    std::cout << "Destroyed FileSystemManager" << std::endl;
}


void FileSystemManager::list_directory(char* path){
    
    std::cout << "Listing directory" << std::endl;

    std::vector<std::string> directory_list;
    parse_path(path, directory_list);
    
    int next_inode = 0;

    for(std::string str: directory_list){
        next_inode = get_inode_number(next_inode, str);
        if(next_inode == -1){
            std::cout << "Directory not found" << std::endl;
            return;
        }
    }

    list_directory_in_block_index(next_inode);

}


void FileSystemManager::create_directory(char* path){
    
    std::cout << "Creating directory" << std::endl;

    std::vector<std::string> path_vector;
    parse_path(path, path_vector);

    int next_inode = 0, prev_inode = 0, parent = 0;
    
    long unsigned int i = 0;
    for(i = 0; i < path_vector.size(); i++){

        prev_inode = next_inode;
        next_inode = get_inode_number(next_inode, path_vector[i]);
        if(next_inode == -1 && i != path_vector.size() - 1){
            std::cout << "Directory not found" << std::endl;
            return;
        }
        parent = prev_inode;
    }

    if(next_inode != -1){
        std::cout << "Directory already exists" << std::endl;
        return;
    }

    int free_inode = get_next_free_inode_index();
    int use_inode = get_next_use_inode_index();

    if(free_inode == -1 || use_inode == -1){
        std::cout << "No free inodes" << std::endl;
        return;
    }

    int free_block = get_next_free_block_index();
    int use_block = get_next_use_block_index();

    if(free_block == -1 || use_block == -1){
        std::cout << "No free blocks" << std::endl;
        return;
    }

    FileAttribute attr[2];

    attr[0].type = INODE_DIR;
    attr[0].size = 2 * sizeof(FileAttribute);
    strcpy(attr[0].name, ".");
    attr[0].i_node_index = use_inode;
    attr[0].last_modified = time(NULL);

    attr[1].type = INODE_DIR;
    attr[1].size = 2 * sizeof(FileAttribute);
    strcpy(attr[1].name, "..");
    attr[1].i_node_index = parent;
    attr[1].last_modified = attr[0].last_modified;

    // Create new inode
    INode new_inode;
    new_inode.file_attribute.size = 2 * sizeof(FileAttribute);
    strcpy(new_inode.file_attribute.name, path_vector[path_vector.size() - 1].c_str());
    new_inode.file_attribute.type = INODE_DIR;
    new_inode.file_attribute.last_modified = attr[0].last_modified;
    new_inode.file_attribute.i_node_index = use_inode;

    new_inode.direct_data_block[0] = use_block;
    for(int i = 1; i < INODE_DIRECT_BLOCK_COUNT; i++){
        new_inode.direct_data_block[i] = -1;
    }
    for(int i = 0; i < INODE_INDIRECT_BLOCK_COUNT; i++){
        new_inode.indirect_data_block[i] = -1;
    }

    inode_bitmap.block_in_use[use_inode] = block_bitmap.block_in_use[use_block] = 1;
    inode_bitmap.free_blocks[free_inode] = block_bitmap.free_blocks[free_block] = 0;

    char *buffer = new char[super_block.block_size];
    memcpy(buffer, &attr, sizeof(FileAttribute) * 2);
    memcpy(block_ptr->data, buffer, super_block.block_size);

    write_block_table_entry(use_block);

    int write_block_index = get_next_file_index_block(parent);
    strcpy(attr[0].name, path_vector[path_vector.size() - 1].c_str());
    add_file_to_block(parent, write_block_index, attr[0]);

    write_block_bitmap();
    write_inode_bitmap();

    i_node_ptr = &new_inode;
    write_inode_table_entry(use_inode);

    read_inode_table_entry(parent);
    i_node_ptr->file_attribute.size += sizeof(FileAttribute);
    write_inode_table_entry(parent);

    std::cout << "Created directory" << std::endl;

}

void FileSystemManager::remove_directory(char* path){
    
    std::cout << "Removing directory" << std::endl;

}


void FileSystemManager::dumpe2fs(){
    
    std::cout << "Dumping file system" << std::endl;

    print_super_block();

    cout << endl;
    cout << "Used Blocks indexes: " << endl;
    for(int i = 0; i < super_block.block_count; i++){
        if(block_bitmap.block_in_use[i] == 1){
            read_block_table_entry(i);
            std::cout << i << ", ";

        }
    }

    cout << endl;
    cout << "Free Blocks indexes: " << endl;
    for(int i = 0; i < super_block.block_count; i++){
        if(block_bitmap.block_in_use[i] == 0){
            std::cout << i << ", ";
        }
    }

    cout << endl;
    cout << "Used Inodes indexes: " << endl;
    for(int i = 0; i < super_block.inode_count; i++){
        if(inode_bitmap.block_in_use[i] == 1){
            read_inode_table_entry(i);
            std::cout << i << ", ";
        }
    }

    cout << endl;
    cout << "Free Inodes indexes: " << endl;
    for(int i = 0; i < super_block.inode_count; i++){
        if(inode_bitmap.block_in_use[i] == 0){
            std::cout << i << ", ";
        }
    }

    cout << endl;
    cout << "Files: " << endl;
    for(int i = 0; i < super_block.inode_count; i++){
        if(inode_bitmap.block_in_use[i] == 1){
            read_inode_table_entry(i);
            print_inode_table_entry();
        }
    }


}


void FileSystemManager::write(char* path, char* file_path){
    
    std::cout << "Writing file" << std::endl;

    // Creating File
    std::vector<std::string> path_vector;
    parse_path(path, path_vector);

    int next_inode = 0, prev_inode = 0, parent = 0;
    long unsigned int i = 0;
    for(i = 0; i < path_vector.size(); i++){

        prev_inode = next_inode;
        next_inode = get_inode_number(next_inode, path_vector[i]);
        if(next_inode == -1 && i != path_vector.size() - 1){
            std::cout << "Directory not found 1" << std::endl;
            return;
        }
        parent = prev_inode;
    }

    // cout << "Next inode " << next_inode << endl;
    // cout << "Previous inode " << prev_inode << endl;
    // cout << "Parent inode " << parent << endl;

    int free_inode = get_next_free_inode_index();
    int use_inode = get_next_use_inode_index();

    if(free_inode == -1 || use_inode == -1){
        std::cout << "No free inodes" << std::endl;
        return;
    }

    int free_block = get_next_free_block_index();
    int use_block = get_next_use_block_index();

    if(free_block == -1 || use_block == -1){
        std::cout << "No free blocks" << std::endl;
        return;
    }

    // cout << "Free inode " << free_inode << endl;
    // cout << "Use inode " << use_inode << endl;
    // cout << "Free block " << free_block << endl;
    // cout << "Use block " << use_block << endl;

    std::string content = readFileIntoString(file_path);
    std::vector<std::string> blocks_string;

    long unsigned int block_count = content.size() / super_block.block_size;
    if(content.size() % super_block.block_size != 0){
        block_count++;
    }

    for(long unsigned int i = 0; i < block_count; i++){
        blocks_string.push_back(content.substr(i * super_block.block_size, super_block.block_size));
    }

    FileAttribute attr;
    attr.type = INODE_FILE;
    attr.size = content.size();
    strcpy(attr.name, path_vector[path_vector.size() - 1].c_str());
    attr.i_node_index = use_inode;
    attr.last_modified = time(NULL);

    // Create new inode
    INode new_inode;
    new_inode.file_attribute.size = content.size();
    strcpy(new_inode.file_attribute.name, attr.name);
    new_inode.file_attribute.type = INODE_FILE;
    new_inode.file_attribute.last_modified = attr.last_modified;
    new_inode.file_attribute.i_node_index = use_inode;

    long unsigned int block_address_index = 0;
    for(block_address_index = 0; block_address_index < block_count; ++block_address_index){
        new_inode.direct_data_block[block_address_index] = use_block;
        block_bitmap.block_in_use[use_block] = 1;
        block_bitmap.free_blocks[free_block] = 0;
        
        use_block = get_next_use_block_index();
        free_block = get_next_free_block_index();
        if(use_block == -1 || free_block == -1){
            std::cout << "No free blocks" << std::endl;
            return;
        }
    }

    for(i = block_count; i < INODE_DIRECT_BLOCK_COUNT; i++){
        new_inode.direct_data_block[i] = -1;
    }
    for(i = 0; i < INODE_INDIRECT_BLOCK_COUNT; i++){
        new_inode.indirect_data_block[i] = -1;
    }

    inode_bitmap.block_in_use[use_inode] =  1;
    inode_bitmap.free_blocks[free_inode] = 0;
    
    write_inode_bitmap();
    write_block_bitmap();

    i_node_ptr = &new_inode;
    write_inode_table_entry(attr.i_node_index);

    // Write data to blocks
    Block block;
    block.data = new char[super_block.block_size];

    for(i = 0; i < block_count; i++){
        strcpy(block.data, blocks_string[i].c_str());
        block_ptr = &block;
        write_block_table_entry(new_inode.direct_data_block[i]);
    }
    
    int write_block_index = get_next_file_index_block(parent);
    add_file_to_block(parent, write_block_index, attr);

    read_inode_table_entry(parent);
    i_node_ptr->file_attribute.size += sizeof(FileAttribute);
    write_inode_table_entry(parent);

    cout << "File written" << endl;

}

void FileSystemManager::read(char* path, char* file_path){
    
    std::cout << "Reading file" << std::endl;

    // Creating File
    std::vector<std::string> path_vector;

    parse_path(path, path_vector);

    int next_inode = 0;

    long unsigned int i = 0;
    for(i = 0; i < path_vector.size(); i++){

        next_inode = get_inode_number(next_inode, path_vector[i]);
        if(next_inode == -1 && i != path_vector.size() - 1){
            std::cout << "Directory not found 1" << std::endl;
            return;
        }
    }

    read_inode_table_entry(next_inode);

    std::ofstream outfile;

    outfile.open(file_path, std::ios::out | std::ios::binary);

    if(!outfile.is_open()){
        std::cout << "File not found" << std::endl;
        return;
    }

    for(i = 0; i < INODE_DIRECT_BLOCK_COUNT; i++){
        if(i_node_ptr->direct_data_block[i] != -1){
            read_block_table_entry(i_node_ptr->direct_data_block[i]);
            outfile.write(block_ptr->data, strlen(block_ptr->data));
            // cout << block_ptr->data << endl;
        }
    }

    // for(i = 0; i < INODE_INDIRECT_BLOCK_COUNT; i++){
    //     if(i_node_ptr->indirect_data_block[i] != -1){
    //         read_block_table_entry(i_node_ptr->indirect_data_block[i]);
    //         int* indirect_block_ptr = (int*)block_ptr->data;
    //         for(int j = 0; j < super_block.block_size / sizeof(int); j++){
    //             if(indirect_block_ptr[j] != -1){
    //                 read_block_table_entry(indirect_block_ptr[j]);
    //                 outfile.write(block_ptr->data, super_block.block_size);
    //             }
    //         }
    //     }
    // }

    outfile.close();

    cout << "File read" << endl;
}


void FileSystemManager::delete_file(char* path){
    
    std::cout << "Deleting file" << std::endl;

    // Deleting File
    std::vector<std::string> path_vector;

    parse_path(path, path_vector);

    int next_inode = 0, prev_inode = 0, parent = 0;

    long unsigned int i = 0;

    for(i = 0; i < path_vector.size(); i++){
        prev_inode = next_inode;
        next_inode = get_inode_number(next_inode, path_vector[i]);
        if(next_inode == -1 && i != path_vector.size() - 1){
            std::cout << "Directory not found 2" << std::endl;
            return;
        }
        parent = prev_inode;
    }

    read_inode_table_entry(next_inode);

    if(i_node_ptr->file_attribute.type == INODE_DIR){
        std::cout << "Cannot delete directory" << std::endl;
        return;
    }

    for(i = 0; i < INODE_DIRECT_BLOCK_COUNT; i++){
        if(i_node_ptr->direct_data_block[i] != -1){
            block_bitmap.block_in_use[i_node_ptr->direct_data_block[i]] = 0;
            block_bitmap.free_blocks[i_node_ptr->direct_data_block[i]] = 1;
        }
    }

    inode_bitmap.block_in_use[next_inode] = 0;
    inode_bitmap.free_blocks[next_inode] = 1;

    write_inode_bitmap();
    write_block_bitmap();

    INode inode;
    inode.file_attribute.type = 0;
    inode.file_attribute.size = 0;
    inode.file_attribute.i_node_index = -1;
    inode.file_attribute.last_modified = 0;
    strcpy(inode.file_attribute.name, "");

    for(int i = 0; i < INODE_DIRECT_BLOCK_COUNT; i++){
        inode.direct_data_block[i] = -1;
    }
    for(int i = 0; i < INODE_INDIRECT_BLOCK_COUNT; i++){
        inode.indirect_data_block[i] = -1;
    }

    i_node_ptr = &inode;
    write_inode_table_entry(next_inode);

    read_inode_table_entry(parent);
    
    int write_index = get_block_index_number(i_node_ptr->direct_data_block[0], path_vector[path_vector.size() - 1]);
    remove_file_from_block(i_node_ptr->direct_data_block[0], write_index);
   
    i_node_ptr->file_attribute.size -= sizeof(FileAttribute);
    write_inode_table_entry(parent);

    std::cout << "File deleted" << std::endl;

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File System Reading and initialization functions    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void FileSystemManager::open_in_disk(){

    disk_in.open(disk_name, std::ios::in | std::ios::ate | std::ios::binary);
    if(!disk_in.is_open()){
        std::cout << "Error opening disk" << std::endl;
        exit(1);
    }
    std::cout << "Opened in disk" << std::endl;

}

void FileSystemManager::open_out_disk(){

    disk_out.open(disk_name, std::ios::out | std::ios::in | std::ios::binary);
    if(!disk_out.is_open()){
        std::cout << "Error opening disk" << std::endl;
        exit(1);
    }
    std::cout << "Opened out disk" << std::endl;

}

void FileSystemManager::close_in_disk(){

    disk_in.close();
    std::cout << "Closed in disk" << std::endl;

}

void FileSystemManager::close_out_disk(){

    disk_out.close();
    std::cout << "Closed out disk" << std::endl;

}



void FileSystemManager::read_super_block(){

    disk_in.clear();
    disk_in.seekg(0);

    disk_in.read((char*)&super_block, sizeof(SuperBlock));

}

void FileSystemManager::print_super_block(){

    std::cout << "Printing SuperBlock" << std::endl;

    std::cout << "SuperBlock File Size:    " << super_block.file_system_size << std::endl;
    
    std::cout << "SuperBlock Inode Bitmap Start:   " << super_block.inode_bitmap_start << std::endl;
    std::cout << "SuperBlock Block Bitmap Start:   " << super_block.block_bitmap_start << std::endl;

    std::cout << "SuperBlock Inode Count:  " << super_block.inode_count << std::endl;
    std::cout << "SuperBlock Inode Size:   " << super_block.inode_size << std::endl;
    std::cout << "SuperBlock Inode Start:   " << super_block.inode_start << std::endl;

    std::cout << "SuperBlock Block Count:    " << super_block.block_count << std::endl;
    std::cout << "SuperBlock Block Size:   " << super_block.block_size << std::endl;
    std::cout << "SuperBlock Block Start:   " << super_block.block_start << std::endl;

    std::cout << std::endl;

}


void FileSystemManager::read_inode_bitmap(){

    disk_in.clear();
    disk_in.seekg(0);

    char* buffer = new char[sizeof(long long) * super_block.inode_count];
    inode_bitmap.free_blocks = new long long[super_block.inode_count];
    inode_bitmap.block_in_use = new long long[super_block.inode_count];

    disk_in.seekg(super_block.inode_bitmap_start);

    disk_in.read(buffer, sizeof(long long) * super_block.inode_count);
    memcpy(inode_bitmap.free_blocks, buffer, sizeof(long long) * super_block.inode_count);

    disk_in.read(buffer, sizeof(long long) * super_block.inode_count);
    memcpy(inode_bitmap.block_in_use, buffer, sizeof(long long) * super_block.inode_count);
    
    delete[] buffer;

}

void FileSystemManager::print_inode_bitmap(){


    for(int i = 0; i < super_block.inode_count; i++){
        std::cout << "[" << i << "]: (" 
        << inode_bitmap.free_blocks[i] << ", " << inode_bitmap.block_in_use[i] << ") - ";     

        // if(inode_bitmap.block_in_use[i] == 1){
        //     std::cout << "In use" << std::endl;
        // }else{
        //     std::cout << "Free" << std::endl;
        // }
    }

    std::cout << std::endl;

}



void FileSystemManager::read_block_bitmap(){

    disk_in.clear();
    disk_in.seekg(0);

    char* buffer = new char[sizeof(long long) * super_block.block_count];
    block_bitmap.free_blocks = new long long[super_block.block_count];
    block_bitmap.block_in_use = new long long[super_block.block_count];

    disk_in.seekg(super_block.block_bitmap_start);

    disk_in.read(buffer, sizeof(long long) * super_block.block_count);
    memcpy(block_bitmap.free_blocks, buffer, sizeof(long long) * super_block.block_count);

    disk_in.read(buffer, sizeof(long long) * super_block.block_count);
    memcpy(block_bitmap.block_in_use, buffer, sizeof(long long) * super_block.block_count);

    delete[] buffer;

}

void FileSystemManager::print_block_bitmap(){

    for(int i = 0; i < super_block.block_count; i++){
        std::cout << "[" << i << "]: (" 
        << block_bitmap.free_blocks[i] << ", " << block_bitmap.block_in_use[i] << ") - ";     
    }

    std::cout << std::endl;

}

void FileSystemManager::read_root_block(){

    disk_in.clear();
    disk_in.seekg(0);

    disk_in.seekg(super_block.root_start);
    disk_in.read((char*)&root, sizeof(FileAttribute));

}   

void FileSystemManager::print_root_block(){

    cout << "Root block: " << endl;
    cout << "Name: " << root.name << endl;
    cout << "Size: " << root.size << endl;
    cout << "Type: " << root.type << endl;
    cout << "I Node Index: " << root.i_node_index << endl;
    cout << "Last Mode: " << root.last_modified << endl;

}


void FileSystemManager::read_inode_table(){

    disk_in.clear();
    disk_in.seekg(0);

    inode_table = new INode[super_block.inode_count];

    disk_in.seekg(super_block.inode_start);

    for(int i = 0; i < super_block.inode_count; i++){
        disk_in.read((char*)&inode_table[i], sizeof(INode));
    }

}

void FileSystemManager::print_inode_table(){

    for(int i = 0; i < super_block.inode_count; i++){
        std::cout << "(" << i << ") -> ";
        std::cout << "N: " << inode_table[i].file_attribute.name << " - ";
        std::cout << "S: " << inode_table[i].file_attribute.size << " - ";
        std::cout << "T: " << inode_table[i].file_attribute.type << " - ";
        std::cout << "LM: " << inode_table[i].file_attribute.last_modified << " ## ";

        for(int j=0; j<INODE_DIRECT_BLOCK_COUNT; ++j){
            std::cout << "( " << j << ") " << inode_table[i].direct_data_block[j] << ", ";
        }
        cout << " ,,, ";
        for(int j=0; j<INODE_INDIRECT_BLOCK_COUNT; ++j){
            std::cout << "(" << j << ") " << inode_table[i].indirect_data_block[j] << ", ";
        }
        std::cout << std::endl;
    }

}


void FileSystemManager::read_block_table(){

    disk_in.clear();
    disk_in.seekg(0);


    blocks = new Block[super_block.block_count];
    for(int i=0 ; i<super_block.block_count; ++i){
        blocks->data = new char[super_block.block_size];
    }

    disk_in.seekg(super_block.block_start);

    for(int i = 0; i < super_block.block_count; i++){
        disk_in.read((char*)blocks[i].data, super_block.block_size);
    }

}

void FileSystemManager::print_block_table(){

    std::cout << "Printing Block Table" << std::endl;

    for(int i = 0; i < super_block.block_count; i++){
        std::cout << "(" << i << ") = ";
        std::cout << blocks[i].data << "  ##  ";
    }

}


void FileSystemManager::read_inode_table_entry(int index){

    if(index >= super_block.inode_count){
        std::cout << "ERROR: Index out of bounds" << std::endl;
        exit(1);
    }

    disk_in.clear();
    disk_in.seekg(0);

    i_node_ptr = new INode[1];

    disk_in.seekg(super_block.inode_start + index * sizeof(INode));
    disk_in.read((char*)i_node_ptr, sizeof(INode));

}

void FileSystemManager::print_inode_table_entry(){

    std::cout << "-----------------------------" << endl;
    std::cout << "Printing Inode Table Entry" << std::endl;

    std::cout << "Name: " << i_node_ptr->file_attribute.name << endl;
    std::cout << "Size: " << i_node_ptr->file_attribute.size << endl;
    std::cout << "Type: " << i_node_ptr->file_attribute.type << endl;
    std::cout << "INode: " << i_node_ptr->file_attribute.i_node_index << endl;
    std::cout << "LM: " << i_node_ptr->file_attribute.last_modified << std::endl;

    for(int i=0; i<INODE_DIRECT_BLOCK_COUNT; ++i){
        std::cout << "(" << i << ")-> " << i_node_ptr->direct_data_block[i] << ", ";
    }
    std::cout << std::endl;
    for(int i=0; i<INODE_INDIRECT_BLOCK_COUNT; ++i){
        std::cout << "(" << i << ") " << i_node_ptr->indirect_data_block[i] << ", ";
    }
    std::cout << std::endl;

}


void FileSystemManager::read_block_table_entry(int index){

    if(index >= super_block.block_count){
        std::cout << "Block index out of range" << std::endl;
        exit(1);
    }

    disk_in.clear();
    disk_in.seekg(0);

    block_ptr = new Block[1];
    block_ptr->data = new char[super_block.block_size];

    disk_in.seekg(super_block.block_start + (index * super_block.block_size));
    disk_in.read((char*)block_ptr->data, super_block.block_size);

}

void FileSystemManager::print_block_table_entry(){

    std::cout << "Printing Block Table" << std::endl;

    std::cout << "##" << block_ptr->data << "##" << endl;

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File System Writing and Update Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void FileSystemManager::write_super_block(){

    disk_out.clear();
    disk_out.seekp(0);

    disk_out.write((char*)&super_block, sizeof(SuperBlock));

}


void FileSystemManager::write_inode_bitmap(){
    
    disk_out.clear();
    disk_out.seekp(0);

    disk_out.seekp(super_block.inode_bitmap_start);
    disk_out.write((char*)inode_bitmap.free_blocks, sizeof(long long) * super_block.inode_count);
    disk_out.write((char*)inode_bitmap.block_in_use, sizeof(long long) * super_block.inode_count);

}


void FileSystemManager::write_block_bitmap(){

    disk_out.clear();
    disk_out.seekp(0);

    disk_out.seekp(super_block.block_bitmap_start);
    disk_out.write((char*)block_bitmap.free_blocks, sizeof(long long) * super_block.block_count);
    disk_out.write((char*)block_bitmap.block_in_use, sizeof(long long) * super_block.block_count);

}


void FileSystemManager::write_inode_table_entry(int index){

    if(index >= super_block.inode_count){
        std::cout << "ERROR: Index out of bounds" << std::endl;
        exit(1);
    }

    disk_out.clear();
    disk_out.seekp(0);

    disk_out.seekp(super_block.inode_start + index * sizeof(INode));
    disk_out.write((char*)i_node_ptr, sizeof(INode));

}


void FileSystemManager::write_block_table_entry(int index){

    if(index >= super_block.block_count){
        std::cout << "Block index out of range" << std::endl;
        exit(1);
    }

    disk_out.clear();
    disk_out.seekp(0);

    disk_out.seekp(super_block.block_start + (index * super_block.block_size));
    disk_out.write((char*)block_ptr->data, super_block.block_size);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// File System Creation Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////


void FileSystemManager::set_super_block(const SuperBlock& _super_block){

    super_block = _super_block;

}


void FileSystemManager::set_block_ptr_data(const Block& block){
    
    block_ptr = new Block[1];
    block_ptr->data = new char[super_block.block_size];

    strcpy(block_ptr->data, block.data);

}


void FileSystemManager::set_inode_ptr_data(const INode& inode){
    
    i_node_ptr = new INode[1];

    strcpy(i_node_ptr->file_attribute.name, inode.file_attribute.name);
    i_node_ptr->file_attribute.size = inode.file_attribute.size;
    i_node_ptr->file_attribute.last_modified = inode.file_attribute.last_modified;

    for(int i=0; i<INODE_DIRECT_BLOCK_COUNT; ++i){
        i_node_ptr->direct_data_block[i] = inode.direct_data_block[i];
    }

    for(int i=0; i<INODE_INDIRECT_BLOCK_COUNT; ++i){
        i_node_ptr->indirect_data_block[i] = inode.indirect_data_block[i];
    }
    
}


SuperBlock FileSystemManager::get_super_block(){
    return super_block;
}


void FileSystemManager::set_inode_bitmap(BitMap _inode_bitmap){

    inode_bitmap = _inode_bitmap;

}

void FileSystemManager::set_block_bitmap(BitMap _block_bitmap){

    block_bitmap = _block_bitmap;

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions for reading and writing
/////////////////////////////////////////////////////////////////////////////////////////////////////////


int FileSystemManager::get_next_use_inode_index(){

    for(int i=0; i<super_block.inode_count; ++i){
        if(inode_bitmap.block_in_use[i] == 0){
            return i;
        }
    }

    return -1;
}

int FileSystemManager::get_next_use_block_index(){

    for(int i=0; i<super_block.block_count; ++i){
        if(block_bitmap.block_in_use[i] == 0){
            return i;
        }
    }

    return -1;

}


int FileSystemManager::get_next_free_inode_index(){

    for(int i=0; i<super_block.inode_count; ++i){
        if(inode_bitmap.free_blocks[i] == 1){
            return i;
        }
    }

    return -1;

}

int FileSystemManager::get_next_free_block_index(){

    for(int i=0; i<super_block.block_count; ++i){
        if(block_bitmap.free_blocks[i] == 1){
            return i;
        }
    }

    return -1;

}


void  FileSystemManager::add_file_to_block(int block_index, int file_index, FileAttribute file){

    if(block_index >= super_block.block_count){
        std::cout << "Block index out of range" << std::endl;
        exit(1);
    }

    if(file_index >= INODE_DIRECT_BLOCK_COUNT){
        std::cout << "File index out of range" << std::endl;
        exit(1);
    }

    disk_out.clear();
    disk_out.seekp(0);

    disk_out.seekp(super_block.block_start + (block_index * super_block.block_size + (file_index * sizeof(FileAttribute))));
    disk_out.write((char*)&file, sizeof(FileAttribute));

}

void FileSystemManager::remove_file_from_block(int block_index, int file_index){

    if(block_index >= super_block.block_count){
        std::cout << "Block index out of range" << std::endl;
        exit(1);
    }

    if(file_index >= INODE_DIRECT_BLOCK_COUNT){
        std::cout << "File index out of range" << std::endl;
        exit(1);
    }

    disk_out.clear();
    disk_out.seekp(0);

    disk_out.seekp(super_block.block_start + (block_index * super_block.block_size + (file_index * sizeof(FileAttribute))));
    
    FileAttribute file;
    memset(&file, 0, sizeof(FileAttribute));
    disk_out.write((char*)&file, sizeof(FileAttribute));

}



int FileSystemManager::get_inode_number(int index, std::string str){

    read_block_table_entry(index);

    FileAttribute file;

    for(long long i=0; i<super_block.block_size; i+=sizeof(FileAttribute)){

        memcpy(&file, block_ptr->data + i, sizeof(FileAttribute));

        if(strcmp(file.name, "") == 0){
            continue;
        }

        if(strcmp(file.name, str.c_str()) == 0){
            return file.i_node_index;
        }
    }
    
    return -1;
}


int FileSystemManager::get_next_file_index_block(int index){

    read_block_table_entry(index);

    FileAttribute file;

    for(long long i=0; i<super_block.block_size; i+=sizeof(FileAttribute)){

        memcpy(&file, &block_ptr->data[i], sizeof(FileAttribute));

        if(strcmp(file.name, "") == 0){
            return i / sizeof(FileAttribute);
        }
        
    }
    
    return -1;
}

int FileSystemManager::get_block_index_number(int index, std::string str){
    
    read_block_table_entry(index);

    FileAttribute file;

    for(long long i=0; i<super_block.block_size; i+=sizeof(FileAttribute)){

        memcpy(&file, block_ptr->data + i, sizeof(FileAttribute));

        if(strcmp(file.name, "") == 0){
            continue;;
        }
        if(strcmp(file.name, str.c_str()) == 0){
            return i / sizeof(FileAttribute);
        }
    }
    
    return -1;
}

void FileSystemManager::list_directory_in_block_index(int index){

    read_block_table_entry(index);
    FileAttribute file;
    for(long long i=0; i<super_block.block_size; i+=sizeof(FileAttribute)){

        memcpy(&file, (&block_ptr->data[i]), sizeof(FileAttribute));
        if(strcmp(file.name, "") == 0){
            continue;
        }

        cout << "-------------------------" << endl;
        cout << "File name [" << i << "] : " << file.name << endl;
        cout << "File size: " << file.size << endl;
        cout << "File last modified: " << file.last_modified << endl;
        cout << "File Index: " << file.i_node_index << endl;
        if(file.type == -1){
            cout << "File type: Directory" << endl;
        }
        else{
            cout << "File type: File" << endl;
        }
    }

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// File System Helper Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileSystemManager::parse_path(char* path, std::vector<string>& path_list){
    
    std::stringstream ss(path);
    std::string item;

    while(std::getline(ss, item, '\\')){
        path_list.push_back(item);
    }
    
    path_list[0] = ".";

}

