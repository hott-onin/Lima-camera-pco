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

#ifndef __linux__
#include <windows.h>
#include <tchar.h>
#endif
#include <stdio.h>

#include <cstdlib>

#ifndef __linux__
#include <process.h>
#else
#include <unistd.h>
#endif

#include <sys/stat.h>

#include <sys/timeb.h>
#include <time.h>

#include "lima/HwSyncCtrlObj.h"

#include "lima/Exceptions.h"

#include "PcoSdkVersion.h"

#include "PcoCamera.h"
#include "PcoCameraUtils.h"
#include "PcoSyncCtrlObj.h"
#include "PcoBufferCtrlObj.h"

using namespace lima;
using namespace lima::Pco;


#define BUFF_INFO_SIZE 10000
static char buff[BUFF_INFO_SIZE +16];


//====================================================================
// SIP - attributes
//====================================================================


void Camera::getPixelRate(int &val)
{
	DWORD pixRate, pixRateNext; int error;

	_pco_GetPixelRate(pixRate, pixRateNext, error);
	val = pixRate;
}

void Camera::setPixelRate(int val)
{
	int error;

	DWORD pixRate = val;
	_presetPixelRate(pixRate, error);
}

//====================================================================
//====================================================================
void Camera::getAcqTimeoutRetry(int &val)
{
	val = m_pcoData->acqTimeoutRetry;
}

void Camera::setAcqTimeoutRetry(int val)
{
	m_pcoData->acqTimeoutRetry = val < 0 ? 0 : val;
}
//====================================================================
//====================================================================
void Camera::getAdc(int &adc)
{
	int adc_working, adc_max;

	//int error = _pco_GetADCOperation(adc_working, adc_max);
	_pco_GetADCOperation(adc_working, adc_max);
	adc = adc_working;
}

void Camera::setAdc(int adc_new)
{
	int adc_working;

	//int error = _pco_SetADCOperation(adc_new, adc_working);
	_pco_SetADCOperation(adc_new, adc_working);
}

void Camera::getAdcMax(int &adc)
{
	
	int adc_working, adc_max;

	//int error = _pco_GetADCOperation(adc_working, adc_max);
	_pco_GetADCOperation(adc_working, adc_max);

	adc = adc_max;
}

//====================================================================
//====================================================================
void Camera::getCocRunTime(double &coc)
{
	coc = m_pcoData->cocRunTime;
}

void Camera::getFrameRate(double &framerate)
{
	framerate = m_pcoData->frameRate;
}

//====================================================================
//====================================================================
void Camera::getLastImgRecorded(unsigned long & img)
{

	img =  m_pcoData->traceAcq.nrImgRecorded;
}

void Camera::getLastImgAcquired(unsigned long & img)
{

	img =  m_pcoData->traceAcq.nrImgAcquired;
}
//====================================================================
//====================================================================
void Camera::getMaxNbImages(unsigned long & nr)
{
    int err;
    WORD wActSeg;        

    if(!_isCameraType(Dimax | Pco2k | Pco4k ))
    {
        nr = 0;
        return;
    }
 
   _pco_GetActiveRamSegment(wActSeg, err);
        
    if(err)
    {
        nr = 0;
        return;
    }
       
	nr = _pco_GetNumberOfImagesInSegment_MaxCalc(wActSeg);

    return;
}
//====================================================================
//====================================================================

void Camera::getPcoLogsEnabled(int & enabled)
{
	enabled =  m_pcoData->pcoLogActive;
}

//====================================================================
// SIP - attrib
//====================================================================
void Camera::getCamType(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	_camInfo(ptr, ptrMax, CAMINFO_CAMERATYPE);
	o_sn = buff;
}

void Camera::getCamInfo(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	_camInfo(ptr, ptrMax, CAMINFO_ALL);
	o_sn = buff;
}

void Camera::getVersion(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	_camInfo(ptr, ptrMax, CAMINFO_VERSION);
	o_sn = buff;
}

//====================================================================
//====================================================================

