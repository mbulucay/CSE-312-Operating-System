#include <pagereplacement.h>

using namespace myos;
using namespace myos::common;


void printf2(char* str)
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

void printfHex2(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf2(foo);
}

int number_of_digits2(int num){
    if(num == 0) return 0;
    return 1 + number_of_digits2(num / 10);
}

void printfDec2(char c[], int x){

    int index = number_of_digits2(x) - 1;
    while (x > 0)
    {
        int num = x % 10;
        c[index--] = num + '0';
        x /= 10;
    }

    c[index] = '\0';
    printf2(c);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////STATIC VARIABLES//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


PageEntry PageTable::pageTable[PAGE_TABLE_SIZE] = {0};

common::int32_t Memory::memory[MEMORY_SIZE][PAGE_SIZE] = {0};
common::int32_t Memory::isFree[MEMORY_SIZE] = {0};

common::int32_t MEM::LMEM[DISK_SIZE][PAGE_SIZE] = {0};
common::int32_t MEM::isFree[DISK_SIZE] = {0};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////PAGE TABLE//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


PageTable::PageTable(){

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        pageTable[i].disk_index = -1;
        pageTable[i].reference = 0;
        pageTable[i].modified = 0;
        pageTable[i].present = 0;
        pageTable[i].pageNumber = -1;
        pageTable[i].isFree = 1;
    }

}

PageTable::~PageTable()
{   }


void PageTable::setInitialPageTable(){

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        pageTable[i].disk_index = -1;
        pageTable[i].reference = 0;
        pageTable[i].modified = 0;
        pageTable[i].present = 0;
        pageTable[i].pageNumber = -1;
        pageTable[i].isFree = 1;
        pageTable[i].memory_index = -1;

    }

}


PageEntry PageTable::getPageEntry(int i){

    if(i > PAGE_TABLE_SIZE)
        return {-1, -1, -1, -1};
    
    return pageTable[i];
}


int PageTable::setPageEntry(int i, PageEntry p){

    if(i >= PAGE_TABLE_SIZE)
        return -1;

    pageTable[i] = p;
    return 0;
}


int PageTable::getNextFreePage(){

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        if(pageTable[i].isFree){
            return i;
        }
    }

    return -1;
}


void PageTable::printPageTable(){

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){

        if(pageTable[i].isFree){
            printf2("Page ");
            printfHex2(i);
            printf2(" is free\n");
            continue;
        }

        printfHex2(i);
        printf2(")");
        printfHex2(pageTable[i].disk_index);
        printf2(" ");
        printfHex2(pageTable[i].reference);
        printf2(" ");
        printfHex2(pageTable[i].modified);
        printf2(" ");
        printfHex2(pageTable[i].present);
        printf2(" ");
        printfHex2(pageTable[i].pageNumber);
        printf2(" ");
        printfHex2(pageTable[i].isFree);

        printf2("\n");
    }

}


int32_t PageTable::writeBackToDisk(int pageNumber){
    
    if(pageNumber >= PAGE_TABLE_SIZE)
        return -1;
    
    int memory_index = getMemoryIndex(pageNumber);
    if(memory_index == -1)
        return -1;
    int disk_index = getDiskIndex(pageNumber);
    if(disk_index == -1)
        return -1;
    
    for(int i=0; i<PAGE_SIZE; ++i){
        MEM::LMEM[disk_index][i] = Memory::memory[memory_index][i];
    }

    Memory::isFree[memory_index] = true;

    return -1;
}


int32_t PageTable::getFromDisk(int pageNumber){

    int disk_index = MEM::isInDisk(pageNumber);

    if(disk_index != -1){

        int32_t* tmp = MEM::getPage(disk_index);

    }


    return -1;
}


common::int32_t PageTable::getPageUsingPageNumber(int pageFrame){

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        if(pageTable[i].pageNumber == pageFrame){
            return i;
        }
    }

    return -1;

}


void PageTable::removePresentFromMemory(int pageNumber){

    int page_index = getPageUsingPageNumber(pageNumber);

    if(page_index != -1){
        pageTable[page_index].present = 0;
    }

    return;
}

common::int32_t PageTable::getMemoryIndex(int pageNumber){

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        if(pageTable[i].pageNumber == pageNumber && pageTable[i].present){
            return pageTable[i].memory_index;
        }
    }

    return -1;

}

common::int32_t PageTable::getDiskIndex(int pageNumber){

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        if(pageTable[i].pageNumber == pageNumber && !pageTable[i].present){
            return pageTable[i].disk_index;
        }
    }

    return -1;

}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////MEMORY//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Memory::Memory(){

}

Memory::~Memory(){

}


void Memory::setInitialMemory(){

    for(int i=0; i<MEMORY_SIZE; ++i){
        for(int j=0; j<PAGE_SIZE; ++j){
            memory[i][j] = 0;
        }
    }

    for(int i=0; i<MEMORY_SIZE; ++i){
        isFree[i] = 1;
    }

}


int32_t Memory::writePage(int index, int pageNumber ,common::int32_t page[PAGE_SIZE]){

    //If the page is not in the memory, then we need to allocate it
    for(int i=0; i<PAGE_SIZE; ++i){
        memory[index][i] = page[i];
    }
    isFree[index] = 0;

    //If the page is in the memory, then we need to update the page entry
    int getNextFreePage = PageTable::getNextFreePage();

    if(getNextFreePage != -1){
        PageEntry p;
        p.disk_index = index;
        p.reference = true;
        p.modified = 0;
        p.present = true;
        p.pageNumber = pageNumber;
        p.isFree = 0;
        PageTable::setPageEntry(getNextFreePage, p);
        return 0;
    }

    return -1;
}


