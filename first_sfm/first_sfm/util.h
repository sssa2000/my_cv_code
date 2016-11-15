
#pragma once

#include <chrono>
#include <string>
class fun_contex
{
public:
	fun_contex();
	~fun_contex();
	void OnBlockBegin();
	void OnBlockEnd();
	int GetIndent() { return m_indent_value; }
private:
	int m_indent_value;
};



class fun_timer_obj
{
public:
	fun_timer_obj(const char* funname, fun_contex* _gContex);
	~fun_timer_obj();
protected:
	typedef std::chrono::time_point<std::chrono::steady_clock> TimePointType;
	TimePointType beg;
	TimePointType end;
	std::string m_fun_name;
	fun_contex* gContex;
};
