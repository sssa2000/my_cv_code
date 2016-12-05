#include "stdafx.h"
#include "Win32FileHelper.h"
#include "ScopeGuard.h"
#include "HippoFilePath.h"
#include <io.h>
#include <direct.h>
//����-1��ʾ·��������,����1��ʾ��dir������0��ʾ��file
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
		if ('.' == findFileData.cFileName[0])//��.(����Ŀ¼)��..(��һ��Ŀ¼)
			continue;
		else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//��Ŀ¼���ݹ�����
		{
			fp->OnFind(findFileData.cFileName, dir, FP_DIR);
			if (bRecurse)
			{
				HippoFilePath newdir(dir);
				newdir.Append(findFileData.cFileName);
				EnumFile(newdir.c_str(), true, fp);
			}
		}
		else //���ļ�
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
		//��ȡ�ļ�����
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
	//����Ŀ¼���������
	if (!IsFileExist(srcdir) || !IsFileExist(dstdir))
		return 0;
	
	//���ԭĿ¼��������Ŀ¼
	SyncDirStructCB cb;
	EnumFile(srcdir, true, &cb);
	
	//һ��һ������
	HippoFilePath src(srcdir);
	HippoFilePath dst(dstdir);
	size_t offset = src.length();
	for (size_t i=0;i<cb.m_con.size();++i)
	{
		
		//ƴĿ¼
		HippoFilePath newpath=dst.AppendTwoPath(dstdir, srcdir + offset);
		CreateDirBatch(newpath.c_str());
	}

	return 1;
}

//��������Ŀ¼������ֵ����һ�������˶��ٸ�Ŀ¼������-1��ʾ����
//���磬����c:\1\2\3\4 ���������ᴴ��4��Ŀ¼
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