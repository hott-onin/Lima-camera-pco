/**************************************************************************
###########################################################################
 This file is part of LImA, a Library for Image Acquisition

 Copyright (C) : 2009-2011
 European Synchrotron Radiation Facility
 BP 220, Grenoble 38043
 FRANCE

 This is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This software is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, see <http://www.gnu.org/licenses/>.
###########################################################################
**************************************************************************/
#include <cstdlib>
#include <process.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <time.h>

#include "lima/Exceptions.h"
#include "lima/HwSyncCtrlObj.h"

#include "PcoCamera.h"
#include "PcoSyncCtrlObj.h"

using namespace lima;
using namespace lima::Pco;







//=================================================================================================
//=================================================================================================
//SC2_SDK_FUNC int WINAPI PCO_SetImageParameters(HANDLE ph, WORD wxres, WORD wyres, DWORD dwflags, void* param, int ilen);
// Necessary while using a soft-roi enabled interface. It is recommended to use this function
// with all interface types of pco when soft-roi is enabled. This function can be used as a replacement for
// PCO_CamLinkSetImageParameters
// If there is a change in buffer size (ROI, binning) and/or ROI relocation this function must
// be called with the new x and y resolution. Additionally this function has to be called,
// if you switch to another camera internal memory segment with different x and y size or ROI and like to get images.
// In: HANDLE ph -> Handle to a previously opened camera.
//     WORD wxres -> X Resolution of the images to be transferred
//     WORD wyres -> Y Resolution of the images to be transferred
//     DWORD dwflags -> Flags to select the correct soft-ROI for interface preparation
//                      Set IMAGEPARAMETERS_READ_FROM_SEGMENTS when the next image operation will read images
//                      from one of the camera internal memory segments (if available).
//                      Set IMAGEPARAMETERS_READ_WHILE_RECORDING when the next image operation is a recording
//     void* param -> Pointer to a structure for future use (set to NULL); Currently not used.
//     int ilen -> int to hold the length of the param structure for future use (set to 0); Currently not used.
// Out: int -> Error message.

char *Camera::_pco_SetImageParameters(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	char *pcoFn;
    WORD wXres, wYres;

	void *param = NULL;
	int iLenParam = 0;
	DWORD dwFlags = 0;

	dwFlags = _isCameraType(Edge) ? 
					IMAGEPARAMETERS_READ_WHILE_RECORDING :
					IMAGEPARAMETERS_READ_FROM_SEGMENTS;

    wXres= m_pcoData->wXResActual;
    wYres= m_pcoData->wYResActual;
	
	DEB_TRACE() << "PCO_SetImageParameters: " << DEB_VAR3(wXres, wYres, dwFlags);

	//error = PCO_SetImageParameters(m_handle, wXres, wYres, dwFlags, param, iLenParam);

	PCO_FN6(error, pcoFn,PCO_SetImageParameters, m_handle, wXres, wYres, dwFlags, param, iLenParam);

	if(error) 
	{ 
		DEB_ALWAYS() << "ERROR: \n" << DEB_VAR2(pcoFn, error);
		throw LIMA_HW_EXC(Error, pcoFn); 
	}

	return fnId;
}











//=================================================================================================
//=================================================================================================
char* Camera::_pco_SetDelayExposureTime(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	char *msg;

    DWORD dwExposure, dwDelay;
	WORD wExposure_base, wDelay_base;
    double _exposure, _delay;
    m_sync->getExpTime(_exposure);
    m_sync->getLatTime(_delay);
	double _delay0 = _delay;

	_pco_time2dwbase(_exposure, dwExposure, wExposure_base);
	_pco_time2dwbase(_delay,  dwDelay, wDelay_base);

	PCO_FN5(error, msg,PCO_SetDelayExposureTime, m_handle, dwDelay, dwExposure, wDelay_base, wExposure_base);

	m_pcoData->traceAcq.dLimaExposure = _exposure;
	m_pcoData->traceAcq.dLimaDelay = _delay;

	m_pcoData->traceAcq.iPcoExposure = dwExposure;
	m_pcoData->traceAcq.iPcoDelay = dwDelay;
	m_pcoData->traceAcq.iPcoExposureBase = wExposure_base;
	m_pcoData->traceAcq.iPcoDelayBase = wDelay_base;


	if(error || _getDebug(DBG_EXP)) {
		DEB_ALWAYS() << DEB_VAR3(_exposure, dwExposure, wExposure_base);
		DEB_ALWAYS() << DEB_VAR3(_delay,  dwDelay, wDelay_base);
	}

	if(error) {
		return msg;
	}

	return fnId;
}





	
//=================================================================================================
//=================================================================================================

