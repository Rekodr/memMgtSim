#include "stdafx.h"
#include "File.h"
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

File::File()
{

}

File::File(std::string& name)
{
	file.exceptions(ifstream::failbit | ifstream::badbit | ifstream::eofbit);

	try {
	file.open(name);
	}
	catch (ifstream::failure e) {
	cerr << "faillure to open input file" << endl;
	exit(1);
	}
}

File::~File()
{
	file.close();
}

querry File::next()
{
	char buffer[querryLn];
	string tmp;
	char a;
	char b;
	unsigned int pid;
	unsigned int pageRef; 
	char* endPtr;
	try {
		file.getline(buffer, querryLn);

		// parse the input
		// cout << buffer << endl; 
		stringstream ss(buffer); 
		ss >> a >> pid >> b >> tmp; 
		pageRef = strtol(tmp.c_str(), &endPtr, 2);
		return make_pair(pid, pageRef);
	}
	catch (ifstream::failure e) {
		throw exception("End of file reached");
	}

}
