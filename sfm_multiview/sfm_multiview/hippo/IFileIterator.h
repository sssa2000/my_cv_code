/*!
 * \file IFileIterator.h
 *
 * \author Administrator
 * \date 二月 2015
 *
 * 
 */
#pragma once
#include "HippoFilePath.h"
//用于枚举文件的过滤器接口
class IFileFilter
{
public:
	virtual bool Pass(const HippoFilePath& f) = 0;

};

//用于枚举文件的迭代器
class IFileIterator
{
public:
	virtual  bool GetNext(HippoFilePath& res) = 0;
};