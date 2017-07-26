
// remotcontrolDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "remotcontrol.h"
#include "remotcontrolDlg.h"
#include "afxdialogex.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include <list>
#pragma comment(lib, "Iphlpapi.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HINSTANCE g_hInstanceDll = NULL;
static HHOOK hhkMouse = NULL;
static HHOOK hhkKeyBoard = NULL;
static	HWND g_hWnd = NULL;

static char buff[30] = {"tcp send test"};
static int threadnum = 0;
static int fresh[20];
using namespace std;
list<SOCKET> socklist;
HANDLE listmutex;
DWORD WINAPI beconproc(LPVOID lparam)
{
	struct my_struct
	{
		SOCKET s;
		SOCKADDR_IN addr;
		SOCKADDR_IN addr_tcp;
		
	}param;
	param = *(my_struct*)lparam;
	while (1)
	{
		sendto(param.s,(char*)&param.addr_tcp,sizeof(SOCKADDR_IN),0,(LPSOCKADDR)&(param.addr),sizeof(SOCKADDR_IN));
		Sleep(200);
	}
}


/*DWORD WINAPI communicatethread(LPVOID lparam)
{
	int i;
	threadnum++;
	if (threadnum == 1)
		i = 1;
	else
	{
		for (i = 1; i < 20; i++)
			if (fresh[i] == 3 || i == threadnum)
			{
				fresh[i] = 1;
				break;
			}

	}


	while (1)
	{
		if (fresh[i] == 1)
			if (SOCKET_ERROR == send(*(SOCKET*)lparam, buff, 28, 0))
				break;
		fresh[i] = 0;

	}
	if (threadnum != 0)
		threadnum--;
	fresh[i] = 3;
	return 0;
}
*/
DWORD WINAPI keyboardsend(LPVOID lparam)
{
	struct Keyboard
	{
		WPARAM wparam;
		KBDLLHOOKSTRUCT code;
		char space[3];
	};
	WaitForSingleObject(listmutex, 300);
	list<SOCKET>::iterator it;
	for (it = socklist.begin(); it != socklist.end(); ++it)
	{
		if (SOCKET_ERROR == send(*it, (char*)lparam, sizeof(Keyboard) + 1, 0))
			socklist.erase(it);
	}
	ReleaseMutex(listmutex);
	return 0;

}

DWORD WINAPI mousesend(LPVOID lparam)
{
	struct Mouse
	{
		WPARAM wparam;
		MSLLHOOKSTRUCT position;

	};

	WaitForSingleObject(listmutex, 300);
	list<SOCKET>::iterator it;
	for (it = socklist.begin(); it != socklist.end(); ++it)
	{
		if (SOCKET_ERROR == send(*it, (char*)lparam, sizeof(Mouse) + 1, 0))
			socklist.erase(it);
	}
	ReleaseMutex(listmutex);
	return 0;

}


DWORD WINAPI listenproc(LPVOID lparam)
{
	SOCKET clientsocket=NULL;
	while (1)
	{
		clientsocket = accept(*(SOCKET*)lparam, NULL, NULL);
		if (clientsocket == INVALID_SOCKET)
		{
			closesocket(clientsocket);
			break;
		}
		//if (!CreateThread(NULL, 0, communicatethread, (LPVOID)&clientsocket, 0, NULL))
			//break;
		WaitForSingleObject(listmutex,300);
		socklist.push_back(clientsocket);
		socklist.unique();
		ReleaseMutex(listmutex);
	}
	WSACleanup();
	return 0;
}

void freshdata()
{
	int i;
	for (i = 0; i < threadnum; i++)
		if (fresh[i] != 3)
			fresh[i] = 1;
}

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


// CremotcontrolDlg �Ի���



CremotcontrolDlg::CremotcontrolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_REMOTCONTROL_DIALOG, pParent)
	, x_position(0)
	, y_position(0)
	, key_value(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CremotcontrolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_X, x_position);
	DDX_Text(pDX, IDC_EDIT_Y, y_position);
	DDX_Text(pDX, IDC_EDIT_VALUE, key_value);
	DDX_Control(pDX, IDC_COMBO1, cb_ipselsect);
}

