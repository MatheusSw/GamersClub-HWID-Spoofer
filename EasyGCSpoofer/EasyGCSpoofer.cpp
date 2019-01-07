#include "header.h"

void SpoofSerial(char* serial, bool is_smart);
unsigned long g_startup_time; //Extern declaration

JSONLogger * jsLogger = new JSONLogger();
int runs = 1;
int secondrun = 1;

using DeviceIoControl_t = bool(__stdcall *)(
	HANDLE       hDevice,
	DWORD        dwIoControlCode,
	LPVOID       lpInBuffer,
	DWORD        nInBufferSize,
	LPVOID       lpOutBuffer,
	DWORD        nOutBufferSize,
	LPDWORD      lpBytesReturned,
	LPOVERLAPPED lpOverlapped);

DeviceIoControl_t originalDeviceIo = { NULL };

bool __stdcall hookedDeviceIOControl(
	HANDLE       hDevice,
	DWORD        dwIoControlCode,
	LPVOID       lpInBuffer,
	DWORD        nInBufferSize,
	LPVOID       lpOutBuffer,
	DWORD        nOutBufferSize,
	LPDWORD      lpBytesReturned,
	LPOVERLAPPED lpOverlapped)
{
	jsLogger->logDeviceIoControl["Hit Count"] = runs++;
	if (!hDevice || !(dwIoControlCode == IOCTL_STORAGE_QUERY_PROPERTY || dwIoControlCode == SMART_RCV_DRIVE_DATA) || secondrun++ == 1)
		return originalDeviceIo(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
	jsLogger->logDeviceIoControl["IoControlCode"] += (dwIoControlCode == IOCTL_STORAGE_QUERY_PROPERTY ? "IOCTL_STORAGE_QUERY_PROPERTY" : "SMART_RCV_DRIVE_DATA");

	originalDeviceIo(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped); //Let them fill the struct

	STORAGE_DEVICE_DESCRIPTOR * pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)lpOutBuffer; //Get the struct
	jsLogger->logDeviceIoControl["Device Descriptor"]["Version"] += pDeviceDescriptor->Version;
	jsLogger->logDeviceIoControl["Device Descriptor"]["Size"] += pDeviceDescriptor->Size;
	jsLogger->logDeviceIoControl["Device Descriptor"]["DeviceType"] += pDeviceDescriptor->DeviceType;
	jsLogger->logDeviceIoControl["Device Descriptor"]["VendorID"] += pDeviceDescriptor->VendorIdOffset;
	jsLogger->logDeviceIoControl["Device Descriptor"]["ProductID"] += pDeviceDescriptor->ProductIdOffset;
	jsLogger->logDeviceIoControl["Device Descriptor"]["Serial Offset"] += pDeviceDescriptor->SerialNumberOffset;
	jsLogger->logDeviceIoControl["Device Descriptor"]["Raw Properties Length"] += pDeviceDescriptor->RawPropertiesLength;

	auto serial = (char*)((DWORD)lpOutBuffer + pDeviceDescriptor->SerialNumberOffset);
	Log("Changing Serial");

	jsLogger->logDeviceIoControl["Serial Numbers"]["Before"] = serial;
	DWORD oldprot = 0;
	VirtualProtect(serial, sizeof(char[20]), PAGE_EXECUTE_READWRITE, &oldprot);
	SpoofSerial(serial, false);
	VirtualProtect(serial, sizeof(char[20]), oldprot, 0);

	jsLogger->logDeviceIoControl["Serial Numbers"]["After"] = serial;
	Log("Changed");

	return true;
}

void InitializeMisc() noexcept {
	FILETIME time = { 0 };
	GetSystemTimeAsFileTime(&time);
	g_startup_time = time.dwLowDateTime;

	AllocConsole();
	FILE * f = { nullptr };
	freopen_s(&f, "CONOUT$", "w", stdout);
}

bool Main(LPVOID param) {
	InitializeMisc();
	Log("Initialized");

	auto deviceIoControlAddy = GetProcAddress(LoadLibrary(L"kernel32.dll"), "DeviceIoControl");
	Log("Found DeviceIOControl at address [ %p ]", ReCa<void*>(deviceIoControlAddy));
	Log("Detouring...");
	originalDeviceIo = ReCa<DeviceIoControl_t>(DetourFunction((PBYTE)deviceIoControlAddy, ReCa<PBYTE>(hookedDeviceIOControl)));
	Log("Detoured with %p", hookedDeviceIOControl);
	Log("Press F1 to Quit and save logs");
	while (!(GetAsyncKeyState(VK_F1) & 1)) {
		Sleep(100);
	}
	jsLogger->WriteLogs();
	Log("Removing Detour and exiting");
	DetourRemove((PBYTE)originalDeviceIo, ReCa<PBYTE>(hookedDeviceIOControl));
	Log("Detour Removed");
	Log("Exiting");

	FreeConsole();
	FreeLibraryAndExitThread(ReCa<HMODULE>(param), 1);
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
		break;
	}
	return TRUE;
}