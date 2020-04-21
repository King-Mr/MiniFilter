
#include <fltKernel.h>
#include <tchar.h>
#pragma comment(lib,"FltMgr.lib")

#define MINISPY_PORT_NAME  L"\\MiniPort"
PFLT_PORT g_ServerPort = NULL;
PVOID g_FilterHandle = NULL;
enum ControlCode
{
	ReadMemory,
	WriteMemory,
	GetModuleBase,
	ProtectProcess
};
typedef struct _ReadInfo
{
	ControlCode Code;
	SIZE_T Size;
	HANDLE ProcessId;
	PVOID RwAddr;
	WCHAR ModuleName[20];
}ReadInfo, *lpReadInfo;
NTSTATUS
FLTAPI FDMiniConnect(
	_In_ PFLT_PORT ClientPort,
	_In_opt_ PVOID ServerPortCookie,
	_In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID *ConnectionPortCookie
)
{
	DbgPrint("FDMiniConnect Execution\n");
	return 0;
}

VOID
FLTAPI FDMiniDisconnect(
	_In_opt_ PVOID ConnectionCookie
)
{
	DbgPrint("FDMiniDisconnect Execution\n");
}
NTSTATUS
FLTAPI FDMiniMessage(
	_In_opt_ PVOID PortCookie,
	_In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
	_In_ ULONG OutputBufferLength,
	_Out_ PULONG ReturnOutputBufferLength
)
{
	lpReadInfo Info = (lpReadInfo)InputBuffer;
	DbgPrint("Control Code:%x,Size:%x,ProcessId:%x,RwAddr:%x,ModuleName:%ws\n", Info->Code, Info->Size, Info->ProcessId, Info->RwAddr, Info->ModuleName);
	DbgPrint("OutBuffer Point:%p\n", OutputBuffer);
	DbgPrint("FDMiniMessage Execution\n");
	return 0;
}
NTSTATUS Unload(PDRIVER_OBJECT driver)
{
	if (NULL != g_ServerPort) {
		FltCloseCommunicationPort(g_ServerPort);
	}
	if (NULL != g_FilterHandle) {
		FltUnregisterFilter((PFLT_FILTER)g_FilterHandle);
	}
	return 0;
}
NTSTATUS DynamicLoadMiniFilter(PUNICODE_STRING Path)
{
	//MiniFilter驱动需要yongInf文件安装，Inf文件做的事就是在注册表里写入一些数据
	//所以要实现动态加载，我们自己定义了一个函数来像注册表写入数据，这样就可以实现动态加载。
	OBJECT_ATTRIBUTES ObjectAttr = { 0 };
	HANDLE Key = NULL;
	ULONG DisPosition = 0;
	NTSTATUS Status = -1;
	wchar_t Myname[100] = { 0 };
	wchar_t NewItem[100] = { 0 };
	//Path是驱动在注册表的路径，通过获取最后一个“\”后面的字符来确定驱动名称
	wcscat(Myname, _tcsrchr(Path->Buffer, '\\') + 1);	
	wcscat(NewItem, Myname);
	wcscat(NewItem, L" Instance");
	//在Path的字符串后面加上了新的项的名字，这个项是MiniFilter需要的，正常是没有的，要新建
	//在字符串后面加上了新的项名字之后，要更新一下unicode的长度，否则创建项的时候会出问题
	wcscat(Path->Buffer, L"\\Instances");			
													
	RtlInitUnicodeString(Path, Path->Buffer);
	InitializeObjectAttributes(&ObjectAttr, Path, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
	Status = ZwCreateKey(&Key, KEY_ALL_ACCESS, &ObjectAttr, NULL, NULL, REG_OPTION_NON_VOLATILE, &DisPosition);
	if (!NT_SUCCESS(Status) || Key == NULL)
	{
		DbgPrint("ZwCreateKey Error!\n");
		return -1;
	}
	UNICODE_STRING ValueName;
	RtlInitUnicodeString(&ValueName, L"DefaultInstance");
	//wcslen计算结尾前的字符个数，因为这个长度的单位是字节，wcslen获得的是字符个数
	//所以要*2,并且wcslen获得的是结尾0之前的字符个数，所以要+2，这里处理不好注册过滤器的时候会返回c0000034
	Status = ZwSetValueKey(Key, &ValueName, NULL, REG_SZ, NewItem, wcslen(NewItem)*2+2);
	if (!NT_SUCCESS(Status))
	{
		DbgPrint("ZwSetValueKey Error! Error Code:%x\n", Status);
	}
	ZwClose(Key);
	wcscat(Path->Buffer, L"\\");
	wcscat(Path->Buffer, NewItem);
	RtlInitUnicodeString(Path, Path->Buffer);
	InitializeObjectAttributes(&ObjectAttr, Path, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
	Status = ZwCreateKey(&Key, KEY_ALL_ACCESS, &ObjectAttr, NULL, NULL, REG_OPTION_NON_VOLATILE, &DisPosition);
	if (!NT_SUCCESS(Status) || Key == NULL)
	{
		DbgPrint("ZwCreateKey Error!\n");
		return -1;
	}
	RtlInitUnicodeString(&ValueName, L"Altitude");
	Status = ZwSetValueKey(Key, &ValueName, NULL, REG_SZ, L"370030", sizeof(L"370030"));
	if (!NT_SUCCESS(Status))
	{
		DbgPrint("ZwSetValueKey Error! Error Code:%x\n", Status);
	}
	RtlInitUnicodeString(&ValueName, L"Flags");
	DWORD32 Altitude = 0;
	Status = ZwSetValueKey(Key, &ValueName, NULL, REG_DWORD, &Altitude, 4);
	if (!NT_SUCCESS(Status))
	{
		DbgPrint("ZwSetValueKey Error! Error Code:%x\n", Status);
	}
	ZwClose(Key);
	return Status;
}
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING Path)
{
	NTSTATUS status;
	UNICODE_STRING uniString;
	PSECURITY_DESCRIPTOR sd;
	OBJECT_ATTRIBUTES oa;

	FLT_REGISTRATION FilterRegistration = { 0 };

	DynamicLoadMiniFilter(Path);



	UNREFERENCED_PARAMETER(Path);

	FilterRegistration.Version = FLT_REGISTRATION_VERSION;

	status = FltRegisterFilter(driver, &FilterRegistration, (PFLT_FILTER*)&g_FilterHandle);
	if (!NT_SUCCESS(status))
	{
		return -1;
	}
	status = FltStartFiltering((PFLT_FILTER)g_FilterHandle);
	if (!NT_SUCCESS(status))
	{
		return -1;
	}

	status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);
	if (!NT_SUCCESS(status))
	{
		return -1;
	}
	RtlInitUnicodeString(&uniString, MINISPY_PORT_NAME);

	InitializeObjectAttributes(&oa,
		&uniString,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL,
		sd);
	status = FltCreateCommunicationPort((PFLT_FILTER)g_FilterHandle,
		&g_ServerPort,
		&oa,
		NULL,
		FDMiniConnect,
		FDMiniDisconnect,
		FDMiniMessage,
		50);            //修改客户端最大连接数

	if (!NT_SUCCESS(status))
	{

		if (NULL != g_ServerPort) {
			FltCloseCommunicationPort(g_ServerPort);
		}

		if (NULL != g_FilterHandle) {
			FltUnregisterFilter((PFLT_FILTER)g_FilterHandle);
		}
	}
	driver->DriverUnload = (PDRIVER_UNLOAD)Unload;
	return status;
}