void Camera::_pco_GetTransferParameter(void* buffer, int ilen, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	char *pcoFn;

	err = 0;
	PCO_FN3(err, pcoFn,PCO_GetTransferParameter, m_handle, buffer, ilen);
	return;
}


//=================================================================================================
//=================================================================================================

void Camera::_pco_FillStructures(int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//char *pcoFn;
	int pcoErr;
	err = 0;

	// PCO_GETGENERAL(hCam, &strGeneral)

	// -- Get General
	m_pcoData->stcPcoGeneral.wSize= sizeof(m_pcoData->stcPcoGeneral);
	m_pcoData->stcPcoGeneral.strCamType.wSize= sizeof(m_pcoData->stcPcoGeneral.strCamType);

	pcoErr = PCO_GetGeneral(m_handle, &m_pcoData->stcPcoGeneral);
	err |= PCO_CHECK_ERROR(pcoErr, "PCO_GetGeneral");

	// PCO_GETCAMERATYPE(hCam, &strCamType)

	m_pcoData->stcPcoCamType.wSize = sizeof(m_pcoData->stcPcoCamType);
	pcoErr = PCO_GetCameraType(m_handle, &m_pcoData->stcPcoCamType);
	err |= PCO_CHECK_ERROR(pcoErr, "PCO_GetCameraType");

	// PCO_GETSENSORSTRUCT(hCam, &strSensor)

	// -- Get Sensor struct
	m_pcoData->stcPcoSensor.wSize= sizeof(m_pcoData->stcPcoSensor);
	m_pcoData->stcPcoSensor.strDescription.wSize= sizeof(m_pcoData->stcPcoSensor.strDescription);
	m_pcoData->stcPcoSensor.strDescription2.wSize= sizeof(m_pcoData->stcPcoSensor.strDescription2);

	pcoErr = PCO_GetSensorStruct(m_handle, &m_pcoData->stcPcoSensor);
	err |= PCO_CHECK_ERROR(pcoErr, "PCO_GetSensorStruct");


	// PCO_GETCAMERADESCRIPTION(hCam, &strDescription)

	// -- Get camera description
	m_pcoData->stcPcoDescription.wSize= sizeof(m_pcoData->stcPcoDescription);

	pcoErr = PCO_GetCameraDescription(m_handle, &m_pcoData->stcPcoDescription);
	err |= PCO_CHECK_ERROR(pcoErr, "PCO_GetCameraDescription"); 	

	// PCO_GETTIMINGSTRUCT(hCam, &strTiming)

	// -- Get timing struct
	m_pcoData->stcPcoTiming.wSize= sizeof(m_pcoData->stcPcoTiming);

	pcoErr = PCO_GetTimingStruct(m_handle, &m_pcoData->stcPcoTiming);
	err |= PCO_CHECK_ERROR(pcoErr, "PCO_GetTimingStruct");

	// PCO_GETRECORDINGSTRUCT(hCam, &strRecording)

	// -- Get recording struct
	m_pcoData->stcPcoRecording.wSize= sizeof(m_pcoData->stcPcoRecording);

	pcoErr = PCO_GetRecordingStruct(m_handle, &m_pcoData->stcPcoRecording);
	err |= PCO_CHECK_ERROR(pcoErr, "PCO_GetRecordingStruct");


	// -- Get storage struct
	m_pcoData->stcPcoStorage.wSize= sizeof(m_pcoData->stcPcoStorage);

	pcoErr = PCO_GetStorageStruct(m_handle, &m_pcoData->stcPcoStorage);
	err |= PCO_CHECK_ERROR(pcoErr, "PCO_GetStorageStruct");

}




//=================================================================================================
//=================================================================================================
//SC2_SDK_FUNC int WINAPI PCO_GetBinning(HANDLE ph, WORD* wBinHorz, WORD* wBinVert)

void Camera::_pco_GetBinning(Bin &bin, int &err)
{
	DEB_MEMBER_FUNCT();

	WORD wBinHorz, wBinVert;

#ifndef __linux__
	err = PCO_GetBinning(m_handle, &wBinHorz, &wBinVert);

	if(PCO_CHECK_ERROR(err, "PCO_GetBinning"))
	{
		wBinHorz, wBinVert = 1;
		DEB_ALWAYS() << "ERROR - PCO_GetBinning";
	}

#else

#endif

	bin = Bin(wBinHorz, wBinVert);

}


