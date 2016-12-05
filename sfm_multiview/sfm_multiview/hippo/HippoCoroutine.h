/********************************************************************
	created:	2013/11/26
	created:	26:11:2013   18:34
	filename: 	D:\Code\X52\X52_Community_Version\TestCode\Utility\CommonCode\HippoUtil\HippoCoroutine.h
	file path:	D:\Code\X52\X52_Community_Version\TestCode\Utility\CommonCode\HippoUtil
	file base:	HippoCoroutine
	file ext:	h
	author:		sssa2000
	
	purpose:	
*********************************************************************/
#pragma once

#include <map>
#include <string>
#include<Windows.h>
//coroutine的函数签名

typedef VOID (WINAPI *COTROUTINE_FUN)(
	LPVOID lpThreadParameter
	);

//表示一个coroutine
class IHippoCoroutine
{
public:
	virtual ~IHippoCoroutine()=0{};
	virtual void SwitchToThis()=0; //转到该coroutine执行
	virtual const char* GetName()=0;
};


class HippoCoroutineImpl:public IHippoCoroutine
{
public:
	HippoCoroutineImpl(std::string& name,PVOID handle);
	HippoCoroutineImpl(std::string& name,COTROUTINE_FUN funptr,void* param);
	~HippoCoroutineImpl();
	virtual void SwitchToThis();
	virtual const char* GetName();
protected:
private:
	void* m_fiber_handle;
	std::string m_name;
};


class HippoCoroutineManager
{
public:
	HippoCoroutineManager();
	~HippoCoroutineManager();

	void Init();
	IHippoCoroutine* CreateCoroutine(std::string& name,COTROUTINE_FUN funptr,void* param);
	IHippoCoroutine* FindCoroutineWithName(std::string& name);
	IHippoCoroutine* GetMainCoroutine(){return m_main_fiber;}
private:
	IHippoCoroutine* m_main_fiber;
	std::map<std::string,IHippoCoroutine*> m_all_cor;
};