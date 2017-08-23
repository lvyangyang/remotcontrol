
// remotcontrolDlg.cpp : 实现文件
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

//---------------udp--becon---------------------
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
	
		//sendto(param.s, buff, 30, 0, (LPSOCKADDR)&(param.addr), sizeof(SOCKADDR_IN));
		sendto(param.s,(char*)&param.addr_tcp,sizeof(SOCKADDR_IN),0,(LPSOCKADDR)&(param.addr),sizeof(SOCKADDR_IN));
		Sleep(200);
		//AfxMessageBox(_T("becon"));
	}
	return 0;
}



DWORD WINAPI screenshow(LPVOID lparam)
{

	

	SOCKET sendsocket = *(SOCKET *)lparam;
//----------设置长  宽数据---------------
	char *screeninfo_bufer;
	screeninfo_bufer =(char *) malloc(4);
	if (recv(sendsocket, screeninfo_bufer, 4, 0) == SOCKET_ERROR)
		return 3;
		
	struct l_w
	{
		uint16_t width;
		uint16_t height;
	}screeninfo;
	screeninfo = *(l_w *)screeninfo_bufer;
	//-获得ip---------
	struct sockaddr_in sa;
	int len = sizeof(sa);
	getpeername(sendsocket, (struct sockaddr *)&sa, &len);
	
	// inet_ntoa(sa.sin_addr);
	char ipdotdec[20];
	inet_ntop(AF_INET, (void *)&sendsocket, ipdotdec, 16);
	
	
//	int width = 1920;
//	int height = 1080;
	int width = screeninfo.width;
	int height = screeninfo.height;


/*	FILE *outfile;
	fopen_s(&outfile, "recv.h264", "wb");

	if (!outfile) {
		printf("Could not open %s\n");
		exit(1);
	}

*/
	int once_recv = 0;
	int all_recv = 0;
	int pts = 0, dts = 0;
	int packet_data_len;
	int packetsize;

	char *databuffer = (char *)malloc(width * 4*height);
	//uint8_t *preparedata = (uint8_t *)malloc(1920 * 1080);
	uint8_t *packetdata = (uint8_t *)malloc(width * height);
	uint8_t * preparedata;
	
	//---frame-------------------------
	AVFrame *decodec_Frame;
	AVFrame *out_pYUVFrame;
	struct SwsContext *out_img_convert_ctx;
	uint8_t * decodec_yuv_buff;
	uint8_t * out_yuv_buff;
	//-------------codec------------
	AVCodec *decodec;
	AVPacket *packet;
	int ret;
	int got_picture;
	AVCodecContext  *decodecctx;
	AVFormatContext *pFormatCtx;
	//-----------sdl----------------
	SDL_Window *screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	SDL_Event myevent;
	int screen_w = 0, screen_h = 0;

//sdl初始化-----------------------------------------------------------------------------------
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	screen_w = width;
	screen_h = height;
	//SDL 2.0 Support for multiple windows  
	screen = SDL_CreateWindow(ipdotdec, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w/2, screen_h/2,
		SDL_WINDOW_OPENGL| SDL_WINDOW_RESIZABLE);

	if (!screen) {
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}

	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	//IYUV: Y + U + V  (3 planes)  
	//YV12: Y + V + U  (3 planes)  
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_w, screen_h);

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;
	
	//------------------------------------------frame 结构完备 数据结构准备---------------------------------------------------------
	out_img_convert_ctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P,
		width , height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	decodec_yuv_buff = (uint8_t *)malloc(width * height * 1.5);
    out_yuv_buff = (uint8_t *)malloc(width * height * 1.5);
	decodec_Frame = av_frame_alloc();
	out_pYUVFrame = av_frame_alloc();

	
	//-------------------------------------------------填充frame----------------------------------------
	av_image_fill_arrays(decodec_Frame->data, decodec_Frame->linesize, decodec_yuv_buff,
		AV_PIX_FMT_YUV420P, width, height, 1);

	av_image_fill_arrays(out_pYUVFrame->data, out_pYUVFrame->linesize, out_yuv_buff,
		AV_PIX_FMT_YUV420P, width, height, 1);

