#include "HippoWindow.h"
#include "ErrReport.h"
#include "Win32MsgUtil.h"
#include <algorithm>
#include "HippoGlobal.h"
HippoWindow::HippoWindow(const char* name,unsigned int w,unsigned int h,unsigned int left,unsigned int top)
	:m_name(name)
{
	m_width=w;
	m_height=h;
	m_pos_left=left;
	m_pos_top=top;
	//m_hInst=hInst;
	CreateWnd();
}
HippoWindow::~HippoWindow()
{
	custom_msg_proc = nullptr;
	PostMessage(m_wnd_handle, WM_CLOSE, 0, 0);
	SetWindowLongPtr(m_wnd_handle, GWL_USERDATA, NULL);
}

void HippoWindow::CreateWnd()
{
	HINSTANCE hInst =::GetModuleHandle(NULL);

	WNDCLASSEX wc;
	wc.cbSize			= sizeof(wc);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= HippoWindow::StaticWindowProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof(this);
	wc.hInstance		= hInst;
	wc.hIcon			= NULL;
	wc.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= m_name.c_str();
	wc.hIconSm			= NULL;

	if( !::RegisterClassEx( &wc ) )
	{
		DWORD dwError = GetLastError();
		ReportErrWithLastErr(dwError);
		return;
	}

	RECT rc = { 0, 0, (LONG)m_width,(LONG)m_height};
	::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	m_wnd_handle = ::CreateWindowA(
		m_name.c_str(), 
		m_name.c_str(),
		WS_OVERLAPPEDWINDOW, 
		m_pos_left, 
		m_pos_top,
		rc.right - rc.left, 
		rc.bottom - rc.top, 
		0, 
		0, 
		hInst, 
		0);

	if( m_wnd_handle == NULL )
	{
		DWORD dwError = GetLastError();
		ReportErrWithLastErr(dwError);
		return ;
	}
	//����Ӧ��Ϊ�ͻ�����С
	::GetClientRect(m_wnd_handle, &rc);
	m_width = rc.right - rc.left;
	m_height = rc.bottom - rc.top;
	::SetWindowLongPtrW(m_wnd_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	::ShowWindow(m_wnd_handle, SW_SHOWNORMAL);
	::UpdateWindow(m_wnd_handle);

	//�����ܹ�������ק�ļ�
	DragAcceptFiles( m_wnd_handle,TRUE );
}
LRESULT CALLBACK HippoWindow::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HippoWindow* pHippoWnd = (HippoWindow*)(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	if (pHippoWnd)
	{
		return pHippoWnd->HippoWndProc(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

LRESULT HippoWindow::HippoWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(custom_msg_proc)
	{
		LRESULT r= custom_msg_proc(uMsg,wParam,lParam);
		if(r) //����0��ʾ�Ը���Ϣ������Ȥ
			return 1;
	}
	PAINTSTRUCT ps;
	
	//Hippo_WriteConsoleAndLog(CC_WHITE,"msg:%d",uMsg);
	switch (uMsg)
	{
	case WM_DESTROY:
		//SetWindowLongPtr(hWnd, GWL_USERDATA, NULL);
		PostQuitMessage(0);
		break;

	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		// TODO: �ڴ���������ͼ����...
		EndPaint(hWnd, &ps);
		break;

	case WM_KEYDOWN:
		{
			std::for_each(
				keydown_event.begin(),
				keydown_event.end(),
				[&](KeyCallback& e){e((unsigned int)(wParam));}
			);

			break;
		}

	case WM_KEYUP:
		{
			std::for_each(
				keyup_event.begin(),
				keyup_event.end(),
				[&](KeyCallback& e){e((unsigned int)(wParam));}
			);
			break;
		}

	case WM_LBUTTONDOWN:
		{
			std::for_each(
				mouse_leftdown_event.begin(),
				mouse_leftdown_event.end(),
				[&](MouseKeyCallback& e){e(Win32MsgUtil::GetMouseEventInfoFromMsg(uMsg,wParam,lParam));}
			);

		}
		break;
	case WM_LBUTTONUP:
		{
			std::for_each(
				mouse_leftup_event.begin(),
				mouse_leftup_event.end(),
				[&](MouseKeyCallback& e){e(Win32MsgUtil::GetMouseEventInfoFromMsg(uMsg,wParam,lParam));}
			);
		}
		break;
	case WM_MBUTTONDOWN:
		{
			std::for_each(
				mouse_middledown_event.begin(),
				mouse_middledown_event.end(),
				[&](MouseKeyCallback& e){e(Win32MsgUtil::GetMouseEventInfoFromMsg(uMsg,wParam,lParam));}
			);

		}
		break;
	case WM_MBUTTONUP:
		{
			std::for_each(
				mouse_middleup_event.begin(),
				mouse_middleup_event.end(),
				[&](MouseKeyCallback& e){e(Win32MsgUtil::GetMouseEventInfoFromMsg(uMsg,wParam,lParam));}
			);
		}
		break;
	case WM_RBUTTONDOWN:
		{
			std::for_each(
				mouse_rightdown_event.begin(),
				mouse_rightdown_event.end(),
				[&](MouseKeyCallback& e){e(Win32MsgUtil::GetMouseEventInfoFromMsg(uMsg,wParam,lParam));}
			);
		}
		break;
	case WM_RBUTTONUP:
		{
			std::for_each(
				mouse_rightup_event.begin(),
				mouse_rightup_event.end(),
				[&](MouseKeyCallback& e){e(Win32MsgUtil::GetMouseEventInfoFromMsg(uMsg,wParam,lParam));}
			);
		}
		break;

	case WM_MOUSEWHEEL:
		{
			std::for_each(
				mouse_wheel_event.begin(),
				mouse_wheel_event.end(),
				[&](MouseWheelCallback& e){e(Win32MsgUtil::GetWheelEventInfoFromMsg(uMsg,wParam,lParam));}
			);
		}
		break;

	case WM_MOUSEMOVE:
		{
			std::for_each(
				mouse_move_event.begin(),
				mouse_move_event.end(),
				[&](MouseKeyCallback& e){e(Win32MsgUtil::GetMouseEventInfoFromMsg(uMsg,wParam,lParam));}
			);
		}
		break;
	case WM_SIZE:
		{
			std::for_each(
				wnd_size_event.begin(),
				wnd_size_event.end(),
				[&](WndSizeCallback& e){e(Win32MsgUtil::GetHippoSizeEventFromMsg(uMsg,wParam,lParam));}
			);

			break;
		}
	case WM_DROPFILES:
		{
			std::for_each(
				dropfile_event.begin(),
				dropfile_event.end(),
				[&](DropFileCallback& e){e((HDROP)wParam);} //wparam����hdrop,lParam=0
			);
		}
		break;
	case WM_CHAR:
		{
			std::for_each(
				keychar_event.begin(),
				keychar_event.end(),
				[&](KeyCallback& e) {e((unsigned int)(wParam)); }
			);
		}
		break;
	default:
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}
void HippoWindow::RegisterWndSizeEvent(WndSizeCallback& e)
{
	wnd_size_event.push_back(e);
}
void HippoWindow::RegisterMouseLeftDownEvent(MouseKeyCallback& e)
{
	mouse_leftdown_event.push_back(e);
}
void HippoWindow::RegisterMouseLeftUpEvent(MouseKeyCallback& e)
{
	mouse_leftup_event.push_back(e);
}
void HippoWindow::RegisterMouseMiddleDownEvent(MouseKeyCallback& e)
{
	mouse_middledown_event.push_back(e);
}
void HippoWindow::RegisterMouseMiddleUpEvent(MouseKeyCallback& e)
{
	mouse_middleup_event.push_back(e);
}
void HippoWindow::RegisterMouseRightDownEvent(MouseKeyCallback& e)
{
	mouse_rightdown_event.push_back(e);
}
void HippoWindow::RegisterMouseRightUpEvent(MouseKeyCallback& e)
{
	mouse_rightup_event.push_back(e);
}
void HippoWindow::RegisterMouseMoveEvent(MouseKeyCallback& e)
{
	mouse_move_event.push_back(e);
}
void HippoWindow::RegisterKeyDownEvent(KeyCallback& e)
{
	keydown_event.push_back(e);
}
void HippoWindow::RegisterKeyUpEvent(KeyCallback& e)
{
	keyup_event.push_back(e);
}
void HippoWindow::RegisterMouseWheelEvent(MouseWheelCallback& e)
{
	mouse_wheel_event.push_back(e);
}
void HippoWindow::RegisterDropFileEvent(DropFileCallback& e)
{
	dropfile_event.push_back(e);
}

void HippoWindow::RegisterChar(KeyCallback & e)
{
	keychar_event.push_back(e);
}

void HippoWindow::GetWidthHeight(unsigned int& w, unsigned int&h)
{
	RECT rc;
	::GetClientRect(m_wnd_handle, &rc);
	w = rc.right - rc.left;
	h = rc.bottom - rc.top;
}
