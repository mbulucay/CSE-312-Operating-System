
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

#include <pagereplacement.h>

// #define GRAPHICSMODE

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;

const int KB = 1024;
const int MB = KB * KB;


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

    c[index] = '\0';
    printf(c);
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
// #define MEM::LMEM log_mem;
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

void printMemory(){
    
    for(int i=0; i<MEMORY_SIZE; ++i){
        for(int j=0; j<PAGE_SIZE; ++j){
            printfHex(Memory::memory[i][j]);
            printf(" ");
        }
    }
    printf("\n");
}

void printPageTable(){

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){

        if(PageTable::pageTable[i].isFree){
            continue;
        }
        printfHex(i);
        printf(")");
        printfHex(PageTable::pageTable[i].disk_index);
        printf(" ");
        printfHex(PageTable::pageTable[i].memory_index);
        printf(" ");
        printfHex(PageTable::pageTable[i].reference);
        printf(" ");
        printfHex(PageTable::pageTable[i].modified);
        printf(" ");
        printfHex(PageTable::pageTable[i].present);
        printf(" ");
        printfHex(PageTable::pageTable[i].pageNumber);
        printf(" ");
        printfHex(PageTable::pageTable[i].isFree);
        printf("\n");

    }
    printf("\n");

}

void printDisk(){

    for(int i=0; i<DISK_SIZE / 2; ++i){
        for(int j=0; j<PAGE_SIZE; ++j){
            printfHex(MEM::LMEM[i][j]);
            printf(" ");
        }
    }
    printf("\n");   

}

int in_fifo(PageEntry fifo[], int size, int value){
    for(int i=0; i<size; ++i){
        if(fifo[i].memory_index == value){
            return i;
        }
    }
    return -1;
}


int32_t data_size = 16;

int32_t data[] = {
    51, 93, 98, 7, 28, 94, 38, 25, 29, 29, 94, 29, 9, 42, 36, 19
};

