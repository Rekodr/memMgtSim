#pragma once

#include <vector>
#include <map>


typedef std::pair<unsigned int, unsigned int> querry;


//page 
typedef enum { NEW_PAGE_FAULT, REF_FAULT, ACTIVE } pgSt;
typedef struct {
	unsigned int number;
	bool referenced;
	pgSt st;
	unsigned int frameNumber;
} page;

typedef page*  pagePtr;
typedef std::vector<pagePtr> pageVector;

//process page table
typedef struct {
	unsigned owner_pid;
	unsigned int page_fault;
	pageVector table;
} pageTable;

//process PCB
typedef pageTable* pageTablePtr;

typedef struct {
	unsigned pid;
	pageTablePtr tablePtr;
} pcb;
typedef pcb* pcbPtr;

//PCB table
typedef std::map<int, pcb> pcbTable;

typedef enum {
	CLEAN,
	REFERENCED,
	AVAILABLE
} frame_state;

//memFrame
typedef struct {
    unsigned int number;
	frame_state state;
	pagePtr pgPtr;
} memFrame;
typedef memFrame* framePtr;


typedef std::vector<framePtr> frameVec;

bool initMem();

pcbPtr getPCB(unsigned int pid);
pagePtr search_pgRef(pageTablePtr pt, unsigned int pgNumber);
void search_PCB(querry Q);
bool frameAllocator(pagePtr pg);
bool searchFreePool(pagePtr pg);
void allocateFrame(pagePtr pg);