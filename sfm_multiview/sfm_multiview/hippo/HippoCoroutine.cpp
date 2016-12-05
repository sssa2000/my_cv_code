#include "HippoCoroutine.h"
#include <Windows.h>
#include "ErrReport.h"
#include "HippoGlobal.h"

HippoCoroutineManager::HippoCoroutineManager()
{
	m_main_fiber = NULL;

}
HippoCoroutineManager::~HippoCoroutineManager()
{
	auto beg=m_all_cor.begin();
	while (beg!=m_all_cor.end())
	{
		delete beg->second;
		++beg;
	}
	m_all_cor.clear();
	ConvertFiberToThread();
	//delete m_main_fiber;
}

IHippoCoroutine* HippoCoroutineManager::CreateCoroutine(std::string& name,COTROUTINE_FUN funptr,void* param)
{
	IHippoCoroutine* co=FindCoroutineWithName(name);
	if(!co)
	{
		co=new HippoCoroutineImpl(name,funptr,param);
		m_all_cor[name]=co;
	}
	else
		ReportErr("创建同名的Coroutine，请检查，name=%s",name.c_str());
	return co;
}



IHippoCoroutine* HippoCoroutineManager::FindCoroutineWithName(std::string& name)
{
	auto itr=m_all_cor.find(name);
	if(itr==m_all_cor.end())
		return 0;
	return itr->second;
}

void HippoCoroutineManager::Init()
{
	//线程本身的fiber要特殊对待
	PVOID mainco=ConvertThreadToFiber(0);
	if(!mainco)
	{
		ReportErrWithLastErr(GetLastError(),"ConvertThreadToFiber失败");
	}
	m_main_fiber=new HippoCoroutineImpl(std::string("main"),mainco);
}



HippoCoroutineImpl::HippoCoroutineImpl(std::string& name,COTROUTINE_FUN funptr,void* param)
	:m_name(name)
{
	
	m_fiber_handle=CreateFiber(0,funptr,param);
	if(!m_fiber_handle)
	{
		ReportErrWithLastErr(GetLastError(),"CreateFiber失败");
	}
}

HippoCoroutineImpl::HippoCoroutineImpl(std::string& name,PVOID handle)
	:m_name(name)
{
	
	m_fiber_handle=handle;
}

HippoCoroutineImpl::~HippoCoroutineImpl()
{
	Hippo_WriteConsoleAndLog(CC_GREEN,"Delete Fiber：%p",m_fiber_handle);
	DeleteFiber(m_fiber_handle);
}

void HippoCoroutineImpl::SwitchToThis()
{
	SwitchToFiber(m_fiber_handle);
}

const char* HippoCoroutineImpl::GetName()
{
	return m_name.c_str();
}