/********************************************************************
	created:	2013/11/27
	created:	27:11:2013   11:56
	filename: 	D:\Code\X52\X52_Community_Version\TestCode\FunctionalTest\HippoTestManager\HIppoTestInputTransmit.h
	file path:	D:\Code\X52\X52_Community_Version\TestCode\FunctionalTest\HippoTestManager
	file base:	HIppoTestInputTransmit
	file ext:	h
	author:		sssa2000
	
	purpose:	
*********************************************************************/
#pragma once
#include "Win32MsgUtil.h"


class HippoTestInputTransmit
{
public:
	HippoTestInputTransmit();
	~HippoTestInputTransmit();

	MouseKeyCallback GetMouseLeftDownCallback();
	MouseKeyCallback GetMouseRightDownCallback();
	MouseKeyCallback GetMouseMoveCallback();
	MouseKeyCallback GetMouseLeftUpCallback();
	MouseWheelCallback GetMouseWheelCallback();
	KeyCallback GetKeyDownCallback();


protected:
	virtual int OnKeyDown(unsigned int key);
	virtual int OnLeftDown(HippoMouseEvent& e);
	virtual int OnLeftUp(HippoMouseEvent& e);
	virtual int OnRightDown(HippoMouseEvent& e);
	virtual int OnRightUp(HippoMouseEvent& e);
	virtual int OnWheel(HippoWheelEvent& e);
private:
};