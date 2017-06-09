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

#ifndef __linux__
#include <process.h>
#endif

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

// 4294967295.0 = double(DWORD(0xFFFFFFFF)) 
#define DWORD_MAX_FLOAT 4294967295.0

#define	MAX_DWORD_MS (double(4294967295.0e-3))
#define	MAX_DWORD_US (double(4294967295.0e-6))
#define	MAX_DWORD_NS (double(4294967295.0e-9))


void _pco_time2dwbase(double exp_time, DWORD &dwExp, WORD &wBase) {
	// conversion time(s) to PCO standard DWORD + UNIT(ms, us, ns)
		// exp & lat time is saved in seconds (LIMA). 
		// PCO requires them expressed in DWORD as ms(base=2), us(base=1) or ns(base=0)
		// max DWORD 0xFFFFFFFF = 4294967295.0
		// find the lowest unit (ns -> us -> ms) which does not overflow DWORD
    
	if(exp_time <= MAX_DWORD_NS) {   
		dwExp = DWORD(exp_time * 1.0e9); wBase = 0; // ns
	} else 	if(exp_time <= MAX_DWORD_US) {  
		dwExp = DWORD(exp_time * 1.0e6); wBase = 1; // us
	} else {  
		dwExp = DWORD(exp_time * 1.0e3); wBase = 2; // ms
	}

	DWORD mask = 0x7;
	DWORD min = 0x1000;

	if(dwExp > min){
		dwExp |= mask;
		dwExp ^= mask;
	}

	return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetAcqEnblSignalStatus(WORD &wAcquEnableState, int &error)
{

	DEB_MEMBER_FUNCT();
	DEF_FNID;
	WORD _wAcquEnableState;



#ifdef __linux__
    error = camera->PCO_GetAcqEnblSignalStatus(&_wAcquEnableState);   
    PCO_CHECK_ERROR(error, "PCO_GetAcqEnblSignalStatus");

#else
	error = PcoCheckError(__LINE__, __FILE__, PCO_GetAcqEnblSignalStatus(m_handle, &_wAcquEnableState));

#endif

    if(error) return;
    
    wAcquEnableState = _wAcquEnableState;
}

//=================================================================================================
//=================================================================================================

double Camera::pcoGetCocRunTime()
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
    return m_pcoData->cocRunTime;
}

double Camera::pcoGetFrameRate()
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	
	return m_pcoData->frameRate;

}


//=================================================================================================
//=================================================================================================
void Camera::_pco_GetCOCRuntime(int &error){
		
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg;

	//====================================== get the coc runtime 
    //---- only valid if it was used PCO_SetDelayExposureTime
	//---- and AFTER armed the cam

	// Get and split the 'camera operation code' runtime into two DWORD. One will hold the longer
	// part, in seconds, and the other will hold the shorter part, in nanoseconds. This function can be
	// used to calculate the FPS. The sum of dwTime_s and dwTime_ns covers the delay, exposure and
	// readout time. If external exposure is active, it returns only the readout time.

	DWORD dwTime_s, dwTime_ns;
    double runTime;

#ifndef __linux__
	PCO_FN3(error, msg,PCO_GetCOCRuntime, m_handle, &dwTime_s, &dwTime_ns);
    PCO_PRINT_ERR(error, msg); 	if(error) return;
#else

    error=camera->PCO_GetCOCRuntime(&dwTime_s, &dwTime_ns);
    msg = "PCO_GetCOCRuntime" ; PCO_CHECK_ERROR(error, msg);
    if(error) return;
#endif


    m_pcoData->cocRunTime = runTime = ((double) dwTime_ns * NANO) + (double) dwTime_s;
    m_pcoData->frameRate = (dwTime_ns | dwTime_s) ? 1.0 / runTime : 0.0;

	
	
	DEB_TRACE() << DEB_VAR2(m_pcoData->frameRate, m_pcoData->cocRunTime);

	return;

}


//=================================================================================================
//=================================================================================================
void Camera::_pco_GetFirmwareInfo(WORD wDeviceBlock, PCO_FW_Vers* pstrFirmWareVersion, int &error)
{
#ifndef __linux__
		error =  PCO_GetFirmwareInfo(m_handle, wDeviceBlock, pstrFirmWareVersion);
#else
        error = -1;
#endif
}



