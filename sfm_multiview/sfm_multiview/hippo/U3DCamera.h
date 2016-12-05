/********************************************************************
	created:	2011/12/23
	created:	23:12:2011   23:59
	filename: 	f:\TestHippo\TestHippo\HIPPO_FrameWork\camera\U3DCamera.h
	file path:	f:\TestHippo\TestHippo\HIPPO_FrameWork\camera
	file base:	U3DCamera
	file ext:	h
	author:		sssa2000
	
	purpose:	modelview�����������������˶��ģ����������λ����m_r��������λ��m_roation����
*********************************************************************/
#pragma once


#include "Win32MsgUtil.h"
#include "CameraBase.h" //must include after "Win32MsgUtil.h"

class U3DCamera:public CameraBase
{
public:
	U3DCamera();
	~U3DCamera();

	//from base
	const H3DVec3& GetPos();
	const H3DVec3& GetViewAtDir();
	const H3DVec3& GetUp();
	void SetPos(const H3DVec3& pos);
	void SetLookAtDir(const H3DVec3& at);
	void SetLookAtPos(const H3DVec3& at);

	int FrameUpdate(float fElapsedTime);
	void LookAt(H3DI::IRender* pRender);

	void SetWindow( int nWidth, int nHeigh);
	void SetModelCenter( H3DVec3& vModelCenter ) { m_RotCenter = vModelCenter;  m_selModel = true;}
	void ClearModelCenter() { m_RotCenter = m_Target;  m_selModel = false;}
	//!��ȡ��ǰ����ת
	const H3DQuat& GetRotation(){return m_roation;}

	MouseKeyCallback GetMouseLeftDownCallback();
	MouseKeyCallback GetMouseMiddleDownCallback();
	MouseKeyCallback GetMouseRightDownCallback();
	MouseKeyCallback GetMouseMoveCallback();
	MouseKeyCallback GetMouseLeftUpCallback();
	MouseKeyCallback GetMouseMiddleUpCallback();
	MouseKeyCallback GetMouseRightUpCallback();
	MouseWheelCallback GetMouseWheelCallback();
	KeyCallback GetKeyDownCallback();
	KeyCallback GetKeyUpCallback();

	void SyncCamera(CameraBase* pCam);
	void CalcVelocity(float fElapsedTime);
protected:
private:
	int OnMouseLeftDown(HippoMouseEvent& e);
	int OnMouseLeftUp(HippoMouseEvent& e);
	int OnMouseRightDown(HippoMouseEvent& e);
	int OnMouseRightUp(HippoMouseEvent& e);
	int OnMouseMiddleDown(HippoMouseEvent& e);
	int OnMouseMiddleUp(HippoMouseEvent& e);
	int OnMouseMove(HippoMouseEvent& e);
	int OnMouseWheel(HippoWheelEvent& w);
	int OnKeyDown(unsigned int);
	int OnKeyUp(unsigned int);
	//!�������������ϵ��up��right��lookat
	void UpdateCameraCoord();
	int AnalysisKeyInput(float fElapsedTime);
	int AnalysisMouseInput(float fElapsedTime);

	//!����Ļ��ת������λ���ϵĵ�
	H3DVec3 ConvertScreenPoint2SpherePoint(int x,int y);

	//!����w
	int m_nHalafWidth;
	//!����h
	int m_nHalafHeight;
	//!�������ת���ĵ�
	H3DVec3 m_RotCenter;

	H3DVec3 m_Target;
	//!���������İ뾶��������յ�λ���ɸı�������
	float m_r;

	//!��ת����Ԫ��
	H3DQuat m_roation;
	H3DQuat m_tmp_roation;

	//!��¼��ʼ��ת�ĵ�
	H3DVec3 m_BeginRoatePoint;
	H3DVec3 m_BeginTarget;

	H3DVec3 m_Pos;
	H3DVec3 m_ViewAtDir;
	H3DVec3 m_UpDir;

	float m_fDragTimer;
	H3DVec3 m_vVelocityDrag;
	H3DVec3 m_vVelocity;
	H3DVec3 m_vDeltaVelocity;
	bool m_bLeftHasDown;
	bool m_bRightHasDown;
	bool m_bMiddleHasDown;
	bool m_bMove;
	int m_mousepx;
	int m_mousepy;

	//�Ƿ�ѡ��ģ��
	bool m_selModel;

	float m_convertScale;
};