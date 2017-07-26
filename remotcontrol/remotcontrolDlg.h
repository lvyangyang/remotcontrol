
// remotcontrolDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#define WM_MOUSEMSG WM_USER + 106
#define WM_HOOKKEY WM_USER+107

// CremotcontrolDlg 对话框
class CremotcontrolDlg : public CDialogEx
{
// 构造
public:
	CremotcontrolDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTCONTROL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnMouseMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnKEYMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedy();
	afx_msg void OnStnClickedX();
	double x_position;
	double y_position;
	double key_value;
	WSADATA wsad;
	SOCKET s,s_tcp,listensocket,clientsocket;
	SOCKADDR_IN addr_in,addr_to,addr_tcp;
	BOOL bBoardcast;
	struct Mouse
	{
		WPARAM wparam;
		MSLLHOOKSTRUCT position;

	}*pmouse_data,mouse_data;
	struct Keyboard
	{
		WPARAM wparam;
		KBDLLHOOKSTRUCT code;
		char space[3];
	}*pkeyboard_data,keyboard_data;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnCbnSelchangeCombo1();
	CComboBox cb_ipselsect;
};