/*
    Get Page According to the page number of program
    Bir programin ram de bulunan sayfa numarasının verilmesi gerekir
    Ve o sayfa ile ilgili status bilgisi döndürülmelidir
*/

PageEntry Memory::getPageStatus(int pageNumber){

    if(pageNumber >= PAGE_TABLE_SIZE)
        return {-1, -1, -1, -1, -1, -1};

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        if( PageTable::pageTable[i].pageNumber == pageNumber && 
            PageTable::pageTable[i].present == true){
            
            return PageTable::pageTable[i];
        }
    }
    
    return {-1, -1, -1, -1, -1, -1};
}


int32_t Memory::isInMemory(int pageNumber){

    if(pageNumber >= PAGE_TABLE_SIZE)
        return -1;

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        if( PageTable::pageTable[i].pageNumber == pageNumber && 
            PageTable::pageTable[i].present == 1){
            return i;
        }
    }

    return -1;
}


common::int32_t* Memory::getPage(int index){

    if(index >= MEMORY_SIZE)
        return nullptr;
    
    return memory[index];
}


common::int32_t Memory::getNextFreePage(){

    for(int i=0; i<MEMORY_SIZE; ++i){
        if(isFree[i] == 1){
            return i;
        }
    }

    return -1;
}


void Memory::printMemory(){

    for(int i=0; i<MEMORY_SIZE; ++i){
        printfHex2(i);
        printf2(")");
        for(int j=0; j<PAGE_SIZE; ++j){
            printfHex2(memory[i][j]);
            printf2(" ");
        }
        printf2(" ");
    }
    printf2("\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////DISK//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


MEM::MEM(){

}

MEM::~MEM(){

}


void MEM::setInitialMEM(){
    
    for(int i=0; i<DISK_SIZE; ++i){
        for(int j=0; j<PAGE_SIZE; ++j){
            LMEM[i][j] = 0;
        }
    }

    for(int i=0; i<DISK_SIZE; ++i){
        isFree[i] = 1;
    }

}


int32_t MEM::writePage(int index, int pageNumber, common::int32_t page[PAGE_SIZE]){

    //If the page is not in the LMEM, then we need to allocate it
    
    for(int i=0; i<PAGE_SIZE; ++i){
        LMEM[index][i] = page[i];
    }
    isFree[index] = false;

    int getNextFreePage = PageTable::getNextFreePage();
    if(getNextFreePage != -1){
        PageEntry p;
        p.disk_index = index;
        p.reference = true;
        p.modified = false;
        p.present = false;
        p.pageNumber = pageNumber;
        p.isFree = false;
        PageTable::setPageEntry(getNextFreePage, p);
        return 0;
    }

    return -1;

}

int32_t MEM::writePage2(int index, int pageNumber, common::int32_t page[PAGE_SIZE]){

    //If the page is not in the LMEM, then we need to allocate it
    
    for(int i=0; i<PAGE_SIZE; ++i){
        LMEM[index][i] = page[i];
    }
    isFree[index] = false;

    PageTable::pageTable[index].modified = false;

    return 0;

}


PageEntry MEM::getPageStatus(int pageNumber){

    if(pageNumber >= PAGE_TABLE_SIZE)
        return {-1, -1, -1, -1, -1, -1};

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        if( PageTable::pageTable[i].pageNumber == pageNumber && 
            PageTable::pageTable[i].present == false){
            
            return PageTable::pageTable[i];
        }
    }

    return {-1, -1, -1, -1, -1, -1};

}


int32_t MEM::isInDisk(int pageNumber){

    if(pageNumber >= PAGE_TABLE_SIZE)
        return -1;

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        if( PageTable::pageTable[i].pageNumber == pageNumber && 
            PageTable::pageTable[i].present == 0){
            return i;
        }
    }

    return -1;
}


common::int32_t* MEM::getPage(int index){

    if(index >= DISK_SIZE)
        return nullptr;

    return LMEM[index];
}


common::int32_t getPageUsingPageFrame(int pageFrame){

    if(pageFrame >= PAGE_TABLE_SIZE)
        return -1;

    return PageTable::pageTable[pageFrame].pageNumber;

}


int32_t MEM::getNextFreePage(){

    for(int i=0; i<DISK_SIZE; ++i){
        if(isFree[i] == 1){
            return i;
        }
    }

    return -1;

}


void MEM::printDisk(){

    for(int i=0; i<DISK_SIZE; ++i){
        printfHex2(i);
        printf2(")");
        for(int j=0; j<PAGE_SIZE; ++j){
            printfHex2(LMEM[i][j]);
            printf2(" ");
        }
        printf2("\n");
    }

}


common::int32_t PageTable::inPage(common::int32_t pageNumber, common::int32_t present){

    if(pageNumber >= PAGE_TABLE_SIZE)
        return -1;

    for(int i=0; i<PAGE_TABLE_SIZE; ++i){
        if( PageTable::pageTable[i].pageNumber == pageNumber && 
            PageTable::pageTable[i].present == present){
            return i;
        }
    }

    return -1;
}

