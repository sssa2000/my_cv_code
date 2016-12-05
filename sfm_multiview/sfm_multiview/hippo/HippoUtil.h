/********************************************************************
	created:	2015/02/15
	created:	15:2:2015   9:46
	filename: 	G:\TAC\trunk\X51\Master\Code\X51Engine\TestCodeForStress\framework\HippoUtil.h
	file path:	G:\TAC\trunk\X51\Master\Code\X51Engine\TestCodeForStress\framework
	file base:	HippoUtil
	file ext:	h
	author:		zhangshuai
	
	purpose:	共用的辅助函数
*********************************************************************/
#pragma once
class H3DVec3;
class H3DMat4;


//int
int GetRandomInt(int imin, int imax);

//float
float GetRandomFloat(float fmin, float fmax);

// bool
bool GetRandomBool();

//H3DVec3
H3DVec3 GetRandomPostion();

//H3DMat4
H3DMat4 GetRandomLocationMat();