#include "HippoTestInputTransmit.h"
#include "HippoTestManager.h"
#include "HippoGlobal.h"
#include "HippoConsole.h"

HippoTestInputTransmit::HippoTestInputTransmit()
{

}
HippoTestInputTransmit::~HippoTestInputTransmit()
{

}

int HippoTestInputTransmit::OnKeyDown(unsigned int key)
{
	HippoTestManager* pMng=HippoTestManager::GetInstance();
	HippoTCBase* testcase = pMng->GetCurrentTestCase();
	if(!testcase)
		return 0;
	if(key==VK_RETURN && pMng->IsManualMode())
	{
		//手动运行模式下 如果按回车表示切换到下一个用例
		testcase->SetState(CASE_CLEANUP);
		return 1;
	}
	if (key==VK_F6)
	{
		Hippo_GetConsole()->Show();
	}
	if (key == VK_F3)
	{
		testcase->PrintTask();
	}
	return testcase->OnKeyDown(key);
}

int HippoTestInputTransmit::OnLeftDown(HippoMouseEvent& e)
{
	HippoTestManager* pMng=HippoTestManager::GetInstance();
	HippoTCBase* testcase = pMng->GetCurrentTestCase();
	if(!testcase)
		return 0;

	return testcase->OnLeftDown(e);
}
int HippoTestInputTransmit::OnLeftUp(HippoMouseEvent& e)
{
	HippoTestManager* pMng=HippoTestManager::GetInstance();
	HippoTCBase* testcase = pMng->GetCurrentTestCase();
	if(!testcase)
		return 0;

	return testcase->OnLeftUp(e);
}
int HippoTestInputTransmit::OnRightDown(HippoMouseEvent& e)
{
	HippoTestManager* pMng=HippoTestManager::GetInstance();
	HippoTCBase* testcase = pMng->GetCurrentTestCase();
	if(!testcase)
		return 0;

	return testcase->OnRightDown(e);
}
int HippoTestInputTransmit::OnRightUp(HippoMouseEvent& e)
{
	HippoTestManager* pMng=HippoTestManager::GetInstance();
	HippoTCBase* testcase = pMng->GetCurrentTestCase();
	if(!testcase)
		return 0;

	return testcase->OnRightUp(e);
}
int HippoTestInputTransmit::OnWheel(HippoWheelEvent& e)
{
	HippoTestManager* pMng=HippoTestManager::GetInstance();
	HippoTCBase* testcase = pMng->GetCurrentTestCase();
	if(!testcase)
		return 0;

	return testcase->OnWheel(e);
}

MouseKeyCallback HippoTestInputTransmit::GetMouseLeftDownCallback()
{
	MouseKeyCallback cb= std::tr1::bind(&HippoTestInputTransmit::OnLeftDown,this,std::placeholders::_1);
	return cb;
}
MouseKeyCallback HippoTestInputTransmit::GetMouseRightDownCallback()
{
	MouseKeyCallback cb= std::tr1::bind(&HippoTestInputTransmit::OnRightDown,this,std::placeholders::_1);
	return cb;
}

MouseKeyCallback HippoTestInputTransmit::GetMouseLeftUpCallback()
{
	MouseKeyCallback cb= std::tr1::bind(&HippoTestInputTransmit::OnLeftUp,this,std::placeholders::_1);
	return cb;
}
MouseWheelCallback HippoTestInputTransmit::GetMouseWheelCallback()
{
	MouseWheelCallback cb= std::tr1::bind(&HippoTestInputTransmit::OnWheel,this,std::placeholders::_1);
	return cb;
}
KeyCallback HippoTestInputTransmit::GetKeyDownCallback()
{
	KeyCallback cb= std::tr1::bind(&HippoTestInputTransmit::OnKeyDown,this,std::placeholders::_1);
	return cb;
}