//--------------------------------------codecctx packet 结构完备  数据结构准备------------------------------
	

	avcodec_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();


	//---------------------------内存播放处理-------------------------------------------------------------
/*	unsigned char *aviobuffer = (unsigned char *)av_malloc(200000);
	AVIOContext *avio = avio_alloc_context(aviobuffer, 32768, 0, NULL, read_buffer, NULL, NULL);
	pFormatCtx->pb = avio;

	if (avformat_open_input(&pFormatCtx, NULL, NULL, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
		printf("Couldn't find stream information.\n");
		return -1;
	}
	*/
	decodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	decodecctx = avcodec_alloc_context3(decodec);
	packet = av_packet_alloc();
	decodecctx->width = width;
	decodecctx->height = height;
	ret = avcodec_open2(decodecctx, decodec, NULL);

	//--------------------parse-------------------
	AVCodecParserContext *parser = NULL;
	parser = av_parser_init(AV_CODEC_ID_H264);
	parser->flags |= PARSER_FLAG_ONCE;
//---------------------------------AVIOContext----------------------------------------------------------------	
	
	while (1)
	{	
	/*	while (1)
		{
			once_recv = recv(sendsocket, databuffer, 300000, 0);
			if (once_recv == SOCKET_ERROR)
				break;
			fwrite(databuffer, 1, once_recv, outfile);

		}*/
		
		int parsenum=0;
		goto getpacket;
		while (1)
		{
			
			once_recv = recv(sendsocket, databuffer+all_recv, 300000, 0);
			if (once_recv == SOCKET_ERROR)
				goto endloop;
		//	fwrite(databuffer + all_recv, 1, once_recv, outfile);
			all_recv += once_recv;

			getpacket:
			packet_data_len = av_parser_parse2(parser, decodecctx, &preparedata, &packetsize, (uint8_t*)(databuffer+parsenum), all_recv-parsenum,
				pts, dts, AV_NOPTS_VALUE);
			parsenum += packet_data_len;
			
			if (packetsize)
			{

				memcpy(packetdata, preparedata, packetsize);
				memcpy(databuffer, databuffer + parsenum, all_recv-parsenum);
				
				all_recv -= packetsize;
				break;
			}
			if (once_recv == SOCKET_ERROR)
				break;
		}
		
		//--------------创建packet_data--缓冲-------------
	
		//---------------------------解码显示--------------------------
		//packet_mutex.Lock();
		packet->data = packetdata;
		packet->size = packetsize;
		//packet_mutex.Unlock();
		ret = avcodec_send_packet(decodecctx, packet);
		got_picture = avcodec_receive_frame(decodecctx, decodec_Frame);
		if (got_picture != 0)
		{
			continue;
		}
		

		//-----------------大小转换------------------------
		sws_scale(out_img_convert_ctx, (const unsigned char* const*)decodec_Frame->data, decodec_Frame->linesize, 0, height,
			out_pYUVFrame->data, out_pYUVFrame->linesize);

		SDL_PollEvent(&myevent);
		if (myevent.type == SDL_QUIT)
			goto endloop;
		if (myevent.type == SDL_KEYDOWN || myevent.type == SDL_KEYUP || myevent.type == SDL_MOUSEBUTTONDOWN || myevent.type == SDL_MOUSEBUTTONUP || myevent.type == SDL_MOUSEMOTION
			|| myevent.type == SDL_MOUSEWHEEL||myevent.type==SDL_WINDOWEVENT)
		{
			while (1)
			{
				int totalsend = 0;
				int oncesend = 0;
				oncesend = send(sendsocket, (char *)&myevent + totalsend, sizeof(myevent) - totalsend, 0);
				if (oncesend == SOCKET_ERROR)
					break;
				totalsend += oncesend;
				if (sizeof(myevent) - totalsend <= 0)
					break;

			}
		}
	
		SDL_UpdateTexture(sdlTexture, &sdlRect, out_pYUVFrame->data[0], out_pYUVFrame->linesize[0]);
		SDL_RenderClear(sdlRenderer);
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
		SDL_RenderPresent(sdlRenderer);
	}
