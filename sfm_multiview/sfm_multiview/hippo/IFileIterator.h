/*!
 * \file IFileIterator.h
 *
 * \author Administrator
 * \date ���� 2015
 *
 * 
 */
#pragma once
#include "HippoFilePath.h"
//����ö���ļ��Ĺ������ӿ�
class IFileFilter
{
public:
	virtual bool Pass(const HippoFilePath& f) = 0;

};

//����ö���ļ��ĵ�����
class IFileIterator
{
public:
	virtual  bool GetNext(HippoFilePath& res) = 0;
};