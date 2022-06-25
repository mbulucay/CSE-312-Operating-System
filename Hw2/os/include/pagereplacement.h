#ifndef __MYOS__PAGEREPLACEMENT_H
#define __MYOS__PAGEREPLACEMENT_H


#include <common/types.h>
#include <gdt.h>


namespace myos
{

    #define PAGE_SIZE 2
    #define MEMORY_SIZE 4
    #define DISK_SIZE 16
    #define PAGE_TABLE_SIZE (DISK_SIZE + MEMORY_SIZE)

    class PageEntry;

    class Memory{

            Memory();
            ~Memory();

        public:

            static void setInitialMemory();

            static common::int32_t writePage(int index, int pageNumber, common::int32_t page[PAGE_SIZE]);
            
            static PageEntry getPageStatus(int pageNumber);
            static common::int32_t isInMemory(int pageNumber);

            static common::int32_t* getPage(int index);
            static common::int32_t getNextFreePage();

            static void printMemory();

            static common::int32_t memory[MEMORY_SIZE][PAGE_SIZE];
            static common::int32_t isFree[MEMORY_SIZE];

    };


    class MEM{

            MEM();
            ~MEM();

        public:

            static void setInitialMEM();

            static common::int32_t writePage(int index, int pageNumber, common::int32_t page[PAGE_SIZE]);
            static common::int32_t writePage2(int index, int pageNumber, common::int32_t page[PAGE_SIZE]);
            
            static PageEntry getPageStatus(int pageNumber);
            static common::int32_t isInDisk(int pageNumber);

            static common::int32_t* getPage(int index);
            static common::int32_t getNextFreePage();

            static common::int32_t getPageUsingPageFrame(int pageFrame);

            static void printDisk();

            static common::int32_t LMEM[DISK_SIZE][PAGE_SIZE];
            static common::int32_t isFree[DISK_SIZE];

    };

    typedef struct PageEntry{

        common::int32_t memory_index;
        common::int32_t disk_index;         // bulundugu index
        common::int32_t reference;          // son cicyle da kullanıldı mı?
        common::int32_t modified;           // is modified or not
        common::int32_t present;            // is present or not
        common::int32_t pageNumber;         // programin kacinci page i olduguna dair
        common::int32_t isFree;             // in use or not

    }PageEntry;


    class PageTable{


        public:
            PageTable();
            ~PageTable();

            static void setInitialPageTable();

            static PageEntry getPageEntry(int i);
            static int setPageEntry(int i, PageEntry p);

            static int getNextFreePage();
            static common::int32_t getPageUsingPageNumber(int pageFrame);

            static void removePresentFromMemory(int pageNumber);

            static common::int32_t inPage(common::int32_t pageNumber, common::int32_t reference);

            static common::int32_t writeBackToDisk(int pageNumber);
            static common::int32_t getFromDisk(int pageNumber);

            static common::int32_t getMemoryIndex(int pageNumber);
            static common::int32_t getDiskIndex(int pageNumber);

            static void printPageTable();

            static PageEntry pageTable[MEMORY_SIZE + DISK_SIZE];

    };


}


#endif // ! __MYOS__PAGEREPLACEMENT_H