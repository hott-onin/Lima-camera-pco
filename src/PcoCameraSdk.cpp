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
//#include <process.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <time.h>

#include "lima/Exceptions.h"
#include "lima/HwSyncCtrlObj.h"

#include "PcoCamera.h"
#include "PcoSyncCtrlObj.h"

using namespace lima;
using namespace lima::Pco;

void _pco_time2dwbase(double exp_time, DWORD &dwExp, WORD &wBase);

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
#define LEN_ERRSTR 127

WORD Camera::_pco_GetActiveRamSegment()
{
	DEB_MEMBER_FUNCT();

	WORD act_segment;
	int err;
	char errstr[LEN_ERRSTR+1];

	if((m_pcoData->stcPcoDesc1.dwGeneralCaps1&GENERALCAPS1_NO_RECORDER)==0)
	{
		err=PCO_GetActiveRamSegment(m_handle,&act_segment);
		if(err!=PCO_NOERROR)
		{
			PCO_GetErrorText(err, errstr,LEN_ERRSTR);
			DEB_ALWAYS() << "ERROR: " << DEB_VAR2(err, errstr);
		}
	}
	else
		act_segment=1;

	m_pcoData->wActiveRamSegment = act_segment;

	return act_segment;
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

    error=camera->PCO_SetBitAlignment(wBitAlignment);
    msg = "PCO_SetBitAlignment" ; PCO_CHECK_ERROR(error, msg);
    PCO_THROW_OR_TRACE(error, msg) ;

	
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
	const char *msg __attribute__((unused));

	int error;
	WORD wADCOperation;

	adc_max = m_pcoData->stcPcoDesc1.wNumADCsDESC; // nr of ADC in the system

	if(adc_max == 2) {
	
	error=camera->PCO_GetADCOperation(&wADCOperation);
    PCO_CHECK_ERROR(error, "PCO_GetADCOperation");
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
	const char *msg __attribute__((unused));

	int error, adc_max;

	error = _pco_GetADCOperation(adc_working, adc_max);

	DEB_ALWAYS() << fnId << ": " DEB_VAR2(adc_max, adc_working);

	if(error) return error;

	if((adc_new >=1) && (adc_new <= adc_max) && (adc_new != adc_working) ){
	
        error=camera->PCO_SetADCOperation((WORD) adc_new);
        PCO_CHECK_ERROR(error, "PCO_SetADCOperation");
		_pco_GetADCOperation(adc_working, adc_max);
	}
	m_pcoData->wNowADC = adc_working;
	return error;
}


//=================================================================================================
//=================================================================================================
void Camera::_pco_SetPixelRate(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	error = 0;
	const char *msg;
	DWORD _dwPixelRate, _dwPixelRateOld, _dwPixelRateReq;
	DWORD _dwPixelRateMax __attribute__((unused));
    DWORD _dwPixelRateNext;

	if(_isCameraType(Edge)) {

        _pco_GetPixelRate(m_pcoData->dwPixelRate, _dwPixelRateNext, error);
		PCO_THROW_OR_TRACE(error, "_pco_GetPixelRate") ;

		_dwPixelRateOld = m_pcoData->dwPixelRate;
		_dwPixelRateReq = m_pcoData->dwPixelRateRequested;
		//DEB_ALWAYS() << "PIXEL rate (actual/req): " << DEB_VAR2(_dwPixelRateOld, _dwPixelRateReq) ;

		if(_isValid_pixelRate(_dwPixelRateReq) && (_dwPixelRateOld != _dwPixelRateReq)) {

            error=camera->PCO_SetPixelRate(_dwPixelRateReq);
            msg = "PCO_SetPixelRate" ; PCO_CHECK_ERROR(error, msg);
			PCO_THROW_OR_TRACE(error, msg) ;

            _pco_GetPixelRate(m_pcoData->dwPixelRate, _dwPixelRateNext, error);
    		PCO_THROW_OR_TRACE(error, "_pco_GetPixelRate") ;

			_dwPixelRate = m_pcoData->dwPixelRate;
			//DEB_ALWAYS() << "PIXEL rate SET (old/new): "  << DEB_VAR2(_dwPixelRateOld, _dwPixelRate) ;

			_armRequired(true);
		}
		m_pcoData->dwPixelRateRequested = 0;
		return;
	}

	if(_isCameraType(Pco2k | Pco4k)) {

        _pco_GetPixelRate(m_pcoData->dwPixelRate, _dwPixelRateNext, error);
		PCO_THROW_OR_TRACE(error, "_pco_GetPixelRate") ;


		_dwPixelRateOld = m_pcoData->dwPixelRate;
		_dwPixelRateMax = m_pcoData->dwPixelRateMax;
		_dwPixelRateReq = m_pcoData->dwPixelRateRequested;

		//DEB_ALWAYS() << "PIXEL rate (requested/actual/max): " << DEB_VAR3(_dwPixelRateReq, _dwPixelRateOld, _dwPixelRateMax) ;

		if(_isValid_pixelRate(_dwPixelRateReq) && (_dwPixelRateOld != _dwPixelRateReq)) {
		
            error=camera->PCO_SetPixelRate(_dwPixelRateReq);
            msg = "PCO_SetPixelRate" ; PCO_CHECK_ERROR(error, msg);
			PCO_THROW_OR_TRACE(error, msg) ;

            _pco_GetPixelRate(m_pcoData->dwPixelRate, _dwPixelRateNext, error);
			PCO_THROW_OR_TRACE(error, "_pco_GetPixelRate") ;
			
			_dwPixelRate = m_pcoData->dwPixelRate;
			DEB_ALWAYS() << "PIXEL rate SET (old/new): "  << DEB_VAR2(_dwPixelRateOld, _dwPixelRate) ;

			_armRequired(true);
		}
		return;
	}
return;
}
//=================================================================================================
//=================================================================================================
int Camera::_pco_GetBitAlignment(int &alignment){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	int error = 0;
	//const char *msg;
	WORD wBitAlignment;
	
    error=camera->PCO_GetBitAlignment(&wBitAlignment);
    PCO_CHECK_ERROR(error, "PCO_GetBitAlignment");
    PCO_THROW_OR_TRACE(error, "PCO_GetBitAlignment") ;
	
	alignment = m_pcoData->wBitAlignment = wBitAlignment;

	return error;
}
//=================================================================================================
//=================================================================================================

/******************************************************************************************
typedef struct
{
  DWORD  FrameTime_ns;                 // Frametime replaces COC_Runtime
  DWORD  FrameTime_s;   

  DWORD  ExposureTime_ns;
  DWORD  ExposureTime_s;               // 5

  DWORD  TriggerSystemDelay_ns;        // System internal min. trigger delay

  DWORD  TriggerSystemJitter_ns;       // Max. possible trigger jitter -0/+ ... ns

  DWORD  TriggerDelay_ns;              // Resulting trigger delay = system delay
  DWORD  TriggerDelay_s;               // + delay of SetDelayExposureTime ... // 9

} PCO_ImageTiming;
******************************************************************************************/


int Camera::_pco_GetImageTiming(double &frameTime, double &expTime, double &sysDelay, double &sysJitter, double &trigDelay ){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg __attribute__((unused));
	int error;

	SC2_Get_Image_Timing_Response pstrImageTiming;
	//PCO_ImageTiming pstrImageTiming;

    error=camera->PCO_GetImageTiming(&pstrImageTiming);
    msg = "PCO_GetImageTiming" ; PCO_CHECK_ERROR(error, msg);

	//PCON2(error, msg,PCO_GetImageTiming, m_handle, &pstrImageTiming);

	frameTime = (pstrImageTiming.FrameTime_ns * NANO) + pstrImageTiming.FrameTime_s ;
	expTime = (pstrImageTiming.ExposureTime_ns * NANO) + pstrImageTiming.ExposureTime_s ;
	sysDelay = (pstrImageTiming.TriggerSystemDelay_ns * NANO) ;
	sysJitter = (pstrImageTiming.TriggerSystemJitter_ns * NANO) ;
	trigDelay = (pstrImageTiming.TriggerDelay_ns * NANO) + pstrImageTiming.TriggerDelay_s ;



	return error;
}
//=================================================================================================
//=================================================================================================
const char *Camera::_pco_SetCamLinkSetImageParameters(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
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


	return fnId;
}


#define USERSET
//=================================================================================================
//=================================================================================================
void Camera::_pco_SetTransferParameter_SetActiveLookupTable(int &err){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	struct stcPcoData _pcoData;
	char *pbla = mybla;
	const char *msg;
	DWORD pixelrate, pixRateNext;
	WORD width, height, wXResMax, wYResMax;
	WORD actlut;
	//WORD lutparam;
	int pcoBuffNr = 10;

    _pco_GetSizes( &width, &height, &wXResMax, &wYResMax, err);

#ifdef USERSET
    //transfer dataformat must be changed depending on pixelrate and horizontal resolution
    WORD lut;

    _pco_GetPixelRate(pixelrate, pixRateNext, err);
    err=camera->PCO_GetPixelRate(&pixelrate);
    msg = "PCO_GetPixelRate" ; PCO_CHECK_ERROR(err, msg);


    if((width>1920)&&(pixelrate>=286000000)&&(camtype==CAMERATYPE_PCO_EDGE))
    {
        clpar.DataFormat=SCCMOS_FORMAT_TOP_CENTER_BOTTOM_CENTER|PCO_CL_DATAFORMAT_5x12;
        lut=0x1612;
		pbla += sprintf_s(pbla,myblamax - pbla, 
		    "%s> width[%d] > 1920  && pixelrate[%d] >= 286000000 -> Dataformat[0x%x] lut[0x%x]",
					fnId, width, pixelrate,clpar.DataFormat, lut);
		m_pcoData->sClTransferParameterSettings = "Edge 5x12 topCenter botCenter lut 0x1612";
    }
    else
    {
        clpar.DataFormat=SCCMOS_FORMAT_TOP_CENTER_BOTTOM_CENTER|PCO_CL_DATAFORMAT_5x16;
        lut=0;
		pbla += sprintf_s(pbla,myblamax - pbla, 
		    "%s> width[%d] <= 1920  && pixelrate[%d] < 286000000 -> Dataformat[0x%x] lut[0x%x]",
					fnId, width, pixelrate,clpar.DataFormat, lut);
		m_pcoData->sClTransferParameterSettings = "PCO 5x16 topCenter botCenter lut 0";
    }

    pbla += sprintf_s(pbla,myblamax - pbla, 
		    " / width[%d][%d] height[%d][%d]", width, wXResMax, height, wYResMax);

    DEB_ALWAYS() << mybla;
    //mylog->writelog(INFO_M, "%s", bla);
    mylog->writelog(INFO_M, mybla);
    
    actlut=lut; 
    err=camera->PCO_SetLut(actlut,0);
    msg = "PCO_SetLut" ; PCO_CHECK_ERROR(err, msg);


    err=camera->PCO_SetTransferParameter(&clpar,sizeof(clpar));
    if(err!=PCO_NOERROR)
        printf("PCO_TransferParameter() Error 0x%x\n",err);

    err=camera->PCO_ArmCamera();
    msg = "PCO_ArmCamera()" ; PCO_CHECK_ERROR(err, msg);


    err=grabber->Set_DataFormat(clpar.DataFormat);
    msg = "Set_DataFormat" ; PCO_CHECK_ERROR(err, msg);


    err=grabber->Set_Grabber_Size(width,height);
    msg = "Set_Grabber_Size" ; PCO_CHECK_ERROR(err, msg);

    err=grabber->PostArm(1);
    msg = "PostArm(1)" ; PCO_CHECK_ERROR(err, msg);

#else

    err=grabber->PostArm();
    msg = "PostArm(0)" ; PCO_CHECK_ERROR(err, msg);
#endif


    err=grabber->Allocate_Framebuffer(pcoBuffNr);
    msg = "Allocate_Framebuffer" ; PCO_CHECK_ERROR(err, msg);
    err = 0;
    
}


//=================================================================================================
//=================================================================================================
void Camera::_pco_SetTransferParameter_SetActiveLookupTable_win(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	struct stcPcoData _pcoData;
	//const char msg[ERRMSG_SIZE + 1];
	const char *pcoFn;
	bool doLut = false;
	const char *msg;

	// sizes are only updated AFTER arm, so i will use the last roi settings
	// to get the size. after arm, the size is updated with this value
	WORD wXResActual = m_pcoData->wRoiX1Now - m_pcoData->wRoiX0Now +1;

	//================================================================================================
	// PCO_SetTransferParameter
	//================================================================================================

	if (_getInterfaceType()==INTERFACE_CAMERALINK) 
	{

        error=camera->PCO_GetTransferParameter(&m_pcoData->clTransferParam, sizeof(PCO_SC2_CL_TRANSFER_PARAM));
        msg = "PCO_GetTransferParameter" ; PCO_CHECK_ERROR(error, msg);
		//PCOFN3(error, pcoFn,PCO_GetTransferParameter, m_handle, &m_pcoData->clTransferParam, sizeof(PCO_SC2_CL_TRANSFER_PARAM));
		PCO_THROW_OR_TRACE(error, pcoFn) ;
	
		memcpy(&_pcoData.clTransferParam, &m_pcoData->clTransferParam,sizeof(PCO_SC2_CL_TRANSFER_PARAM));
	
		m_pcoData->clTransferParam.baudrate = PCO_CL_BAUDRATE_115K2;

		if(_isCameraType(Dimax)){
				//m_pcoData->clTransferParam.Transmit = 1;
				//_pcoData.clTransferParam.Transmit = m_pcoData->clTransferParam.Transmit;
				m_pcoData->clTransferParam.DataFormat=PCO_CL_DATAFORMAT_2x12; //=2
					m_pcoData->sClTransferParameterSettings = "Dimax 2x12";
		} else 

		if(_isCameraType(EdgeGL)) {
			m_pcoData->clTransferParam.Transmit = 1;
			m_pcoData->clTransferParam.DataFormat=PCO_CL_DATAFORMAT_5x12 | 
				SCCMOS_FORMAT_TOP_CENTER_BOTTOM_CENTER;
				//SCCMOS_FORMAT_TOP_BOTTOM;
			m_pcoData->wLUT_Identifier = 0; //Switch LUT->off
			m_pcoData->sClTransferParameterSettings = "EdgeGL 5x12 topCenter botCenter lutNone";
			doLut = true;
		} else 
			
		if(_isCameraType(EdgeRolling)){
				m_pcoData->clTransferParam.Transmit = 1;

				if(m_pcoData->dwPixelRate <= PCO_EDGE_PIXEL_RATE_LOW){
					m_pcoData->clTransferParam.DataFormat=PCO_CL_DATAFORMAT_5x16 | 
						SCCMOS_FORMAT_TOP_CENTER_BOTTOM_CENTER;
					m_pcoData->wLUT_Identifier = PCO_EDGE_LUT_NONE; // Switch LUT->off
					m_pcoData->sClTransferParameterSettings = "EdgeR 5x16 topCenter botCenter lutNone";
				} else 
				if( ((m_pcoData->dwPixelRate >= PCO_EDGE_PIXEL_RATE_HIGH) & 
						(wXResActual > PCO_EDGE_WIDTH_HIGH))) {
					m_pcoData->clTransferParam.DataFormat=PCO_CL_DATAFORMAT_5x12L | 
						SCCMOS_FORMAT_TOP_CENTER_BOTTOM_CENTER;
					m_pcoData->wLUT_Identifier = PCO_EDGE_LUT_SQRT; //Switch LUT->sqrt
					m_pcoData->sClTransferParameterSettings = "EdgeR 5x12 topCenter botCenter lutSqrt";
				} else {
					m_pcoData->clTransferParam.DataFormat = PCO_CL_DATAFORMAT_5x16 | 
						SCCMOS_FORMAT_TOP_CENTER_BOTTOM_CENTER;
					m_pcoData->wLUT_Identifier = PCO_EDGE_LUT_NONE; // Switch LUT->off
					m_pcoData->sClTransferParameterSettings = "EdgeR 5x16 topCenter botCenter lutNone";
				}
				doLut = true;
		} 

		if((_pcoData.clTransferParam.baudrate != m_pcoData->clTransferParam.baudrate) ||
			(_pcoData.clTransferParam.DataFormat != m_pcoData->clTransferParam.DataFormat) ||
			(_pcoData.clTransferParam.Transmit != m_pcoData->clTransferParam.Transmit)	)
		{
			DEB_ALWAYS() << "ERROR PCO_SetTransferParameter (ckTransferParam)" ;

            error=camera->PCO_SetTransferParameter(&m_pcoData->clTransferParam, sizeof(m_pcoData->clTransferParam));
            msg = "PCO_SetTransferParameter" ; PCO_CHECK_ERROR(error, msg);
			//PCON3(error, pcoFn,PCO_SetTransferParameter,m_handle, &m_pcoData->clTransferParam, sizeof(m_pcoData->clTransferParam));
			if(error){
			    char *msg = NULL;
				sprintf_s(msg,ERRMSG_SIZE, "PCO_SetTransferParameter - baudrate[%d][%d] dataFormat[x%08x][x%08x] trasmit[%d][%d]",
					_pcoData.clTransferParam.baudrate, m_pcoData->clTransferParam.baudrate,
					_pcoData.clTransferParam.DataFormat, m_pcoData->clTransferParam.DataFormat,
					_pcoData.clTransferParam.Transmit, m_pcoData->clTransferParam.Transmit);
				throw LIMA_HW_EXC(Error, msg); 
			} 
			_armRequired(true);
		}
	} // if cameralink



	//================================================================================================
	// PCO_SetActiveLookupTable
	//================================================================================================

	if(doLut) {
		WORD _wLUT_Identifier, _wLUT_Parameter;

		PCO_FN3(error, pcoFn,PCO_GetActiveLookupTable, m_handle, &_wLUT_Identifier, &_wLUT_Parameter);
	    PCO_THROW_OR_TRACE(error, pcoFn) ;

		if(_wLUT_Identifier != m_pcoData->wLUT_Identifier) {
			PCO_FN3(error, pcoFn,PCO_SetActiveLookupTable, m_handle, &m_pcoData->wLUT_Identifier, &m_pcoData->wLUT_Parameter);
		    PCO_THROW_OR_TRACE(error, pcoFn) ;
			_armRequired(true);

			PCO_FN3(error, pcoFn,PCO_GetActiveLookupTable, m_handle, &m_pcoData->wLUT_Identifier, &m_pcoData->wLUT_Parameter);
		    PCO_THROW_OR_TRACE(error, pcoFn) ;
		}
	}



	return;
}
//=================================================================================================
//=================================================================================================
const char * Camera::_pco_SetTriggerMode_SetAcquireMode(int &error){
	DEB_MEMBER_FUNCT();
	const char *msg;


	DEF_FNID;
	//------------------------------------------------- triggering mode 
    lima::TrigMode limaTrigMode;
	WORD trigmode;
	WORD acqmode;
	
	m_sync->getTrigMode(limaTrigMode);
	m_sync->xlatLimaTrigMode2Pco( limaTrigMode,trigmode,acqmode, m_pcoData->bExtTrigEnabled, error); 
	
    error=camera->PCO_SetTriggerMode(trigmode);
    msg = "PCO_SetTriggerMode" ; PCO_CHECK_ERROR(error, msg);
	if(error) 
	{
        DEB_ALWAYS() << "ERROR PCO_SetTriggerMode" << DEB_VAR1(trigmode) ;
	    return msg;
    }
	//PCO_THROW_OR_TRACE(error, "PCO_SetTriggerMode") ;
	//DEB_TRACE() << DEB_VAR1(trigmode);

	
	//------------------------------------- acquire mode : ignore or not ext. signal


    error=camera->PCO_SetAcquireMode(acqmode);
    msg = "PCO_SetAcquireMode" ; PCO_CHECK_ERROR(error, msg);
	if(error) 
	{
        DEB_ALWAYS() << "ERROR PCO_SetAcquireMode" << DEB_VAR1(acqmode) ;
	    return msg;
    }
   //PCO_THROW_OR_TRACE(error, "PCO_SetAcquireMode") ;

	return fnId;
}



//=================================================================================================
// ----------------------------------------- storage mode (recorder + sequence)
// current storage mode
//
// case RecSeq
// case RecRing
// - 0x0000 = [recorder] mode
//		. images are recorded and stored within the internal camera memory (camRAM)
//      . Live View transfers the most recent image to the PC (for viewing / monitoring)
//      . indexed or total image readout after the recording has been stopped
//
// case Fifo
// - 0x0001 = [FIFO buffer] mode
//      . all images taken are transferred to the PC in chronological order
//      . camera memory (camRAM) is used as huge FIFO buffer to bypass short bottlenecks in data transmission
//      . if buffer overflows, the oldest images are overwritten
//      . if Set Recorder = [stop] is sent, recording is stopped and the transfer of the current image to the PC is finished.
//      . Images not read are stored within the segment and can be read with the Read Image From Segment command.
//
// current recorder submode:
//
// case RecSeq
// - 0x0000 = [sequence]
//      . recording is stopped when the allocated buffer is full
//
// case RecRing
// - 0x0001 = [ring buffer].
//      . camera records continuously into ring buffer
//      . if the allocated buffer overflows, the oldest images are overwritten
//      . recording is stopped by software or disabling acquire signal (<acq enbl>)
//
// for the case of ExtTrigSingle (dimax) we use RecRing
//    case RecRing
//       StorageMode 0 - record mode
//       RecorderSubmode 1 - ring buffer
//  Triggermode 0 - auto
//  Acquiremode 0 - auto / ignored
//=================================================================================================
//=================================================================================================
// sets storage mode and subrecord mode
//    PCO_SetStorageMode
//    PCO_SetRecorderSubmode
//=================================================================================================
const char * Camera::_pco_SetStorageMode_SetRecorderSubmode(enumPcoStorageMode mode, int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg, *sMode;

	sMode = "invalid";
	switch(mode) {
		case RecSeq:  m_pcoData->storage_mode = 0; m_pcoData->recorder_submode = 0; sMode = "RecSeq" ; break;
		case RecRing: m_pcoData->storage_mode = 0; m_pcoData->recorder_submode = 1; sMode = "RecRing" ;  break;
		case Fifo:    m_pcoData->storage_mode = 1; m_pcoData->recorder_submode = 0;  sMode = "Fifo" ; break;
		default: 
			throw LIMA_HW_EXC(Error,"FATAL - invalid storage mode!" );
	}
    DEB_ALWAYS() << "\n>>> storage/recorder mode: " << DEB_VAR2(sMode, mode) ;

	traceAcq.sPcoStorageRecorderMode = sMode;
	traceAcq.iPcoStorageMode = m_pcoData->storage_mode;
	traceAcq.iPcoRecorderSubmode = m_pcoData->recorder_submode;

    error=camera->PCO_SetStorageMode(m_pcoData->storage_mode);
    msg = "PCO_SetStorageMode" ; PCO_CHECK_ERROR(error, msg);
    //PCO_THROW_OR_TRACE(error, msg) ;

    error=camera->PCO_SetRecorderSubmode(m_pcoData->recorder_submode);
    msg = "PCO_SetRecorderSubmode" ; PCO_CHECK_ERROR(error, msg);
    //PCO_THROW_OR_TRACE(error, msg) ;

	return fnId;
}

//=================================================================================================
//=================================================================================================
int Camera::_pco_GetStorageMode_GetRecorderSubmode(){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg;

	WORD wStorageMode, wRecSubmode;
	int error;

    error=camera->PCO_GetStorageMode(&wStorageMode);
    msg = "PCO_GetStorageMode" ; PCO_CHECK_ERROR(error, msg);
	if(error){ 
		    PCO_THROW_OR_TRACE(error, msg) ;
	}


    error=camera->PCO_GetRecorderSubmode(&wRecSubmode);
    msg = "PCO_getRecorderSubmode" ; PCO_CHECK_ERROR(error, msg);
    if(error) {
		    PCO_THROW_OR_TRACE(error, msg) ;
	}

	m_pcoData->storage_mode = wStorageMode;
	m_pcoData->recorder_submode = wRecSubmode;

	if((wStorageMode == 0) && (wRecSubmode == 0)) { m_pcoData->storage_str= "RecSeq"; return RecSeq; }
	if((wStorageMode == 0) && (wRecSubmode == 1)) { m_pcoData->storage_str= "RecRing"; return RecRing; }
	if((wStorageMode == 1) && (wRecSubmode == 0)) { m_pcoData->storage_str= "Fifo"; return Fifo; }

	m_pcoData->storage_str= "INVALID"; 
	return RecInvalid;
}


//=================================================================================================
//=================================================================================================
void Camera::_pco_GetHWIOSignal(int &errorTot){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	int error;
	errorTot = 0;
	const char *msg __attribute__((unused));

	int i, imax;
	
	if(!( _isCameraType(Dimax | Edge | Pco2k | Pco4k))  ) {
		errorTot = -1;
		return;
	}

    error=camera->PCO_GetHWIOSignalCount(&m_pcoData->wNrPcoHWIOSignal0);
    msg = "PCO_GetHWIOSignalCount" ; PCO_CHECK_ERROR(error, msg);
	errorTot |= error;

	imax = m_pcoData->wNrPcoHWIOSignal = 
		(m_pcoData->wNrPcoHWIOSignal0 <= SIZEARR_stcPcoHWIOSignal) ? m_pcoData->wNrPcoHWIOSignal0 : SIZEARR_stcPcoHWIOSignal;

	//DEB_ALWAYS()  << "--- size" << DEB_VAR3(imax, m_pcoData->wNrPcoHWIOSignal0 , m_pcoData->wNrPcoHWIOSignal ) ;

    WORD Enabled,Type,Polarity,FilterSetting, Selected;

	for(i=0; i< imax; i++) {
		//DEB_ALWAYS()  << "---  descriptor" << DEB_VAR2(i, m_pcoData->stcPcoHWIOSignalDesc[i].wSize) ;
        
        error=camera->PCO_GetHWIOSignalDescriptor( i, 
            (SC2_Get_HW_IO_Signal_Descriptor_Response *) &m_pcoData->stcPcoHWIOSignalDesc[i]);
        msg = "PCO_GetHWIOSignalDescriptor" ; PCO_CHECK_ERROR(error, msg);
		errorTot |= error;

		//DEB_ALWAYS()  << "---  signal" << DEB_VAR2(i, m_pcoData->stcPcoHWIOSignal[i].wSize) ;

        error=camera->PCO_GetHWIOSignal( i, &Enabled,&Type, &Polarity,&FilterSetting, &Selected);
        msg = "PCO_GetHWIOSignal" ; PCO_CHECK_ERROR(error, msg);

        m_pcoData->stcPcoHWIOSignal[i].wEnabled = Enabled;
		m_pcoData->stcPcoHWIOSignal[i].wType =Type;
		m_pcoData->stcPcoHWIOSignal[i].wPolarity = Polarity;
		m_pcoData->stcPcoHWIOSignal[i].wFilterSetting = FilterSetting;
		m_pcoData->stcPcoHWIOSignal[i].wSelected = Selected;
		//PCO3(error, msg,PCO_GetHWIOSignal, m_handle, i, &m_pcoData->stcPcoHWIOSignal[i]);
		errorTot |= error;
	}

}


//=================================================================================================
//=================================================================================================

/**************************************************************************************************
	name[Acquire Enable] idx[0] num[0]
	-def:     def[0x1] type[0xf] pol[0x3] filt[0x7]
	-sig:    enab[0x1] type[0x1] pol[0x1] filt[0x1] sel[0x0]

	name[Exposure Trigger] idx[1] num[1]
	-def:     def[0x1] type[0xf] pol[0xc] filt[0x7]
	-sig:    enab[0x1] type[0x1] pol[0x4] filt[0x1] sel[0x0]

	name[Status Expos] idx[2] num[2]
	-def:     def[0x3] type[0x1] pol[0x3] filt[0x0]
	-sig:    enab[0x1] type[0x1] pol[0x1] filt[0x0] sel[0x0]

	name[Ready Status] idx[3] num[3]
	-def:     def[0x3] type[0x1] pol[0x3] filt[0x0]
	-sig:    enab[0x1] type[0x1] pol[0x1] filt[0x0] sel[0x0]

	name[Set Ready] idx[4] num[4]
	-def:     def[0x1] type[0xf] pol[0x3] filt[0x7]
	-sig:    enab[0x1] type[0x1] pol[0x1] filt[0x1] sel[0x0]
**************************************************************************************************/


void Camera::_pco_initHWIOSignal(int mode, int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;

	int  _err, idx;
	error = 0;
	char *name;
	WORD val;


	if(!( _isCameraType(Dimax |Edge))  ) {
		error = -1;
		return;
	}

	_pco_GetHWIOSignal(_err); error |= _err;

	//	name[Acquire Enable] idx[0] num[0]
	idx = 0; val = 2;
	name = m_pcoData->stcPcoHWIOSignalDesc[idx].strSignalName[0];
	m_pcoData->stcPcoHWIOSignal[idx].wPolarity = 2;

	_pco_SetHWIOSignal(idx, _err); error |= _err;

	DEB_ALWAYS() << "set PcoHWIOSignal polarity "  << DEB_VAR3(name, idx, val) ;


}


//=================================================================================================
//=================================================================================================
void Camera::_pco_SetDelayExposureTime(int &error, int ph){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	bool doIt;
	//const char *msg;
	int err;

    DWORD dwExposure, dwDelay;
	WORD wExposure_base, wDelay_base;
    double _exposure, _delay;
    m_sync->getExpTime(_exposure);
    m_sync->getLatTime(_delay);
	double _delay0 = _delay;

	doIt = TRUE;
	
	ph = 0;
	
	if(ph != 0){ 
		doIt = FALSE;

		if((_isCameraType(Edge)) && (m_pcoData->dwPixelRate >= PCO_EDGE_PIXEL_RATE_HIGH) ) {
			double pixels = ((double) m_pcoData->wXResActual)* ((double) m_pcoData->wYResActual);
			double bytes = (m_pcoData->wLUT_Identifier == PCO_EDGE_LUT_SQRT) ? 1.5 : 2.0;
			double period = bytes * pixels / (m_pcoData->fTransferRateMHzMax * 1000000.);

			printf("--- %s>period[%g] -> cocRunTime[%g]\n", fnId, period , m_pcoData->cocRunTime);
			if(period > m_pcoData->cocRunTime) {
				_delay += period - m_pcoData->cocRunTime;
				doIt = TRUE;
				printf("--- %s> delay forced [%g] -> [%g]\n", fnId, _delay0, _delay);
			}
		}
	}

	if(!doIt) return;
	
	
	
    error = 0;	

	_pco_time2dwbase(_exposure, dwExposure, wExposure_base);
	_pco_time2dwbase(_delay,  dwDelay, wDelay_base);
	
	
    err=camera->PCO_SetTimebase(wDelay_base,wExposure_base);
    PCO_CHECK_ERROR(err, "PCO_SetTimebase");
    error |= err;
    
    err=camera->PCO_SetDelayExposure(dwDelay,dwExposure);
    PCO_CHECK_ERROR(err, "PCO_SetDelayExposure");
    error |= err;


	if(error || _getDebug(DBG_EXP)) {
		DEB_ALWAYS() << DEB_VAR3(_exposure, dwExposure, wExposure_base);
		DEB_ALWAYS() << DEB_VAR3(_delay,  dwDelay, wDelay_base);
	}

	return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_SetHWIOSignal(int sigNum, int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg __attribute__((unused));


		if(!( _isCameraType(Dimax |Edge))  || 
			(sigNum < 0) || (sigNum >= m_pcoData->wNrPcoHWIOSignal) ) {
			error = -1;
			return;
		}

        error=camera->PCO_SetHWIOSignal( sigNum,
            m_pcoData->stcPcoHWIOSignal[sigNum].wEnabled,
		    m_pcoData->stcPcoHWIOSignal[sigNum].wType,
		    m_pcoData->stcPcoHWIOSignal[sigNum].wPolarity,
		    m_pcoData->stcPcoHWIOSignal[sigNum].wFilterSetting,
		    m_pcoData->stcPcoHWIOSignal[sigNum].wSelected);

        msg = "PCO_SetHWIOSignal" ; PCO_CHECK_ERROR(error, msg);

		//PCO3(error, msg,PCO_SetHWIOSignal, m_handle, sigNum, &m_pcoData->stcPcoHWIOSignal[sigNum]);
		
}





//=================================================================================================
//=================================================================================================
const char *Camera::_pco_SetMetaDataMode(WORD wMetaDataMode, int &error){
		
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg;

	m_pcoData->wMetaDataSize = m_pcoData->wMetaDataVersion = 0;
	if(_isCameraType(Dimax)) {
		m_pcoData->wMetaDataMode = wMetaDataMode;

        error=camera->PCO_SetMetadataMode(wMetaDataMode, &m_pcoData->wMetaDataSize, &m_pcoData->wMetaDataVersion);
        msg = "PCO_SetMetadataMode" ; PCO_CHECK_ERROR(error, msg);
    	if(error) return msg;
	}
	return fnId;
}
//=================================================================================================
//=================================================================================================
void Camera::_pco_GetSizes( WORD *wXResActual, WORD *wYResActual, WORD *wXResMax,WORD *wYResMax, int &error){
		
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg;
   	char *bla = mybla;

    DWORD dwXResActual, dwYResActual;
    //DWORD dwXResMax, dwYResMax;

    error = 0;
         
    error=camera->PCO_GetActualSize(&dwXResActual, &dwYResActual);
    msg = "PCO_GetActualSize" ; PCO_CHECK_ERROR(error, msg);

    //PCOFN5(error, msg,PCO_GetSizes , m_handle, &dwXResActual, &dwYResActual, &dwXResMax, &dwYResMax) ;

    if(error) return;

    *wXResActual = (WORD)dwXResActual;
    *wYResActual = (WORD)dwYResActual;
    *wXResMax = m_pcoData->stcPcoDesc1.wMaxHorzResStdDESC;
    *wYResMax = m_pcoData->stcPcoDesc1.wMaxVertResStdDESC;
    
    
    
    
	bla += sprintf_s(bla,myblamax - bla,
	    "%s> resAct[%d][%d] resStdMax[%d][%d] resExtMax[%d][%d]",
	    fnId,
	    *wXResActual, *wYResActual, *wXResMax, *wYResMax,
	    m_pcoData->stcPcoDesc1.wMaxHorzResExtDESC,
	    m_pcoData->stcPcoDesc1.wMaxVertResExtDESC);
    mylog->writelog(INFO_M, mybla);

    
	return;

}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetCameraInfo(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//int errTot=0;
	const char *msg;
	//const char *ptr;
    SC2_Camera_Description_Response dummy;
    dummy.wSize = sizeof(dummy);

	m_pcoData->frames_per_buffer = 1; // for PCO DIMAX


	// -- Get camera description
	//m_pcoData->stcPcoDesc1.wSize= sizeof(m_pcoData->stcPcoDesc1);

    error = camera->PCO_GetCameraDescriptor(&m_pcoData->stcPcoDesc1);
    msg = "PCO_GetCameraDescriptor"; PCO_CHECK_ERROR(error, msg); 

    error = camera->PCO_GetCameraDescriptionEx(&dummy, &m_pcoData->stcPcoDesc2, 1);
    msg = "PCO_GetCameraDescriptionEx"; PCO_CHECK_ERROR(error, msg); 

    double min_exp_time0, max_exp_time0;
	double min_lat_time0, max_lat_time0;
    double min_exp_time, max_exp_time;
	double min_lat_time, max_lat_time;
	double step_exp_time, step_lat_time;

	step_exp_time = m_pcoData->step_exp_time = (m_pcoData->stcPcoDesc1.dwMinExposureStepDESC) * NANO ;	//step exposure time in ns
	
	min_exp_time0 = m_pcoData->min_exp_time = (m_pcoData->stcPcoDesc1.dwMinExposureDESC) * NANO ;	//Minimum exposure time in ns
	min_exp_time = m_pcoData->min_exp_time_err = m_pcoData->min_exp_time - m_pcoData->step_exp_time ;	

	max_exp_time0 = m_pcoData->max_exp_time = (m_pcoData->stcPcoDesc1.dwMaxExposureDESC) * MILI ;   // Maximum exposure time in ms  
	max_exp_time = m_pcoData->max_exp_time_err = m_pcoData->max_exp_time + m_pcoData->step_exp_time ;	


	step_lat_time = m_pcoData->step_lat_time = (m_pcoData->stcPcoDesc1.dwMinDelayStepDESC) * NANO ;	//step delay time in ns

	min_lat_time0 = m_pcoData->min_lat_time = (m_pcoData->stcPcoDesc1.dwMinDelayDESC) * NANO ; // Minimum delay time in ns
	min_lat_time = m_pcoData->min_lat_time_err = 
		(m_pcoData->min_lat_time < m_pcoData->step_lat_time) ? m_pcoData->min_lat_time : m_pcoData->min_lat_time - m_pcoData->step_lat_time ;

	max_lat_time0 = m_pcoData->max_lat_time = (m_pcoData->stcPcoDesc1.dwMaxDelayDESC) * MILI ; // Maximum delay time in ms
	max_lat_time = m_pcoData->max_lat_time_err = m_pcoData->max_lat_time + m_pcoData->step_lat_time ;	

	DEB_ALWAYS() 
			<< "\n   " <<  DEB_VAR2(step_exp_time, step_lat_time)
			<< "\n   " <<  DEB_VAR2(min_exp_time0, max_exp_time0)
			<< "\n   " <<  DEB_VAR2(min_lat_time0, max_lat_time0)
			<< "\n   " <<  DEB_VAR2(min_exp_time, max_exp_time)
			<< "\n   " <<  DEB_VAR2(min_lat_time, max_lat_time)
			;	
	
	// callback to update in lima the valid_ranges from the last stcPcoDescription read
	if(m_sync) {
		HwSyncCtrlObj::ValidRangesType valid_ranges;
		m_sync->getValidRanges(valid_ranges);		// from stcPcoDescription
		m_sync->validRangesChanged(valid_ranges);	// callback
		DEB_ALWAYS() << fnId << ": callback - new valid_ranges: " << DEB_VAR1(valid_ranges);
	}
	
	
	m_pcoData->bMetaDataAllowed = !!(m_pcoData->stcPcoDesc1.dwGeneralCaps1 & GENERALCAPS1_METADATA) ;
	
	if(m_pcoData->bMetaDataAllowed) 
	{
	    error = camera->PCO_GetMetadataMode(&m_pcoData->wMetaDataMode, &m_pcoData->wMetaDataSize, &m_pcoData->wMetaDataVersion);
        msg = "PCO_GetMetadataMode"; PCO_CHECK_ERROR(error, msg); 
	} 
	else 
	{
        m_pcoData->wMetaDataMode = m_pcoData->wMetaDataSize = m_pcoData->wMetaDataVersion = 0;
	}

	


    {
        // get the max CAMERA pixel rate (Hz) from the description structure

    	DWORD _dwPixRate;
        dwPixelRateMax = m_pcoData->dwPixelRateMax = 0;
        iPixelRateValidNr=0;
        
        
        char validPixelRate[128];
        char *ptr = validPixelRate;
        char *ptrMax = ptr + sizeof(validPixelRate);
        
        ptr += sprintf_s(ptr, ptrMax - ptr, "validRates:");
        for(int i=0; i<4; i++) 
        {
            _dwPixRate = m_pcoData->stcPcoDesc1.dwPixelRateDESC[i];
            if(_dwPixRate > 0) 
            {
	            dwPixelRateValid[iPixelRateValidNr++] = _dwPixRate;
		        ptr += sprintf_s(ptr, ptrMax - ptr, "  [%d]",_dwPixRate);
            }
	        if(dwPixelRateMax < _dwPixRate){
				        dwPixelRateMax = m_pcoData->dwPixelRateMax = _dwPixRate;
	        }
        }	

        DEB_ALWAYS() 
            << "\n   " << DEB_VAR1(iPixelRateValidNr) 
            << "\n   " << DEB_VAR1(validPixelRate) 
            ;
    }
    



	m_pcoData->bMetaDataAllowed = !!(m_pcoData->stcPcoDesc1.dwGeneralCaps1 & GENERALCAPS1_METADATA) ;


	// -- Get General
	//m_pcoData->stcPcoGeneral.wSize= sizeof(m_pcoData->stcPcoGeneral);
	//m_pcoData->stcPcoGeneral.strCamType.wSize= sizeof(m_pcoData->stcPcoGeneral.strCamType);


	//PCO2(error, msg,PCO_GetGeneral,m_handle, &m_pcoData->stcPcoGeneral);
	//PCO_CHECK_ERROR(error, msg); 	if(error);


	// -- Get Sensor struct
	//m_pcoData->stcPcoSensor.wSize= sizeof(m_pcoData->stcPcoSensor);
	//m_pcoData->stcPcoSensor.strDescription.wSize= sizeof(m_pcoData->stcPcoSensor.strDescription);
	//m_pcoData->stcPcoSensor.strDescription2.wSize= sizeof(m_pcoData->stcPcoSensor.strDescription2);

	//PCO2(error, msg,PCO_GetSensorStruct, m_handle, &m_pcoData->stcPcoSensor);
	//PCO_CHECK_ERROR(error, msg); 	if(error) return;

	// -- Get timing struct
	//m_pcoData->stcPcoTiming.wSize= sizeof(m_pcoData->stcPcoTiming);

	//PCO2(error, msg,PCO_GetTimingStruct, m_handle, &m_pcoData->stcPcoTiming);
	//PCO_CHECK_ERROR(error, msg); 	if(error) return;


	// -- Get recording struct
	//m_pcoData->stcPcoRecording.wSize= sizeof(m_pcoData->stcPcoRecording);

	//PCO2(error, msg,PCO_GetRecordingStruct,m_handle, &m_pcoData->stcPcoRecording);
	//PCO_CHECK_ERROR(error, msg); 	if(error) return;


	// -- Get storage struct
	//m_pcoData->stcPcoStorage.wSize= sizeof(m_pcoData->stcPcoStorage);

	//PCO2(error, msg,PCO_GetStorageStruct, m_handle, &m_pcoData->stcPcoStorage);
	//PCO_CHECK_ERROR(error, msg); 	if(error) return;



	return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetCameraType(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//const char *msg = NULL;
    const char *ptr;
    int errTot = 0;
    
    //------------------ GigE
    //------------------ PCO_GetCameraType ---> camtype, serialnumber, iftype
    //------------------ PCO_GetInfo idem
    
    WORD camtype;
    WORD iftype = 0;
    DWORD serialnumber;

	m_pcoData->frames_per_buffer = 1; // for PCO DIMAX

    DEB_ALWAYS()  << fnId ;


    if(1) 
    {
        m_pcoData->ipField[0] = m_pcoData->ipField[1] = m_pcoData->ipField[2] =m_pcoData->ipField[3]= 0;
    }


    error = camera->PCO_GetCameraType(&camtype, &serialnumber, &iftype);
    PCO_CHECK_ERROR(error, "PCO_GetCameraType"); 
    if(error) return;
  

	m_pcoData->stcPcoCamType.wCamType = m_pcoData->wCamType = camtype;
    m_pcoData->stcPcoCamType.wCamSubType = 0;        
	m_pcoData->stcPcoCamType.dwSerialNumber = m_pcoData->dwSerialNumber = serialnumber;
	m_pcoData->stcPcoCamType.wInterfaceType = m_pcoData->wIfType = iftype;

	ptr = xlatPcoCode2Str(camtype, ModelType, error);
	strcpy_s(m_pcoData->model, MODEL_TYPE_SIZE, ptr);
	errTot |= error;

	ptr = xlatPcoCode2Str(iftype, InterfaceType, error);
	strcpy_s(m_pcoData->iface, INTERFACE_TYPE_SIZE, ptr);
	errTot |= error;

	sprintf_s(m_pcoData->camera_name, CAMERA_NAME_SIZE, "%s (IF %s) (SN %u)", 
			m_pcoData->model, m_pcoData->iface, m_pcoData->dwSerialNumber);
	DEB_ALWAYS() 
		<< "\n   " <<  DEB_VAR1(m_pcoData->model)
		<< "\n   " <<  DEB_VAR1(m_pcoData->iface)
		<< "\n   " <<  DEB_VAR1(m_pcoData->camera_name);




	error=camera->PCO_GetInfo(0,&m_pcoData->nameCamIf, sizeof(m_pcoData->nameCamIf) -1);
	error=camera->PCO_GetInfo(1,&m_pcoData->nameCam, sizeof(m_pcoData->nameCam) -1);
	error=camera->PCO_GetInfo(2,&m_pcoData->nameSensor, sizeof(m_pcoData->nameSensor) -1);

	DEB_ALWAYS() 
			<< "\n   " <<  DEB_VAR1(m_pcoData->nameCamIf)
			<< "\n   " <<  DEB_VAR1(m_pcoData->nameCam)
			<< "\n   " <<  DEB_VAR1(m_pcoData->nameSensor);





	return;
}


//=================================================================================================
//=================================================================================================
void Camera::_pco_SetCameraToCurrentTime(int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    err=camera->PCO_SetCameraToCurrentTime();
    PCO_CHECK_ERROR(err, "PCO_SetCameraToCurrentTime"); 
    
    return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetTransferParameter(int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    err=camera->PCO_GetTransferParameter(&clpar,sizeof(clpar));
    PCO_CHECK_ERROR(err, "PCO_GetTransferParameter"); 
    if(err) return;

    DEB_ALWAYS() 
        << "\n   " << DEB_VAR1(clpar.baudrate) 
        << "\n   " << DEB_VAR1(clpar.ClockFrequency) 
        << "\n   " << DEB_VAR1(clpar.DataFormat) 
        << "\n   " << DEB_VAR1(clpar.Transmit) 
        ;

    return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetTemperatureInfo(int &err){
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    err=camera->PCO_GetTemperature(&sTempCcd,&sTempCam,&sTempPS);
    PCO_CHECK_ERROR(err, "PCO_GetTemperature"); 
    if(err) { sTempCcd = sTempCam = sTempPS = 0; }

    err=camera->PCO_GetCoolingSetpointTemperature(&sCoolingSetpoint);
    PCO_CHECK_ERROR(err, "PCO_GetCoolingSetpointTemperature"); 
    if(err) {sCoolingSetpoint = 0;}

    DEB_ALWAYS() 
        << "\n   " << DEB_VAR1(sTempCcd) 
        << "\n   " << DEB_VAR1(sTempCam) 
        << "\n   " << DEB_VAR1(sTempPS) 
        << "\n   " << DEB_VAR1(sCoolingSetpoint) 
        ;

	m_pcoData->temperature.wCcd = sTempCcd;
	m_pcoData->temperature.wCam = sTempCam;
	m_pcoData->temperature.wPower =sTempPS;



#if 0



	sprintf_s(msg, MSG_SIZE, "* cooling temperature: MIN [%d]  Max [%d]\n",  m_pcoData->temperature.wMinCoolSet, m_pcoData->temperature.wMaxCoolSet);

	m_pcoData->temperature.wMinCoolSet = m_pcoData->stcPcoDesc1.sMinCoolSetDESC;
	m_pcoData->temperature.wMaxCoolSet = m_pcoData->stcPcoDesc1.sMaxCoolSetDESC;

	// -- Set/Get cooling temperature
	if (m_pcoData->temperature.wSetpoint != -1) {
		if (m_pcoData->temperature.wSetpoint < m_pcoData->temperature.wMinCoolSet)	m_pcoData->temperature.wSetpoint = m_pcoData->temperature.wMinCoolSet;
		if (m_pcoData->temperature.wSetpoint > m_pcoData->temperature.wMaxCoolSet)	m_pcoData->temperature.wSetpoint= m_pcoData->temperature.wMaxCoolSet;
	} else {
		PCOFN2(error, pcoFn,PCO_GetCoolingSetpointTemperature, m_handle, &m_pcoData->temperature.wSetpoint);
		if(error) return pcoFn;
		//PCO_THROW_OR_TRACE(error, "PCO_GetCoolingSetpointTemperature") ;
	}
	sprintf_s(msg, MSG_SIZE, "* Cooling Setpoint = %d\n", m_pcoData->temperature.wSetpoint);
	//DEB_TRACE() <<   msg;
	m_log.append(msg);

#endif
	return;
}



//=================================================================================================
//=================================================================================================
void Camera::_pco_GetPixelRate(DWORD &pixRateActual, DWORD &pixRateNext, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	DWORD _dwPixRate;
    const char *msg;

    err=camera->PCO_GetPixelRate(&_dwPixRate);
    msg = "PCO_GetPixelRate" ; PCO_CHECK_ERROR(err, msg);
    if(err) _dwPixRate = 0;

    m_pcoData->dwPixelRate = pixRateActual = _dwPixRate;


	pixRateNext = ((m_pcoData->dwPixelRateRequested != 0) && (pixRateActual != m_pcoData->dwPixelRateRequested)) ?
			m_pcoData->dwPixelRateRequested : pixRateActual;


        DEB_ALWAYS() 
            << "\n   " << DEB_VAR1(pixRateActual) 
            << "\n   " << DEB_VAR1(pixRateNext) 
            ;


}

//=================================================================================================
//=================================================================================================
void Camera::_pco_GetLut(int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    err=camera->PCO_GetLut(&wLutActive, &wLutParam);
    PCO_CHECK_ERROR(err, "PCO_GetLut"); if(err) return;

    DEB_ALWAYS() 
        << "\n   " << DEB_VAR1(wLutActive) 
        << "\n   " << DEB_VAR1(wLutParam) 
        ;

    return;
}


//=================================================================================================
//=================================================================================================

/**************************************************************************************************
	If a set recording status = [stop] command is sent and the current status is already
	[stop]\92ped, nothing will happen (only warning, error message). 
	
	If the camera is in
	[run]\92ing state, it will last some time (system delay + last image readout), until the
	camera is stopped. The system delay depends on the PC and the image readout
	depends on the image size transferred. The SetRecordingState = [stop] checks for a
	stable stop state by calling GetRecordingState.  --- 165 ms 
	
	Please call PCO_CancelImages to remove pending buffers from the driver.   --- 1.5 s
**************************************************************************************************/

const char *Camera::_pco_SetRecordingState(int state, int &err){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//const char *msg;
	//LARGE_INTEGER usStart;
	TIME_UTICKS usStart;
	


	WORD wRecState_new, wRecState_actual;

	wRecState_new = state ? 0x0001 : 0x0000 ; // 0x0001 => START acquisition

	usElapsedTimeSet(usStart);

    wRecState_actual = _pco_GetRecordingState(err);

	//_setCameraState(CAMSTATE_RECORD_STATE, !!(wRecState_actual));

	traceAcq.usTicks[traceAcq_GetRecordingState].value = usElapsedTime(usStart);
	traceAcq.usTicks[traceAcq_GetRecordingState].desc = "PCO_GetRecordingState execTime";
	usElapsedTimeSet(usStart);

	//if(wRecState_new == wRecState_actual) {error = 0; return fnId; }


	// ------------------------------------------ cancel images 
	if(wRecState_new == 0) {
		int count = 1;

		_setCameraState(CAMSTATE_RECORD_STATE, false);

#if 0
		PCOFN2(error, msg,PCO_GetPendingBuffer, m_handle, &count);
		PCO_CHECK_ERROR(error, msg); 	if(error) return msg;
#endif
		if(count && !_isCameraType(Edge)) 
		{
			DEB_ALWAYS() << fnId << ": PCO_CancelImages";

            err=camera->PCO_CancelImage();
            PCO_CHECK_ERROR(err, "PCO_CancelImage");

            err=camera->PCO_CancelImageTransfer();
            PCO_CHECK_ERROR(err, "PCO_CancelImageTransfer");
		}
	}

    err=camera->PCO_SetRecordingState(wRecState_new);
    PCO_CHECK_ERROR(err, "PCO_SetRecordingState");
    

    wRecState_actual = _pco_GetRecordingState(err);

	_setCameraState(CAMSTATE_RECORD_STATE, !!(wRecState_actual));

	traceAcq.usTicks[traceAcq_SetRecordingState].value = usElapsedTime(usStart);
	traceAcq.usTicks[traceAcq_SetRecordingState].desc = "PCO_SetRecordingState execTime";
	usElapsedTimeSet(usStart);

	_armRequired(true);

	traceAcq.usTicks[traceAcq_CancelImages].value = usElapsedTime(usStart);
	traceAcq.usTicks[traceAcq_CancelImages].desc = "PCO_CancelImages execTime";
	usElapsedTimeSet(usStart);

	//DEB_ALWAYS() << fnId << ": " << DEB_VAR4(error, state, wRecState_actual, wRecState_new);
	return fnId;

}


//=================================================================================================
//=================================================================================================
WORD Camera::_pco_GetRecordingState(int &err){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
    WORD wRecState_actual;
    
    err=camera->PCO_GetRecordingState(&wRecState_actual);
    PCO_CHECK_ERROR(err, "PCO_GetRecordingState");


	return wRecState_actual;

}


//=================================================================================================
//=================================================================================================
void Camera::_pco_SetTimestampMode(WORD mode, int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    WORD modeNew, modeOld;
    
    err=camera->PCO_GetTimestampMode(&modeOld);
    PCO_CHECK_ERROR(err, "PCO_GetTimestampMode"); 

    err=camera->PCO_SetTimestampMode(mode);
    PCO_CHECK_ERROR(err, "PCO_SetTimestampMode"); 

    err=camera->PCO_GetTimestampMode(&modeNew);
    PCO_CHECK_ERROR(err, "PCO_GetTimestampMode"); 

    DEB_ALWAYS() 
        << "\n   " << DEB_VAR3(mode, modeOld, modeNew) 
        ;

    return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_Open_Cam(int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    DEB_ALWAYS()  << "creating the camera" ;

    camera= new CPco_com_cl_me4();
    camera->SetLog(mylog);

    DEB_ALWAYS()  << "Try to open Camera" ;
    err=camera->Open_Cam(0);
    PCO_CHECK_ERROR(err, "Open_Cam close application"); 
    if(err)
    {
        delete camera;
        camera = NULL;
        THROW_HW_ERROR(Error) ;
    }
    
    DEB_ALWAYS()  << "After open Camera" ;

    return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_Close_Cam(int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    error = 0;
    
    if(grabber)
    {
        delete grabber;
        grabber = NULL;
    }

    if(camera)
    {
        error = camera->Close_Cam();
        PCO_CHECK_ERROR(error, "Close_Cam");

        delete camera;
        camera = NULL;
    }

}


//=================================================================================================
//=================================================================================================
void Camera::_pco_Open_Grab(int &err)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	//int iErr;

    
    camtype = _getCameraType();
 
    if(camtype==CAMERATYPE_PCO_EDGE)
    {
        DEB_ALWAYS()  << "Grabber is CPco_grab_cl_me4_edge";
        grabber=new CPco_grab_cl_me4_edge((CPco_com_cl_me4*)camera);
    }
    else if(camtype==CAMERATYPE_PCO_EDGE_42)
    {
        DEB_ALWAYS()  << "Grabber is CPco_grab_cl_me4_edge42";
        grabber=new CPco_grab_cl_me4_edge42((CPco_com_cl_me4*)camera);
    }
    else
    {
        DEB_ALWAYS()  << "ERROR Wrong camera for this application";
        camera->Close_Cam();
        delete camera;
        camera = NULL;
        grabber = NULL;
        THROW_HW_ERROR(Error) ;
    }

    if(!grabber)
    {
        DEB_ALWAYS()  << "ERROR new grabber creation";
        camera->Close_Cam();
        delete camera;
        camera = NULL;
        grabber = NULL;
        THROW_HW_ERROR(Error) ;
    }

    grabber->SetLog(mylog);

    DEB_ALWAYS()  << "Try to open Grabber";
    err=grabber->Open_Grabber(board);
	PCO_CHECK_ERROR(err, "Open_Grabber, close application");
    if(err)
    {
        delete grabber;
        grabber = NULL;

        camera->Close_Cam();
        delete camera;
        camera = NULL;
        THROW_HW_ERROR(Error) ;
    }

    return;
}

//=================================================================================================
//=================================================================================================
void Camera::_pco_ResetSettingsToDefault(int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
    int iErr = 0;
    
	// -- Reset to default settings

    _pco_SetRecordingState(0, iErr);
    
    error=camera->PCO_ResetSettingsToDefault();
	PCO_CHECK_ERROR(error, "PCO_ResetSettingsToDefault");

    error |= iErr;
    
    return;
}



//=================================================================================================
//=================================================================================================
void Camera::_pco_SetBinning(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;

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


	return;

}
//=================================================================================================
//=================================================================================================
void Camera::_pco_SetROI(int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    WORD &wRoiX0Now = m_pcoData->wRoiX0Now;
	WORD &wRoiY0Now = m_pcoData->wRoiY0Now;
    WORD &wRoiX1Now = m_pcoData->wRoiX1Now;
	WORD &wRoiY1Now = m_pcoData->wRoiY1Now;

    WORD wRoiX0, wRoiY0; // Roi upper left x y
    WORD wRoiX1, wRoiY1; // Roi lower right x y
	unsigned int x0, x1, y0, y1;

	_get_Roi(x0, x1, y0, y1);
    wRoiX0 = (WORD) x0; wRoiX1 = (WORD) x1;
    wRoiY0 = (WORD) y0; wRoiY1 = (WORD) y1;

    error=camera->PCO_GetROI(&wRoiX0Now, &wRoiY0Now, &wRoiX1Now, &wRoiY1Now);
    PCO_CHECK_ERROR(error, "PCO_GetROI");
	PCO_THROW_OR_TRACE(error, "PCO_GetROI") ;

	bool test;
	test = ((wRoiX0Now != wRoiX0) ||	(wRoiX1Now != wRoiX1) || (wRoiY0Now != wRoiY0) || (wRoiY1Now != wRoiY1));
	DEB_ALWAYS() 
		<< "\n> " << DEB_VAR1(test)
		<< "\n   _get_Roi> " << DEB_VAR4(x0, x1, y0, y1)
		<< "\n   _get_Roi> " << DEB_VAR4(wRoiX0, wRoiX1, wRoiY0, wRoiY1)
		<< "\n   PCO_GetROI> " << DEB_VAR4(wRoiX0Now, wRoiY0Now, wRoiX1Now, wRoiY1Now);
	test = true;
	if(test) {
		
		DEB_ALWAYS() 
			<< "\n   PCO_SetROI> " << DEB_VAR5(m_RoiLima, wRoiX0, wRoiY0, wRoiX1, wRoiY1);

        error=camera->PCO_SetROI(wRoiX0, wRoiY0, wRoiX1, wRoiY1);
        PCO_CHECK_ERROR(error, "PCO_SetROI");
    	PCO_THROW_OR_TRACE(error, "PCO_SetROI") ;

		_armRequired(true);

        error=camera->PCO_GetROI(&wRoiX0Now, &wRoiY0Now, &wRoiX1Now, &wRoiY1Now);
        PCO_CHECK_ERROR(error, "PCO_GetROI");
	    PCO_THROW_OR_TRACE(error, "PCO_GetROI") ;

		DEB_ALWAYS() 
			<< "\n   PCO_GetROI> " 
			    << DEB_VAR4(wRoiX0Now, wRoiY0Now, wRoiX1Now, wRoiY1Now)
			<< "\n   PCO_GetROI> " 
			    << DEB_VAR4(m_pcoData->wRoiX0Now, m_pcoData->wRoiY0Now, m_pcoData->wRoiX1Now, m_pcoData->wRoiY1Now);

	}

	return;

}


//=================================================================================================
//=================================================================================================
size_t Camera::_pco_GetHardwareVersion_Firmware(char *ptrOut, size_t lgMax, int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    char *ptr = ptrOut;
    char *ptrMax = ptr + lgMax;
    int lg = ptrMax - ptr;

    error = camera->PCO_GetHardwareVersion (ptr, &lg);
    PCO_CHECK_ERROR(error, "PCO_GetROI");
    if(error) return 0;
    
    lg = strlen(ptrOut) ;
    ptr = ptrOut + lg;
    lg = ptrMax - ptr;

    if(lg) 
    {
        error = camera->PCO_GetFirmwareVersion (ptr, &lg);
        PCO_CHECK_ERROR(error, "PCO_GetROI");
        if(error) return 0;
    }

    lg = strlen(ptrOut) ;

    return lg;
}
//=================================================================================================
//=================================================================================================
void Camera::_pco_GetAcqEnblSignalStatus(WORD &wAcquEnableState, int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	WORD _wAcquEnableState;

    error = camera->PCO_GetAcqEnblSignalStatus(&_wAcquEnableState);   
    PCO_CHECK_ERROR(error, "PCO_GetAcqEnblSignalStatus");

    if(error) return;
    
    wAcquEnableState = _wAcquEnableState;
}

//=================================================================================================
//=================================================================================================
size_t Camera::_pco_roi_info(char *ptrOut, size_t lgMax, int &error)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;

    char *ptr = ptrOut;
    char *ptrMax = ptr + lgMax;
    //int lg = ptrMax - ptr;

			unsigned int x0, x1, y0, y1;
			Roi new_roi;

			Roi limaRoi;
			_get_Roi(limaRoi);

			_get_Roi(x0, x1, y0, y1);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* roi PCO X(%d,%d) Y(%d,%d) size(%d,%d)\n",  
					x0, x1, y0, y1, x1-x0+1, y1-y0+1);
			{
			Point top_left = m_RoiLima.getTopLeft();
			Point bot_right = m_RoiLima.getBottomRight();
			Size size = m_RoiLima.getSize();

			ptr += sprintf_s(ptr, ptrMax - ptr, "* roiLima PCO XY0(%d,%d) XY1(%d,%d) size(%d,%d)\n",  
					top_left.x, top_left.y,
					bot_right.x, bot_right.y,
					size.getWidth(), size.getHeight());

			top_left = m_RoiLimaRequested.getTopLeft();
			bot_right = m_RoiLimaRequested.getBottomRight();
			size = m_RoiLimaRequested.getSize();

			ptr += sprintf_s(ptr, ptrMax - ptr, "* roiLima REQUESTED XY0(%d,%d) XY1(%d,%d) size(%d,%d)\n",  
					top_left.x, top_left.y,
					bot_right.x, bot_right.y,
					size.getWidth(), size.getHeight());
			}

    

    return ptr - ptrOut;
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

    error=camera->PCO_GetCOCRuntime(&dwTime_s, &dwTime_ns);
    msg = "PCO_GetCOCRuntime" ; PCO_CHECK_ERROR(error, msg);
    if(error) return;

    m_pcoData->cocRunTime = runTime = ((double) dwTime_ns * NANO) + (double) dwTime_s;
    m_pcoData->frameRate = (runTime > 0.) ? 1.0 / runTime : -1.;

    DEB_TRACE() << DEB_VAR2(m_pcoData->frameRate, m_pcoData->cocRunTime);

	return;

}


