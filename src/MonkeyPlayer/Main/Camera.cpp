// Camera.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Contains view/projection matrices
#include "d3dUtil.h"

#include "Camera.h"

Camera::Camera()
{
	mPos = D3DXVECTOR3(0, 0, -10.0f);
	mTarget = D3DXVECTOR3(0, 0, 0);
	mUp =  D3DXVECTOR3(0, 1.0f, 0);
	mFov = .5;
	mNear = 1.0f;
	mFar = 500.0f;

	mScreenWidth = 800;
	mScreenHeight = 600;
	mAspect = (float)mScreenWidth / (float)mScreenHeight;

}
Camera::Camera(D3DXVECTOR3 pos, D3DXVECTOR3 target, D3DXVECTOR3 up,
		float fov, int screenWidth, int screenHeight, float nearPlane, float farPlane) 
{
	mPos = pos;
	mTarget = target;
	mUp = up;
	mFov = fov;
	mNear = nearPlane;
	mFar = farPlane;

	mAspect = (float)screenWidth / (float)screenHeight;
	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;

	build();
}

Camera::~Camera()
{
}

void Camera::configure(D3DXVECTOR3 pos, D3DXVECTOR3 target, D3DXVECTOR3 up)
{
	mPos = pos;
	mTarget = target;
	mUp = up;
	build();
}
void Camera::configure(D3DXVECTOR3 pos, D3DXVECTOR3 target, D3DXVECTOR3 up,
		float fov, int screenWidth, int screenHeight)
{
	mPos = pos;
	mTarget = target;
	mUp = up;
	mFov = fov;

	mAspect = (float)screenWidth / (float)screenHeight;
	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;

	build();
}

void Camera::build()
{
	// Sets up the camera 1000 units back looking at the origin.
	D3DXMatrixLookAtLH(&mView, &mPos, &mTarget, &mUp);

	D3DXMatrixPerspectiveFovLH(&mProjection, mFov, mAspect, mNear, mFar);
}

D3DXMATRIX Camera::getView()
{
	return mView;
}
D3DXMATRIX Camera::getProjection()
{
	return mProjection;
}
int Camera::getScreenWidth()
{
	return mScreenWidth;
}
int Camera::getScreenHeight()
{
	return mScreenHeight;
}