void Camera::getClTransferParam(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	_camInfo(ptr, ptrMax, CAMINFO_CAMERALINK);
	o_sn = buff;
}

void Camera::getPixelRateInfo(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	_camInfo(ptr, ptrMax, CAMINFO_PIXELRATE);
	o_sn = buff;
}

void Camera::getLastError(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	sprintf_s(ptr, ptrMax - ptr, "[x%08x] [%s]\n", 
				m_pcoData->pcoError, m_pcoData->pcoErrorMsg);
	o_sn = buff;
}

//====================================================================
// SIP - attrib
//====================================================================
void Camera::getTraceAcq(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);

	time_t _timet;

	if(0 && !(_isCameraType(Dimax | Pco2k | Pco4k))) {
		ptr += sprintf_s(ptr, ptrMax - ptr, "* ERROR - only for DIMAX / 2K");
		o_sn = buff;
		return;
	}

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"\n"
		"* fnId[%s] nrEvents[%d]\n"
		"* ... fnIdXfer[%s]\n",
		m_pcoData->traceAcq.fnId,
		PCO_BUFFER_NREVENTS,
		m_pcoData->traceAcq.fnIdXfer);

	ptr += sprintf_s(ptr, ptrMax - ptr, "* ... testCmdMode [0x%llx]\n",  m_pcoData->testCmdMode);

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* msExposure[%g] msDelay[%g]\n",
		m_pcoData->traceAcq.sExposure * 1000.,
		m_pcoData->traceAcq.sDelay * 1000.);

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* ... msLimaExposure[%g] Pco exposure[%d] base[%d]\n",
		m_pcoData->traceAcq.dLimaExposure * 1000.,
		m_pcoData->traceAcq.iPcoExposure, 
		m_pcoData->traceAcq.iPcoExposureBase);

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* ... msLimaDelay[%g] Pco delay[%d] base[%d]\n",
		m_pcoData->traceAcq.dLimaDelay * 1000.,
		m_pcoData->traceAcq.iPcoDelay, 
		m_pcoData->traceAcq.iPcoDelayBase);

	ptr += sprintf_s(ptr, ptrMax - ptr, "* pcoBin horz[%d] vert[%d]\n",  
			m_pcoData->traceAcq.iPcoBinHorz, 
			m_pcoData->traceAcq.iPcoBinVert);

	Roi limaRoi;
	int error;
	_pco_GetROI(limaRoi, error);

	Point top_left = limaRoi.getTopLeft();
	Point bot_right = limaRoi.getBottomRight();
	Size size = limaRoi.getSize();			
	unsigned int bytesPerPix; getBytesPerPixel(bytesPerPix);

	ptr += sprintf_s(ptr, ptrMax - ptr, "* limaRoi xy0[%d,%d] xy1[%d,%d] size[%d,%d]\n",  
			top_left.x, top_left.y,
			bot_right.x, bot_right.y,
			size.getWidth(), size.getHeight());


	ptr += sprintf_s(ptr, ptrMax - ptr, "* ... pcoRoi x[%d,%d] y[%d,%d]\n",  
			m_pcoData->traceAcq.iPcoRoiX0, 
			m_pcoData->traceAcq.iPcoRoiX1, 
			m_pcoData->traceAcq.iPcoRoiY0, 
			m_pcoData->traceAcq.iPcoRoiY1);


	long long imgSize = size.getWidth()* size.getHeight() * bytesPerPix;
	long long totSize = imgSize * m_pcoData->traceAcq.nrImgRequested;
	double mbTotSize =  totSize/(1024.*1024.);
	double totTime = m_pcoData->traceAcq.msXfer / 1000.;
	double xferSpeed = mbTotSize / totTime;
	double framesPerSec = m_pcoData->traceAcq.nrImgRequested / totTime;
	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* ... imgSize[%lld B] totSize[%lld B][%g MB]\n",  
		imgSize, totSize, mbTotSize);

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* nrImgRequested[%d] nrImgAcquired[%d]\n",
		m_pcoData->traceAcq.nrImgRequested,
		m_pcoData->traceAcq.nrImgAcquired);


	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* ... nrImgRequested0[%d] nrImgRecorded[%d] maxImgCount[%d]\n",
		m_pcoData->traceAcq.nrImgRequested0,
		m_pcoData->traceAcq.nrImgRecorded,
		m_pcoData->traceAcq.maxImgCount);

	ptr += sprintf_s(ptr, ptrMax - ptr,	
		"* limaTriggerMode[%s]\n",
		m_pcoData->traceAcq.sLimaTriggerMode);
	ptr += sprintf_s(ptr, ptrMax - ptr,	
		"* ... pcoTriggerMode[%s] [%d]\n",
		m_pcoData->traceAcq.sPcoTriggerMode,
		m_pcoData->traceAcq.iPcoTriggerMode);
	ptr += sprintf_s(ptr, ptrMax - ptr,	
		"* ... pcoAcqMode[%s] [%d]\n",
		m_pcoData->traceAcq.sPcoAcqMode,
		m_pcoData->traceAcq.iPcoAcqMode);


	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* msStartAcqStart[%ld]  msStartAcqEnd[%ld]\n",
		m_pcoData->traceAcq.msStartAcqStart, m_pcoData->traceAcq.msStartAcqEnd);
	

	for(int _i = 0; _i < LEN_TRACEACQ_TRHEAD; _i++){
		const char *desc = m_pcoData->traceAcq.usTicks[_i].desc;
		if(desc != NULL) {
			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* ... usTicks[%d][%5.3f] (ms)   (%s)\n", 
				_i, m_pcoData->traceAcq.usTicks[_i].value/1000.,
				desc);
	
		}
	}

	_timet = m_pcoData->traceAcq.endRecordTimestamp;

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* msImgCoc[%.3g] fps[%.3g] msTout[%ld] msTotal[%ld]\n",
		m_pcoData->traceAcq.msImgCoc, 
		1000. / m_pcoData->traceAcq.msImgCoc,
		m_pcoData->traceAcq.msTout,
		m_pcoData->traceAcq.msTotal);

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* ... msRecordLoop[%ld] msRecord[%ld] endRecord[%s]\n",
		m_pcoData->traceAcq.msRecordLoop,
		m_pcoData->traceAcq.msRecord,
		_timet ? getTimestamp(Iso, _timet) : "");

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* ... msXfer[%ld] endXfer[%s]\n",
		m_pcoData->traceAcq.msXfer,
		getTimestamp(Iso, m_pcoData->traceAcq.endXferTimestamp));

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* ... xferTimeTot[%g s] xferSpeed[%g MB/s][%g fps]\n",  
		totTime, xferSpeed, framesPerSec);

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"* ... checkImgNr pco[%d] lima[%d] diff[%d]\n",  
		m_pcoData->traceAcq.checkImgNrPco,
		m_pcoData->traceAcq.checkImgNrLima,
		m_pcoData->traceAcq.checkImgNrPco -	m_pcoData->traceAcq.checkImgNrLima);

	ptr += sprintf_s(ptr, ptrMax - ptr, 
		"%s\n", m_pcoData->traceAcq.msg);

	o_sn = buff;
}
//====================================================================
//====================================================================
void Camera::getPixelRateValidValues(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	DWORD dwPixRate, dwPixRateNext ; int error, i, nr;

    _pco_GetPixelRate(dwPixRate, dwPixRateNext, error);

	for(nr = i=0; i<4; i++) {
		dwPixRate = m_pcoData->stcPcoDescription.dwPixelRateDESC[i];
		if(dwPixRate){
			nr++;
			ptr += sprintf_s(ptr, ptrMax - ptr, "%d  ",dwPixRate);
		}  
	}	

	if(nr == 0)			
		ptr += sprintf_s(ptr, ptrMax - ptr, "%d  ",nr);

	o_sn = buff;
}

