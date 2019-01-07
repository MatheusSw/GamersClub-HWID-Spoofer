#pragma once
#include "header.h"
#include <fstream>
#include <iomanip>

using json = nlohmann::json;

class JSONLogger
{
public:
	//Loggers, each one gets its own file
	json logDeviceIoControl;

	void WriteLogs();
	JSONLogger();
	~JSONLogger();
};

