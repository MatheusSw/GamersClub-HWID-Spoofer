// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include <windows.h>
#include "targetver.h"
#include <iostream>
#include "detours.h"
#include <json.hpp>
#include "JSONLogger.hpp"

#pragma comment(lib, "detours.lib")

#define ReCa reinterpret_cast
#define DBG	true

#if DBG
#define Log(format, ...)	printf_s("[ %-20s ] ", __FUNCTION__); \
						printf_s( format, __VA_ARGS__); \
						printf_s("\n");
#else
#define Log //
#endif

using DeviceIoControl_t = bool(__stdcall*)(
	HANDLE       hDevice,
	DWORD        dwIoControlCode,
	LPVOID       lpInBuffer,
	DWORD        nInBufferSize,
	LPVOID       lpOutBuffer,
	DWORD        nOutBufferSize,
	LPDWORD      lpBytesReturned,
	LPOVERLAPPED lpOverlapped);
