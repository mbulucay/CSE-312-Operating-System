#ifndef FILE_SYSTEM_MANAGER_H
#define FILE_SYSTEM_MANAGER_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "blocks.h"


class FileSystemManager{

    public:

        FileSystemManager(std::string _disk_name);
        ~FileSystemManager();

        void test();

        // File System Operations
        void list_directory(char* path);
        void create_directory(char* path);
        void remove_directory(char* path);
        void dumpe2fs();
        void write(char* path, char* file_path);
        void read(char* path, char* file_path);
        void delete_file(char* path);

        // File System Reading and initialization functions
        void open_in_disk();
        void open_out_disk(); 
        void close_in_disk();
        void close_out_disk();

        void read_super_block();
        void print_super_block();


        void read_inode_bitmap();
        void print_inode_bitmap();
        void read_block_bitmap();
        void print_block_bitmap();

        void read_root_block();
        void print_root_block();

        void read_inode_table();
        void print_inode_table();

        void read_block_table();
        void print_block_table();

        void read_inode_table_entry(int index);
        void print_inode_table_entry();

        void read_block_table_entry(int index);
        void print_block_table_entry();


        // File System Writing and updating functions
        void write_super_block();
        void write_inode_bitmap();
        void write_block_bitmap();
        void write_inode_table_entry(int index);
        void write_block_table_entry(int index);

        // Setters and Getters
        void set_super_block(const SuperBlock& _super_block);
        void set_block_ptr_data(const Block& block);
        void set_inode_ptr_data(const INode& inode);
        void set_inode_bitmap(BitMap _inode_bitmap);
        void set_block_bitmap(BitMap _block_bitmap);
        SuperBlock get_super_block();
        
        // Helper functions
        void parse_path(char* path, std::vector<string>& path_list);

        // Helper functions for reading and writing
        int get_next_use_inode_index();
        int get_next_use_block_index();
                
        int get_next_free_inode_index();
        int get_next_free_block_index();

        int get_inode_number(int index, std::string str);
        int get_block_index_number(int index, std::string str);

        void add_file_to_block(int block_index, int file_index, FileAttribute file);
        void remove_file_from_block(int block_index, int file_index);

        int get_next_file_index_block(int index);
        
        void list_directory_in_block_index(int index);

    private:

        std::string disk_name;

        SuperBlock super_block;

        BitMap block_bitmap;
        BitMap inode_bitmap;

        FileAttribute root;

        INode* inode_table;

        Block* blocks;

        INode* i_node_ptr;
        Block* block_ptr;

        std::ifstream disk_in;
        std::fstream disk_out;

};

#endif // FILE_SYSTEM_MANAGER_H
