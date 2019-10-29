#include "JSONLogger.hpp"

void JSONLogger::write_device_ioctl_logs() {
	std::ofstream ioctl_log("DeviceIOCTL.json", std::ofstream::trunc);
	ioctl_log << std::setw(4) << (this->loggers->device_ioctl) << std::endl;
}

//If a new log is implemented don't forget to add it here
void JSONLogger::write_all_logs()
{
	write_device_ioctl_logs();
}

JSONLogger::JSONLogger()
{
}


JSONLogger::~JSONLogger()
{
}
