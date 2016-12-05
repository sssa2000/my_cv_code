#include "stdafx.h"
#include "Win32FileHelper.h"
#include "ScopeGuard.h"
#include "HippoFilePath.h"
#include <io.h>
#include <direct.h>
//返回-1表示路径不存在,返回1表示是dir，返回0表示是file
int Win32FileHelper::IsDir(const char* path)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFileExA(path, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH); //winxp do not support

	ON_SCOPE_EXIT([&](){FindClose(hFind); });

	if (INVALID_HANDLE_VALUE == hFind)
		return -1;
	else if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return 1;

	return 0;
}

void Win32FileHelper::EnumFile(const char* dir, bool bRecurse, IFindCallBack* fp)
{
	WIN32_FIND_DATA findFileData;
	HippoFilePath findstr(dir);
	findstr.Append("*.*");

	HANDLE hFind = ::FindFirstFileExA(findstr.c_str(), FindExInfoBasic, &findFileData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH); //winxp do not support
	ON_SCOPE_EXIT([&](){FindClose(hFind); });
	if (hFind == INVALID_HANDLE_VALUE)
		return;
	do 
	{
		if ('.' == findFileData.cFileName[0])//是.(本层目录)或..(上一层目录)
			continue;
		else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//是目录，递归搜索
		{
			fp->OnFind(findFileData.cFileName, dir, FP_DIR);
			if (bRecurse)
			{
				HippoFilePath newdir(dir);
				newdir.Append(findFileData.cFileName);
				EnumFile(newdir.c_str(), true, fp);
			}
		}
		else //是文件
		{
			fp->OnFind(findFileData.cFileName, dir,FP_FILE);
		}
	} while (::FindNextFileA(hFind, &findFileData));
}


void Win32FileHelper::GetFileTime(const char* path, FILETIME& fileTime)
{
	
	HANDLE hFile = CreateFileA(path,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		//获取文件日期
		::GetFileTime(hFile, NULL, NULL, &fileTime);
		CloseHandle(hFile);
	}
}

bool Win32FileHelper::IsFileExist(const char* path)
{
	return (_access(path, 0) == 0);
}


int Win32FileHelper::CopyDiskFile(const char* src, const char* dst, bool bOverwrite)
{
	BOOL res = CopyFileA(src, dst, bOverwrite?FALSE:TRUE);
	if (!res)
	{
		
		if (bOverwrite)
		{
			//try clear read only flag
			DWORD fileAttributes = GetFileAttributes(dst);
			fileAttributes &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(dst, fileAttributes);
			res = CopyFileA(src, dst, bOverwrite ? FALSE : TRUE);
		}
	}
	return res;
}


class SyncDirStructCB :public IFindCallBack
{
public:
	std::vector<std::string> m_con;
	int OnFind(const char* finded_fn, const char* parent_dir, FILE_PROPERTY fproperty)
	{
		//only care dir
		if (FP_DIR == fproperty)
		{
			m_con.push_back(std::string(finded_fn));
		}
		return 1;
	}
	
};

int Win32FileHelper::SyncDirStruct(const char* srcdir, const char* dstdir)
{
	//两个目录都必须存在
	if (!IsFileExist(srcdir) || !IsFileExist(dstdir))
		return 0;
	
	//获得原目录的所有子目录
	SyncDirStructCB cb;
	EnumFile(srcdir, true, &cb);
	
	//一个一个创建
	HippoFilePath src(srcdir);
	HippoFilePath dst(dstdir);
	size_t offset = src.length();
	for (size_t i=0;i<cb.m_con.size();++i)
	{
		
		//拼目录
		HippoFilePath newpath=dst.AppendTwoPath(dstdir, srcdir + offset);
		CreateDirBatch(newpath.c_str());
	}

	return 1;
}

//批量创建目录，返回值返回一共创建了多少个目录，返回-1表示出错
//例如，传入c:\1\2\3\4 ，本函数会创建4个目录
int Win32FileHelper::CreateDirBatch(const char* dirpath)
{
	HippoFilePath strPath = dirpath;
	if (_access(strPath.c_str(), 0) != -1)
		return -1;
	std::vector<std::string> alldir;
	strPath.Split(alldir);
	int num = 0;
	for (size_t i=0;i<alldir.size();++i)
	{
		std::string& d=alldir.at(i);
		int res=_mkdir(d.c_str());
		if (res == 0)
			++num;
	}

	return num;
}