//====================================================================
//====================================================================
void Camera::getLastFixedRoi(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);

//------
	unsigned int x0, x1, y0, y1;
	Roi new_roi;
	int error;

	Roi limaRoiRequested, limaRoiFixed, limaRoi;
	time_t dt;
	
	_get_logLastFixedRoi(limaRoiRequested, limaRoiFixed,  dt);

	
	_pco_GetROI(limaRoi, error);
	x0 = limaRoi.getTopLeft().x - 1;
	x1 = limaRoi.getBottomRight().x - 1;
	y0 = limaRoi.getTopLeft().y - 1;
	y1 = limaRoi.getBottomRight().y - 1;

	ptr += sprintf_s(ptr, ptrMax - ptr, "* roi PCO ACTUAL X(%d,%d) Y(%d,%d) size(%d,%d)\n",  
			x0, x1, y0, y1, x1-x0+1, y1-y0+1);

	if(dt)
	{
		limaRoi = limaRoiRequested; 
		x0 = limaRoi.getTopLeft().x - 1;
		x1 = limaRoi.getBottomRight().x - 1;
		y0 = limaRoi.getTopLeft().y - 1;
		y1 = limaRoi.getBottomRight().y - 1;

		ptr += sprintf_s(ptr, ptrMax - ptr, "* roi PCO REQUESTED X(%d,%d) Y(%d,%d) size(%d,%d) [%s]\n",  
				x0, x1, y0, y1, x1-x0+1, y1-y0+1, getTimestamp(Iso, dt));

		limaRoi = limaRoiFixed; 
		x0 = limaRoi.getTopLeft().x - 1;
		x1 = limaRoi.getBottomRight().x - 1;
		y0 = limaRoi.getTopLeft().y - 1;
		y1 = limaRoi.getBottomRight().y - 1;

		ptr += sprintf_s(ptr, ptrMax - ptr, "* roi PCO FIXED X(%d,%d) Y(%d,%d) size(%d,%d)\n",  
				x0, x1, y0, y1, x1-x0+1, y1-y0+1);
	}
	else
	{
		ptr += sprintf_s(ptr, ptrMax - ptr, "* roi PCO FIXED - no roi fixed yet!\n");  
	
	}

	bool bSymX, bSymY;
	unsigned int xMax, yMax, xSteps, ySteps, xMinSize, yMinSize;
	getXYdescription(xSteps, ySteps, xMax, yMax, xMinSize, yMinSize); 
	getRoiSymetrie(bSymX, bSymY );

	ptr += sprintf_s(ptr, ptrMax - ptr, "* xySteps[%d,%d] xyMinSize[%d,%d] xyMaxSize[%d,%d] xySym[%d,%d]\n",  
			xSteps, ySteps, xMinSize, yMinSize, xMax, yMax, bSymX, bSymY);


