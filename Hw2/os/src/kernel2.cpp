
#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>


// #define GRAPHICSMODE

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;


void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}

int number_of_digits(int num){
    if(num == 0) return 0;
    return 1 + number_of_digits(num / 10);
}

void printfDec(char c[], int x){

    int index = number_of_digits(x) - 1;
    while (x > 0)
    {
        int num = x % 10;
        c[index--] = num + '0';
        x /= 10;
    }
}


class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }
    
};

GlobalDescriptorTable gdt;

TaskManager taskManager;

void taskA()
{
    while(true)
        printf("A");
}
void taskB()
{
    while(true)
        printf("B");
}


void tmp_foo(){

    uint8_t i[] = {1,1,1,1,1,1,1,1,1};
    uint8_t j[] = {1,1,1,1,1,1,1,1,1};
    uint8_t k[] = {1,1,1,1,1,1,1,1,1};
    uint8_t l[] = {1,1,1,1,1,1,1,1,1};
    uint8_t m[] = {1,1,1,1,1,1,1,1,1};
    
    uint8_t n[] = {1,1,1,1,1,1,1,1,1};
    uint8_t o[] = {1,1,1,1,1,1,1,1,1};
    uint8_t p[] = {1,1,1,1,1,1,1,1,1};
    uint8_t r[] = {1,1,1,1,1,1,1,1,1};
    uint8_t s[] = {1,1,1,1,1,1,1,1,1};

    while (1);

}

void print_task_stack(void *t){

    Task* task = (Task*)t;
    printf("Task ");
    printfHex(task->id);
    printf(" stack: ");
    for(int i = 0; i < 600; i++){
        printfHex(i);
        printf("-");
        printfHex(task->stack[i]);
        printf(" ");
    }
    printf("\n");

    while(1);
}

void sleep(int time = 1000){
    for (int i=0; i<time / 2; ++i)
		for (int j=0; j<time; ++j)
			for(int k=0; k<time; ++k);

}

void print_dinamic_mem(void* ptr, size_t size){

    for(int i = 0; i < size; i++){
        printfHex(((uint8_t*)ptr)[i]);
        printf(" ");
        sleep(1000);
    }
    printf("\n");
}

void print_size_t(size_t size){

    // printf("size_t: 0x");
    printfHex((size >> 8 ) & 0xFF);
    printfHex((size      ) & 0xFF);
    // printf("\n");
}

void print_size_t2(size_t size){

    printf("size_t: 0x");
    printfHex((size >> 24) & 0xFF);
    printfHex((size >> 16) & 0xFF);
    printfHex((size >> 8 ) & 0xFF);
    printfHex((size      ) & 0xFF);
    printf("\n");
}

int in_page_table_fifo(Page vm_pages[], uint32_t len, uint32_t page_number){

    for(uint32_t i=0; i<len; i++){
        if(vm_pages[i].page_number == page_number)
            return i;
    }

    return -1;
}

const int KB = 1024;
const int MB = KB * KB;
const int GB = KB * KB * KB;