void FIFO(){

    MEM::setInitialMEM();
    PageTable::setInitialPageTable();


// Boblle Sort
    {

        for(int i=0; i<data_size / PAGE_SIZE; ++i){

            int page[PAGE_SIZE];
            for(int j=0; j<PAGE_SIZE; ++j){
                page[j] = data[i * PAGE_SIZE + j];
            }

            for(int j=0; j<PAGE_SIZE; ++j){
                MEM::LMEM[i][j] = page[j];
            }

            PageTable::pageTable[i].reference = 0;
            PageTable::pageTable[i].modified = 0;
            PageTable::pageTable[i].isFree = 0;

            PageTable::pageTable[i].present = 0;
            PageTable::pageTable[i].memory_index = 0;
            PageTable::pageTable[i].disk_index = i;
            PageTable::pageTable[i].pageNumber = i;
        }


        int miss = 0, hit = 0, next = 0, write_counter = 0;
        int num1 = 0, num2 = 0;
        PageEntry fifo[MEMORY_SIZE];

        for(int i=0; i<data_size - 1; ++i){
            
            for(int j=0; j < data_size - i - 1; ++j ){
                

                int page_1 = j / PAGE_SIZE;
                if(in_fifo(fifo, MEMORY_SIZE, page_1) == -1){
                    miss++;

                    PageTable::pageTable[next % PAGE_TABLE_SIZE].reference = 0;

                    PageTable::pageTable[page_1].reference = 1;
                    PageTable::pageTable[page_1].modified = 0;
                    PageTable::pageTable[page_1].isFree = 0;
                    PageTable::pageTable[page_1].present = 1;
                    PageTable::pageTable[page_1].memory_index = next % MEMORY_SIZE;

                    fifo[next % MEMORY_SIZE] = PageTable::pageTable[page_1];

                    // copy memory to memory_index
                    for(int k=0; k<PAGE_SIZE; ++k){
                        Memory::memory[PageTable::pageTable[page_1].memory_index][k] = MEM::LMEM[page_1][k];
                    }

                    next++;
                }else{
                    hit++;
                }
                
                int page_2 = (j + 1) / PAGE_SIZE;
                if(in_fifo(fifo, MEMORY_SIZE, page_2) == -1){
                    miss++;

                    PageTable::pageTable[next % PAGE_TABLE_SIZE].reference = 0;

                    PageTable::pageTable[page_2].reference = 1;
                    PageTable::pageTable[page_2].modified = 0;
                    PageTable::pageTable[page_2].isFree = 0;
                    PageTable::pageTable[page_2].present = 1;
                    PageTable::pageTable[page_2].memory_index = next % MEMORY_SIZE;

                    fifo[next % MEMORY_SIZE] = PageTable::pageTable[page_2];

                    // copy memory to memory_index
                    for(int k=0; k<PAGE_SIZE; ++k){
                        Memory::memory[PageTable::pageTable[page_2].memory_index][k] = MEM::LMEM[page_2][k];
                    }

                    next++;
                }else{
                    hit++;
                }

                if(MEM::LMEM[page_1][j % PAGE_SIZE] > MEM::LMEM[page_2][(j + 1) % PAGE_SIZE] 
                    && MEM::LMEM[page_1][j % PAGE_SIZE] > MEM::LMEM[page_2][(j + 1) % PAGE_SIZE]){
                    int32_t temp = MEM::LMEM[page_1][j % PAGE_SIZE];
                    MEM::LMEM[page_1][j % PAGE_SIZE] = MEM::LMEM[page_2][(j + 1) % PAGE_SIZE];
                    MEM::LMEM[page_2][(j + 1) % PAGE_SIZE] = temp;
                    write_counter++;
                }

                // for(int i=0; i<PAGE_SIZE; ++i){
                //     MEM::LMEM[page_1][i] = Memory::memory[page_1][i];
                //     MEM::LMEM[page_2][i] = Memory::memory[page_2][i];
                // }
            }

        }

        printf("FIFO\n");
        printPageTable();
        printMemory();
        printDisk();
        printf("Miss : ");
        printfHex(miss);
        printf("\n");
        printf("Hit : ");
        printfHex(hit);
        printf("\n");
        printf("Write : ");
        printfHex(write_counter);
        printf("\n");
        printf("Loaded : ");
        printfHex(miss);
        printf("\n");

    }



// Inersertion Sort

/*
    {

        for(int i=0; i<data_size / PAGE_SIZE; ++i){

            int page[PAGE_SIZE];
            for(int j=0; j<PAGE_SIZE; ++j){
                page[j] = data[i * PAGE_SIZE + j];
            }

            for(int j=0; j<PAGE_SIZE; ++j){
                MEM::LMEM[i][j] = page[j];
            }

            PageTable::pageTable[i].reference = 0;
            PageTable::pageTable[i].modified = 0;
            PageTable::pageTable[i].isFree = 0;

            PageTable::pageTable[i].present = 0;
            PageTable::pageTable[i].memory_index = -1;
            PageTable::pageTable[i].disk_index = i;
            PageTable::pageTable[i].pageNumber = i;
        }

        int miss = 0, hit = 0, next = 0, write_counter = 0;
        int num1 = 0, num2 = 0, key;
        PageEntry fifo[MEMORY_SIZE];

        for(int i=0; i<data_size; ++i){
            
            int page_number = i / PAGE_SIZE;
            key = MEM::LMEM[page_number][i % PAGE_SIZE];
            int keyRow[PAGE_SIZE];

            for(int j=0; j<PAGE_SIZE; ++j){
                keyRow[j] = MEM::LMEM[page_number][j];
            }
            
            int index_fifo = in_fifo(fifo, MEMORY_SIZE, page_number);

            if(fifo[index_fifo].reference == 0){
                miss++;

                PageTable::pageTable[page_number].reference = 1;
                PageTable::pageTable[page_number].modified = 0;
                PageTable::pageTable[page_number].isFree = 0;
                PageTable::pageTable[page_number].present = 1;
                PageTable::pageTable[page_number].memory_index = index_fifo;
                
                for(int i=0; i<PAGE_SIZE; ++i){
                    Memory::memory[next % MEMORY_SIZE][i] = MEM::LMEM[page_number][i];
                }

            }else{
                hit++;
            }

            int j = i - 1;
            while(j >= 0 && key < MEM::LMEM[page_number][j % PAGE_SIZE]){
                MEM::LMEM[page_number][((j+1)  % PAGE_SIZE)] = MEM::LMEM[page_number][j % PAGE_SIZE];
                j--;
            }

            for(int k=0; k<PAGE_SIZE; ++k){
                MEM::LMEM[((j+1)  % PAGE_SIZE)][k] = keyRow[k];
            }

        }
    
        printf("Insertion Sort\n");
        printPageTable();
        printMemory();
        printDisk();
        printf("Miss : ");
        printfHex(miss);
        printf("\n");
        printf("Hit : ");
        printfHex(hit);
        printf("\n");
        printf("Write : ");
        printfHex(write_counter);
        printf("\n");
        printf("Loaded : ");
        printfHex(miss);
        printf("\n");

    }
*/
    while(true);

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
    printf("Hello World! --- http://www.AlgorithMan.de\n");

    GlobalDescriptorTable gdt;
    
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);

    for(int i = 0; i<13; i++)
        printf("\n");

    TaskManager taskManager;

    Task task1(&gdt, FIFO);
    taskManager.AddTask(&task1);
    
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    
    // printf("Initializing Hardware, Stage 1\n");
    
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
        
    // printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();
        
    // printf("Initializing Hardware, Stage 3\n");

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
