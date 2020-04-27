#include <Windows.h>
#include <tchar.h>
#include <fltUser.h>
#include "resource.h"
#pragma comment (lib,"advapi32.lib")
#pragma comment (lib,"User32.lib")

#pragma comment(lib,"fltLib.lib")

#define MINISPY_PORT_NAME  L"\\MiniPort"

#define CurrentModuleName L"Rw.dll"
typedef unsigned __int64 _SIZE_T;
typedef unsigned __int64 _HANDLE;
typedef unsigned __int64  VOID64;
TCHAR szFilePath[MAX_PATH] = { 0 };
TCHAR szFileName[MAX_PATH] = { 0 };
HANDLE MiniPort = 0;
enum ControlCode
{
	ReadMemory,
	WriteMemory,
	GetModuleBase,
	ProtectProcess,
	FileDelete
};
typedef struct _ReadInfo
{
	ControlCode Code;
	_SIZE_T Size;
	_HANDLE ProcessId;
	VOID64 RwAddr;
	WCHAR ModuleName[20];
	WCHAR DeleteFilePath[0x100];
}ReadInfo, *lpReadInfo;



bool InstallDriver(LPCWSTR DerverName, LPCWSTR DriverPath);
bool StartDriver(LPCWSTR DerverName);
bool StopDriver(LPCWSTR DerverName);
bool UnloadDriver(LPCWSTR DerverName);
bool ConnectMiniPort()
{
	DWORD Status = FilterConnectCommunicationPort(MINISPY_PORT_NAME, NULL, NULL, NULL, NULL, &MiniPort);
	if (SUCCEEDED(Status))
	{
		return true;
	}
	MiniPort = NULL;
	return false;
}
bool DisConnectMiniPort()
{
	CloseHandle(MiniPort);
}
extern "C" _declspec(dllexport) BOOL MandatoryDeleteFile(WCHAR* FilePath)
{
	bool State;
	ReadInfo Info;
	DWORD dw;
	DWORD RetSize;
	ZeroMemory(&Info, sizeof(Info));
	if (MiniPort == NULL)
	{
		return FALSE;
	}
	Info.Code = FileDelete;
	wcscat(Info.DeleteFilePath, FilePath);
	DWORD Status = FilterSendMessage(MiniPort, &Info, sizeof(ReadInfo), NULL, NULL, &RetSize);
	if (SUCCEEDED(Status))
	{
		return true;
	}
	return false;
}
extern "C" _declspec(dllexport) BOOL ReadMem(HANDLE ProcessId, SIZE_T ReadSize, PVOID ReadAddr, LPVOID Buffer)
{
	bool State;
	ReadInfo Info;
	DWORD dw;
	DWORD RetSize;
	ZeroMemory(&Info, sizeof(Info));
	if (MiniPort == NULL)
	{
		return FALSE;
	}
	Info.Code = ReadMemory;
	Info.ProcessId = (_HANDLE)ProcessId;
	*(DWORD*)&Info.RwAddr = (DWORD)ReadAddr;
	Info.Size = ReadSize;
	DWORD Status = FilterSendMessage(MiniPort, &Info, sizeof(ReadInfo), Buffer, ReadSize, &RetSize);
	if (SUCCEEDED(Status))
	{
		return true;
	}
	return false;
}
extern "C" _declspec(dllexport) BOOL WriteMem(HANDLE ProcessId, SIZE_T WriteSize, PVOID WriteAddr, LPVOID Buffer)
{
	ReadInfo Info;
	DWORD dw;
	DWORD RetSize;
	ZeroMemory(&Info, sizeof(Info));
	if (MiniPort == NULL)
	{
		return FALSE;
	}
	Info.Code = WriteMemory;
	Info.ProcessId = (_HANDLE)ProcessId;
	*(DWORD*)&Info.RwAddr = (DWORD)WriteAddr;
	Info.Size = WriteSize;
	DWORD Status = FilterSendMessage(MiniPort, &Info, sizeof(ReadInfo), Buffer, WriteSize, &RetSize);
	if (SUCCEEDED(Status))
	{
		return true;
	}
	return false;
}


extern "C" _declspec(dllexport) PVOID64 GetModuleEx(DWORD ProcessId, wchar_t *ModuleName)
{
	ReadInfo Info;
	DWORD RetSize;
	ZeroMemory(&Info, sizeof(Info));
	Info.Code = GetModuleBase;
	Info.ProcessId = (int)ProcessId;
	Info.Size = 4;
	wcscpy(Info.ModuleName, ModuleName);
	DWORD dw;
	__int64 ModuleAddr = NULL;
	if (MiniPort == NULL)
	{
		return FALSE;
	}
	DWORD Status = FilterSendMessage(MiniPort, &Info, sizeof(ReadInfo), &ModuleAddr, sizeof(ModuleAddr), &RetSize);

	return (PVOID64)ModuleAddr;
}
extern "C" _declspec(dllexport) bool ProcessProtect()
{
	return true;
}