//------


	o_sn = buff;
}

//====================================================================
// SIP - attrib
//====================================================================
void Camera::getCDIMode(int &cdi)
{
	int error;
	WORD wCdi;

	_pco_GetCDIMode(wCdi, error);

	cdi = error ? -1 : wCdi;
}

void Camera::setCDIMode(int cdi)
{
	int error;
	WORD wCdi = (WORD) cdi;

	_pco_SetCDIMode(wCdi, error);

}




//=================================================================================================
//=================================================================================================

void Camera::getRoiSymetrie(bool &bSymX, bool &bSymY ){
	DEB_MEMBER_FUNCT();
	DEF_FNID;

	bSymY = bSymX = false;
	if(_isCameraType(Dimax)){ bSymX = bSymY = true; }
	if(_isCameraType(Edge)) { bSymY = true; }

	int adc_working, adc_max;
	_pco_GetADCOperation(adc_working, adc_max);
	if(adc_working != 1) { bSymX = true; }
}
//=================================================================================================
//=================================================================================================

void usElapsedTimeSet(LARGE_INTEGER &tick0) {

#ifndef __linux__
	QueryPerformanceCounter(&tick0);
#else
	tick0 = 0;
#endif

}

long long usElapsedTime(LARGE_INTEGER &tick0) {

#ifndef __linux__
	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER tick;   // A point in time
	long long uS, uS0;

	QueryPerformanceFrequency(&ticksPerSecond); 
	QueryPerformanceCounter(&tick);

	double ticsPerUSecond = ticksPerSecond.QuadPart/1.0e6;
	uS = (long long) (tick.QuadPart/ticsPerUSecond);
	uS0 = (long long) (tick0.QuadPart/ticsPerUSecond);

	return uS - uS0;
#else
	return 0;
#endif	
}


