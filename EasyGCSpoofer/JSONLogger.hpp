#pragma once
#include "header.h"
#include <fstream>
#include <iomanip>

using json = nlohmann::json;

class JSONLogger
{
	void write_device_ioctl_logs();
public:
	int device_ioctl_hits = 1;

	//Loggers, each one get its own file
	static struct s_loggers
	{
		json device_ioctl;
	} *loggers;

	void write_all_logs();
	JSONLogger();
	~JSONLogger();
};

