
// remotcontrolDlg.h : ͷ�ļ�
//
extern "C"
{
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h"  
#include "libavutil/opt.h"
#include "SDL/SDL.h"  
};
#pragma once
#include "afxwin.h"
#define WM_MOUSEMSG WM_USER + 106
#define WM_HOOKKEY WM_USER+107

// CremotcontrolDlg �Ի���
class CremotcontrolDlg : public CDialogEx
{
// ����
public:
	CremotcontrolDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTCONTROL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedy();
	afx_msg void OnStnClickedX();

	WSADATA wsad;
	SOCKET s,s_tcp,listensocket,clientsocket;
	SOCKADDR_IN addr_in,addr_to,addr_tcp;
	BOOL bBoardcast;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnCbnSelchangeCombo1();
	CComboBox cb_ipselsect;
	afx_msg void OnEnChangeEditX();
};
