
// remotcontrol.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CremotcontrolApp: 
// �йش����ʵ�֣������ remotcontrol.cpp
//

class CremotcontrolApp : public CWinApp
{
public:
	CremotcontrolApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CremotcontrolApp theApp;