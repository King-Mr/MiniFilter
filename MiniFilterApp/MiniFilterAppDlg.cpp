
// MiniFilterAppDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MiniFilterApp.h"
#include "MiniFilterAppDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CMiniFilterAppDlg 对话框



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


// CMiniFilterAppDlg 消息处理程序

BOOL CMiniFilterAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMiniFilterAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
	// TODO: 在此添加控件通知处理程序代码
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
	// TODO: 在此添加控件通知处理程序代码
	ReadMem = NULL;
	WriteMem = NULL;
	GetModuleEx = NULL;
	FreeLibrary(Module);
}


void CMiniFilterAppDlg::OnBnClickedButton3()	//Read
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL State;
	DWORD ErrorCode;
	DWORD buffer;
	State = ReadMem((HANDLE)1, 4, (PVOID)4, (PVOID)&buffer);
	if (State == 0)
	{
		ErrorCode = GetLastError();
		WCHAR temp[100];
		wsprintf(temp, L"错误代码：%x", ErrorCode);
		MessageBox(temp, L"Error", MB_OK);
	}
}


void CMiniFilterAppDlg::OnBnClickedButton4()	//GetModule
{
	// TODO: 在此添加控件通知处理程序代码
	GetModuleEx((DWORD)1, L"mfc.dll");

}


void CMiniFilterAppDlg::OnBnClickedButton5()	//Write
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD TEMP;
	WriteMem((HANDLE)1, 4, (PVOID)4, (PVOID)&TEMP);
}
