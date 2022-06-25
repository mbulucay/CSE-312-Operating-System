
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


#define FALSE 0
#define TRUE 1
#define N 2                                                             /* number of processes */

int turn;                                                               /* whose turn is it? */
int interested[N] = { 0, 0 };                                           /* all values initially 0 (FALSE) */

void enter_region(int process)                                          /* process is 0 or 1 */
{
    int other;                                                          /* number of the other process */
    other = 1 - process;                                                /* the opposite of process */
    interested[process] = TRUE;                                         /* show that you are interested */
    turn = process;                                                     /* set flag */

    while (turn == process && interested[other] == TRUE);               /* null statement */
}


void leave_region(int process)                                          /* process: who is leaving */
{
    interested[process] = FALSE;                                        /* indicate departure from critical region */
}


#define SIZE 100
int count = 0;

void producer(){

    int item;

    while(true){

        enter_region(0);

        if(count == SIZE){
            leave_region(0);
            continue;
        }

        item = count;
        count++;

        leave_region(0);
        enter_region(1);

        printf("prod:");
        printfHex(item);
        printf("--");
        // for(int i=0; i<2000; i++)
        //     for(int j = 0; j < 2000; j++);
        
        leave_region(1);
    }
}

void consumer(){

    int item;

    while(true){
            
            enter_region(1);
    
            if(count == 0){
                leave_region(1);
                continue;
            }
    
            item = count;
            count--;
    
            leave_region(1);
            enter_region(0);
            
            printf("cons:");
            printfHex(item);
            printf("--");
            // for(int i=0; i<5000; i++)
            //     for(int j = 0; j < 5000; j++);


            leave_region(0);
    }
}


void thread_create(Thread* thread, GlobalDescriptorTable* gdt, void(*f)()){
    
    thread->set_cpu_state(gdt, f);
    thread->setState(0);
    thread->setPC(0);
    // thread->setFunc(f);
}

// thread sil
void thread_exit(Thread* thread, Task* task){
    task->delete_thread(thread);
}

// verilen threadi bekliyor
int thread_join(Task* task, Thread* thread_1, Thread* thread_2, void* retval){
    
    thread_1->setJoin(thread_2->getid());

    return 0;
}

// bir sonraki threade yer veriyor
void thread_yield(Task* task, Thread* thread_1, Thread* thread_2){
    thread_1->setYield(thread_2->getid());
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


void taskA()
{
    while(true){
        printf("A");
        for(int i = 0; i < 10000; i++)
            for(int j = 0; j < 10000; j++);

    }
}
void taskB()
{
    while(true){
        printf("B");  
        for(int i = 0; i < 10000; i++)
            for(int j = 0; j < 10000; j++);

    }
}
void taskC()
{
    while(true){
        printf("C");
        for(int i = 0; i < 10000; i++)
            for(int j = 0; j < 10000; j++);
    }
}
void taskD()
{
    while(true){
        printf("D");
        for(int i = 0; i < 10000; i++)
            for(int j = 0; j < 10000; j++);
    }
}
void taskE()
{
    while(true){
        printf("E");
        for(int i = 0; i < 10000; i++)
            for(int j = 0; j < 10000; j++);
    }
}


#define CAP 20
int product_counter = 0;

void producer_racecondition(){
    while(true){
        if(product_counter < CAP){
            int temp = product_counter + 1;
            //could be interrupt
            product_counter = temp;
            printf("P: ");
            printfHex(temp);
            printf(" = ");
        }
    }
}

void consumer_racecondition(){
    while(true){
        if(product_counter > 0){
            //coul be interrupt
            int temp = product_counter - 1;
            product_counter = temp;
            printf("C: ");
            printfHex(temp);
            printf(" = ");

            // for(int i=0; i<10000; i++)
            //     for(int j = 0; j < 10000; j++);
        }
    }
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
    GlobalDescriptorTable gdt;

    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    
    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF);
    
    void* allocated = memoryManager.malloc(1024);
    printf("\nallocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8 ) & 0xFF);
    printfHex(((size_t)allocated      ) & 0xFF);
    printf("\n");

    TaskManager taskManager;

    printf("Initializing Hardware, Stage 1\n");

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
