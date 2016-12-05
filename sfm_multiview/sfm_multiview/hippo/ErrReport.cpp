#include "ErrReport.h"
#include <stdio.h>
#include <Windows.h>


void GetLastErrorString(std::string& res,unsigned int dwError)
{
	if (dwError!=0) //0 means there is no err
	{
		void* lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(char*) &lpMsgBuf,
			0, NULL );		
		static char szParsedString[2048];
		sprintf_s(szParsedString,2048,"Error! LastErr=%u\nReason:%s",dwError,(char*)lpMsgBuf);
		LocalFree(lpMsgBuf);
		res=szParsedString;
	}
}

void ReportErrWithLastErr(unsigned int dwError)
{
	if (dwError!=0) //0 means there is no err
	{
		std::string msg;
		GetLastErrorString(msg,dwError);
		::MessageBoxA(0,msg.c_str(),"Error",0);

	}
}


void ReportErrWithLastErr(unsigned int dwError,const char* szString,...)
{
	if (dwError!=0) //0 means there is no err
	{
		static char buff[1024];
		va_list va;
		va_start( va, szString );
		vsprintf_s( buff,1024, szString, va );
		va_end( va );

		void* lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(char*) &lpMsgBuf,
			0, NULL );		

		static char szParsedString[2048];
		sprintf_s(szParsedString,2048,"%s,Error! LastErr=%u\nReason:%s",buff,dwError,(char*)lpMsgBuf);
		LocalFree(lpMsgBuf);
		::MessageBoxA(0,szParsedString,"Error",0);

	}
}

void ReportErr(const char* szString,...)
{
	static char buff[1024];
	va_list va;
	va_start( va, szString );
	vsprintf_s( buff,1024, szString, va );
	va_end( va );

	::MessageBoxA(0,buff,"Error",0);
}