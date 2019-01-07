#include "JSONLogger.hpp"

void JSONLogger::WriteLogs()
{
	std::ofstream ioctl_log("IOCTLs.json", std::ofstream::trunc);
	ioctl_log << std::setw(4) << logDeviceIoControl << std::endl;
}

JSONLogger::JSONLogger()
{
}


JSONLogger::~JSONLogger()
{
}
