#pragma once
#include <fstream>
#include "core.h"

#define querryLn 32

class File
{
public:
	File();
	File::File(std::string& name);
	~File();
	querry next(); 

private:
	std::ifstream file;

};

