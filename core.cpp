#include "stdafx.h"
#include "core.h"
#include <iostream>

using namespace std;


namespace memProject {

#define memSize 16 // in KB
#define frameSize 1 // in KB
#define NUM_FRAMES (memSize / frameSize)
#define MAX_SNAP_HIST 4
	ofstream f;

	Mem::Mem()
	{
		initMem();
	}

	Mem::Mem(std::ifstream* f) : Mem()
	{
		input = f;
		
	}

	string Mem::search_PCB(querry Q) 
	{
		auto pid = Q.first;
		auto pgNum = Q.second;
		//get the pcb
		pcbPtr p = getPCB(pid);
		pageTablePtr table = p->tablePtr; // get the page table;
		
		pagePtr page = search_pgRef(table, pgNum); // get the page;
		if (page->st == PAGE_FAULT) {
			table->page_fault++;
			//std::cout << "page fault occured" << std::endl;
		}
		++(table->numOfRef);
		page->hasChanged = true;
		bool found = frameAllocator(page);

		auto st = saveJson();
		return st;
	}



	pcbPtr Mem::getPCB(unsigned int pid) 
	{
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

	pagePtr Mem::search_pgRef(pageTablePtr pt, unsigned int pgNumber)
	{

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
				if ((*it)->resident == false) {
					(*it)->referenced = false; // mark the page as referenced
					(*it)->st = PAGE_FAULT;	// the page is memory;
					fault = true; 
					return *it;
				}

				(*it)->referenced = true; //update the reference.
				(*it)->st = IN_MEM;
				return *it;
			}
		}

		//if reached here, the page has never been referenced.
		pagePtr pg = new page;
		pg->number = pgNumber;
		pg->referenced = false;
		pg->resident = false;
		pg->pid = pt->owner_pid;
		pg->st = PAGE_FAULT;
		fault = true;
		pt->table.push_back(pg);