void FIFO_PAGING(){

    uint32_t data_size = 128;

    uint32_t page_size = 16 * sizeof(uint32_t);
    uint32_t vm_page_count = 4;
    uint32_t mem_page_count = 16;

    uint32_t vm_page_size = vm_page_count * page_size;
    uint32_t mem_page_size = mem_page_count * page_size; 

    uint32_t page_table[vm_page_count];
    for(int i = 0; i < vm_page_count; i++){
        page_table[i] = -1;
    }

    uint32_t vm[vm_page_size];
    for(int i = 0; i < vm_page_size; i++){
        vm[i] = 0;
    }

    uint32_t mem[mem_page_size];
    for(int i = 0; i < mem_page_size; i++){
        mem[i] = 0;
    }

    Page vm_pages[vm_page_count];
    for(int i = 0; i < vm_page_count; i++){
        vm_pages[i].page_number = -1;
        vm_pages[i].is_used = 0;
        vm_pages[i].referenced = 0;
        vm_pages[i].modified = 0;
    }

    uint32_t data[data_size] = {
        51, 93, 98, 7, 28, 94, 38, 25, 29, 29, 94, 29, 9, 42, 36, 19, 
        57, 75, 31, 90, 57, 89, 25, 43, 48, 61, 29, 16, 79, 27, 28, 30, 
        21, 26, 90, 1, 20, 28, 26, 49, 9, 72, 30, 18, 67, 66, 90, 24, 
        93, 73, 15, 50, 62, 92, 46, 63, 6, 75, 79, 85, 55, 7, 68, 76, 
        85, 58, 77, 5, 38, 56, 6, 47, 80, 36, 66, 47, 54, 8, 72, 0, 
        81, 39, 2, 96, 31, 48, 59, 37, 76, 90, 75, 31, 97, 43, 59, 82, 
        1, 88, 39, 91, 44, 97, 38, 25, 33, 56, 24, 39, 64, 48, 39, 98, 
        87, 94, 94, 71, 42, 5, 8, 18, 95, 35, 1, 44, 78, 60, 78, 31,
    };

    uint32_t data_page_count = sizeof(data) / page_size;
    for(uint32_t i=0; i<data_size; ++i){
        mem[i] = data[i];
    }

    /*
        sort mem_pages values with page fault
        in fifo
    */

    {
        int next = 0;
        uint8_t miss = 0;
        for(int i=0; i<data_size; ++i){
            for(int j=0; j < data_size - i; ++j){

                int page_number_left = j / page_size;
                int page_number_right = j+1 / page_size;

                int ret_l = in_page_table_fifo(vm_pages, vm_page_count, page_number_left);
                int ret_r = in_page_table_fifo(vm_pages, vm_page_count, page_number_right);

                if(ret_l < 0){
                    ret_l = page_number_left;
                    vm_pages[next].page_number = page_number_left;
                    vm_pages[next].is_used = 1;
                    vm_pages[next].referenced = 1;
                    vm_pages[next].modified = 1;
                    next = (next + 1) % vm_page_count;

                    for(int k=0; k<page_size; ++k){
                        vm[page_number_left * page_size + k] = mem[page_number_left * page_size + k];
                    }
                    miss++;
                }

                if(ret_r < 0){
                    ret_r = page_number_right;
                    vm_pages[next].page_number = page_number_right;
                    vm_pages[next].is_used = 1;
                    vm_pages[next].referenced = 1;
                    vm_pages[next].modified = 1;
                    next = (next + 1) % vm_page_count;

                    for(int k=0; k<page_size; ++k){
                        vm[page_number_right * page_size + k] = mem[page_number_right * page_size + k];
                    }
                    miss++;
                }

                if(ret_l >= 0){
                    vm_pages[ret_l].referenced = 1;
                }

                if(ret_r >= 0){
                    vm_pages[ret_r].referenced = 1;
                }

                if(vm[ret_l * page_size + j] > vm[ret_r * page_size + j + 1]){
                    uint32_t tmp = vm[ret_l * page_size + j + 1];
                    vm[ret_l * page_size + j] = vm[ret_r * page_size + j];
                    vm[ret_r * page_size + j] = tmp;
                }

                /*
                    If pages are modified put back to mem
                */
                for(int k=0; k<page_size && vm_pages[ret_l].modified == 1; ++k){
                    mem[page_number_left * page_size + k] = vm[ret_l * page_size + k];
                }

                for(int k=0; k<page_size && vm_pages[ret_r].modified == 1; ++k){
                    mem[page_number_right * page_size + k] = vm[ret_r * page_size + k];
                }

                vm_pages[ret_l].modified = 0;
                vm_pages[ret_r].modified = 0;
            }
        }

        printf("MISS :");
        printfHex(miss);
        printf("\n");
    }

    {
        int next = 0;
        uint8_t miss = 0;

        int key, j;
        for(int i = 1; i < data_size; ++i){

            int page_number = i / page_size;
            
            int ret = in_page_table_fifo(vm_pages, vm_page_count, page_number);

            if(ret < 0){
                ret = page_number;
                vm_pages[next].page_number = page_number;
                vm_pages[next].is_used = 1;
                vm_pages[next].referenced = 0;
                vm_pages[next].modified = 1;
                next = (next + 1) % vm_page_count;

                for(int k=0; k<page_size; ++k){
                    vm[page_number * page_size + k] = mem[page_number * page_size + k];
                }
                miss++;

                key = vm[page_number * page_size + i % page_size];

            }else{
                key = vm[ret * page_size + i % page_size];
            }

            if(ret >= 0){
                vm_pages[ret].referenced = 1;
            }


            j = i - 1;
            while(j >=0 && vm[page_number * page_size + j % page_size] > key){
                vm[page_number * page_size + j % page_size] = vm[page_number * page_size + (j % page_size) + 1];
                j--;
            }

            for(j = i - 1; j >= 0 && vm_pages[ret].modified == 1; --j){
                mem[page_number * page_size + j % page_size] = vm[ret * page_size + j % page_size];
            }
        }

        printf("MISS :");
        printfHex(miss);
        printf("\n");
    }


    while (1)
    {}
}    


