#include "KinectFrame.h"


CKinectFrame::CKinectFrame()
: m_pKinectSensor(NULL)
, m_pColorFrameReader(NULL)
, m_pDepthFrameReader(NULL)
, m_pInfraredFrameReader(NULL)
{
	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];
}


CKinectFrame::~CKinectFrame()
{
	SafeRelease(m_pColorFrameReader);
	SafeRelease(m_pDepthFrameReader);
	SafeRelease(m_pInfraredFrameReader);

	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}
	SafeRelease(m_pKinectSensor);
}

void CKinectFrame::Run()
{
	HRESULT hr = InitializeDefaultSensor();

	if (SUCCEEDED(hr))
	{
		int key = 0;
		while (key != 27)
		{
			Update();

			key = cv::waitKey(1);
		}
	}
}

void CKinectFrame::Update()
{
	if (!m_pDepthFrameReader)
	{
		return;
	}

	UpdateColor();
	UpdateDepth();
	UpdateInfrared();
}

void CKinectFrame::UpdateColor()
{
	if (!m_pColorFrameReader)
	{
		return;
	}

	IColorFrame* pColorFrame = NULL;

	HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

	if (SUCCEEDED(hr))
	{

		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nBufferSize = 0;
		RGBQUAD *pBuffer = NULL;

		hr = pColorFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
			}
			else if (m_pColorRGBX)
			{
				pBuffer = m_pColorRGBX;
				nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
			}
		}

		if (SUCCEEDED(hr))
		{
			ProcessColor(nTime, pBuffer, nHeight, nWidth);
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pColorFrame);
}

void CKinectFrame::UpdateDepth()
{
	if (!m_pDepthFrameReader)
	{
		return;
	}

	IDepthFrame* pDepthFrame = NULL;

	HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		USHORT nDepthMinReliableDistance = 0;
		USHORT nDepthMaxReliableDistance = 0;
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;
		hr = pDepthFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxReliableDistance);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
		}

		if (SUCCEEDED(hr))
		{
			ProcessDepth(nTime, pBuffer, nHeight, nWidth, nDepthMinReliableDistance, nDepthMaxReliableDistance);
		}

		SafeRelease(pFrameDescription);
	}
	SafeRelease(pDepthFrame);
}

void CKinectFrame::UpdateInfrared()
{
	if (!m_pInfraredFrameReader)
	{
		return;
	}

	IInfraredFrame* pInfraredFrame = NULL;

	HRESULT hr = m_pInfraredFrameReader->AcquireLatestFrame(&pInfraredFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;

		hr = pInfraredFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = pInfraredFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pInfraredFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
		}

		if (SUCCEEDED(hr))
		{
			ProcessInfrared(nTime, pBuffer, nHeight, nWidth);
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pInfraredFrame);
}

HRESULT CKinectFrame::InitializeDefaultSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);

	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		IColorFrameSource* pColorFrameSource = NULL;
		IDepthFrameSource* pDepthFrameSource = NULL;
		IInfraredFrameSource* pInfraredFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		// ColorFrame
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
		}

		// DepthFrame
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
		}

		// InfraredFrame
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_InfraredFrameSource(&pInfraredFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pInfraredFrameSource->OpenReader(&m_pInfraredFrameReader);
		}

		SafeRelease(pColorFrameSource);
		SafeRelease(pDepthFrameSource);
		SafeRelease(pInfraredFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		return E_FAIL;
	}

	return hr;

}

void CKinectFrame::ProcessColor(INT64 nTime, RGBQUAD* pBuffer, int nHeight, int nWidth)
{
	cv::Mat ColorFrame(nHeight, nWidth, CV_8UC4, reinterpret_cast<unsigned char *>(pBuffer));
	cv::imshow("ColorFrame", ColorFrame);
}

void CKinectFrame::ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth, USHORT nMinDepth, USHORT nMaxDepth)
{
	cv::Mat DepthFrame(nHeight, nWidth, CV_16UC1, const_cast<unsigned short *>(pBuffer));
	cv::Mat DepthFrame32FC1(nHeight, nWidth, CV_32FC1);
	DepthFrame.convertTo(DepthFrame32FC1, CV_32FC1, 1.0 / (double)nMaxDepth);
	cv::imshow("DepthFrame", DepthFrame32FC1);
}

void CKinectFrame::ProcessInfrared(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth)
{
	cv::Mat InfraredFrame(nHeight, nWidth, CV_16UC1, (unsigned short *)pBuffer);
	cv::Mat InfraredFrame32FC1(nHeight, nWidth, CV_32FC1);
	//InfraredFrame.convertTo(InfraredFrame32FC1, CV_32FC1, 1.0 / (double))
	cv::imshow("InfraredFrame", InfraredFrame);
}
