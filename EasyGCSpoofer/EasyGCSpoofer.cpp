#include "header.h"

void SpoofSerial(char* serial, bool is_smart);
unsigned long g_startup_time; //Extern declaration

JSONLogger * jsLogger = new JSONLogger();

bool second_time = false;
DeviceIoControl_t o_device_ioctl = { 0 };

void change_serial(char* p_serial) {
	jsLogger->loggers->device_ioctl["Serial Number"]["Before"] = p_serial;

	DWORD oldprot = 0;
	VirtualProtect(p_serial, sizeof(char[20]), PAGE_EXECUTE_READWRITE, &oldprot);
	SpoofSerial(p_serial, false);
	VirtualProtect(p_serial, sizeof(char[20]), oldprot, 0);

	jsLogger->loggers->device_ioctl["Serial Number"]["After"] = p_serial;
	return;
}

char* get_current_serial(PSTORAGE_DEVICE_DESCRIPTOR p_device_descriptor) {
	DWORD serial_number_offset = p_device_descriptor->SerialNumberOffset;
	auto serial = ReCa<char*>(ReCa<DWORD>(p_device_descriptor) + serial_number_offset);
	return serial;
}

void log_device_descriptor(PSTORAGE_DEVICE_DESCRIPTOR p_device_descriptor) {
	jsLogger->loggers->device_ioctl["Device Descriptor"]["Version"] += p_device_descriptor->Version;
	jsLogger->loggers->device_ioctl["Device Descriptor"]["Size"] += p_device_descriptor->Size;
	jsLogger->loggers->device_ioctl["Device Descriptor"]["DeviceType"] += p_device_descriptor->DeviceType;
	jsLogger->loggers->device_ioctl["Device Descriptor"]["VendorID"] += p_device_descriptor->VendorIdOffset;
	jsLogger->loggers->device_ioctl["Device Descriptor"]["ProductID"] += p_device_descriptor->ProductIdOffset;
	jsLogger->loggers->device_ioctl["Device Descriptor"]["Serial Offset"] += p_device_descriptor->SerialNumberOffset;
	jsLogger->loggers->device_ioctl["Device Descriptor"]["Raw Properties Length"] += p_device_descriptor->RawPropertiesLength;
	return;
}

bool __stdcall hk_device_ioctl(
	HANDLE       hDevice,
	DWORD        dwIoControlCode,
	LPVOID       lpInBuffer,
	DWORD        nInBufferSize,
	LPVOID       lpOutBuffer, //PSTORAGE_DEVICE_DESCRIPTOR
	DWORD        nOutBufferSize,
	LPDWORD      lpBytesReturned,
	LPOVERLAPPED lpOverlapped)
{
	jsLogger->loggers->device_ioctl["Hit Count"] = jsLogger->device_ioctl_hits++;
	if (!hDevice || !(dwIoControlCode == IOCTL_STORAGE_QUERY_PROPERTY || dwIoControlCode == SMART_RCV_DRIVE_DATA) || !second_time) {
		second_time = true;
		return o_device_ioctl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
	}

	//let them fill the struct (lpOutBuffer)
	o_device_ioctl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);

	//Now it's our time to shine
	//lpOutBuffer is now the filled PSTORAGE_DEVICE_DESCRIPTOR struct, containing our serial
	PSTORAGE_DEVICE_DESCRIPTOR p_device_descriptor = ReCa<PSTORAGE_DEVICE_DESCRIPTOR>(lpOutBuffer);

	log_device_descriptor(p_device_descriptor);

	change_serial(get_current_serial(p_device_descriptor));
	return true;
}

void initialize_misc() noexcept {
	FILETIME time = { 0 };
	GetSystemTimeAsFileTime(&time);
	g_startup_time = time.dwLowDateTime;

	AllocConsole();
	FILE* f = { nullptr };
	freopen_s(&f, "CONOUT$", "w", stdout);
}

void initiate_hook() {
	auto device_ioctl_address = GetProcAddress(LoadLibrary(L"kernel32.dll"), "DeviceIoControl");
	Log("Found DeviceIOControl at address [ %p ]", ReCa<void*>(device_ioctl_address));

	Log("Detouring...");
	o_device_ioctl = ReCa<DeviceIoControl_t>(DetourFunction((PBYTE)device_ioctl_address, ReCa<PBYTE>(hk_device_ioctl)));
	Log("Detoured with %p", hk_device_ioctl);
}

void unhook_and_exit(HINSTANCE dll_handle) {
	Log("Removing Detour and exiting");
	DetourRemove(ReCa<PBYTE>(o_device_ioctl), ReCa<PBYTE>(hk_device_ioctl));

	Log("Exiting");
	jsLogger->write_all_logs();
	FreeConsole();
	FreeLibraryAndExitThread(dll_handle, 1);
}

bool Main(HINSTANCE dll_handle) {
	initialize_misc();
	Log("Initialized");

	initiate_hook();
	Log("Press F1 to Quit and Save logs");

	while (!(GetAsyncKeyState(VK_F1) & 1)) {
		Sleep(100);
	}

	unhook_and_exit(dll_handle);
}

bool __stdcall DllMain(
	HINSTANCE hinstDLL,
	DWORD fdwReason,
	LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hinstDLL, 0, 0);
		break;
	case DLL_PROCESS_DETACH:
		delete jsLogger;
		break;
	default:
		delete jsLogger;
		break;
	}
	return TRUE;
}