//=================================================================================================
//=================================================================================================
void Camera::_pco_SetTimestampMode(WORD mode, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    WORD modeNew, modeOld;


#ifndef __linux__
	char *msg;
	int error;
	err = error = 0;

    WORD modeMax;
    
	DWORD capsDesc1; 
	_pco_GetGeneralCapsDESC(capsDesc1, error);

	if(capsDesc1 & BIT8)
	{
	    DEB_ALWAYS() << "\n   timestampmode not allowed" ;
		err = -1;
		return;
	}
	modeMax = (capsDesc1 & BIT3) ? 3 : 2;
	if(mode > modeMax)
	{
	    DEB_ALWAYS() << "\n  invalid value" << DEB_VAR2(mode, modeMax);
		err = -1;
		return;
	}

	_pco_GetTimestampMode(modeOld, err);
	if(err) return;

	if(mode == modeOld)
	{
	    DEB_TRACE()<< "\n   no change " << DEB_VAR2(mode, modeOld); 
		return;
	}

	PCO_FN2(error, msg,PCO_SetTimestampMode,m_handle, mode);
	PCO_PRINT_ERR(error, msg); 	
	err |= error;
	if(err) return;

	_pco_GetTimestampMode(modeNew, err);
	if(err) return;

#else

    err=camera->PCO_GetTimestampMode(&modeOld);
    PCO_CHECK_ERROR(err, "PCO_GetTimestampMode"); 

    err=camera->PCO_SetTimestampMode(mode);
    PCO_CHECK_ERROR(err, "PCO_SetTimestampMode"); 

    err=camera->PCO_GetTimestampMode(&modeNew);
    PCO_CHECK_ERROR(err, "PCO_GetTimestampMode"); 

#endif

	DEB_TRACE() 
        << "\n   " << DEB_VAR3(mode, modeOld, modeNew) 
        ;

    return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetTimestampMode(WORD &mode, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

#ifndef __linux__
	char *msg;
	int error;
	err = error = 0;
    
	PCO_FN2(error, msg,PCO_GetTimestampMode,m_handle, &mode);
	PCO_PRINT_ERR(error, msg); 	
	err |= error;
	if(err) return;
#else

    err=camera->PCO_GetTimestampMode(&mode);
    PCO_CHECK_ERROR(err, "PCO_GetTimestampMode"); 

#endif

	DEB_TRACE() 
        << "\n   " << DEB_VAR1(mode) 
        ;
    return;
}
//=================================================================================================
//=================================================================================================
WORD Camera::_pco_GetRecordingState(int &err){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
    WORD wRecState_actual;
    

#ifndef __linux__	
	char *msg;
	int error;
	msg = _PcoCheckError(__LINE__, __FILE__, 
		PCO_GetRecordingState(m_handle, &wRecState_actual), error);
	
	err = error;

	if(error) {
		printf("=== %s [%d]> ERROR %s\n", fnId, __LINE__, msg);
		throw LIMA_HW_EXC(Error, "PCO_GetRecordingState");
	}
#else
	err=camera->PCO_GetRecordingState(&wRecState_actual);
    PCO_CHECK_ERROR(err, "PCO_GetRecordingState");
#endif


	return wRecState_actual;

}
//=================================================================================================
//=================================================================================================

/**************************************************************************************************
	If a set recording status = [stop] command is sent and the current status is already
	[stop]’ped, nothing will happen (only warning, error message). 
	
	If the camera is in
	[run]’ing state, it will last some time (system delay + last image readout), until the
	camera is stopped. The system delay depends on the PC and the image readout
	depends on the image size transferred. The SetRecordingState = [stop] checks for a
	stable stop state by calling GetRecordingState.  --- 165 ms 
	
	Please call PCO_CancelImages to remove pending buffers from the driver.   --- 1.5 s
**************************************************************************************************/

const char * Camera::_pco_SetRecordingState(int state, int &err){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	LARGE_INTEGER usStart;


	WORD wRecState_new, wRecState_actual;

	wRecState_new = state ? 0x0001 : 0x0000 ; // 0x0001 => START acquisition

	usElapsedTimeSet(usStart);

    wRecState_actual = _pco_GetRecordingState(err);

#ifndef __linux__
	char *msg;
	

	_setCameraState(CAMSTATE_RECORD_STATE, !!(wRecState_actual));

	m_pcoData->traceAcq.usTicks[8].value = usElapsedTime(usStart);
	m_pcoData->traceAcq.usTicks[8].desc = "PCO_GetRecordingState execTime";
	usElapsedTimeSet(usStart);
#else
	//_setCameraState(CAMSTATE_RECORD_STATE, !!(wRecState_actual));

	m_pcoData->traceAcq.usTicks[traceAcq_GetRecordingState].value = usElapsedTime(usStart);
	m_pcoData->traceAcq.usTicks[traceAcq_GetRecordingState].desc = "PCO_GetRecordingState execTime";
	usElapsedTimeSet(usStart);
#endif

	//if(wRecState_new == wRecState_actual) {error = 0; return fnId; }

	// ------------------------------------------ cancel images 
	if(wRecState_new == 0) {
		int count = 1;

		_setCameraState(CAMSTATE_RECORD_STATE, false);


#if 0
		PCO_FN2(error, msg,PCO_GetPendingBuffer, m_handle, &count);
		PCO_PRINT_ERR(error, msg); 	if(error) return msg;
#endif

#ifndef __linux__
		if(count) {
			DEB_TRACE() << ":  PCO_CancelImages() - CALLING";
			PCO_FN1(err, msg,PCO_CancelImages, m_handle);
			PCO_PRINT_ERR(err, msg); 	if(err) return msg;
		} else {
			DEB_TRACE() << ":  PCO_CancelImages() - BYPASSED";
		}
	}

	if(wRecState_new != wRecState_actual) 
	{
		DEB_TRACE() << fnId << ": PCO_SetRecordingState " << DEB_VAR1(wRecState_new);
		PCO_FN2(err, msg,PCO_SetRecordingState, m_handle, wRecState_new);
		PCO_PRINT_ERR(err, msg); 	if(err) return msg;
	}

	if(wRecState_new) 
		m_sync->setExposing(pcoAcqRecordStart);

#else
if(count && !_isCameraType(Edge)) 
		{
			DEB_TRACE() << fnId << ": PCO_CancelImages";

            err=camera->PCO_CancelImage();
            PCO_CHECK_ERROR(err, "PCO_CancelImage");

            err=camera->PCO_CancelImageTransfer();
            PCO_CHECK_ERROR(err, "PCO_CancelImageTransfer");
		}
	}

    err=camera->PCO_SetRecordingState(wRecState_new);
    PCO_CHECK_ERROR(err, "PCO_SetRecordingState");
#endif

    wRecState_actual = _pco_GetRecordingState(err);
	_setCameraState(CAMSTATE_RECORD_STATE, !!(wRecState_actual));

#ifndef __linux__
	m_pcoData->traceAcq.usTicks[9].value = usElapsedTime(usStart);
	m_pcoData->traceAcq.usTicks[9].desc = "PCO_SetRecordingState execTime";
	usElapsedTimeSet(usStart);
#else
	m_pcoData->traceAcq.usTicks[traceAcq_SetRecordingState].value = usElapsedTime(usStart);
	m_pcoData->traceAcq.usTicks[traceAcq_SetRecordingState].desc = "PCO_SetRecordingState execTime";
	usElapsedTimeSet(usStart);
#endif

	_armRequired(true);

#ifndef __linux__
	m_pcoData->traceAcq.usTicks[10].value = usElapsedTime(usStart);
	m_pcoData->traceAcq.usTicks[10].desc = "PCO_CancelImages execTime";
	usElapsedTimeSet(usStart);
#else
	m_pcoData->traceAcq.usTicks[traceAcq_CancelImages].value = usElapsedTime(usStart);
	m_pcoData->traceAcq.usTicks[traceAcq_CancelImages].desc = "PCO_CancelImages execTime";
	usElapsedTimeSet(usStart);
#endif



	//DEB_TRACE() << fnId << ": " << DEB_VAR4(error, state, wRecState_actual, wRecState_new);
	return fnId;

}
//=================================================================================================
//=================================================================================================

