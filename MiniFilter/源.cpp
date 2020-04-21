
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
	//MiniFilter������ҪyongInf�ļ���װ��Inf�ļ������¾�����ע�����д��һЩ����
	//����Ҫʵ�ֶ�̬���أ������Լ�������һ����������ע���д�����ݣ������Ϳ���ʵ�ֶ�̬���ء�
	OBJECT_ATTRIBUTES ObjectAttr = { 0 };
	HANDLE Key = NULL;
	ULONG DisPosition = 0;
	NTSTATUS Status = -1;
	wchar_t Myname[100] = { 0 };
	wchar_t NewItem[100] = { 0 };
	//Path��������ע����·����ͨ����ȡ���һ����\��������ַ���ȷ����������
	wcscat(Myname, _tcsrchr(Path->Buffer, '\\') + 1);	
	wcscat(NewItem, Myname);
	wcscat(NewItem, L" Instance");
	//��Path���ַ�������������µ�������֣��������MiniFilter��Ҫ�ģ�������û�еģ�Ҫ�½�
	//���ַ�������������µ�������֮��Ҫ����һ��unicode�ĳ��ȣ����򴴽����ʱ��������
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
	//wcslen�����βǰ���ַ���������Ϊ������ȵĵ�λ���ֽڣ�wcslen��õ����ַ�����
	//����Ҫ*2,����wcslen��õ��ǽ�β0֮ǰ���ַ�����������Ҫ+2�����ﴦ����ע���������ʱ��᷵��c0000034
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
		50);            //�޸Ŀͻ������������

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