		return pg;
	}

	bool Mem::frameAllocator(pagePtr pg) 
	{

		switch (pg->st)
		{
		case IN_MEM:
			//std::cout << "active" << std::endl; 
			break;
		case PAGE_FAULT:  assignFrame(pg);
			break;
		case CHK_FREE_POOL: return searchFreePool(pg); // Used here.
			break;
		default:
			break;
		}

		return false;
	}

	bool Mem::searchFreePool(pagePtr pg) 
	{
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

		/*
		* reactivate a frame if it is in the free pool and then
		* re-ferenced before it got overwrited.
		*/

		if (it != freePool.begin()) {
			(*it)->pgPtr->referenced = true;
			(*it)->pgPtr->frameNumber = (*it)->number;
			(*it)->pgPtr->st = IN_MEM; // no page fault;
			(*it)->state = REFERENCED;

			// move the frame from the free pool to the active pool
			activePool.push_back(*it);
			freePool.erase(it);
			return true;
		}

		pg->referenced = false; // just making sure, but this line is not needed.
		assignFrame(pg); // assign a new memFrame.

		//was not in the free pool
		return false;
	}

	void Mem::assignFrame(pagePtr pg) 
	{
		//std::cout << "new page fault" << std::endl;

		if (freePool.empty())
			return;

		bool page_replaced = false;

		while (page_replaced == false) {
			framePtr tmp = freePool.at(clockHand);
			if (tmp->pgPtr == nullptr) {
				tmp->pgPtr = pg;
				pg->resident = true;
				pg->frameNumber = tmp->number;
				page_replaced = true;
				tmp->hasChanged = true;
			}

			else if (tmp->pgPtr->referenced) {
				tmp->pgPtr->referenced = false;
				tmp->hasChanged = true; 
			}
			else {
				// At this point the clock hand has made a complete round
				// and it clearing the first page with ref bit set to 0 
				tmp->pgPtr->resident = false; //update the page Info;
				tmp->pgPtr->hasChanged = true; 
				tmp->pgPtr = pg;
				pg->resident = true;
				pg->frameNumber = tmp->number;
				page_replaced = true;
				tmp->hasChanged = true;
			}
			clockHand = (++clockHand) % NUM_FRAMES;
		}
	}

	
	string Mem::saveJson()
	{

		Json::Value snapshot;
		snapshot["atime"] = snapId++;
		snapshot["num_procs"] = pcbs.size();
		snapshot["faulted"] = fault;
		snapshot["clockHandPosition"] = clockHand;
		fault = false;
		snapshot["pcbTable"];

		for (auto it = pcbs.begin(); it != pcbs.end(); ++it) {
			//saving PCBs
			auto pageTable = (it->second).tablePtr->table;
			Json::Value pcb;
			Json::Value table;
			pcb["id"] = (it->second).pid;

			table["pageFaults"] = (it->second).tablePtr->page_fault;
			table["numOfRef"] = (it->second).tablePtr->numOfRef; 
			table["pageTableSize"] = (it->second).tablePtr->table.size();
			//saving table element
			for (auto& x : pageTable) {
				//saving each page
				Json::Value pg;
				pg["pageId"] = x->number;
				pg["resident"] = x->resident;
				pg["referenced"] = x->referenced;
				pg["frameId"] = x->frameNumber;
				pg["hasChanged"] = x->hasChanged;
				x->hasChanged = false; 
				table["pages"].append(pg);
			}
			pcb["pgTable"] = table; 
			snapshot["pcbTable"].append(pcb);
		}

		//saving frames states
		for (auto frame : freePool) {
			Json::Value f;
			f["id"] = frame->number;
			f["pid"] = -1;
			f["pageId"] = -1;
			f["referenced"] = false;
			f["free"] = true;
			f["isClockHand"] = false;
			
			if (frame->pgPtr == nullptr) {
				
			}
			else {
				f["pid"] = frame->pgPtr->pid;
				f["pageId"] = frame->pgPtr->number;
				f["referenced"] = frame->pgPtr->referenced;
				f["free"] = false;
			}


			f["hasChanged"] = frame->hasChanged;
			frame->hasChanged = false;

			if (frame->number == clockHand)
				f["isClockHand"] = true;

			snapshot["frameTable"].append(f);
		}

		jsroot["snapshots"].append(snapshot);

		auto n = jsroot["snapshots"].size();
		if (n > MAX_SNAP_HIST) {
			Json::Value tmp;
			jsroot["snapshots"].removeIndex(0, &tmp);

		}


		string st;
		stringstream ss;
		Json::StreamWriterBuilder builder;
		builder["commentStyle"] = "None";
		builder["indentation"] = "\t"; 
		std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
		writer->write(jsroot, &ss);
		f.open("data.Json");
		f << jsroot;
		f.close();
		st = ss.str();
		return st;
	}

	int Mem::loadJson(Json::Value& value, unsigned int atime)
	{
		clear();
		Json::Value snapshots = value["snapshots"];
		Json::Value  js;
		auto i{ 0 };
		for (auto snapShot : snapshots) {
			if (snapShot["atime"].asInt() == atime) {

				js = snapShot;
				break;
			}
			i++;
		}

		snapId = js["atime"].asInt();
		clockHand = js["clockHandPosition"].asInt();
		fault = js["faulted"].asBool();

		Json::Value pcbTable = js["pcbTable"];
		for (auto& pc : pcbTable) {
			unsigned int id = pc["id"].asInt();
			pcbPtr p = getPCB(id);
			pageTablePtr table = p->tablePtr;

			Json::Value pgTable = pc["pgTable"];
			table->numOfRef = pgTable["numOfRef"].asInt();
			table->owner_pid = id;
			table->page_fault = pgTable["pageTableSize"].asInt();
			table->page_fault = pgTable["pageFaults"].asInt();

			Json::Value pages = pgTable["pages"];
			for (auto& pg : pages) {
				unsigned int pgNum = pg["pageId"].asInt();
				pagePtr pageT = search_pgRef(table, pgNum);
				pageT->hasChanged = pg["hasChanged"].asBool();
				pageT->frameNumber = pg["frameId"].asInt();
				pageT->resident = pg["resident"].asBool();
				pageT->referenced = pg["referenced"].asBool();
				for (auto& f : freePool) {
					if (f->number == pageT->frameNumber) {
						f->pgPtr = pageT;
						f->hasChanged = pageT->hasChanged;
					}
				}
			}
		}

		Json::Value t;
		jsroot["snapshots"].removeIndex(i + 1, &t);
		jsroot["snapshots"].removeIndex(i, &t);
		
		
		saveJson();
		return 0;
	}

	void Mem::clear()
	{
		for (auto& frame : freePool) {
			frame->pgPtr = nullptr;
		}

		freePool.clear();

		for (auto& x : pcbs) {
			pcb tmp = x.second;
			pageTablePtr tt = tmp.tablePtr;
			pageVector table = tt->table;

			for (auto& pg : table) {
				free(pg);
			}
			free(tt);
		}

		pcbs.clear();
		free(memoryFrames);
		initMem();
		jsroot.clear();

		clockHand = 0;
		snapId = 0;
	}


	bool Mem::initMem()
	{

		
		//init the mem size 
		memoryFrames = (framePtr)malloc(sizeof(memFrame) * NUM_FRAMES);
		for (int i = 0; i < NUM_FRAMES; i++) {
			//create frames
			memoryFrames[i].number = i;
			memoryFrames[i].state = CLEAN; // not used
			memoryFrames[i].pgPtr = nullptr;
			//initially all frames are in the free pool;
			freePool.push_back(&memoryFrames[i]);
		}

		if (freePool.size() != NUM_FRAMES)
			return false;

		return true;
	}
}

