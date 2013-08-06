// Camera.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Contains view/projection matrices

#include <d3dx9.h>

#ifndef CAMERA_H
#define CAMERA_H

class Camera
{

public:

	Camera();
	Camera(D3DXVECTOR3 pos, D3DXVECTOR3 target, D3DXVECTOR3 up,
			float fov, int screenWidth, int screenHeight, float nearPlane, float farPlane);
	~Camera();

	void configure(D3DXVECTOR3 pos, D3DXVECTOR3 target, D3DXVECTOR3 up);
	void configure(D3DXVECTOR3 pos, D3DXVECTOR3 target, D3DXVECTOR3 up,
			float fov, int screenWidth, int screenHeight);
	D3DXMATRIX getView();
	D3DXMATRIX getProjection();

	int getScreenWidth();
	int getScreenHeight();


private:

	void build();

	D3DXVECTOR3 mPos;
	D3DXVECTOR3 mTarget;
	D3DXVECTOR3 mUp;

	float mFov;
	float mAspect;
	float mNear;
	float mFar;

	D3DXMATRIX mView;
	D3DXMATRIX mProjection;

	int mScreenWidth;
	int mScreenHeight;

};

#endif