//====================================================================
// SIP - attrib
//====================================================================
void Camera::getTemperatureInfo(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	int error;

	_pco_GetTemperatureInfo(ptr, ptrMax, error);
	
	o_sn = buff;
}

//====================================================================
// SIP - attributes
//====================================================================


void Camera::getCoolingTemperature(int &val)
{
	int error;
	_pco_GetCoolingSetpointTemperature(val, error);

}

void Camera::setCoolingTemperature(int val)
{
	int error;
	_pco_SetCoolingSetpointTemperature(val, error);
}

//====================================================================
// SIP - attributes
//====================================================================

void Camera::getSdkRelease(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);

	ptr += sprintf_s(ptr, ptrMax - ptr, PCO_SDK_RELEASE );
	
	o_sn = buff;
}

//=================================================================================================
// SIP - attributes
//=================================================================================================
void Camera::getCameraName(std::string &o_sn)
{
  DEB_MEMBER_FUNCT();
  DEB_TRACE() << DEB_VAR1(m_pcoData->camera_name);

  o_sn = m_pcoData->camera_name;
}

void Camera::getCameraNameBase(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	int error;

	_pco_GetInfoString(1, ptr, (int) (ptrMax - ptr), error);

	o_sn = buff;
}

void Camera::getCameraNameEx(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	int error;

	_pco_GetInfoString(0, ptr, (int) (ptrMax - ptr), error);
	ptr += strlen(ptr);

	ptr += sprintf_s(ptr, ptrMax - ptr, "\nCamera name: " );
	_pco_GetInfoString(1, ptr, (int) (ptrMax - ptr), error);
	ptr += strlen(ptr);

	ptr += sprintf_s(ptr, ptrMax - ptr, "\nSensor: " );
	_pco_GetInfoString(2, ptr, (int) (ptrMax - ptr), error);
	ptr += strlen(ptr);

	o_sn = buff;
}
//=================================================================================================
// SIP - attributes
//=================================================================================================
void Camera::getBinningInfo(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	int error;

	_pco_GetBinningInfo(ptr, (int) (ptrMax - ptr), error);

	o_sn = buff;
}
void Camera::getFirmwareInfo(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	int error;

	_pco_GetFirmwareInfo(ptr, (int) (ptrMax - ptr), error);

	o_sn = buff;
}
void Camera::getRoiInfo(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	int error;

	_pco_GetRoiInfo(ptr, (int) (ptrMax - ptr), error);

	o_sn = buff;
}

//=================================================================================================
// SIP - msgLog
//=================================================================================================
void Camera::getMsgLog(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);

	// 0 -> decreasing / older - last
	// 1 -> increasing / newer - last
	m_msgLog->dump(ptr, (int)(ptrMax - ptr), 1);

	o_sn = buff;
}

//=================================================================================================
// SIP - msgLog
//=================================================================================================
void Camera::getCamerasFound(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);

	ptr += sprintf_s(ptr, ptrMax - ptr, "search:\n%s",m_pcoData->camerasFound);
	ptr += sprintf_s(ptr, ptrMax - ptr, "opened:\n%s",_getCameraIdn());

	o_sn = buff;
}
//====================================================================
// SIP - attrib
//====================================================================
void Camera::getDoubleImageMode(int &mode)
{
	int error;
	WORD wMode;

	_pco_GetDoubleImageMode(wMode, error);

	mode = error ? -1 : wMode;
}

void Camera::setDoubleImageMode(int mode)
{
	int error;
	WORD wMode = (WORD) mode;

	_pco_SetDoubleImageMode(wMode, error);

}

