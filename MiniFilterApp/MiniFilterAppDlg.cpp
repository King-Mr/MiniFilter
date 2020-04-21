
// MiniFilterAppDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MiniFilterApp.h"
#include "MiniFilterAppDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMiniFilterAppDlg �Ի���



CMiniFilterAppDlg::CMiniFilterAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MINIFILTERAPP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMiniFilterAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMiniFilterAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMiniFilterAppDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMiniFilterAppDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMiniFilterAppDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CMiniFilterAppDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CMiniFilterAppDlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CMiniFilterAppDlg ��Ϣ�������

BOOL CMiniFilterAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMiniFilterAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMiniFilterAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMiniFilterAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL(*ReadMem)(HANDLE ProcessId, SIZE_T ReadSize, PVOID ReadAddr, LPVOID Buffer);
BOOL(*WriteMem)(HANDLE ProcessId, SIZE_T WriteSize, PVOID WriteAddr, LPVOID Buffer);
PVOID64(*GetModuleEx)(DWORD ProcessId, wchar_t *ModuleName);
HMODULE Module = NULL;
void CMiniFilterAppDlg::OnBnClickedButton1()	//Load
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	Module = LoadLibrary(L"MiniFilterDll.dll");
	if (Module == NULL)
	{
		::MessageBox(NULL, L"Load Module Error!", L"Error!", MB_OK);
	}
	ReadMem = (BOOL(*)(HANDLE, SIZE_T, PVOID, LPVOID))GetProcAddress(Module, "ReadMem");
	WriteMem = (BOOL(*)(HANDLE, SIZE_T, PVOID, LPVOID))GetProcAddress(Module, "WriteMem");
	GetModuleEx = (PVOID64(*)(DWORD, wchar_t*))GetProcAddress(Module, "GetModuleEx");
}


void CMiniFilterAppDlg::OnBnClickedButton2()	//Unload
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ReadMem = NULL;
	WriteMem = NULL;
	GetModuleEx = NULL;
	FreeLibrary(Module);
}


void CMiniFilterAppDlg::OnBnClickedButton3()	//Read
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	BOOL State;
	DWORD ErrorCode;
	DWORD buffer;
	State = ReadMem((HANDLE)1, 4, (PVOID)4, (PVOID)&buffer);
	if (State == 0)
	{
		ErrorCode = GetLastError();
		WCHAR temp[100];
		wsprintf(temp, L"������룺%x", ErrorCode);
		MessageBox(temp, L"Error", MB_OK);
	}
}


void CMiniFilterAppDlg::OnBnClickedButton4()	//GetModule
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetModuleEx((DWORD)1, L"mfc.dll");

}


void CMiniFilterAppDlg::OnBnClickedButton5()	//Write
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD TEMP;
	WriteMem((HANDLE)1, 4, (PVOID)4, (PVOID)&TEMP);
}
