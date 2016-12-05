/********************************************************************
	created:	2013/10/07
	created:	7:10:2013   16:08
	filename: 	E:\ResCheckModule\ResCheckerCore\String_Helper.h
	file path:	E:\ResCheckModule\ResCheckerCore
	file base:	String_Helper
	file ext:	h
	author:		sssa2000
	
	purpose:	
*********************************************************************/
#pragma once

#include <string>


void format_str_impl(const char* str,va_list argptr,std::string& out);
void format_str_impl(std::string& out,const char* str,...);

