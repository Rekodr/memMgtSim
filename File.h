#pragma once
#include <fstream>
#include "core.h"
#include <stack>


#define querryLn 32

namespace memProject {
	class File
	{
	public:
		File();
		File::File(std::string& name);
		~File();
		querry next(); // read the next Q
		void rewind();
		void gotTo(); // set the cursor to the previous position
		void changeFile(std::string& name); // change the input file
		std::string getFileName() { return fileName; };
		std::ifstream* getFilePtr() { return &file; };

	private:
		std::ifstream file;
		std::string fileName = "";
		std::stack<std::streampos> positions; // store previous cursor position
	};

}
