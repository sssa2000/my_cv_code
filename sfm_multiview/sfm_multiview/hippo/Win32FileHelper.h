#pragma once
#include <Windows.h>
enum FILE_PROPERTY
{
	FP_DIR,
	FP_FILE
};

//回调函数，遍历文件时，每发现一个文件(夹)，该回调就会被调用一次
class IFindCallBack
{
public:
	virtual int OnFind(const char* finded_fn, const char* parent_dir, FILE_PROPERTY fproperty) = 0;
};


class Win32FileHelper
{
public:
	static bool IsFileExist(const char* path);

	static int IsDir(const char* path);
	//遍历
	static void EnumFile(const char* path, bool bRecurse, IFindCallBack* fp);
	//获取文件时间
	static void GetFileTime(const char* path, FILETIME& fileTime);

	static int CopyDiskFile(const char* src, const char* dst, bool bOverwrite);

	//批量创建目录，返回值返回一共创建了多少个目录，返回-1表示出错
	//例如，传入c:\1\2\3\4 ，本函数会创建4个目录
	static int CreateDirBatch(const char* dirpath);

	//同步文件夹结构
	static int SyncDirStruct(const char* srcdir, const char* dstdir);
};