//=================================================================================================
//=================================================================================================
int Camera::_pco_GetBitAlignment(int &alignment){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	int error = 0;
	WORD wBitAlignment;

#ifndef __linux__
	char *msg;
	PCO_FN2(error, msg,PCO_GetBitAlignment, m_handle, &wBitAlignment);
	PCO_THROW_OR_TRACE(error, msg) ;
#else
    error=camera->PCO_GetBitAlignment(&wBitAlignment);
    PCO_CHECK_ERROR(error, "PCO_GetBitAlignment");
    PCO_THROW_OR_TRACE(error, "PCO_GetBitAlignment") ;
#endif
	
	alignment = m_pcoData->wBitAlignment = wBitAlignment;

	return error;
}

//=================================================================================================
//=================================================================================================

// wBitAlignment:
// - 0x0000 = [MSB aligned]; all raw image data will be aligned to the MSB. This is thedefault setting.
// - 0x0001 = [LSB aligned]; all raw image data will be aligned to the LSB.


int Camera::_pco_SetBitAlignment(int alignment){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	int error = 0;
	int alignment1;
	const char *msg;
	WORD wBitAlignment;

	if((alignment <0) ||(alignment >1)){
		DEB_ALWAYS() << "ERROR - invalid value " << DEB_VAR1(alignment);
		return -1;
	}

	wBitAlignment = int(alignment);

#ifndef __linux__
	PCO_FN2(error, msg,PCO_SetBitAlignment, m_handle, wBitAlignment);
	PCO_THROW_OR_TRACE(error, msg) ;
#else
    error=camera->PCO_SetBitAlignment(wBitAlignment);
    msg = "PCO_SetBitAlignment" ; PCO_CHECK_ERROR(error, msg);
    PCO_THROW_OR_TRACE(error, msg) ;
#endif

	return _pco_GetBitAlignment(alignment1);

}





//=================================================================================================
//=================================================================================================
	//-------------------------------------------------------------------------------------------------
	// PCO_SetADCOperation
    // Set analog-digital-converter (ADC) operation for reading the image sensor data. Pixel data can be
    // read out using one ADC (better linearity) or in parallel using two ADCs (faster). This option is
    // only available for some camera models. If the user sets 2ADCs he must center and adapt the ROI
    // to symmetrical values, e.g. pco.1600: x1,y1,x2,y2=701,1,900,500 (100,1,200,500 is not possible).
    //
	// DIMAX -> 1 adc
	//-------------------------------------------------------------------------------------------------
int Camera::_pco_GetADCOperation(int &adc_working, int &adc_max)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

	int error;
	WORD wADCOperation;

	adc_max = m_pcoData->stcPcoDescription.wNumADCsDESC; // nr of ADC in the system

	if(adc_max == 2) {

#ifndef __linux__
		const char *msg;
		PCO_FN2(error, msg,PCO_GetADCOperation, m_handle, &wADCOperation);
#else
		error=camera->PCO_GetADCOperation(&wADCOperation);
		PCO_CHECK_ERROR(error, "PCO_GetADCOperation");
#endif

		if(error) wADCOperation = (WORD) 1;
	} else {
		adc_max = 1;
		wADCOperation = (WORD) 1;
	}

	adc_working = wADCOperation;
	m_pcoData->wNowADC= wADCOperation;

	return error;
}

//=================================================================================================
//=================================================================================================

int Camera::_pco_SetADCOperation(int adc_new, int &adc_working)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

	int error, adc_max;

	error = _pco_GetADCOperation(adc_working, adc_max);

	DEB_TRACE() << fnId << ": " DEB_VAR2(adc_max, adc_working);

	if(error) return error;

	if((adc_new >=1) && (adc_new <= adc_max) && (adc_new != adc_working) ){

#ifndef __linux__
		const char *msg;
		PCO_FN2(error, msg,PCO_SetADCOperation, m_handle, (WORD) adc_new);
#else
        error=camera->PCO_SetADCOperation((WORD) adc_new);
        PCO_CHECK_ERROR(error, "PCO_SetADCOperation");
#endif
		_pco_GetADCOperation(adc_working, adc_max);
	}
	m_pcoData->wNowADC = adc_working;
	return error;
}
//=================================================================================================
//=================================================================================================

bool Camera::_isCapsDesc(int caps)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

	DWORD _dwCaps1;

#ifndef __linux__
	int error;
	_pco_GetGeneralCapsDESC(_dwCaps1, error);
	if(error)
	{
		DEB_ALWAYS() << "ERROR - _pco_GetGeneralCapsDESC";
		return FALSE;
	}
#else
	_dwCaps1 = m_pcoData->stcPcoDescription.dwGeneralCaps1;
#endif


	switch(caps)
	{
		case capsCDI:
			return !!(_dwCaps1 & GENERALCAPS1_CDI_MODE);

		case capsDoubleImage:
			return !!(m_pcoData->stcPcoDescription.wDoubleImageDESC);
	
		case capsRollingShutter:
			return _isCameraType(Edge);

		case capsGlobalShutter:
			//#define GENERALCAPS1_NO_GLOBAL_SHUTTER                 0x00080000 // Camera does not support global shutter
			return _isCameraType(Edge) && !(_dwCaps1 & GENERALCAPS1_NO_GLOBAL_SHUTTER);

		case capsGlobalResetShutter:
			//#define GENERALCAPS1_GLOBAL_RESET_MODE                 0x00100000 // Camera supports global reset rolling readout
			return _isCameraType(Edge) && !!(_dwCaps1 & GENERALCAPS1_GLOBAL_RESET_MODE);

		case capsHWIO:
			return !!(_dwCaps1 & GENERALCAPS1_HW_IO_SIGNAL_DESCRIPTOR);

		case capsCamRam:
			return ( (_dwCaps1 & GENERALCAPS1_NO_RECORDER) == 0) ;

		case capsMetadata:
			return !!(_dwCaps1 & GENERALCAPS1_METADATA);


		//----------------------------------------------------------------------------------------------------------
		// dwGeneralCapsDESC1;      // General capabilities:
        //		Bit 3: Timestamp ASCII only available (Timestamp mode 3 enabled)
		//		Bit 8: Timestamp not available
		// m_pcoData->stcPcoDescription.dwGeneralCapsDESC1 & BIT3 / BIT8

		case capsTimestamp3:
			return (!(_dwCaps1 & BIT8)) && (_dwCaps1 & BIT3);

		case capsTimestamp:
			return !(_dwCaps1 & BIT8);

		default:
			return FALSE;
	}

}


