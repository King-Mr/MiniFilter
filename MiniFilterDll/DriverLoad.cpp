#include <windows.h>
bool InstallDriver(LPCWSTR DerverName, LPCWSTR DriverPath)
{
	bool ReturnValue = 0;
	SC_HANDLE schSCManager;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager)
	{
		SC_HANDLE schService = CreateService(schSCManager, DerverName, DerverName, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, DriverPath, NULL, NULL, NULL, NULL, NULL);
		GetLastError();
		CloseServiceHandle(schService);
		ReturnValue = 1;
	}
	return ReturnValue;
}
bool StartDriver(LPCWSTR DerverName)
{
	bool ReturnValue = 0;
	SC_HANDLE schSCManager;
	SC_HANDLE hs;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager)
	{
		hs = OpenService(schSCManager, DerverName, SERVICE_ALL_ACCESS);
		GetLastError();
		if (hs)
		{
			if (StartService(hs, 0, 0))
			{
				//printf("��������ɹ�\n");
				ReturnValue = 1;
			}
			GetLastError();
			CloseServiceHandle(hs);
		}
		CloseServiceHandle(schSCManager);
	}
	return ReturnValue;
}

bool StopDriver(LPCWSTR DerverName)//ֹͣ
{
	bool ReturnValue = 0;
	SC_HANDLE schSCManager;
	SC_HANDLE hs;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager)
	{
		hs = OpenService(schSCManager, DerverName, SERVICE_ALL_ACCESS); //�򿪷���
		if (hs)
		{
			SERVICE_STATUS status;
			int num = 0;

			QueryServiceStatus(hs, &status);
			if (status.dwCurrentState != SERVICE_STOPPED &&

				status.dwCurrentState != SERVICE_STOP_PENDING)
			{
				ControlService(hs, SERVICE_CONTROL_STOP, &status);
				do
				{
					Sleep(50);
					num++;
					QueryServiceStatus(hs, &status);
				} while (status.dwCurrentState != SERVICE_STOPPED ||

					num > 80);
			}

			if (num > 80)
			{
				//printf("ֹͣ����ʧ��\n");
			}
			else
			{
				ReturnValue = 1;
				//printf("ֹͣ����ɹ�\n");
			}
			CloseServiceHandle(hs);
		}
		CloseServiceHandle(schSCManager);
	}
	return ReturnValue;
}
bool UnloadDriver(LPCWSTR DerverName)//ж��
{
	bool ReturnValue = 0;
	SC_HANDLE schSCManager;
	SC_HANDLE hs;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager)
	{
		hs = OpenService(schSCManager, DerverName, SERVICE_ALL_ACCESS); //�򿪷���
		if (hs)
		{
			bool a = DeleteService(hs);   //ɾ������
			if (!a)
			{
				//printf("ɾ������ʧ��\n");
			}
			else
			{
				//	printf("��ɾ������\n");
				ReturnValue = 1;
			}

			CloseServiceHandle(hs);//�ͷ����������ɴӷ��������ʧ �ͷ�ǰ��

		}
		CloseServiceHandle(schSCManager);
	}
	return ReturnValue;
}