int in_page_table_second_fifo(Page vm_pages[], uint32_t len, uint32_t page_number){

    for(uint32_t i=0; i<len; i++){
        if(vm_pages[i].page_number == page_number){
            return i;
        }   
        vm_pages[i].referenced = 0;            
    }

    for(int i=0; i<len; i++){
        if(vm_pages[i].referenced == 0)
            return i;
    }

    return -1;
}


void SECOND_CHANCE_FIFO(){

    uint32_t data_size = 128;

    uint32_t page_size = 16 * sizeof(uint32_t);
    uint32_t vm_page_count = 4;
    uint32_t mem_page_count = 16;

    uint32_t vm_page_size = vm_page_count * page_size;
    uint32_t mem_page_size = mem_page_count * page_size; 

    uint32_t page_table[vm_page_count];
    uint32_t vm[vm_page_size];
    uint32_t mem[mem_page_size];


    for(int i = 0; i < vm_page_count; i++){
        page_table[i] = -1;
    }
    for(int i = 0; i < vm_page_size; i++){
        vm[i] = 0;
    }
    for(int i = 0; i < mem_page_size; i++){
        mem[i] = 0;
    }

    Page vm_pages[vm_page_count];
    for(int i = 0; i < vm_page_count; i++){
        vm_pages[i].page_number = -1;
        vm_pages[i].is_used = 0;
        vm_pages[i].referenced = 0;
        vm_pages[i].modified = 0;
    }

    uint32_t data[data_size] = {
        51, 93, 98, 7, 28, 94, 38, 25, 29, 29, 94, 29, 9, 42, 36, 19, 
        57, 75, 31, 90, 57, 89, 25, 43, 48, 61, 29, 16, 79, 27, 28, 30, 
        21, 26, 90, 1, 20, 28, 26, 49, 9, 72, 30, 18, 67, 66, 90, 24, 
        93, 73, 15, 50, 62, 92, 46, 63, 6, 75, 79, 85, 55, 7, 68, 76, 
        85, 58, 77, 5, 38, 56, 6, 47, 80, 36, 66, 47, 54, 8, 72, 0, 
        81, 39, 2, 96, 31, 48, 59, 37, 76, 90, 75, 31, 97, 43, 59, 82, 
        1, 88, 39, 91, 44, 97, 38, 25, 33, 56, 24, 39, 64, 48, 39, 98, 
        87, 94, 94, 71, 42, 5, 8, 18, 95, 35, 1, 44, 78, 60, 78, 31,
    };

    uint32_t data_page_count = sizeof(data) / page_size;

    for(uint32_t i=0; i<data_size; ++i){
        mem[i] = data[i];
    }

    {

        int next = 0;
        uint8_t miss = 0;

        for(int i = 0; i < data_size; ++i){
            for(int j = 0; j < data_size - i; ++j){

                int page_number_left = j / page_size;
                int page_number_right = (j + 1) / page_size;

                int ret_l = in_page_table_second_fifo(vm_pages, vm_page_count, page_number_left);
                int ret_r = in_page_table_second_fifo(vm_pages, vm_page_count, page_number_right);

                if(ret_l < 0){
                    ret_l = page_number_left;
                    vm_pages[next].page_number = page_number_left;
                    vm_pages[next].is_used = 1;
                    vm_pages[next].referenced = 0;
                    vm_pages[next].modified = 1;
                    next = (next + 1) % vm_page_count;

                    for(int k=0; k<page_size; ++k){
                        vm[page_number_left * page_size + k] = mem[page_number_left * page_size + k];
                    }

                    miss++;
                }
                if(ret_r < 0){
                    ret_r = page_number_right;
                    vm_pages[next].page_number = page_number_right;
                    vm_pages[next].is_used = 1;
                    vm_pages[next].referenced = 0;
                    vm_pages[next].modified = 1;
                    next = (next + 1) % vm_page_count;

                    for(int k=0; k<page_size; ++k){
                        vm[page_number_right * page_size + k] = mem[page_number_right * page_size + k];
                    }

                    miss++;
                }

                if(ret_l >= 0){
                    vm_pages[ret_l].referenced = 1;
                }
                if(ret_r >= 0){
                    vm_pages[ret_r].referenced = 1;
                }

                // swap variables
                if(vm[ret_l * page_size + j] < vm[ret_r * page_size + j+1]){
                    uint32_t tmp = vm[ret_l * page_size + j + 1];
                    vm[ret_l * page_size + j] = vm[ret_r * page_size + j];
                    vm[ret_r * page_size + j] = tmp;
                }

                for(int k=0; k<page_size && vm_pages[ret_l].modified == 1; ++k){
                    mem[page_number_left * page_size + k] = vm[ret_l * page_size + k];
                }

                for(int k=0; k<page_size && vm_pages[ret_r].modified == 1; ++k){
                    mem[page_number_right * page_size + k] = vm[ret_r * page_size + k];
                }

                vm_pages[ret_l].modified = 0;
                vm_pages[ret_r].modified = 0;

            }
        }

        printf("MISS :");
        printfHex(miss);
        printf("\n");
    }


    {

        // 
        int next = 0;
        uint8_t miss = 0;

        int key, j;
        for(int i = 1; i < data_size; ++i){

            int page_number = i / page_size;
            
            int ret = in_page_table_second_fifo(vm_pages, vm_page_count, page_number);

            if(ret < 0){
                ret = page_number;
                vm_pages[next].page_number = page_number;
                vm_pages[next].is_used = 1;
                vm_pages[next].referenced = 0;
                vm_pages[next].modified = 1;
                next = (next + 1) % vm_page_count;

                for(int k=0; k<page_size; ++k){
                    vm[page_number * page_size + k] = mem[page_number * page_size + k];
                }
                miss++;

                key = vm[page_number * page_size + i % page_size];
            }else{
                key = vm[ret * page_size + i % page_size];
            }

            if(ret >= 0){
                vm_pages[ret].referenced = 1;
            }

            j = i - 1;
            while(j >=0 && vm[page_number * page_size + j % page_size] > key){
                vm[page_number * page_size + j % page_size] = vm[page_number * page_size + (j % page_size) + 1];
                j--;
            }

            for(j = i - 1; j >= 0 && vm_pages[ret].modified == 1; --j){
                mem[page_number * page_size + j % page_size] = vm[ret * page_size + j % page_size];
            }

        }

        printf("MISS :");
        printfHex(miss);
        printf("\n");

    }

    while (1)
    {}

}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    

    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    

    Task task1(&gdt, &FIFO_PAGING);
    // Task task1(&gdt, &SECOND_CHANCE_FIFO);

    taskManager.AddTask(&task1);

    InterruptManager interrupts(0x20, &gdt, &taskManager);
    
    #ifdef GRAPHICSMODE
        Desktop desktop(320,200, 0x00,0x00,0xA8);
    #endif
    
    DriverManager drvManager;
    
        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);
        
    
        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
        #endif
        drvManager.AddDriver(&mouse);
        
        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);

        VideoGraphicsArray vga;
        
    printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();
        
    printf("Initializing Hardware, Stage 3\n");

    #ifdef GRAPHICSMODE
        vga.SetMode(320,200,8);
        Window win1(&desktop, 10,10,20,20, 0xA8,0x00,0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40,15,30,30, 0x00,0xA8,0x00);
        desktop.AddChild(&win2);
    #endif


    interrupts.Activate();
    
    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    }
}