//=================================================================================================
//=================================================================================================
int Camera::_binning_fit(int binRequested, int binMax, int binMode)
{
	int binLast, bin;

	if(binRequested < 1) return 1;
	if(binRequested >= binMax) return binMax;

	binLast = bin = 1;

	while(true)
	{
		if(bin == binRequested) return bin;
		if(bin > binRequested) return binLast;
		binLast = bin;
		bin = binMode ? bin+1 : bin*2;
		if(bin > binMax) return binLast;
	}
}



//=================================================================================================
//=================================================================================================
//SC2_SDK_FUNC int WINAPI PCO_SetBinning(HANDLE ph, WORD wBinHorz, WORD wBinVert)

void Camera::_pco_SetBinning(Bin binNew, Bin &binActual, int &err)
{
	DEB_MEMBER_FUNCT();

	WORD wBinHorz, wBinVert;
	Bin binOld;
	int err0;

	_pco_GetBinning(binOld, err);

	if(binOld == binNew)
	{
		binActual = binOld;
		err = 0;
		return;
	}
	wBinHorz = binNew.getX();
	wBinVert = binNew.getY();

#ifndef __linux__
	err = PCO_SetBinning(m_handle, wBinHorz, wBinVert);

	if(PCO_CHECK_ERROR(err, "PCO_SetBinning"))
	{
		DEB_ALWAYS() << "ERROR - PCO_SetBinning";
	}

#else

#endif

	_pco_GetBinning(binActual, err0);

}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetBinningInfo(char *buf_in, int size_in, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

	char *ptr = buf_in;
	char *ptrMax = ptr + size_in;

	Bin aBin;
	
	int binX, binY, binMaxX, binModeX, binMaxY, binModeY;
	_pco_GetBinning(aBin, err);

	binX = aBin.getX();
	binY = aBin.getY();

#ifndef __linux__
	binMaxX = m_pcoData->stcPcoDescription.wMaxBinHorzDESC;
	binModeX = m_pcoData->stcPcoDescription.wBinHorzSteppingDESC;

	binMaxY = m_pcoData->stcPcoDescription.wMaxBinVertDESC;
	binModeY = m_pcoData->stcPcoDescription.wBinVertSteppingDESC;
#else
	error = -1;
	*buf_in = 0;
	DEB_ALWAYS() << "ERROR - NOT IMPLEMENTED!" ;
#endif

	ptr += sprintf_s(ptr, ptrMax - ptr, 
			"bin[%d,%d] binMax[%d,%d] binStepMode[%d,%d][%s,%s]",
			binX, binY, binMaxX, binMaxY,
			binModeX, binModeY, 
			binModeX ? "lin" : "bin",
			binModeY ? "lin" : "bin");

}

//=================================================================================================
//=================================================================================================

void Camera::_pco_GetROI(Roi &roi, int &err)
{
	DEB_MEMBER_FUNCT();

	WORD wX0, wY0, wX1, wY1;

#ifndef __linux__
	err = PCO_GetROI(m_handle, &wX0, &wY0, &wX1, &wY1);

	if(PCO_CHECK_ERROR(err, "PCO_GetROI"))
	{
		DEB_ALWAYS() << "ERROR - PCO_GetROI";
	}

#else

#endif

	_xlatRoi_pco2lima(roi, wX0, wX1, wY0, wY1 );
}

void Camera::_pco_SetROI(Roi roi, int &err)
{
	DEB_MEMBER_FUNCT();

	unsigned int  uiX0, uiY0, uiX1, uiY1;

	_xlatRoi_lima2pco(roi, uiX0, uiX1, uiY0, uiY1 );

#ifndef __linux__

	err = PCO_SetROI(m_handle, uiX0, uiY0, uiX1, uiY1);

	if(PCO_CHECK_ERROR(err, "PCO_SetROI"))
	{
		DEB_ALWAYS() << "ERROR - PCO_SetROI";
	}

#else

#endif

}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetRoiInfo(char *buf_in, int size_in, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

	char *ptr = buf_in;
	char *ptrMax = ptr + size_in;

	Roi roi;
	
	unsigned int x0, x1, y0, y1;
	_pco_GetROI(roi, err);

	_xlatRoi_lima2pco(roi, x0, x1, y0, y1);


#ifndef __linux__
	//
#else
	error = -1;
	*buf_in = 0;
	DEB_ALWAYS() << "ERROR - NOT IMPLEMENTED!" ;
#endif

	ptr += sprintf_s(ptr, ptrMax - ptr, 
			"pco[x<%d,%d> y<%d,%d>] lima[<%d,%d>-<%dx%d>]",
			x0, x1, 
			y0, y1,
			roi.getTopLeft().x,roi.getTopLeft().y,
			roi.getSize().getWidth(), roi.getSize().getHeight());

}