endloop:
	free(databuffer);
	free(packetdata);
	free(decodec_yuv_buff);
	free(out_yuv_buff);

	av_frame_free(&decodec_Frame);
    av_frame_free(&out_pYUVFrame);
	av_parser_close(parser);
	av_packet_free(&packet);
	
	avcodec_close(decodecctx);
	avformat_free_context(pFormatCtx);
	

	
//    free(preparedata);
	
	sws_freeContext(out_img_convert_ctx);

	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(screen);

    SDL_Quit();
	closesocket(sendsocket);
 //   fclose(outfile);
	return 1;

}

//---------------等待链接------------------------------------------------
DWORD WINAPI listenproc(LPVOID lparam)
{
	SOCKET clientsocket=NULL;
	SOCKET TEMP;
	
	while (1)
	{	TEMP = *(SOCKET*)lparam;
		clientsocket = accept(*(SOCKET*)lparam, NULL, NULL);
		if (clientsocket == INVALID_SOCKET)
		{
			closesocket(clientsocket);
			break;
		}
		//if (!CreateThread(NULL, 0, communicatethread, (LPVOID)&clientsocket, 0, NULL))
			//break;
		CreateThread(NULL,0,screenshow,&clientsocket,0,NULL);
		//CreateThread(NULL, 0, recvpacket, &clientsocket, 0, NULL);
	//	WaitForSingleObject(listmutex,300);
	//	socklist.push_back(clientsocket);
	//	socklist.unique();
	//	ReleaseMutex(listmutex);
	}
	WSACleanup();
	return 3;
}




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


// CremotcontrolDlg 对话框



CremotcontrolDlg::CremotcontrolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_REMOTCONTROL_DIALOG, pParent)
	
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CremotcontrolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_COMBO1, cb_ipselsect);
}

BEGIN_MESSAGE_MAP(CremotcontrolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_STN_CLICKED(IDC_X, &CremotcontrolDlg::OnStnClickedX)
	ON_BN_CLICKED(IDOK, &CremotcontrolDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CremotcontrolDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON4, &CremotcontrolDlg::OnBnClickedButton4)
	
	ON_CBN_SELCHANGE(IDC_COMBO1, &CremotcontrolDlg::OnCbnSelchangeCombo1)
	ON_EN_CHANGE(IDC_EDIT_X, &CremotcontrolDlg::OnEnChangeEditX)
END_MESSAGE_MAP()


// CremotcontrolDlg 消息处理程序

BOOL CremotcontrolDlg::OnInitDialog()
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
	
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//得到结构体大小,用于GetAdaptersInfo参数
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//如果函数返回的是ERROR_BUFFER_OVERFLOW
		//则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
		//这也是说明为什么stSize既是一个输入量也是一个输出量
		//释放原来的内存空间
		delete pIpAdapterInfo;
		//重新申请内存空间用来存储所有网卡信息
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}
	if (ERROR_SUCCESS == nRel)
	{
		//输出网卡信息
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
	//释放内存空间
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}
	

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CremotcontrolDlg::OnPaint()
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
HCURSOR CremotcontrolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CremotcontrolDlg::OnStnClickedX()
{
	// TODO: 在此添加控件通知处理程序代码
	
	closesocket(s);
	WSACleanup();

}


void CremotcontrolDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();

}

void CremotcontrolDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();

	closesocket(s);
	WSACleanup();

}

//-----------------开启UDP广播------TCP等待链接（LISTEN）-----------------
void CremotcontrolDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
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
	typedef struct _my_struct
	{
		SOCKET s;
		SOCKADDR_IN addr;
		SOCKADDR_IN addr_tcp;
	}my_struct;
	my_struct param = {s,addr_to,addr_tcp};

	
	/*
	param.s = s;
	param.addr = addr_to;
	param.addr_tcp = addr_tcp;
	*/
	CreateThread(NULL, 0, beconproc, (LPVOID)&param, 0, NULL);
//	listmutex = CreateMutex(NULL,FALSE,NULL);
	Sleep(20);

}

void CremotcontrolDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CremotcontrolDlg::OnEnChangeEditX()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
