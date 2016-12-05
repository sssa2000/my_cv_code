#include "HippoUtil.h"
#include <Windows.h>
#include "dMathHeader.h"


int GetRandomInt(int imin, int imax)
{
	if (imax <= imin)
		return imin;
	return (rand() % (imax - imin + 1) + imin);
}

float GetRandomFloat(float fmin, float fmax)
{
	if (fmax <= fmin)
		return fmin;
	return (((float)rand() / (float)RAND_MAX) * (fmax - fmin) + fmin);
}

bool GetRandomBool()
{
	return GetRandomInt(0,1)?true:false;
}

H3DVec3 GetRandomPostion()
{
	return H3DVec3(GetRandomFloat(-5, 5), GetRandomFloat(-3, 3), 0);
}

H3DMat4 GetRandomLocationMat()
{
	H3DMat4 mat;
	mat.Compose(H3DVec3(1, 1, 1), H3DQuat(0, 0, 0, 1), GetRandomPostion());
	return mat;
}