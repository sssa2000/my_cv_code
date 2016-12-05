#pragma once
#include <Windows.h>
enum FILE_PROPERTY
{
	FP_DIR,
	FP_FILE
};

//�ص������������ļ�ʱ��ÿ����һ���ļ�(��)���ûص��ͻᱻ����һ��
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
	//����
	static void EnumFile(const char* path, bool bRecurse, IFindCallBack* fp);
	//��ȡ�ļ�ʱ��
	static void GetFileTime(const char* path, FILETIME& fileTime);

	static int CopyDiskFile(const char* src, const char* dst, bool bOverwrite);

	//��������Ŀ¼������ֵ����һ�������˶��ٸ�Ŀ¼������-1��ʾ����
	//���磬����c:\1\2\3\4 ���������ᴴ��4��Ŀ¼
	static int CreateDirBatch(const char* dirpath);

	//ͬ���ļ��нṹ
	static int SyncDirStruct(const char* srcdir, const char* dstdir);
};