// mem.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <cstdio>
#include <iostream>
#include <string>
#include "File.h"


using namespace std; 
int main(int argc, char* argv[])
{
	if (argc != 2) {
		cerr << "use: program_name pageStringRef_file" << endl;
		exit(1);
	}

	string name = string(argv[1]);
	File* f = new File(name);

	if (!initMem()) {
		cerr << "Error creating the mem " << endl;
		exit(1);
	}
	while (1) {
		try {
			querry Q = f->next();
			search_PCB(Q);

		}
		catch (exception e) {
			break; 
		}
	}
    return 0;
}

