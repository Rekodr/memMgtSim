#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <fstream>

#include "json.h"


namespace memProject {

	typedef std::pair<unsigned int, unsigned int> querry;

	//page 
	typedef enum { PAGE_FAULT, CHK_FREE_POOL, IN_MEM } pgSt;
	typedef struct {
		unsigned int number;
		unsigned int pid;
		bool resident;
		bool referenced;
		unsigned int frameNumber;
		pgSt st;
		bool hasChanged{ false };
	} page;
	typedef page*  pagePtr;

	typedef std::vector<pagePtr> pageVector;

	//process page table
	typedef struct {
		unsigned owner_pid;
		unsigned int page_fault{ 0 };
		unsigned int numOfRef{ 0 };
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
		bool hasChanged{ false };
	} memFrame;
	typedef memFrame* framePtr;


	typedef std::vector<framePtr> frameVec;


	class Mem {
	public:
		Mem();
		Mem(std::ifstream* inputFile);
		std::string search_PCB(querry Q);
		void clear();
		int loadJson(Json::Value& js, unsigned int atime);
	private:
		bool fault{ false };
		bool initMem();
		pcbPtr getPCB(unsigned int pid);
		pagePtr search_pgRef(pageTablePtr pt, unsigned int pgNumber);
		bool frameAllocator(pagePtr pg);
		bool searchFreePool(pagePtr pg);
		void assignFrame(pagePtr pg);
		std::string saveJson();

		memFrame* memoryFrames;
		pcbTable pcbs;
		frameVec freePool;
		frameVec activePool;
		int clockHand{ 0 };
		unsigned int snapId{ 0 };
		
		std::istream* input;
		std::ofstream file;
		Json::Value jsroot;

	};

}