BOOL GetNtVersionNumbers(DWORD& dwMajorVer, DWORD& dwMinorVer, DWORD& dwBuildNumber)
{
	BOOL bRet = FALSE;
	HMODULE hModNtdll = NULL;
	if (hModNtdll = ::LoadLibraryW(L"ntdll.dll"))
	{
		typedef void (WINAPI *pfRTLGETNTVERSIONNUMBERS)(DWORD*, DWORD*, DWORD*);
		pfRTLGETNTVERSIONNUMBERS pfRtlGetNtVersionNumbers;
		pfRtlGetNtVersionNumbers = (pfRTLGETNTVERSIONNUMBERS)::GetProcAddress(hModNtdll, "RtlGetNtVersionNumbers");
		if (pfRtlGetNtVersionNumbers)
		{
			pfRtlGetNtVersionNumbers(&dwMajorVer, &dwMinorVer, &dwBuildNumber);
			dwBuildNumber &= 0x0ffff;
			bRet = TRUE;
		}

		::FreeLibrary(hModNtdll);
		hModNtdll = NULL;
	}

	return bRet;
}

BOOL CreateSysFile(HMODULE MyModule)
{
	DWORD dwMajorVer, dwMinorVer, dwBuildNumber;

	HRSRC DriverResource;
	HGLOBAL ResourceModule;
	DWORD ResourceSize;
	HANDLE FileHandle;
	DWORD WriteSize;
	DWORD SystemCount = GetTickCount();
	wsprintf(szFileName, L"%d", SystemCount);
	wcscpy(szFileName + wcslen(szFileName), L".sys");
	GetNtVersionNumbers(dwMajorVer, dwMinorVer, dwBuildNumber);
	GetSystemDirectory(szFilePath, MAX_PATH);
	wcscpy_s((wchar_t*)_tcsrchr(szFilePath, '\\') + 1, sizeof(szFileName), szFileName);

	if (dwMajorVer == 10)	//win10系统		
	{
		DriverResource = FindResource(MyModule, MAKEINTRESOURCE(IDR_SYS2), L"SYS");
		ResourceModule = LoadResource(MyModule, DriverResource);
		ResourceSize = SizeofResource(MyModule, DriverResource);
		FileHandle = CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (FileHandle == INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, L"创建文件错误！", L"Caption", MB_OK);
			return FALSE;
		}
		SetFilePointer(FileHandle, NULL, NULL, NULL);
		WriteFile(FileHandle, ResourceModule, ResourceSize, &WriteSize, NULL);
		if (WriteSize != ResourceSize)
		{
			MessageBox(NULL, L"文件写入不完整！", L"Caption", MB_OK);
			return FALSE;
		}
		CloseHandle(FileHandle);
		return TRUE;
	}
	else if (dwMajorVer == 6) //win7或win8系统
	{
		if (dwMinorVer == 1)	//WIN7
		{
			DriverResource = FindResource(MyModule, MAKEINTRESOURCE(IDR_SYS1), L"SYS");
			ResourceModule = LoadResource(MyModule, DriverResource);
			ResourceSize = SizeofResource(MyModule, DriverResource);
			FileHandle = CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (FileHandle == INVALID_HANDLE_VALUE)
			{
				MessageBox(NULL, L"创建文件错误！", L"Caption", MB_OK);
				return FALSE;
			}
			SetFilePointer(FileHandle, NULL, NULL, NULL);
			WriteFile(FileHandle, ResourceModule, ResourceSize, &WriteSize, NULL);
			if (WriteSize != ResourceSize)
			{
				MessageBox(NULL, L"文件写入不完整！", L"Caption", MB_OK);
				return FALSE;
			}
			CloseHandle(FileHandle);
			return TRUE;
		}

	}
	return FALSE;
}
BOOL DeleteSysFile()
{
	DeleteFile(szFilePath);
	return TRUE;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason_for_call, LPVOID lpReserved)
{

	switch (reason_for_call)
	{
	case DLL_PROCESS_ATTACH:					//加载驱动
		if (MiniPort != NULL)
		{
			return TRUE;
		}
		ConnectMiniPort();
		if (MiniPort != NULL)
		{
			return TRUE;
		}
		CreateSysFile(hModule);
		if (!InstallDriver(szFileName, szFilePath))	//安装驱动
		{
			DeleteSysFile();
			return FALSE;
		}
		if (!StartDriver(szFileName))					//启动驱动 
		{
			DeleteSysFile();
			return FALSE;
		}
		ConnectMiniPort();
		if (MiniPort != NULL)
		{
			return TRUE;
		}
		break;
	}

	return FALSE;
}