#include "stdafx.h"
#include "String_Helper.h"
#include <stdarg.h>

void format_str_impl(const char* str,va_list argptr,std::string& out)
{
	//首先尝试使用固定buff 格式化字符串
	const int fix_buff_len=256;//这个值可能需要调整
	char buff[fix_buff_len];
	int sz=_vsnprintf_s(buff,fix_buff_len,_TRUNCATE,str,argptr);
	out=buff;
	if(sz<0)//buff 长度不够，计算出所需长度
	{
		int requair_len=_vscprintf(str,argptr)+1;
		out.resize(requair_len);
		vsprintf_s(&(out[0]),requair_len,str,argptr);
		return;
	}

}

void format_str_impl(std::string& out,const char* str,...)
{
	va_list va;
	va_start(va,str);
	format_str_impl(str,va,out);
	va_end(va);
}