BEGIN_MESSAGE_MAP(CremotcontrolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_STN_CLICKED(IDC_X, &CremotcontrolDlg::OnStnClickedX)
	ON_BN_CLICKED(IDOK, &CremotcontrolDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CremotcontrolDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CremotcontrolDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CremotcontrolDlg::OnBnClickedButton2)
	ON_MESSAGE(WM_MOUSEMSG, &CremotcontrolDlg::OnMouseMsg)
	ON_MESSAGE(WM_HOOKKEY, &CremotcontrolDlg::OnKEYMsg)
	ON_BN_CLICKED(IDC_BUTTON4, &CremotcontrolDlg::OnBnClickedButton4)
	
	ON_CBN_SELCHANGE(IDC_COMBO1, &CremotcontrolDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CremotcontrolDlg ��Ϣ�������

BOOL CremotcontrolDlg::OnInitDialog()
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
	
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//�õ��ṹ���С,����GetAdaptersInfo����
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//����GetAdaptersInfo����,���pIpAdapterInfoָ�����;����stSize��������һ��������Ҳ��һ�������
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//����������ص���ERROR_BUFFER_OVERFLOW
		//��˵��GetAdaptersInfo�������ݵ��ڴ�ռ䲻��,ͬʱ�䴫��stSize,��ʾ��Ҫ�Ŀռ��С
		//��Ҳ��˵��ΪʲôstSize����һ��������Ҳ��һ�������
		//�ͷ�ԭ�����ڴ�ռ�
		delete pIpAdapterInfo;
		//���������ڴ�ռ������洢����������Ϣ
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//�ٴε���GetAdaptersInfo����,���pIpAdapterInfoָ�����
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}
	if (ERROR_SUCCESS == nRel)
	{
		//���������Ϣ
		while (pIpAdapterInfo)
		{
			
			
			IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
			do
			{
				int num= MultiByteToWideChar(0, 0, pIpAddrString->IpAddress.String, -1, NULL, 0);
				wchar_t *wide = new wchar_t[num];
				MultiByteToWideChar(0, 0, pIpAddrString->IpAddress.String, -1, wide, num);
				cb_ipselsect.AddString((LPCTSTR)wide);
				//cout << pIpAddrString->IpAddress.String << endl;
				pIpAddrString = pIpAddrString->Next;
			} while (pIpAddrString);
			pIpAdapterInfo = pIpAdapterInfo->Next;
			//cout << "*****************************************************" << endl;
		}


	}
	//�ͷ��ڴ�ռ�
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CremotcontrolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CremotcontrolDlg::OnPaint()
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
HCURSOR CremotcontrolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CremotcontrolDlg::OnStnClickedX()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (hhkMouse != NULL)
	{
		::UnhookWindowsHookEx(hhkMouse);

	}

	if (hhkKeyBoard != NULL)
	{
		::UnhookWindowsHookEx(hhkKeyBoard);

	}
	closesocket(s);
	WSACleanup();

}


void CremotcontrolDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnOK();

}


void CremotcontrolDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnCancel();
	if (hhkMouse != NULL)
	{
		::UnhookWindowsHookEx(hhkMouse);

	}

	if (hhkKeyBoard != NULL)
	{
		::UnhookWindowsHookEx(hhkKeyBoard);

	}
	closesocket(s);
	WSACleanup();

}



LRESULT CALLBACK lowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (g_hWnd != NULL&&nCode == HC_ACTION)
	{
		::SendMessage(g_hWnd, WM_MOUSEMSG, wParam, lParam);
	}
	return CallNextHookEx(hhkMouse, nCode, wParam, lParam);
}

LRESULT CALLBACK lowLevelKeyProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (g_hWnd != NULL&&nCode == HC_ACTION)
	{
		::SendMessage(g_hWnd, WM_HOOKKEY, wParam, lParam);
	}
	return CallNextHookEx(hhkKeyBoard, nCode, wParam, lParam);
}


void CremotcontrolDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	/*g_hInstanceDll = LoadLibrary(_T("hooklearn.dll"));
	if (NULL == g_hInstanceDll)
	{
		AfxMessageBox(_T("����hooklearn.dllʧ��"));
		return;
	}
	typedef BOOL(CALLBACK *StartHookMouse)(HWND hwnd);
	StartHookMouse startHook;
	startHook = (StartHookMouse) ::GetProcAddress(g_hInstanceDll, "StartHookMouse");
	if (NULL == startHook)
	{
		AfxMessageBox(_T("��ȡ StartHookMouse ����ʧ��"));
		return;
	}
	if (startHook(this->m_hWnd))
	{
		AfxMessageBox(_T("������깳�ӳɹ�"));
	}
	else
	{
		AfxMessageBox(_T("������깳��ʧ��"));
	}*/
	g_hWnd = m_hWnd;
	hhkMouse = SetWindowsHookEx(WH_MOUSE_LL, lowLevelMouseProc, NULL, 0);
	if (hhkMouse == NULL)
	{
		AfxMessageBox(_T("������깳��ʧ��"));
	}
	hhkKeyBoard = SetWindowsHookEx(WH_KEYBOARD_LL, lowLevelKeyProc, GetModuleHandle(NULL), 0);
	if (hhkKeyBoard == NULL)
	{
		AfxMessageBox(_T("�������̹���ʧ��"));
	}

	
}


void CremotcontrolDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	/*typedef VOID(CALLBACK *StopHookMouse)();
	StopHookMouse stopHook;
	g_hInstanceDll = LoadLibrary(_T("hooklearn.dll"));
	if (g_hInstanceDll == NULL)
	{
		AfxMessageBox(_T("����DLLʧ��"));
		return;
	}

	stopHook = (StopHookMouse) ::GetProcAddress(g_hInstanceDll, "StopHookMouse");
	if (stopHook == NULL)
	{
		AfxMessageBox(_T("�ر���깳��ʧ��"));
		return;
	}
	else
	{
		stopHook();
		AfxMessageBox(_T("�ر���깳�ӳɹ�"));
	}

	if (g_hInstanceDll != NULL)
	{
		::FreeLibrary(g_hInstanceDll);
	}*/
	if (hhkMouse != NULL)
	{
		::UnhookWindowsHookEx(hhkMouse);

	}

	if (hhkKeyBoard != NULL)
	{
		::UnhookWindowsHookEx(hhkKeyBoard);

	}

}

