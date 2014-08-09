#pragma once

#ifdef _DEBUG
#include <crtdbg.h>
#define TRACE_F(fmt, ...) _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, NULL, fmt, __VA_ARGS__);
#else
#define TRACE_F(fmt, ...)
#endif

#include <vector>
#include <Windows.h>
#include <Kinect.h>
#include <opencv2\opencv.hpp>

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

class CKinectFrame
{
	static const int cColorWidth = 1920;
	static const int cColorHeight = 1080;

public:
	CKinectFrame();
	~CKinectFrame();

	void Run();

private:
	IKinectSensor* m_pKinectSensor;
	IDepthFrameReader* m_pDepthFrameReader;
	IColorFrameReader* m_pColorFrameReader;
	IInfraredFrameReader* m_pInfraredFrameReader;

	RGBQUAD* m_pColorRGBX;

	HRESULT InitializeDefaultSensor();

	void Update();
	void UpdateColor();
	void UpdateDepth();
	void UpdateInfrared();	
	
	void ProcessColor(INT64 nTime, RGBQUAD* pBuffer, int nHeight, int nWidth);
	void ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth, USHORT nMinDepth, USHORT nMaxDepth);
	void ProcessInfrared(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth);

};

