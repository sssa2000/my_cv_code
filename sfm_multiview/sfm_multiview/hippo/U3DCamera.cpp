
#include "U3DCamera.h"
#include "CameraControlMap.h"
#include "Hippo_InputManager.h"
#include "HippoGlobal.h"
#include "engine_interface.h"
#include "HippoLog.h"
#include "dinput.h"
#include "HIPPO_STL_HEADER.h"

U3DCamera::U3DCamera()
{
	m_mousepx = 0;
	m_mousepy = 0;
	m_convertScale = 0;
	m_roation.Identity();
	m_r=1.0f;
	m_nHalafWidth=0;
	m_nHalafHeight=0;
	m_Target.Set(0,0,0);
	m_RotCenter.Set(0,0,0);
	m_BeginRoatePoint.Set(0,0,0);
	m_roation.Identity();
	m_tmp_roation.Identity();

	m_Pos.Set(0,0,0);
	m_ViewAtDir.Set(0,0,0);
	m_UpDir.Set(0,0,1);

	m_fDragTimer=0;
	m_vVelocityDrag.Set(0,0,0);
	m_vVelocity.Set(0,0,0);
	m_vDeltaVelocity.Set(0,0,0);
	m_bLeftHasDown=false;
	m_bRightHasDown=false;
	m_bMiddleHasDown=false;
	m_bMove=false;
	m_selModel = false;
	UpdateCameraCoord();
}

U3DCamera::~U3DCamera()
{

}

//from base
const H3DVec3& U3DCamera::GetPos()
{
	return m_Pos;
}

const H3DVec3& U3DCamera::GetViewAtDir()
{
	return m_ViewAtDir;

}

const H3DVec3& U3DCamera::GetUp()
{
	return m_UpDir;

}

void U3DCamera::SetPos(const H3DVec3& pos)
{
	m_r=(pos-m_Target).Length();
	UpdateCameraCoord();
}

void U3DCamera::SetLookAtDir(const H3DVec3& at)
{
	m_roation.GetRotationTo(INIT_VIEWAT,at);
	m_roation.Normalize();
	UpdateCameraCoord();
}

void U3DCamera::SetLookAtPos(const H3DVec3& at)
{
	H3DVec3 tmp=at-m_Pos; //dir是从pos射向目标
	tmp.Normalize();
	SetLookAtDir(tmp);
}
void U3DCamera::UpdateCameraCoord()
{
	const H3DQuat& q=GetRotation();

	m_ViewAtDir=INIT_VIEWAT * q;
	m_ViewAtDir.Normalize();

	m_Pos=m_Target-m_ViewAtDir * m_r;

	m_UpDir=INIT_UP * q;
	m_UpDir.Normalize();
}


void U3DCamera::SetWindow( int nWidth, int nHeigh)
{
	m_nHalafWidth=(int)(nWidth*0.5f);
	m_nHalafHeight=(int)(nHeigh*0.5f);
}


void U3DCamera::CalcVelocity(float fElapsedTime)
{
	m_vVelocity=m_vDeltaVelocity * fElapsedTime * 80.0f;

	if (m_bMove)
	{
		return;
	}

	
	if(m_vDeltaVelocity.LengthFast()>0)
	{
		static int timer=0;
		m_vDeltaVelocity*= 0.7f;
		++timer;
		if(timer>10)
		{
			timer=0;
			m_vDeltaVelocity.Set(0,0,0);
		}
	}


}
int U3DCamera::FrameUpdate(float fElapsedTime)
{
	//AnalysisKeyInput(fElapsedTime);
	//AnalysisMouseInput(fElapsedTime);
	CalcVelocity(fElapsedTime);
	UpdateCameraCoord();

	m_Target+=m_vVelocity;
	return 1;
}



void U3DCamera::LookAt(H3DI::IRender* pRender)
{
	pRender->LookAt(m_Pos,m_Pos+m_ViewAtDir,m_UpDir);
	pRender->UpdateCamera();


}