//=================================================================================================
//=================================================================================================
//SC2_SDK_FUNC int WINAPI PCO_GetCDIMode(HANDLE ph, WORD *wCDIMode); 
// Gets the correlated double image mode of the camera, if available.
// Only available with a dimax
// In: HANDLE ph -> Handle to a previously opened camera.
//     WORD *wCDIMode -> Pointer to a WORD variable to receive the correlated double image mode.
// Out: int -> Error message.

//SC2_SDK_FUNC int WINAPI PCO_SetCDIMode(HANDLE ph,  WORD wCDIMode); 
// Sets the correlated double image mode of the camera, if available.
// Only available with a dimax
// In: HANDLE ph -> Handle to a previously opened camera.
//     WORD wCDIMode -> WORD variable to set the correlated double image mode.
// Out: int -> Error message.
// CDI mode is available if in the camera descriptor the flag

//Mode parameter is simple: 0 is off (default), 1 is on.
// CDI mode is available if in the camera descriptor the flag
// #define GENERALCAPS1_CDI_MODE                          0x00010000 // Camera has Correlated Double Image Mode
// in GENERALCAPS1 is set.


void Camera::_pco_GetCDIMode(WORD &wCDIMode, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//char *pcoFn;
	int pcoErr;
	err = 0;

	if(!_isCapsDesc(capsCDI))
	{
		wCDIMode = 0;
		err = 1;
		return;
	}
	
	err = 0;

#ifndef __linux__
	pcoErr = PCO_GetCDIMode(m_handle, &wCDIMode);
#else
	wCDIMode = 0;
	DEB_ALWAYS() <<  "ERROR / TODO / NOT IMPLEMENTED YET";
    pcoErr = -1;
#endif

	err |= pcoErr;

	return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_SetCDIMode(WORD wCDIMode, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//char *pcoFn;
	int pcoErr;
	err = 0;

	if(!_isCapsDesc(capsCDI))
	{
		err = 1;
		return;
	}
	
	if(wCDIMode) 
	{
		wCDIMode = 1;
		_pco_SetDoubleImageMode(0, err); 		// cdi and double image are exclusive
	}

	err = 0;

#ifndef __linux__
	pcoErr = PCO_SetCDIMode(m_handle, wCDIMode);
#else
	DEB_ALWAYS() <<  "ERROR / TODO / NOT IMPLEMENTED YET";
    pcoErr = -1;
#endif
	
	err |= pcoErr;

	return;
}
//=================================================================================================
//=================================================================================================
void Camera::_pco_GetDoubleImageMode(WORD &wDoubleImage, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//char *pcoFn;
	int pcoErr;
	err = 0;

	if(!_isCapsDesc(capsDoubleImage))
	{
		wDoubleImage = 0;
		err = 1;
		return;
	}
	
	err = 0;

#ifndef __linux__
	pcoErr = PCO_GetDoubleImageMode(m_handle, &wDoubleImage);
#else
	wDoubleImage = 0;
	DEB_ALWAYS() <<  "ERROR / TODO / NOT IMPLEMENTED YET";
    pcoErr = -1;
#endif

	err |= pcoErr;

	return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_SetDoubleImageMode(WORD wDoubleImage, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//char *pcoFn;
	int pcoErr;
	err = 0;

	if(!_isCapsDesc(capsDoubleImage))
	{
		err = 1;
		return;
	}
	
	if(wDoubleImage) 
	{
		wDoubleImage = 1;
		_pco_SetCDIMode(0, err);		// cdi and double image are exclusive
	}

	err = 0;

#ifndef __linux__
	pcoErr = PCO_SetDoubleImageMode(m_handle, wDoubleImage);
#else
	DEB_ALWAYS() <<  "ERROR / TODO / NOT IMPLEMENTED YET";
    pcoErr = -1;
#endif


	err |= pcoErr;

	return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetGeneralCapsDESC(DWORD &capsDesc1, int &err)
{
	err = 0;
#ifndef __linux__
	capsDesc1 = m_pcoData->stcPcoDescription.dwGeneralCapsDESC1;
#else
	capsDesc1 = m_pcoData->stcPcoDescription.dwGeneralCaps1;
#endif
	
	
	return;
}


//=================================================================================================
//=================================================================================================

//SC2_SDK_FUNC int WINAPI PCO_GetSegmentImageSettings(HANDLE ph, WORD wSegment,
//                                                    WORD* wXRes,
//                                                    WORD* wYRes,
//                                                    WORD* wBinHorz,
//                                                    WORD* wBinVert,
//                                                    WORD* wRoiX0,
//                                                    WORD* wRoiY0,
//                                                    WORD* wRoiX1,
//                                                    WORD* wRoiY1);
// Gets the sizes information for one segment. X0, Y0 start at 1. X1, Y1 end with max. sensor size.
// In: HANDLE ph -> Handle to a previously opened camera.
//     WORD *wXRes -> Pointer to a WORD variable to receive the x resolution of the image in segment
//     WORD *wYRes -> Pointer to a WORD variable to receive the y resolution of the image in segment
//     WORD *wBinHorz -> Pointer to a WORD variable to receive the horizontal binning of the image in segment
//     WORD *wBinVert -> Pointer to a WORD variable to receive the vertical binning of the image in segment
//     WORD *wRoiX0 -> Pointer to a WORD variable to receive the left x offset of the image in segment
//     WORD *wRoiY0 -> Pointer to a WORD variable to receive the upper y offset of the image in segment
//     WORD *wRoiX1 -> Pointer to a WORD variable to receive the right x offset of the image in segment
//     WORD *wRoiY1 -> Pointer to a WORD variable to receive the lower y offset of the image in segment
//      x0,y0----------|
//      |     ROI      |
//      ---------------x1,y1
// Out: int -> Error message.
//SC2_SDK_FUNC int WINAPI PCO_GetNumberOfImagesInSegment(HANDLE ph, 
//                                             WORD wSegment,
//                                             DWORD* dwValidImageCnt,
//                                             DWORD* dwMaxImageCnt);
// Gets the number of images in the addressed segment.
// In: HANDLE ph -> Handle to a previously opened camera.
//     DWORD *dwValidImageCnt -> Pointer to a DWORD varibale to receive the valid image count.
//     DWORD *dwMaxImageCnt -> Pointer to a DWORD varibale to receive the max image count.
// Out: int -> Error message.


void Camera::_pco_GetSegmentInfo(int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//char *pcoFn;
	int pcoErr;
	err = 0;

	err = 0;
	if(!_isCameraType(Dimax | Pco2k | Pco4k))
	{
		err = 1;
		return;
	}

#ifndef __linux__
	struct stcSegmentInfo *_stc;

	for(int iseg = 0; iseg <  PCO_MAXSEGMENTS; iseg++)
	{

		_stc = &m_pcoData->m_stcSegmentInfo[iseg];
		
		_stc->iSegId = iseg+1;
		_stc->wXRes = _stc->wYRes = 
			_stc->wBinHorz = _stc->wBinVert = 
			_stc->wRoiX0 = _stc->wRoiY0  = 
			_stc->wRoiX1 = _stc->wRoiY1 = 0;
			_stc->dwValidImageCnt = _stc->dwMaxImageCnt = 0;
			_stc->iErr = 0;

		pcoErr = PCO_GetSegmentImageSettings(m_handle, iseg,
			&_stc->wXRes, &_stc->wYRes,
			&_stc->wBinHorz, &_stc->wBinVert,
			&_stc->wRoiX0, &_stc->wRoiY0,
			&_stc->wRoiX1, &_stc->wRoiY1);

			_stc->iErr |= pcoErr;
			err |= pcoErr;

		pcoErr = PCO_GetNumberOfImagesInSegment(m_handle, iseg,
			&_stc->dwValidImageCnt, &_stc->dwMaxImageCnt);
	
			_stc->iErr |= pcoErr;
			err |= pcoErr;

	}
#else
	DEB_ALWAYS() <<  "ERROR / TODO / NOT IMPLEMENTED YET";
    pcoErr = -1;
	err |= pcoErr;
#endif
	return;
}


//=================================================================================================
//=================================================================================================
void Camera::_pco_CloseCamera(int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	
#ifndef __linux__
	int pcoErr;
	
	//PCO_PRINT_ERR(error, msg); 

	err = 0;
	pcoErr = PCO_CloseCamera(m_handle);
	err |= PCO_CHECK_ERROR(pcoErr, "PCO_CloseCamera");

	m_handle = NULL;
#else
    err = 0;
    
    if(grabber)
    {
        delete grabber;
        grabber = NULL;
    }

    if(camera)
    {
        err = camera->Close_Cam();
        PCO_CHECK_ERROR(err, "Close_Cam");

        delete camera;
        camera = NULL;
    }
#endif
	return;
}
//=================================================================================================
//=================================================================================================

bool Camera::_isCooledCamera()
{

	return !((m_pcoData->stcPcoDescription.sMinCoolSetDESC == 0) && (m_pcoData->stcPcoDescription.sMaxCoolSetDESC == 0));


}








//=================================================================================================
//=================================================================================================
void Camera::_pco_GetCoolingSetpointTemperature(int &val, int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

#ifndef __linux__
	char *pcoFn;
	SHORT _sSetpoint;


	if(!_isCooledCamera())
	{
		error = -1;
		val = 0;
		return;
	}

	PCO_FN2(error, pcoFn,PCO_GetCoolingSetpointTemperature, m_handle, &_sSetpoint);
	if(error)
	{
		val = 0;
		return;
	}

	val = _sSetpoint;


#else
	error = -1;

	DEB_ALWAYS() <<  "ERROR / TODO / NOT IMPLEMENTED YET";

	val = 0;
	return;

#endif


}

//=================================================================================================
//=================================================================================================
void Camera::_pco_SetCoolingSetpointTemperature(int val, int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

#ifndef __linux__
	char *pcoFn;
	WORD _wSetpoint = (WORD) val;

	if(
		!_isCooledCamera() ||
		(_wSetpoint < m_pcoData->stcPcoDescription.sMinCoolSetDESC) ||
		(_wSetpoint > m_pcoData->stcPcoDescription.sMaxCoolSetDESC)
		)
	{
		error = -1;
		return;
	}

	PCO_FN2(error, pcoFn,PCO_SetCoolingSetpointTemperature, m_handle, _wSetpoint);
	if(error)
	{
		return;
	}

#else
	error = -1;

	DEB_ALWAYS() <<  "ERROR / TODO / NOT IMPLEMENTED YET";


	return;

#endif



}


//=================================================================================================
//=================================================================================================
//SC2_SDK_FUNC int WINAPI PCO_GetInfoString(HANDLE ph, DWORD dwinfotype, char *buf_in, WORD size_in);
// Gets the name of the camera.
// In: HANDLE ph -> Handle to a previously opened camera.
//     DWORD dwinfotype -> 0: Camera and interface name
//                         1: Camera name only
//                         2: Sensor name
//     char *buf_in -> Pointer to a string, to receive the info string.
//     WORD size_in -> WORD variable which holds the maximum length of the string.
// Out: int -> Error message.

void Camera::_pco_GetInfoString(int infotype, char *buf_in, int size_in, int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

#ifndef __linux__
	DWORD dwinfotype = (DWORD) infotype;
	WORD wsize_in = (WORD) size_in;

	error = PCO_GetInfoString(m_handle, dwinfotype, buf_in, wsize_in);
	PCO_CHECK_ERROR(error, "PCO_GetInfoString");
	if(error)
	{
		*buf_in = 0;
	}

#else
	error = -1;
	*buf_in = 0;
	DEB_ALWAYS() << "ERROR - NOT IMPLEMENTED!" ;

#endif


}


//=================================================================================================
// only in linux - to merge
//=================================================================================================
void Camera::_pco_SetCameraToCurrentTime(int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

#ifdef __linux__
	err=camera->PCO_SetCameraToCurrentTime();
    PCO_CHECK_ERROR(err, "PCO_SetCameraToCurrentTime"); 
#else
	DEB_ALWAYS() << "ERROR - NOT IMPLEMENTED!" ;
	err = -1;
#endif

    return;
}


//=================================================================================================
//=================================================================================================
void Camera::_pco_SetBinning(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;

#ifdef __linux__
	//SetBinning, SetROI, ARM, GetSizes, AllocateBuffer.
    //------------------------------------------------- set binning if needed
    WORD wBinHorz, wBinVert, wBinHorzNow, wBinVertNow;
    if (m_bin.changed == Changed) {
		wBinHorz = (WORD)m_bin.x;
		wBinVert = (WORD)m_bin.y;

        error=camera->PCO_GetBinning(&wBinHorzNow, &wBinVertNow);
        PCO_CHECK_ERROR(error, "PCO_GetBinning");
		PCO_THROW_OR_TRACE(error, "PCO_GetBinning") ;

		if((wBinHorz != wBinHorzNow) || (wBinVert != wBinVertNow)) {

            error=camera->PCO_SetBinning(wBinHorz, wBinVert);
	        PCO_CHECK_ERROR(error, "PCO_SetBinning");
			PCO_THROW_OR_TRACE(error, "PCO_SetBinning") ;
			
			_armRequired(true);

            error=camera->PCO_GetBinning(&wBinHorzNow, &wBinVertNow);
	        PCO_CHECK_ERROR(error, "PCO_GetBinning");
			PCO_THROW_OR_TRACE(error, "PCO_GetBinning") ;
		}
		m_bin.changed= Valid;
		DEB_TRACE() << DEB_VAR4(wBinHorz, wBinVert, wBinHorzNow, wBinVertNow);
    }
#else
	DEB_ALWAYS() << "ERROR - NOT IMPLEMENTED!" ;
	error = -1;
#endif

	return;

}


//=================================================================================================
//=================================================================================================
const char *Camera::_pco_SetCamLinkSetImageParameters(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;

#ifdef __linux__

	const char *pcoFn;

	// camLink -> imgPar
	// GigE    -> imgPar 
	switch (_getInterfaceType()) {
        case INTERFACE_CAMERALINK: 
		case INTERFACE_ETHERNET:
		    WORD wXres, wYres;

            wXres= m_pcoData->wXResActual;
            wYres= m_pcoData->wYResActual;
			
			DEB_ALWAYS() << "ERROR PCO_CamLinkSetImageParameters: " <<  DEB_VAR2(wXres, wYres);

			PCO_FN3(error, pcoFn,PCO_CamLinkSetImageParameters, m_handle, wXres, wYres);
			if(error) { throw LIMA_HW_EXC(Error, pcoFn); }

        default: break;
    } // switch

#else
	DEB_ALWAYS() << "ERROR - NOT IMPLEMENTED!" ;
	error = -1;
#endif
	return fnId;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetHWIOSignalAll(int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	
	if(! _isCapsDesc(capsHWIO) ) {
		error = -1;
		return;
	}

#ifndef __linux__
	const char *msg;

	int i, imax;
	error = 0;

	PCO_FN2(error, msg, PCO_GetHWIOSignalCount, m_handle, &m_pcoData->wNrPcoHWIOSignal0);
	if(error) return;

	imax = m_pcoData->wNrPcoHWIOSignal = 
		(m_pcoData->wNrPcoHWIOSignal0 <= SIZEARR_stcPcoHWIOSignal) ? m_pcoData->wNrPcoHWIOSignal0 : SIZEARR_stcPcoHWIOSignal;

	//DEB_TRACE()  << "--- size" << DEB_VAR3(imax, m_pcoData->wNrPcoHWIOSignal0 , m_pcoData->wNrPcoHWIOSignal ) ;

	for(i=0; i< imax; i++) {
		//DEB_TRACE()  << "---  descriptor" << DEB_VAR2(i, m_pcoData->stcPcoHWIOSignalDesc[i].wSize) ;
		PCO_FN3(error, msg,PCO_GetHWIOSignalDescriptor, m_handle, i, &m_pcoData->stcPcoHWIOSignalDesc[i]);
		if(error) return;

	
		//DEB_TRACE()  << "---  signal" << DEB_VAR2(i, m_pcoData->stcPcoHWIOSignal[i].wSize) ;
		PCO_FN3(error, msg,PCO_GetHWIOSignal, m_handle, i, &m_pcoData->stcPcoHWIOSignal[i]);
		if(error) return;
	}
	return;

#else
    int errorTot;
	errorTot = 0;
	const char *msg __attribute__((unused));

	int iSignal, iSignalMax;
	
	if(!( _isCameraType(Dimax | Edge | Pco2k | Pco4k))  ) {
		errorTot = -1;
		return;
	}

    error=camera->PCO_GetHWIOSignalCount(&m_pcoData->wNrPcoHWIOSignal0);
    msg = "PCO_GetHWIOSignalCount" ; PCO_CHECK_ERROR(error, msg);
	errorTot |= error;

	iSignalMax = m_pcoData->wNrPcoHWIOSignal = 
		(m_pcoData->wNrPcoHWIOSignal0 <= SIZEARR_stcPcoHWIOSignal) ? m_pcoData->wNrPcoHWIOSignal0 : SIZEARR_stcPcoHWIOSignal;

	DEB_ALWAYS()  << "--- size" << DEB_VAR3(iSignalMax, m_pcoData->wNrPcoHWIOSignal0 , m_pcoData->wNrPcoHWIOSignal ) ;

    WORD wEnabled,wType,wPolarity,wFilterSetting, wSelected;

	for(iSignal=0; iSignal< iSignalMax; iSignal++) {
	    int sizeName = SIZESTR_PcoHWIOSignal;
	    char *ptrName = &(m_pcoData->sPcoHWIOSignalDesc[iSignal][0]);
	    
		DEB_ALWAYS()  << "---  descriptor" << DEB_VAR2(iSignal, m_pcoData->stcPcoHWIOSignalDesc[iSignal].wSize) ;
        
        // telegram structure 4 signals * 24 char
	    //memset(&m_pcoData->stcPcoHWIOSignalDesc[iSignal].szSignalName[0][0],0,24*4);

        error=camera->PCO_GetHWIOSignalDescriptor( iSignal, 
            (SC2_Get_HW_IO_Signal_Descriptor_Response *) &m_pcoData->stcPcoHWIOSignalDesc[iSignal]);
        msg = "PCO_GetHWIOSignalDescriptor (struct)" ; PCO_CHECK_ERROR(error, msg);
		errorTot |= error;

		//DEB_ALWAYS()  << "---  signal" << DEB_VAR2(iSignal, m_pcoData->stcPcoHWIOSignal[iSignal].wSize) ;
		
		//DWORD PCO_GetHWIOSignalDescriptor ( WORD SignalNum, char &outbuf, int &size )
        
        // Gets the signal descriptor of the requested signal number as a string for console output.
        *ptrName = 0;
        error=camera->PCO_GetHWIOSignalDescriptor( iSignal,  ptrName, &sizeName);
        msg = "PCO_GetHWIOSignalDescriptor (name)" ; PCO_CHECK_ERROR(error, msg);
		errorTot |= error;
        		
		DEB_ALWAYS()  << "---  signal name " << DEB_VAR3(iSignal, sizeName, ptrName) ;


        //m_pcoData->stcPcoHWIOSignal[iSignal].wSignalNum = iSignal;
        
	    if( _isCameraType(Dimax | Edge) ) 
	    {
	    
                    error=camera->PCO_GetHWIOSignal( iSignal, &wEnabled,&wType, &wPolarity,&wFilterSetting, &wSelected);
                    msg = "PCO_GetHWIOSignal" ; PCO_CHECK_ERROR(error, msg);

		            //PCO3(error, msg,PCO_GetHWIOSignal, m_handle, iSignal, &m_pcoData->stcPcoHWIOSignal[i]);
		            errorTot |= error;

                    m_pcoData->stcPcoHWIOSignal[iSignal].wEnabled = wEnabled;
	                m_pcoData->stcPcoHWIOSignal[iSignal].wType =wType;
	                m_pcoData->stcPcoHWIOSignal[iSignal].wPolarity = wPolarity;
	                m_pcoData->stcPcoHWIOSignal[iSignal].wFilterSetting = wFilterSetting;
	                m_pcoData->stcPcoHWIOSignal[iSignal].wSelected = wSelected;

		            DEB_ALWAYS()  << "---  " << DEB_VAR6( iSignal, wEnabled, wType,  wPolarity, wFilterSetting, wSelected) ;
	    
        }

	    
	}
	
	error = errorTot;

#endif




}



//=================================================================================================
//=================================================================================================
void Camera::_pco_GetNumberOfImagesInSegment(WORD wSegment, DWORD& dwValidImageCnt, DWORD& dwMaxImageCnt, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	err = 0;

	if( (!_isCameraType(Dimax | Pco2k | Pco4k)) 
	    || (( wSegment > PCO_MAXSEGMENTS) || ( wSegment < 1 )) )
	{
		err = -1;
        dwValidImageCnt = dwMaxImageCnt = 0;
		return;
	}
	
#ifndef __linux__
	
	//char *pcoFn;

	err = PCO_GetNumberOfImagesInSegment(m_handle, wSegment,
			&dwValidImageCnt, &dwMaxImageCnt);
	
#else
    //DWORD PCO_GetNumberOfImagesInSegment ( WORD wSegment, DWORD *dwValid, DWORD *dwMax )

    err = camera->PCO_GetNumberOfImagesInSegment(wSegment, &dwValidImageCnt, &dwMaxImageCnt);   
    PCO_CHECK_ERROR(err, "PCO_GetNumberOfImagesInSegment");

#endif


    if(err) 
    {
        dwValidImageCnt = dwMaxImageCnt = 0;
    }

	return;
}
//=========================================================================================================
//=========================================================================================================
unsigned long Camera::_pco_GetNumberOfImagesInSegment_MaxCalc(int segmentPco)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

		int segmentArr = segmentPco-1;
		unsigned long framesMax;

		if(_isCameraType(Edge)) {
			return LONG_MAX;
		}


		if(!_isCameraType(Dimax | Pco2k | Pco4k)) {
            DEB_ALWAYS() << "ERROR camera not valid for this function: " << _getCameraType();
			return 0;
		}

		if((segmentPco <1) ||(segmentPco > PCO_MAXSEGMENTS)) {
            DEB_ALWAYS() << "ERROR in the segment: " << DEB_VAR1(segmentPco);
			return 0;
		}

#ifdef __linux__

		int error;
		DWORD dwValid, dwMax;
		
		_pco_GetNumberOfImagesInSegment((WORD) segmentPco, dwValid, dwMax, error);

        if(error)
        {
            framesMax = 0;
            DEB_ALWAYS() << "ERROR _pco_GetNumberOfImagesInSegment()" ;
        }
        else
        {
            framesMax = dwMax;
        }
        
#else
		unsigned long xroisize,yroisize;
		unsigned long long pixPerFrame, pagesPerFrame;

		xroisize = m_RoiLima.getSize().getWidth();
		yroisize = m_RoiLima.getSize().getHeight();

		//xroisize = m_roi.x[1] - m_roi.x[0] + 1;
		//yroisize = m_roi.y[1] - m_roi.y[0] + 1;

		pixPerFrame = (unsigned long long)xroisize * (unsigned long long)yroisize;

		if(pixPerFrame <0) {
			printf("=== %s> ERROR pixPerFrame[%lld]\n", fnId, pixPerFrame);
			return 0;
		}

		if(m_pcoData->wPixPerPage < 1) {
			printf("=== %s> ERROR m_pcoData->wPixPerPage[%d]\n", fnId, m_pcoData->wPixPerPage);
			return 0;
		}
		pagesPerFrame = (pixPerFrame / m_pcoData->wPixPerPage) + 1;
		if(pixPerFrame % m_pcoData->wPixPerPage) pagesPerFrame++;

		framesMax = m_pcoData->dwMaxFramesInSegment[segmentArr] = (unsigned long)(((long long) m_pcoData->dwSegmentSize[segmentArr] ) / pagesPerFrame);
#endif

		return framesMax;
		
}
//=================================================================================================
//=================================================================================================
void Camera::_pco_GetTemperatureInfo(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;

#ifdef __linux__
    error=camera->PCO_GetTemperature(&sTempCcd,&sTempCam,&sTempPS);
    PCO_CHECK_ERROR(error, "PCO_GetTemperature"); 
    if(error) { sTempCcd = sTempCam = sTempPS = 0; }

    error=camera->PCO_GetCoolingSetpointTemperature(&sCoolingSetpoint);
    PCO_CHECK_ERROR(error, "PCO_GetCoolingSetpointTemperature"); 
    if(error) {sCoolingSetpoint = 0;}

    DEB_ALWAYS() 
        << "\n   " << DEB_VAR1(sTempCcd) 
        << "\n   " << DEB_VAR1(sTempCam) 
        << "\n   " << DEB_VAR1(sTempPS) 
        << "\n   " << DEB_VAR1(sCoolingSetpoint) 
        ;

	m_pcoData->temperature.wCcd = sTempCcd;
	m_pcoData->temperature.wCam = sTempCam;
	m_pcoData->temperature.wPower =sTempPS;

#else

	char msg[MSG_SIZE + 1];
	char *pcoFn;


	// -- Print out current temperatures
	PCO_FN4(error, pcoFn,PCO_GetTemperature, m_handle, &m_pcoData->temperature.wCcd, &m_pcoData->temperature.wCam, &m_pcoData->temperature.wPower);
	if(error) return;
	//PCO_THROW_OR_TRACE(error, "PCO_GetTemperature") ;

	sprintf_s(msg, MSG_SIZE, "* temperature: CCD[%.1f]  CAM[%d]  PS[%d]\n", m_pcoData->temperature.wCcd/10., m_pcoData->temperature.wCam, m_pcoData->temperature.wPower);
	//DEB_TRACE() <<   msg;
	m_log.append(msg);


	m_pcoData->temperature.wMinCoolSet = m_pcoData->stcPcoDescription.sMinCoolSetDESC;
	m_pcoData->temperature.wMaxCoolSet = m_pcoData->stcPcoDescription.sMaxCoolSetDESC;

	// SC2_SDK_FUNC int WINAPI PCO_GetCoolingSetpointTemperature(HANDLE ph, SHORT* sCoolSet)

	if ((m_pcoData->temperature.wMinCoolSet == 0) && (m_pcoData->temperature.wMaxCoolSet == 0)) // no cooled camera
	{
		m_pcoData->temperature.wSetpoint = 0;
		sprintf_s(msg, MSG_SIZE, "*     cooling: NO cooled camera");
		m_log.append(msg);
		return;
	}

	PCO_FN2(error, pcoFn,PCO_GetCoolingSetpointTemperature, m_handle, &m_pcoData->temperature.wSetpoint);


	sprintf_s(msg, MSG_SIZE, "*     cooling: min[%d]  max[%d]  setpoint[%d]\n",  
				m_pcoData->temperature.wMinCoolSet, m_pcoData->temperature.wMaxCoolSet, m_pcoData->temperature.wSetpoint);
	m_log.append(msg);

#endif
	return;
}
//=================================================================================================
//=================================================================================================
void Camera::_pco_GetTemperatureInfo(char *ptr, char *ptrMax, int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

#ifndef __linux__
	char *pcoFn;

	// -- Print out current temperatures
	PCO_FN4(error, pcoFn,PCO_GetTemperature, m_handle, &m_pcoData->temperature.wCcd, &m_pcoData->temperature.wCam, &m_pcoData->temperature.wPower);
	if(error) {
		ptr += sprintf_s(ptr, ptrMax - ptr, "[SDK error - getTemperature]"); 
		return;
	}
	ptr += sprintf_s(ptr, ptrMax - ptr, "CCD[%.1f] CAM[%d] PS[%d]", 
		m_pcoData->temperature.wCcd/10., m_pcoData->temperature.wCam, m_pcoData->temperature.wPower);

	m_pcoData->temperature.wMinCoolSet = m_pcoData->stcPcoDescription.sMinCoolSetDESC;
	m_pcoData->temperature.wMaxCoolSet = m_pcoData->stcPcoDescription.sMaxCoolSetDESC;

	// SC2_SDK_FUNC int WINAPI PCO_GetCoolingSetpointTemperature(HANDLE ph, SHORT* sCoolSet)

	if ((m_pcoData->temperature.wMinCoolSet == 0) && (m_pcoData->temperature.wMaxCoolSet == 0)) // no cooled camera
	{
		m_pcoData->temperature.wSetpoint = 0;
		ptr += sprintf_s(ptr, ptrMax - ptr, " cooling: [NO cooled camera]");
		return;
	}

	PCO_FN2(error, pcoFn,PCO_GetCoolingSetpointTemperature, m_handle, &m_pcoData->temperature.wSetpoint);
	if(error) {
		ptr += sprintf_s(ptr, ptrMax - ptr, " [SDK error - getSetpoint]"); 
		return;
	}

	ptr += sprintf_s(ptr, ptrMax - ptr, " cooling: min[%d] max[%d] setpoint[%d]",  
				m_pcoData->temperature.wMinCoolSet, m_pcoData->temperature.wMaxCoolSet, m_pcoData->temperature.wSetpoint);

	return;

#else
	error = -1;
	ptr += sprintf_s(ptr, ptrMax - ptr, "NOT IMPLEMENTED");

    return;
#endif


}
