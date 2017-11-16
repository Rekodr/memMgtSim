#include "stdafx.h"
#include "core.h"
#include <iostream>

using namespace std;


#define memSize 16 // in KB
#define frameSize 1 // in KB
#define NUM_FRAMES (memSize / frameSize)

memFrame* memoryFrames;
pcbTable pcbs;
frameVec freePool;
frameVec activePool;

pcbPtr getPCB(unsigned int pid) {
	auto it = pcbs.begin();
	for (; it != pcbs.end(); ++it) {
		if (it->first == pid)
			return &it->second;
	}

	pcbPtr p = new pcb;
	pageTablePtr pgT = new pageTable;
	pgT->owner_pid = pid;
	pgT->page_fault = 0;
	pcbs[pid] = pcb{
		pid,
		pgT
	};
	p = &pcbs[pid];
	
	return p;
}



void search_PCB(querry Q) {
	auto pid = Q.first;
	auto pgNum = Q.second;
	//get the pcb
	pcbPtr p = getPCB(pid);
	pageTablePtr table = p->tablePtr; // get the page table;

	pagePtr page = search_pgRef(table, pgNum); // get the page;
	bool found = frameAllocator(page);

	if (found == false) {
		table->page_fault++;
		std::cout << "page fault occured" << std::endl;
	}
}



 pagePtr search_pgRef(pageTablePtr pt, unsigned int pgNumber) {

	/*
	*	Look for the page into the page table. If the page is in the 
	*	table but not referenced, then tell the allocator to look for 
	*	the page in the free pool list. Otherwise, the page is active.
	*	If the page is not found in the page table, then allocate new 
	*   new page and tell the allocator to allocate a new memFrame.
	*/ 
	auto it = pt->table.begin();
	for (; it != pt->table.end(); ++it) {
		if ((*it)->number == pgNumber) {
			if ((*it)->referenced == false) {
				(*it)->st = REF_FAULT;
				return *it;
			}
			(*it)->st = ACTIVE;
			return *it;
		}
	}
	
	
	pagePtr pg = new page;
	pg->number = pgNumber;
	pg->referenced = false;
	pg->st = NEW_PAGE_FAULT;
	pt->table.push_back(pg);
	
	return pg;
}

 bool frameAllocator(pagePtr pg) {

	 switch (pg->st)
	 {
	 case ACTIVE:
		 //std::cout << "active" << std::endl; 
		 break;
	 case NEW_PAGE_FAULT:  allocateFrame(pg);
		 break;
	 case REF_FAULT: return searchFreePool(pg);
		 break;
	 default:
		 break;
	 }
	 return false;
 }

 bool searchFreePool(pagePtr pg) {
	 //std::cout << "reference fault" << std::endl;
	 auto it = freePool.end();

	 /*
	 * search the memFrame in the free pool. If the memFrame
	 * is there, reclaim it -> move it from free pool to
	 * active pool.
	 */ 
	 for (; it != freePool.begin(); --it) {
		 if ((*it)->pgPtr == pg) {
			 break;
		 }
	 }

	 // move memFrame from free pool to active pool
	 if (it != freePool.begin()) {
		 (*it)->pgPtr->referenced = true;
		 (*it)->pgPtr->frameNumber = (*it)->number;
		 (*it)->pgPtr->st = ACTIVE; // no page fault;
		 (*it)->state = REFERENCED;

		 // move the memFrame from the free pool to the active pool
		 activePool.push_back(*it);
		 freePool.erase(it);
		 return true; 
	 }

	 pg->referenced = false; // just making sure, but this line is not needed.
	 allocateFrame(pg); // assign a new memFrame.

	 //was not in the free pool
	 return false;
 }

 void allocateFrame(pagePtr pg) {
	 //std::cout << "new page fault" << std::endl;

	/* 
	* Take the head of the free pool and assign it to a
	* new page. Then remove it from the free pool.
	*/
	 auto it = freePool.begin();
	 if (freePool.empty())
		 return;

	 framePtr f = (*it);
	 f->pgPtr = pg;
	 pg->frameNumber = f->number;
	 pg->referenced = true;
	 pg->st = NEW_PAGE_FAULT;
	 f->state = REFERENCED;

	 activePool.push_back(f);
	 freePool.erase(it);
 }

 bool initMem() {

	 //init the mem size 
	 memoryFrames = (framePtr)malloc(sizeof(memFrame) * NUM_FRAMES);

	 for (int i = 0; i < NUM_FRAMES; i++) {
		 //create frames
		 memoryFrames[i].number = i;
		 memoryFrames[i].state = CLEAN;
		 memoryFrames[i].pgPtr = nullptr;

		 //initially all frames are in the free pool;
		 freePool.push_back(&memoryFrames[i]);
	 }

	 if (freePool.size() == NUM_FRAMES)
		 return true; 

	 return false;
 }

 