int U3DCamera::OnMouseLeftDown(HippoMouseEvent& e)
{
	m_tmp_roation=m_roation;
	m_mousepx = e.PosX;
	m_mousepy = e.PosY;
	m_BeginTarget = m_Target;
	m_bLeftHasDown=true;
	return 1;
}
int U3DCamera::OnMouseLeftUp(HippoMouseEvent& e)
{
	m_bLeftHasDown=false;
	return 1;
}

int U3DCamera::OnMouseRightDown(HippoMouseEvent& e)
{
	m_tmp_roation=m_roation;
	m_mousepx = e.PosX;
	m_mousepy = e.PosY;
	m_bRightHasDown=true;
	return 1;
}
int U3DCamera::OnMouseRightUp(HippoMouseEvent& e)
{
	m_bRightHasDown=false;
	return 1;
}

int U3DCamera::OnMouseMiddleDown(HippoMouseEvent& e)
{
	m_mousepx = e.PosX;
	m_mousepy = e.PosY;
	m_bMiddleHasDown=true;
	return 1;
}
int U3DCamera::OnMouseMiddleUp(HippoMouseEvent& e)
{
	m_bMiddleHasDown=false;
	return 1;
}
int U3DCamera::OnMouseMove(HippoMouseEvent& e)
{
	if (GetKeyState(VK_LBUTTON) >= 0)
	{
		m_bLeftHasDown = false;
	}

	if (GetKeyState(VK_RBUTTON) >= 0)
	{
		m_bRightHasDown = false;
	}

	if (GetKeyState(VK_MBUTTON) >= 0)
	{
		m_bMiddleHasDown = false;
	}

	if (!(m_bLeftHasDown || m_bRightHasDown || m_bMiddleHasDown))
	{
		return 0;
	}

	if(e.PosX > 65000)
	{
		e.PosX = e.PosX - 65535;
	}

	if(e.PosY > 65000)
	{
		e.PosY = e.PosY - 65535;
	}

	if(m_bLeftHasDown && (GetKeyState(VK_MENU)<0))
	{
		int dx = e.PosX - m_mousepx;
		int dy = e.PosY - m_mousepy;

		m_mousepx = e.PosX;
		m_mousepy = e.PosY;

		H3DAngles rota;
		rota.Set(0,0,0);
		rota.yaw += dx * 0.3f;

		m_roation = rota.ToQuat() * m_roation;
		
		rota.Set(0,0,0);
		rota.roll += dy * 0.3f;

		m_roation = m_roation * rota.ToQuat();

		m_roation.Normalize();

		if (m_selModel)
		{
			m_Target = (m_BeginTarget - m_RotCenter) * m_tmp_roation.Inverse() * m_roation + m_RotCenter;
		}

		return 1;
	}

	//
	if (m_bRightHasDown)
	{
		int dx = e.PosX - m_mousepx;
		int dy = e.PosY - m_mousepy;
		m_mousepx = e.PosX;
		m_mousepy = e.PosY;

		H3DAngles rota;
		rota.Set(0,0,0);
		rota.yaw += dx * 0.06f;
		rota.roll += dy * 0.06f;

		m_roation = m_roation * rota.ToQuat();

		m_roation.Normalize();

		m_ViewAtDir = INIT_VIEWAT * m_roation;

		m_Target = m_ViewAtDir * m_r + m_Pos + m_vVelocity;

		return 1;
	}

	if (m_bMiddleHasDown)
	{
		int dx = e.PosX - m_mousepx;
		int dy = e.PosY - m_mousepy;
		m_mousepx = e.PosX;
		m_mousepy = e.PosY;
		H3DVec3 move;
		move.Set(-dx * m_r * 0.00025f,0,dy * m_r * 0.00025f);
		move = m_roation * move;
		m_vDeltaVelocity += move;
	}

	return 0;	
}
int U3DCamera::OnKeyDown(unsigned int k)
{
	m_vDeltaVelocity.Set(0,0,0);
	if (!m_bRightHasDown)
	{
		return 0;
	}
	if( GetKeyState(Camera_ControlMap::GetFPSCameraKeyCodeFromLogicKey(CAM_MOVE_UP)) < 0)
	{
		m_vDeltaVelocity.z = 0.08f;
	}
	else if( GetKeyState(Camera_ControlMap::GetFPSCameraKeyCodeFromLogicKey(CAM_MOVE_DOWN)) < 0)
	{
		m_vDeltaVelocity.z = -0.08f;
	}
	else if( GetKeyState(Camera_ControlMap::GetFPSCameraKeyCodeFromLogicKey(CAM_MOVE_FORWARD)) < 0)
	{
		m_vDeltaVelocity.y = 0.08f;
	}
	else if( GetKeyState(Camera_ControlMap::GetFPSCameraKeyCodeFromLogicKey(CAM_MOVE_BACKWARD)) < 0)
	{
		m_vDeltaVelocity.y = -0.08f;
	}
	else if( GetKeyState(Camera_ControlMap::GetFPSCameraKeyCodeFromLogicKey(CAM_STRAFE_LEFT)) < 0)
	{
		m_vDeltaVelocity.x = -0.08f;
	}
	else if( GetKeyState(Camera_ControlMap::GetFPSCameraKeyCodeFromLogicKey(CAM_STRAFE_RIGHT)) < 0)
	{
		m_vDeltaVelocity.x = 0.08f;
	}
	if (GetKeyState(VK_SHIFT) < 0)
	{
		m_vDeltaVelocity = m_vDeltaVelocity * 2.0f;
	}
	m_vDeltaVelocity = m_roation * m_vDeltaVelocity;
	m_bMove = true;
	return 1;
}
int U3DCamera::OnKeyUp(unsigned int k)
{
	if (k == VK_SHIFT)
	{
		m_vDeltaVelocity = m_vDeltaVelocity * 0.5f;
		return 0;
	}
	m_bMove = false;
	return 1;
}
int U3DCamera::OnMouseWheel(HippoWheelEvent& w)
{
	int delta=w.wheel_delta;
	m_r -= delta * m_r * 0.5f / 240.0f;

	return 1;
}
MouseKeyCallback U3DCamera::GetMouseLeftDownCallback()
{
	MouseKeyCallback cb= boostl_tr1::bind(&U3DCamera::OnMouseLeftDown,this,_1);
	return cb;
}
MouseKeyCallback U3DCamera::GetMouseMiddleDownCallback()
{
	MouseKeyCallback cb= boostl_tr1::bind(&U3DCamera::OnMouseMiddleDown,this,_1);
	return cb;
}
MouseKeyCallback U3DCamera::GetMouseRightDownCallback()
{
	MouseKeyCallback cb= boostl_tr1::bind(&U3DCamera::OnMouseRightDown,this,_1);
	return cb;
}
MouseKeyCallback U3DCamera::GetMouseMoveCallback()
{
	MouseKeyCallback cb= boostl_tr1::bind(&U3DCamera::OnMouseMove,this,_1);
	return cb;
}
MouseKeyCallback U3DCamera::GetMouseLeftUpCallback()
{
	MouseKeyCallback cb= boostl_tr1::bind(&U3DCamera::OnMouseLeftUp,this,_1);
	return cb;
}
MouseKeyCallback U3DCamera::GetMouseMiddleUpCallback()
{
	MouseKeyCallback cb= boostl_tr1::bind(&U3DCamera::OnMouseMiddleUp,this,_1);
	return cb;
}
MouseKeyCallback U3DCamera::GetMouseRightUpCallback()
{
	MouseKeyCallback cb= boostl_tr1::bind(&U3DCamera::OnMouseRightUp,this,_1);
	return cb;
}
MouseWheelCallback U3DCamera::GetMouseWheelCallback()
{
	MouseWheelCallback cb= boostl_tr1::bind(&U3DCamera::OnMouseWheel,this,_1);
	return cb;
}
KeyCallback U3DCamera::GetKeyDownCallback()
{
	KeyCallback cb= boostl_tr1::bind(&U3DCamera::OnKeyDown,this,_1);
	return cb;
}

KeyCallback U3DCamera::GetKeyUpCallback()
{
	KeyCallback cb= boostl_tr1::bind(&U3DCamera::OnKeyUp,this,_1);
	return cb;
}
void U3DCamera::SyncCamera(CameraBase* pCam)
{
	SetPos(pCam->GetPos());
	SetLookAtDir(pCam->GetViewAtDir());
	UpdateCameraCoord();	
}