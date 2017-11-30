#include "stdafx.h"
#include "File.h"
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

namespace memProject {

	File::File()
	{

	}

	File::File(std::string& name)
	{
		file.exceptions(ifstream::failbit | ifstream::badbit);
		changeFile(name);
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
			auto oldPosition = file.tellg();
			positions.push(oldPosition);
			file.getline(buffer, querryLn);

			auto temp = strlen(buffer);
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

	void File::rewind()
	{
		file.clear();
		file.seekg(0, file.beg);
	}

	void File::gotTo() {
		file.clear();
		if (positions.empty() == false) {
			auto oldPosition = positions.top();
			positions.pop();
			file.seekg(oldPosition, file.beg);
		}
		

	}

	void File::changeFile(std::string& name)
	{
		if (fileName.size() > 0) {
			file.clear();
			file.close();
		}
		
		file.exceptions(ifstream::failbit | ifstream::badbit);

		try {
			file.open(name);
			fileName = name;
		}
		catch (ifstream::failure e) {
			cerr << "faillure to open input file" << endl;
			exit(1);
		}
	}
}