//====================================================================
// SIP - attrib
//====================================================================
void Camera::getDebugInt(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	unsigned long long debugLevel;
			
	debugLevel = m_pcoData->debugLevel;
	ptr += sprintf_s(ptr, ptrMax - ptr, "0x%llx (%lld)",  debugLevel, debugLevel);

	o_sn = buff;

}

void Camera::setDebugInt(std::string &i_sn) 
{
	DEB_MEMBER_FUNCT();

	unsigned long long debugLevel;
	const char *strIn = i_sn.c_str();

#ifdef __linux__
 	debugLevel = atoll(strIn);
#else
	debugLevel = _atoi64(strIn);
#endif
	m_pcoData->debugLevel = debugLevel;
	
}


void Camera::getDebugIntTypes(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
			
#define _PRINT_DBG( x )	ptr += sprintf_s(ptr, ptrMax - ptr, "%15s  0x%08x\n", #x, x ) 	

	ptr += sprintf_s(ptr, ptrMax - ptr, "\n");
	_PRINT_DBG( DBG_BUFF ) ;
	_PRINT_DBG( DBG_XFER2LIMA ) ;
	_PRINT_DBG( DBG_LIMABUFF ) ;
	_PRINT_DBG( DBG_EXP ) ;
	_PRINT_DBG( DBG_XFERMULT ) ;
	_PRINT_DBG( DBG_XFERMULT1 ) ;
	_PRINT_DBG( DBG_ASSIGN_BUFF ) ;
	_PRINT_DBG( DBG_DUMMY_IMG ) ;
	_PRINT_DBG( DBG_WAITOBJ ) ;
	_PRINT_DBG( DBG_XFER_IMG ) ;
	_PRINT_DBG( DBG_ROI ) ;

	o_sn = buff;

}


//====================================================================
// SIP - attrib
//====================================================================
void Camera::setTest(int val) 
{
	DEB_MEMBER_FUNCT();
	
	int val0 = val;
	val *= 2;			

	DEB_ALWAYS() << DEB_VAR2(val0, val);
}

void Camera::getTest(int &val) 
{
	DEB_MEMBER_FUNCT();
	int valIn;

	valIn = val;
	val += 10;

	DEB_ALWAYS() << DEB_VAR2(valIn, val);
}

//====================================================================
// SIP - attrib
//====================================================================
void Camera::setTimestampMode(int val) 
{
	DEB_MEMBER_FUNCT();
	int err;	

	WORD mode = val;

	_pco_SetTimestampMode(mode, err);
}

void Camera::getTimestampMode(int &val) 
{
	DEB_MEMBER_FUNCT();
	int err;	
	WORD mode;
	val = 0;
	_pco_GetTimestampMode(mode, err);

	if(!err) val = mode;

}
//====================================================================
// SIP - attrib
//====================================================================
void Camera::getBitAlignment(std::string &o_sn) 
{
	char *ptr = buff;
	char *ptrMax = buff + sizeof(buff);
	int alignment, error;

	error = _pco_GetBitAlignment(alignment);

	if(error)
	{
		ptr += sprintf_s(ptr, ptrMax - ptr, "ERROR");
	}
	else
	{
		ptr += sprintf_s(ptr, ptrMax - ptr, "%s(%d)", (alignment ? "LSB" : "MSB") , alignment);
	}

	o_sn = buff;

}

void Camera::setBitAlignment(std::string &i_sn) 
{
	DEB_MEMBER_FUNCT();

	const char *strIn = i_sn.c_str();
	int _val;

#ifdef __linux__
 	debugLevel = atoll(strIn);
#else
	if( (_stricmp(strIn, "0") == 0) || (_stricmp(strIn, "MSB") == 0) )
	{
		_val = 0;
	}
	else 
	{
		if( (_stricmp(strIn, "1") == 0) || (_stricmp(strIn, "LSB") == 0) )
		{
			_val = 1;
		}
		else
		{
			DEB_ALWAYS() << "ERROR - invalid value: " << strIn;
			return;
		}
	}
#endif
	_pco_SetBitAlignment(_val);
	return;
	
}