LRESULT CremotcontrolDlg::OnMouseMsg(WPARAM wParam, LPARAM lParam)
{
	
	UpdateData(FALSE);
	static PMSLLHOOKSTRUCT mouseHookStruct;
	static PKBDLLHOOKSTRUCT keyboardHookStruct;
	x_position = wParam;
	//if ((wParam == WM_LBUTTONDOWN) || (wParam == WM_LBUTTONUP) || (wParam == WM_MOUSEMOVE) || (wParam == WM_MOUSEWHEEL) || (wParam == WM_MOUSEHWHEEL) || (wParam == WM_RBUTTONDOWN) || (wParam == WM_RBUTTONUP))
	//{
		y_position = ((PMSLLHOOKSTRUCT)lParam)->pt.y;
		x_position = ((PMSLLHOOKSTRUCT)lParam)->pt.x;
		mouse_data.wparam = wParam;
		mouse_data.position = *(PMSLLHOOKSTRUCT)lParam;
		//if(SOCKET_ERROR==sendto(s, (char*)&mouse_data, sizeof(mouse_data)+1, 0, (LPSOCKADDR)&addr_to, sizeof(addr_to)))
		//closesocket(s);
		CreateThread(NULL, 0, mousesend, (LPVOID)&mouse_data, 0, NULL);
	//} 
		
	return 1;
}

LRESULT CremotcontrolDlg::OnKEYMsg(WPARAM wParam, LPARAM lParam)
{	    UpdateData(FALSE);
	//if ((wParam == WM_KEYDOWN) || (wParam == WM_KEYUP) || (wParam == WM_SYSKEYDOWN) || (wParam == WM_SYSKEYUP))
	//{
		x_position = wParam;
		y_position = 0;
		y_position = ((PKBDLLHOOKSTRUCT)lParam)->vkCode;
		keyboard_data.wparam = wParam;
		keyboard_data.code = *(PKBDLLHOOKSTRUCT)lParam;
	//if (SOCKET_ERROR == sendto(s, (char*)&keyboard_data, sizeof(keyboard_data) + 1, 0, (LPSOCKADDR)&addr_to, sizeof(addr_to)))
		//	closesocket(s);
	//}
		CreateThread(NULL, 0, keyboardsend, (LPVOID)&keyboard_data, 0, NULL);

	return 1;
}

void CremotcontrolDlg::OnBnClickedButton4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	WSAStartup(MAKEWORD(2, 2), &wsad);
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	listensocket= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&addr_to, 0, sizeof(addr_to));
	addr_to.sin_family = AF_INET;
	addr_to.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);
	addr_to.sin_port = htons(4677);

	bBoardcast = TRUE;
	if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&bBoardcast, sizeof(bBoardcast)))
	{
		AfxMessageBox(_T("socket error"));
		if (INVALID_SOCKET != s)
		{
			closesocket(s);
			s = INVALID_SOCKET;
		}
		WSACleanup();
	}
	
	//if (SOCKET_ERROR == sendto(s, "123", 4, 0, (LPSOCKADDR)&addr_to, sizeof(addr_to)))
	//	closesocket(s);
	
	
	int nindex = cb_ipselsect.GetCurSel();
	CString ip=0;
	cb_ipselsect.GetLBText(nindex,ip);
	char *cip;
	//wcstombs(cip, ip, ip.GetLength() * 2);
	

	memset(&addr_tcp, 0, sizeof(addr_tcp));
	addr_tcp.sin_family = AF_INET;
	InetPton(AF_INET,ip, &addr_tcp.sin_addr);
	//addr_tcp.sin_addr.S_un.S_addr = htonl(inet_addr(cip));
	addr_tcp.sin_port = htons(5677);

	if (SOCKET_ERROR == bind(listensocket, (LPSOCKADDR)&addr_tcp, sizeof(addr_tcp)))
	{
		AfxMessageBox(_T("socket error"));
		closesocket(listensocket);

	}
	if (SOCKET_ERROR == listen(listensocket, SOMAXCONN))
	{
		AfxMessageBox(_T("socket error"));
		closesocket(listensocket);
	}
	CreateThread(NULL, 0, listenproc, (LPVOID)&listensocket, 0, NULL);

struct 
	{
		SOCKET s;
		SOCKADDR_IN addr;
		SOCKADDR_IN addr_tcp;
	}param;
	param.s = s;
	param.addr = addr_to;
	param.addr_tcp = addr_tcp;
	CreateThread(NULL, 0, beconproc, (LPVOID)&param, 0, NULL);
	listmutex = CreateMutex(NULL,FALSE,NULL);


}




void CremotcontrolDlg::OnCbnSelchangeCombo1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}
