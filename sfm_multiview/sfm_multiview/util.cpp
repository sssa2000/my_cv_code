
#include "util.h"
#include <iostream>

using namespace std;
using namespace std::chrono;

fun_contex::fun_contex()
{
	m_indent_value = 0;
}
fun_contex::~fun_contex()
{

}

void fun_contex::OnBlockBegin()
{
	++m_indent_value;
}
void fun_contex::OnBlockEnd()
{
	--m_indent_value;
}


//////////////////////////////////////////////////////////////////////////


fun_timer_obj::fun_timer_obj(const char* funname, fun_contex* _gContex)
{
	gContex = _gContex;
	gContex->OnBlockBegin();
	m_fun_name = funname;
	beg = std::chrono::steady_clock::now();
	
	//输出缩进 
	for (int i = 0; i < gContex->GetIndent(); ++i)
		cout << " ";
	cout << m_fun_name << " begin." <<endl;
}
fun_timer_obj::~fun_timer_obj()
{
	auto end = std::chrono::steady_clock::now();
	auto millsec = duration_cast<milliseconds>((end - beg)).count();
	//输出缩进 
	for (int i = 0; i < gContex->GetIndent(); ++i)
		cout << " ";
	cout << m_fun_name << " end , time = " << millsec << "ms ." << endl;
	gContex->OnBlockEnd();
}