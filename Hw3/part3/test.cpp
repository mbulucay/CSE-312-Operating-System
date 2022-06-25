#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include "file_system_manager.h"


using namespace std;


int main(int argc, const char** argv) {

 
    FileSystemManager file_system_manager("fileSystem.data");

    // file_system_manager.test();

    file_system_manager.open_in_disk();

    file_system_manager.read_super_block();
    // file_system_manager.print_super_block();
    
    file_system_manager.read_inode_bitmap();
    // file_system_manager.print_inode_bitmap();

    file_system_manager.read_block_bitmap();
    // file_system_manager.print_block_bitmap();

    file_system_manager.read_root_block();
    // file_system_manager.print_root_block();

    // file_system_manager.read_inode_table();
    // file_system_manager.print_inode_table();

    // file_system_manager.read_block_table();
    // file_system_manager.print_block_table();

    // file_system_manager.read_inode_table_entry(0);
    // file_system_manager.print_inode_table_entry();

    // file_system_manager.read_block_table_entry(4);
    // file_system_manager.print_block_table_entry();

    file_system_manager.close_in_disk();

    //////////////////////////////////////////////////////////////////
    //Setting new super block 

    // file_system_manager.open_out_disk();

    // SuperBlock sb;
    // memset(&sb, 0, sizeof(SuperBlock));

    // file_system_manager.set_super_block(sb);
    // file_system_manager.write_super_block();

    // file_system_manager.close_out_disk();

    // file_system_manager.open_in_disk();

    // file_system_manager.read_super_block();
    // file_system_manager.print_super_block();

    // file_system_manager.close_in_disk();


    //////////////////////////////////////////////////////////////////
    // Setting new block

    // file_system_manager.open_out_disk();

    // Block b;
    // strcpy(b.data, "new data");

    // file_system_manager.set_block_ptr_data(b);
    // file_system_manager.write_block_table_entry(0);

    // file_system_manager.close_out_disk();

    // file_system_manager.open_in_disk();

    // file_system_manager.read_block_table_entry(0);
    // file_system_manager.print_block_table_entry();

    // file_system_manager.close_in_disk();

    //////////////////////////////////////////////////////////////////
    // Add file to block

    // file_system_manager.open_in_disk();

    // file_system_manager.read_block_table_entry(0);
    // file_system_manager.print_block_table_entry();

    // file_system_manager.close_in_disk();


    // FileAttribute file_attribute;
    // file_attribute.type = 1;
    // file_attribute.size = 0;
    // file_attribute.i_node_index = 1;
    // file_attribute.last_modified = 0;
    // strcpy(file_attribute.name, "new_file");


    // file_system_manager.open_out_disk();

    // file_system_manager.add_file_to_block(0, 2, file_attribute);

    // file_system_manager.close_out_disk();


    // file_system_manager.open_in_disk();

    // file_system_manager.read_block_table_entry(0);
    // file_system_manager.print_block_table_entry();

    // file_system_manager.list_directory("new_file");


    // file_system_manager.close_in_disk();


    //////////////////////////////////////////////////////////////////
    // INode 
    
    // file_system_manager.open_out_disk();
    // INode inode;
    // memset(&inode, 0, sizeof(INode));

    // strcpy(inode.file_attribute.name, "new_file");
    // inode.file_attribute.size = 10;
    // inode.direct_data_block[0] = 242;
    // inode.direct_data_block[1] = 243;
    // inode.direct_data_block[2] = 244;

    // file_system_manager.set_inode_ptr_data(inode);
    // file_system_manager.write_inode_table_entry(0);

    // file_system_manager.close_out_disk();

    // file_system_manager.open_in_disk();

    // file_system_manager.read_inode_table_entry(0);
    // file_system_manager.print_inode_table_entry();

    // file_system_manager.close_in_disk();


    //////////////////////////////////////////////////////////////////
    // Bitmap 

    // file_system_manager.open_out_disk();

    // BitMap bm;
    // bm.block_in_use = new long long[file_system_manager.get_super_block().block_count];    
    // bm.free_blocks = new long long[file_system_manager.get_super_block().block_count];
    
    // BitMap bm2;
    // bm2.block_in_use = new long long[file_system_manager.get_super_block().inode_count];
    // bm2.free_blocks = new long long[file_system_manager.get_super_block().inode_count];

    // for (int i = 0; i < file_system_manager.get_super_block().block_count; i++) {
    //     bm.block_in_use[i] = 0;
    //     bm.free_blocks[i] = 1;
    // }

    // for (int i = 0; i < file_system_manager.get_super_block().inode_count; i++) {
    //     bm2.block_in_use[i] = 0;
    //     bm2.free_blocks[i] = 1;
    // }

    // bm2.block_in_use[0] = bm.block_in_use[0] = 1;
    // bm2.free_blocks[0] = bm.free_blocks[0] = 0;

    // file_system_manager.set_block_bitmap(bm);
    // file_system_manager.set_inode_bitmap(bm2);

    // file_system_manager.write_block_bitmap();
    // file_system_manager.write_inode_bitmap();


    //////////////////////////////////////////////////////////////////
    // Listing

    file_system_manager.open_in_disk();

    // file_system_manager.list_directory("\\usr\\ysa");
    // file_system_manager.list_directory_from_path(0);

    file_system_manager.close_in_disk();

    //////////////////////////////////////////////////////////////////
    // Creating new directory

    file_system_manager.open_out_disk();
    file_system_manager.open_in_disk();

    // file_system_manager.create_directory("\\usr");
    // file_system_manager.create_directory("\\usr\\ysa");
    // file_system_manager.create_directory("\\bin");
    // file_system_manager.create_directory("\\tmp\\ysa");

    // file_system_manager.read_block_table_entry(1);
    // file_system_manager.print_block_table_entry();

    // file_system_manager.list_directory_in_block_index(0);
    // file_system_manager.list_directory_in_block_index(1);
    // file_system_manager.list_directory_in_block_index(2);
    // file_system_manager.list_directory_in_block_index(3);
    // file_system_manager.list_directory_in_block_index(4);
    // file_system_manager.list_directory_in_block_index(5);

    // file_system_manager.print_block_bitmap();
    // file_system_manager.print_inode_bitmap();

    // file_system_manager.read_inode_table_entry(0);
    // file_system_manager.print_inode_table_entry();
    // file_system_manager.read_inode_table_entry(1);
    // file_system_manager.print_inode_table_entry();
    // file_system_manager.read_inode_table_entry(2);
    // file_system_manager.print_inode_table_entry();
    // file_system_manager.read_inode_table_entry(3);
    // file_system_manager.print_inode_table_entry();

    file_system_manager.close_in_disk();
    file_system_manager.close_out_disk();


    //////////////////////////////////////////////////////////////////
    // Write

    file_system_manager.open_out_disk();
    file_system_manager.open_in_disk();

    // file_system_manager.write("\\usr\\ysa\\test.txt", "write_data");
    // file_system_manager.write("\\usr\\ysa\\test2.txt", "write_data_2");

    // file_system_manager.write("\\test.txt", "write_data");
    // file_system_manager.write("\\test2.txt", "write_data_2");

    file_system_manager.close_in_disk();
    file_system_manager.close_out_disk();


    //////////////////////////////////////////////////////////////////
    // Read

    // file_system_manager.open_in_disk();

    // file_system_manager.read("\\usr\\ysa\\test.txt", "read_data");
    // file_system_manager.read("\\usr\\ysa\\test2.txt", "read_data_2");


    // file_system_manager.close_in_disk();

    //////////////////////////////////////////////////////////////////
    // Delete file

    file_system_manager.open_out_disk();
    file_system_manager.open_in_disk();

    // file_system_manager.delete_file("\\usr\\ysa\\test2.txt");
    
    // file_system_manager.list_directory_in_block_index(2);
    // file_system_manager.read_inode_table_entry(4);
    // file_system_manager.print_inode_table_entry();

    file_system_manager.close_in_disk();
    file_system_manager.close_out_disk();


    //////////////////////////////////////////////////////////////////
    // Dump2fss

    file_system_manager.open_in_disk();

    // file_system_manager.dumpe2fs();

    file_system_manager.close_in_disk();
    

    return 0;
}

