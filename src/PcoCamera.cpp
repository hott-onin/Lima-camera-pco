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

#define PCO_ERRT_H_CREATE_OBJECT
#define BYPASS

#define TOUT_MIN_DIMAX 500
#define ERROR_MSG_LINE 128

//#define BUFF_INFO_SIZE 5000


#include <cstdlib>

#ifndef __linux__
#include <process.h>
#endif 

#include <sys/stat.h>

#ifndef __linux__
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif 

#include <time.h>

#include "lima/Exceptions.h"
#include "lima/HwSyncCtrlObj.h"

#include "Pco.h"
#include "PcoCamera.h"
#include "PcoSyncCtrlObj.h"
#include "PcoBufferCtrlObj.h"
#include "PcoCameraUtils.h"


using namespace lima;
using namespace lima::Pco;

const char *timebaseUnits[] = {"ns", "us", "ms"};

//char *_checkLogFiles();
void _pco_time2dwbase(double exp_time, DWORD &dwExp, WORD &wBase);

const char * _timestamp_pcosyncctrlobj();
const char * _timestamp_pcointerface();
const char * _timestamp_pcobufferctrlobj();
const char * _timestamp_pcodetinfoctrlobj();
const char * _timestamp_pcocamerautils();
const char * _timestamp_pcoroictrlobj();
char *_split_date(const char *s);

char *str_trim_left(char *s);
char *str_trim_right(char *s);
char *str_trim(char *s) ;
char *str_toupper(char *s);


int PCO_GetBitAlignment(HANDLE ph, WORD *){return 1;}
int PCO_SetBitAlignment(HANDLE ph, WORD ){return 1;}

int PCO_GetActiveRamSegment(HANDLE ph, WORD *){return 1;}

int PCO_SetTimeouts(HANDLE ph, void *buf_in, unsigned int size_in) {return 1;}

int PCO_SetHWIOSignal (HANDLE ph, WORD wSignalNum, PCO_Signal* pstrSignal){return 1;}
int PCO_GetHWIOSignal (HANDLE ph, WORD wSignalNum, PCO_Signal* pstrSignal){return 1;}
int PCO_GetHWIOSignalDescriptor (HANDLE ph, WORD wSignalNum, PCO_Single_Signal_Desc* pstrSignal){return 1;}
int PCO_RebootCamera(HANDLE ph){return 1;}
int PCO_GetPendingBuffer(HANDLE ph, int* icount){return 1;}
int PCO_GetCameraDescription(HANDLE ph, PCO_Description* strDescription){return 1;}
int PCO_GetActiveLookupTable(HANDLE ph, WORD *wIdentifier, WORD *wParameter){return 1;}
int PCO_GetTransferParameter(HANDLE ph, void* buffer, int ilen){return 1;}
int PCO_SetTransferParameter(HANDLE ph, void* buffer, int ilen){return 1;}
int PCO_CamLinkSetImageParameters(HANDLE ph, WORD wxres, WORD wyres){return 1;}
int PCO_SetTriggerMode(HANDLE ph, WORD wTriggerMode){return 1;}
int PCO_GetTriggerMode(HANDLE ph, WORD* wTriggerMode){return 1;}
//int PCO_GetImageTiming (HANDLE ph, PCO_Image_Timing* pstrImageTiming){return 1;}
int PCO_GetImageTiming (HANDLE ph, void* pstrImageTiming){return 1;}
int PCO_SetActiveLookupTable(HANDLE ph, WORD *wIdentifier, WORD *wParameter){return 1;}
int PCO_SetAcquireMode(HANDLE ph, WORD wAcquMode){return 1;}
int PCO_CancelImages(HANDLE ph){return 1;}
int PCO_OpenCamera(HANDLE* ph, WORD wCamNum){return 1;}
DWORD PCO_ResetLib() {return 0;}

// int PCO_GetCameraType(HANDLE ph, PCO_CameraType* strCamType){return 1;}
//int PCO_ResetSettingsToDefault(HANDLE ph){return 1;}
//int PCO_GetGeneral(HANDLE ph, PCO_General* strGeneral){return 1;}
//int PCO_GetStorageStruct(HANDLE ph, PCO_Storage* strStorage){return 1;}
//int PCO_GetTimingStruct(HANDLE ph, PCO_Timing* strTiming){return 1;}
//int PCO_GetRecordingStruct(HANDLE ph, PCO_Recording* strRecording){return 1;}
//int PCO_GetSensorStruct(HANDLE ph, PCO_Sensor* strSensor){return 1;}

//=================================================================================================
//=================================================================================================
//---------------------------
//- utility thread
//---------------------------

class Camera::_AcqThread : public Thread
{
    DEB_CLASS_NAMESPC(DebModCamera, "Camera", "_AcqThread");
    public:
        _AcqThread(Camera &aCam);
    virtual ~_AcqThread();
    
    protected:
        virtual void threadFunction();
    
    private:
        Camera&    m_cam;
};

//=========================================================================================================
//=========================================================================================================
int image_nr_from_timestamp(void *buf,int shift)
{
    unsigned short *b;
    int y;
    int image_nr=0;
    b=(unsigned short *)(buf);
    y=100*100*100;
    for(;y>0;y/=100)
    {
        *b>>=shift;
        image_nr+= (((*b&0x00F0)>>4)*10 + (*b&0x000F))*y;
        b++;
    }
    return image_nr;
}


//=========================================================================================================
//=========================================================================================================
const char* _timestamp_pcocamera() {return ID_TIMESTAMP_M ;}


#ifdef WITH_GIT_VERSION
#include "PcoGitVersion.h"
char * _timestamp_gitversion(char *buffVersion, int len)
{
	sprintf_s(buffVersion, len, "%s\n%s\n%s\n%s\n%s\n%s\n", 
				 PCO_GIT_VERSION,
				 PROCLIB_GIT_VERSION,
				 LIBCONFIG_GIT_VERSION,
				 LIMA_GIT_VERSION,
				 TANGO_GIT_VERSION,
				 SPEC_GIT_VERSION
				 );
	return buffVersion;
}
#endif

char * _getComputerName(char *infoBuff, DWORD  bufCharCount);
char * _getUserName(char *infoBuff, DWORD  bufCharCount);
char * _getVSconfiguration(char *infoBuff, DWORD  bufCharCount);

//=========================================================================================================

//=========================================================================================================
//=========================================================================================================
const char *xlatCode2Str(int code, struct stcXlatCode2Str *stc) {

	const char *type;

	while( (type = stc->str) != NULL) {
		if(stc->code == code) return type;
		stc++;
	}

	return NULL;

}
#if 0
//=========================================================================================================
//=========================================================================================================

//enum tblXlatCode2Str {ModelType, InterfaceType};

const char *xlatPcoCode2Str(int code, enumTblXlatCode2Str table, int &err) {
	struct stcXlatCode2Str modelType[] = {
		{CAMERATYPE_PCO1200HS, "PCO 1200 HS"},
		{CAMERATYPE_PCO1300, "PCO 1300"},
		{CAMERATYPE_PCO1600, "PCO 1600"},
		{CAMERATYPE_PCO2000, "PCO 2000"},
		{CAMERATYPE_PCO4000, "PCO 4000"},
		{CAMERATYPE_PCO_DIMAX_STD, "PCO DIMAX STD"},
		{CAMERATYPE_PCO_DIMAX_TV, "PCO DIMAX TV"},
		{CAMERATYPE_PCO_DIMAX_AUTOMOTIVE, "PCO DIMAX AUTOMOTIVE"},
		{CAMERATYPE_PCO_EDGE, "PCO EDGE"},
		{CAMERATYPE_PCO_EDGE_GL, "PCO EDGE GL"},
		{0, NULL}
	};

  struct stcXlatCode2Str interfaceType[] = {
		{INTERFACE_FIREWIRE, "FIREWIRE"},
		{INTERFACE_CAMERALINK, "CAMERALINK"},
		{INTERFACE_USB, "USB"},
		{INTERFACE_ETHERNET, "ETHERNET"},
		{INTERFACE_SERIAL, "SERIAL"},
		{0, NULL}
	};

  struct stcXlatCode2Str *stc;
	const char *ptr;
	static char buff[BUFF_XLAT_SIZE+1];

  switch(table) {
    case ModelType: stc = modelType; break;
    case InterfaceType: stc = interfaceType; break;
    default:
  		sprintf_s(buff, BUFF_XLAT_SIZE, "UNKNOWN XLAT TABLE [%d]", table);
  		err = 1;
	  	return buff;
  }

	if((ptr = xlatCode2Str(code, stc)) != NULL) {
		err = 0;
		return ptr;
	} else {
		sprintf_s(buff, BUFF_XLAT_SIZE, "UNKNOWN %s code [0x%04x]", (table == ModelType) ? "MODEL" : "INTERFACE", code);
		err = 1;
		return buff;
	}
}
#endif
//=========================================================================================================
//=========================================================================================================

//char * Camera::xlatPcoCode2Str(int code, enumTblXlatCode2Str table, int &err) {
const char *xlatPcoCode2Str(int code, enumTblXlatCode2Str table, int &err) 
{
	struct stcXlatCode2Str modelType[] = {
		{CAMERATYPE_PCO1200HS, "PCO 1200 HS"},
		{CAMERATYPE_PCO1300, "PCO 1300"},
		{CAMERATYPE_PCO1400, "PCO 1400"},
		{CAMERATYPE_PCO1600, "PCO 1600"},
		{CAMERATYPE_PCO2000, "PCO 2000"},
		{CAMERATYPE_PCO4000, "PCO 4000"},
		{CAMERATYPE_PCO_DIMAX_STD, "PCO DIMAX STD"},
		{CAMERATYPE_PCO_DIMAX_TV, "PCO DIMAX TV"},
		{CAMERATYPE_PCO_DIMAX_AUTOMOTIVE, "PCO DIMAX AUTOMOTIVE"},
		{CAMERATYPE_PCO_EDGE, "PCO EDGE 5.5 RS"},
		{CAMERATYPE_PCO_EDGE_42, "PCO EDGE 4.2 RS"},
		{CAMERATYPE_PCO_EDGE_GL, "PCO EDGE 5.5 GL"},
		{CAMERATYPE_PCO_EDGE_USB3, "PCO EDGE USB3"},
		{CAMERATYPE_PCO_EDGE_HS, "PCO EDGE hs"},
		{CAMERATYPE_PCO_EDGE, "PCO EDGE"},
		{CAMERATYPE_PCO_EDGE_GL, "PCO EDGE GL"},
		{0, "NO_modelType"},
		{0, NULL}
	};

	struct stcXlatCode2Str modelSubType[] = {
		{CAMERATYPE_PCO1200HS, "PCO 1200 HS"},
		{CAMERASUBTYPE_PCO_DIMAX_Weisscam, "DIMAX_Weisscam"},
		{CAMERASUBTYPE_PCO_DIMAX_HD, "DIMAX_HD"},
		{CAMERASUBTYPE_PCO_DIMAX_HD_plus, "DIMAX_HD_plus"},
		{CAMERASUBTYPE_PCO_DIMAX_X35, "DIMAX_X35"},
		{CAMERASUBTYPE_PCO_DIMAX_HS1, "DIMAX_HS1"},
		{CAMERASUBTYPE_PCO_DIMAX_HS2, "DIMAX_HS2"},
		{CAMERASUBTYPE_PCO_DIMAX_HS4, "DIMAX_HS4"},
		{CAMERASUBTYPE_PCO_EDGE_SPRINGFIELD, "EDGE_SPRINGFIELD"},
		{CAMERASUBTYPE_PCO_EDGE_DEVELOPMENT, "EDGE_DEVELOPMENT"},
		{CAMERASUBTYPE_PCO_EDGE_X2, "EDGE_X2"},
		{CAMERASUBTYPE_PCO_EDGE_RESOLFT, "EDGE_RESOLFT"},
		{CAMERASUBTYPE_PCO_EDGE_GOLD, "EDGE_GOLD"},
		{CAMERASUBTYPE_PCO_EDGE_DUAL_CLOCK, "DUAL_CLOCK"},
		{CAMERASUBTYPE_PCO_EDGE_DICAM, "DICAM"},
		{0, "NO_subType"},
		{0, NULL}
	};

	struct stcXlatCode2Str interfaceType[] = {
		{INTERFACE_FIREWIRE, "FIREWIRE"},
		{INTERFACE_CAMERALINK, "CAMERALINK"},
		{INTERFACE_USB, "USB"},
		{INTERFACE_ETHERNET, "ETHERNET"},
		{INTERFACE_SERIAL, "SERIAL"},
		{INTERFACE_USB3, "USB3"},
		{INTERFACE_CAMERALINKHS, "CAMERALINK_HS"},
		{INTERFACE_COAXPRESS, "COAXPRESS"},
		{0, "NO_interfaceType"},
		{0, NULL}
	};

  struct stcXlatCode2Str *stc;
	const char *ptr;
	static char buff[BUFF_XLAT_SIZE+1];
	const char *errTable ;

  switch(table) {
	case ModelType: stc = modelType; errTable = "modelType" ; break;
	case ModelSubType: stc = modelSubType;  errTable = "modelSubType" ; break;
	case InterfaceType: stc = interfaceType; errTable = "interfaceType" ;  break;

    default:
  		sprintf_s(buff, BUFF_XLAT_SIZE, "UNKNOWN XLAT TABLE [%d]", table);
  		err = 1;
	  	return buff;
  }

	if((ptr = xlatCode2Str(code, stc)) != NULL) {
		err = 0;
		return ptr;
	} else {
		sprintf_s(buff, BUFF_XLAT_SIZE, "UNKNOWN %s code [0x%04x]", errTable, code);
		err = 1;
		return buff;
	}
}


//=========================================================================================================
//=========================================================================================================

#define BUFFER_LEN 256
#define BUFFVERSION_LEN 2048
stcPcoData::stcPcoData(){

	char *ptr, *ptrMax;
	int i;
	char buff[BUFFER_LEN+1];
	
	memset(this, 0, sizeof(struct stcPcoData));

	ptr = version; *ptr = 0;
	ptrMax = ptr + sizeof(version) - 1;

	ptr += sprintf_s(ptr, ptrMax - ptr,  "\n");
	ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", _split_date(_timestamp_pcocamera()));
	ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", _split_date(_timestamp_pcosyncctrlobj()));
	ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", _split_date(_timestamp_pcointerface()));
	ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", _split_date(_timestamp_pcobufferctrlobj()));
	ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", _split_date(_timestamp_pcodetinfoctrlobj()));
	ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", _split_date(_timestamp_pcocamerautils()));
	ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", _split_date(_timestamp_pcoroictrlobj()));

#ifdef WITH_GIT_VERSION
	char buffVersion[BUFFVERSION_LEN+1];
	ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", _timestamp_gitversion(buffVersion, BUFFVERSION_LEN));
#endif

	ptr += sprintf_s(ptr, ptrMax - ptr, "       timestamp: %s\n", getTimestamp(Iso));
	ptr += sprintf_s(ptr, ptrMax - ptr, "   computer name: %s\n", _getComputerName(buff, BUFFER_LEN));
	ptr += sprintf_s(ptr, ptrMax - ptr, "       user name: %s\n", _getUserName(buff, BUFFER_LEN));
	ptr += sprintf_s(ptr, ptrMax - ptr, "VS configuration: %s\n", _getVSconfiguration(buff, BUFFER_LEN));

	stcPcoGeneral.wSize = sizeof(stcPcoGeneral);
	stcPcoGeneral.strCamType.wSize = sizeof(stcPcoGeneral.strCamType);
	stcPcoCamType.wSize = sizeof(stcPcoCamType);
	stcPcoSensor.wSize = sizeof(stcPcoSensor);
	stcPcoSensor.strDescription.wSize = sizeof(stcPcoSensor.strDescription);
	stcPcoSensor.strDescription2.wSize = sizeof(stcPcoSensor.strDescription2);
	stcPcoDesc1.wSize = sizeof(stcPcoDesc1);
	stcPcoDesc2.wSize = sizeof(stcPcoDesc2);
	stcPcoTiming.wSize = sizeof(stcPcoTiming);
	stcPcoStorage.wSize = sizeof(stcPcoStorage);
	stcPcoRecording.wSize = sizeof(stcPcoRecording);

	for(i=0; i < SIZEARR_stcPcoHWIOSignal; i++) {
		stcPcoHWIOSignal[i].wSize = sizeof(stcPcoHWIOSignal[i]);
		stcPcoHWIOSignalDesc[i].wSize = sizeof(stcPcoHWIOSignalDesc[i]);
	}

	bAllocatedBufferDone = 
		false;
	
	msAcqRecTimestamp = msAcqXferTimestamp =
			getTimestamp();

}

//=========================================================================================================
//=========================================================================================================
bool Camera::paramsGet(const char *key, char *&value) {
	//DEF_FNID;
	DEB_CONSTRUCTOR();
	bool ret;

	for(int i = 0; i < m_pcoData->params.nr; i++) {
		if(_stricmp(key, m_pcoData->params.ptrKey[i]) == 0) {
			ret = true;
			value = m_pcoData->params.ptrValue[i];
			DEB_ALWAYS() << DEB_VAR3(key, ret, value);
			return ret;
		}
	}
	ret = false;
	value = (char *) ""; 
	DEB_ALWAYS() << DEB_VAR3(key, ret, value);
	return ret;
}

bool Camera::paramsGet(const char *key, unsigned long long &value) {
	//DEF_FNID;
	DEB_CONSTRUCTOR();
	bool ret;
	char *_str;


    ret = paramsGet(key, _str);
    
    if(!ret) return false;
    
    value = strtoull(_str, NULL, 0);

	return true;
}

//=========================================================================================================
//=========================================================================================================
void Camera::paramsInit(const char *str) 
{
	//DEF_FNID;
	DEB_CONSTRUCTOR();

	int i;
	char *tokNext = NULL;
	char *buff = m_pcoData->params.buff;
	int &nrList = m_pcoData->params.nr;
	int nr;
	
	// --- split of params string
	strcpy_s(buff, PARAMS_LEN_BUFF , str);
	char *ptr = buff;
	
	for(nr = i = 0; i < PARAMS_NR; i++) {
		if( (m_pcoData->params.ptrKey[i] = strtok_s(ptr, ";", &tokNext)) == NULL) break;
		ptr = NULL;
		nr = i+1;
	}

	nrList = 0;

	for(i = 0; i < nr; i++) {
		char *key, *value;
		bool found;

		ptr = str_trim(m_pcoData->params.ptrKey[i]);
		key = strtok_s(ptr, "=", &tokNext);
		value = strtok_s(NULL, "=", &tokNext);
		str_toupper(key);	
		m_pcoData->params.ptrKey[i] = key = str_trim(key);	
		value = str_trim(value);	
		if(value == NULL) value = (char *) "";
		m_pcoData->params.ptrValue[i] = value;	
		
		found = false;
		for(int j = 0; j < nrList; j++) {
			if(_stricmp(m_pcoData->params.ptrKey[j], m_pcoData->params.ptrKey[i]) == 0){
				m_pcoData->params.ptrValue[j] = m_pcoData->params.ptrValue[i];
				found = true;
				break;
			}
		}
		if(!found) {
			key = m_pcoData->params.ptrKey[nrList] = m_pcoData->params.ptrKey[i]; 
			value = m_pcoData->params.ptrValue[nrList] = m_pcoData->params.ptrValue[i] ;
			nrList++;
		}
	}

	for(int j = 0; j < nrList; j++) {
		char *key, *value;
		key = m_pcoData->params.ptrKey[j];
		value = m_pcoData->params.ptrValue[j];
		DEB_ALWAYS() << DEB_VAR2(key, value);
	}
};

//=========================================================================================================
//=========================================================================================================

Camera::Camera(const char *params) 
{
	//DEF_FNID;
	DEB_CONSTRUCTOR();



	m_cam_connected=false;
	m_acq_frame_nb = 1;
	m_sync = NULL;
	m_buffer= NULL;
	m_handle = 0;

    camera = NULL;
    grabber = NULL;
    
    m_quit = false;
    m_wait_flag = true;
    
   	traceAcq.traceAcqClean();

	//delay_time = exp_time = 0;
	
    char fnLog[PATH_MAX];
    //snprintf(fnLog, PATH_MAX,"/tmp/pco_edge_grab%s.log",getTimestamp(FnFull));
    snprintf(fnLog, PATH_MAX,"/tmp/pco_edge_grab.log");
	mylog = new CPco_Log(fnLog);
	
	//int error=0;
	m_config = TRUE;
	DebParams::checkInit();

	_setStatus(Camera::Config,true);
	
	m_msgLog = new ringLog(300) ;
	m_tmpLog = new ringLog(300) ;
	if(m_msgLog == NULL)
		throw LIMA_HW_EXC(Error, "m_msgLog > creation error");
	if(m_tmpLog == NULL)
		throw LIMA_HW_EXC(Error, "m_tmpLog > creation error");

	m_pcoData =new stcPcoData();
	if(m_pcoData == NULL)
		throw LIMA_HW_EXC(Error, "m_pcoData > creation error");

	// properties: params 
	paramsInit(params);

	char *value;
	const char  *key;
	UNUSED bool ret;
	
	mybla = new char[LEN_BLA+1];
	myblamax = mybla + LEN_BLA;

	mytalk = new char[LEN_TALK+1];
	mytalkmax = mytalk + LEN_TALK;

    
	/***
	key = "test";
	key = "withConfig";
	key = "testMode";
	key = "debugPco";
	***/
	key = "extValue";
	ret = paramsGet(key, value);

    m_pcoData->testCmdMode = 0;
    paramsGet("testMode", m_pcoData->testCmdMode);

	DEB_ALWAYS()
		<< ALWAYS_NL << DEB_VAR1(m_pcoData->version) 
		<< ALWAYS_NL << _checkLogFiles()
		;

	m_bin.changed = Invalid;
	
	_init();
	
    DEB_ALWAYS() << "... new _AcqThread";
    m_acq_thread = new _AcqThread(*this);
    DEB_ALWAYS() << "... start";
    m_acq_thread->start();
    DEB_ALWAYS() << "... exit";
	
	m_config = FALSE;
	_setStatus(Camera::Ready,true);

}


//=========================================================================================================
//=========================================================================================================
void Camera::_init(){
	DEB_CONSTRUCTOR();
	DEF_FNID;

	DEB_ALWAYS() << fnId << " [entry]";

	//char msg[MSG_SIZE + 1];
	//int error=0;
	//const char *errMsg;
	UNUSED const char *pcoFn;

	//-------------------- linux
	

    //DWORD err;
    //CPco_com *camera;
    //CPco_grab_cl_me4* grabber;


    board=0;
    //char infostr[100];
    //char number[20];
    //char sMsg[128];

    //int x;
    //char c;
    //int ima_count=100;
    //int loop_count=1;
    //int PicTimeOut=10000; //10 seconds
    //WORD act_recstate;
    //DWORD exp_time,delay_time,pixelrate;
    //WORD exp_timebase,del_timebase;
    //DWORD width,height,secs,nsecs;
    //SC2_Camera_Description_Response description;
    //PCO_SC2_CL_TRANSFER_PARAM clpar;

    //double freq;
    //SHORT ccdtemp,camtemp,pstemp;
    //WORD camtype;
    //DWORD serialnumber;
    //WORD actlut,lutparam;
    int iErr;

    //int bufnum=20;

    DEB_ALWAYS()  << "setting the log" ;
    
    unsigned long long debugSdk = 0;
        
	paramsGet("debugSdk", debugSdk);
    
    
    //mylog->set_logbits(0x0000F0FF);
    mylog->set_logbits(debugSdk);
    printf("Logging set to 0x%x\n",mylog->get_logbits());

    _pco_Open_Cam(iErr);
    
    _pco_GetCameraType(iErr);
    if(iErr)
    {
        camera->Close_Cam();
        delete camera;
        camera = NULL;
        THROW_HW_ERROR(Error) ;
    }

    _pco_Open_Grab(iErr);
    
    _pco_GetCameraInfo(iErr);
    
    _pco_ResetSettingsToDefault(iErr);
    
    _pco_SetCameraToCurrentTime(iErr);
    _pco_GetTransferParameter(iErr);
    _pco_GetTemperatureInfo(iErr);
    
    DWORD pixRateActual, pixRateNext;
    _pco_GetPixelRate(pixRateActual, pixRateNext, iErr);
    DEB_ALWAYS()  << DEB_VAR2(pixRateActual, pixRateNext) ;
    
    _pco_GetLut(iErr);
    _pco_SetRecordingState(0, iErr);

    _pco_SetTimestampMode(2, iErr);


#if 0    

  //---------------- TODO 
  delay_time=exp_time = 0;
  err=camera->PCO_SetDelayExposure(delay_time,exp_time);
  if(err!=PCO_NOERROR)
   printf("PCO_SetDelayExposure() Error 0x%x\n",err);


	//-------------------- linux



	_armRequired(true);


	m_log.clear();
	sprintf_s(msg, MSG_SIZE, "*** Pco log %s\n", getTimestamp(Iso));
	m_log.append(msg);


		// --- Open Camera - close before if it is open
	if(m_handle) {
		UNUSED const char *pcoFn;
		DEB_ALWAYS() << (char *) fnId << " [closing opened camera]";
		PCO_FN1(error, pcoFn,PCO_CloseCamera, m_handle);
		PCO_THROW_OR_TRACE(error, "_init(): PCO_CloseCamera - closing opened cam") ;
		m_handle = 0;
	}

	int retrySleep =30;
	int retryMax = 0;
	int retry = retryMax;
	while(true) {
		PCO_FN2(error, pcoFn,PCO_OpenCamera, &m_handle, 0);
		if(error)
		{
			if(retry--<=0) break;
			DEB_ALWAYS() << "\n... ERROR - PCO_OpenCamera / retry ... " << DEB_VAR2(retry, retrySleep);
			PCO_FN0(error, pcoFn,PCO_ResetLib);
			::Sleep(retrySleep * 1000);
		} else {break;}
	} 

	if(error)
	{ 
		PCO_FN0(error, pcoFn,PCO_ResetLib);
		DEB_ALWAYS() 
			<< "\n... ERROR - PCO_OpenCamera / abort"  
			<< "\n... Waiting to reset the camera .... " << DEB_VAR1(retrySleep); 
		::Sleep(retrySleep * 1000);
		THROW_HW_ERROR(Error) ;
	}
		
	PCO_THROW_OR_TRACE(error, "_init(): PCO_OpenCamera") ;
	
	errMsg = _pco_GetCameraType(error);
	PCO_THROW_OR_TRACE(error, errMsg) ;

	DEB_ALWAYS() << fnId << " [camera opened] " << DEB_VAR1(m_handle);

	// -- Initialise ADC
	//-------------------------------------------------------------------------------------------------
	// PCO_SetADCOperation
    // Set analog-digital-converter (ADC) operation for reading the image sensor data. Pixel data can be
    // read out using one ADC (better linearity) or in parallel using two ADCs (faster). This option is
    // only available for some camera models. If the user sets 2ADCs he must center and adapt the ROI
    // to symmetrical values, e.g. pco.1600: x1,y1,x2,y2=701,1,900,500 (100,1,200,500 is not possible).
    //
	// DIMAX -> 1 adc
	//-------------------------------------------------------------------------------------------------
	
	// set ADC = 1 for better linearity (when it is configurable ...)
	int adc_working;
	_pco_SetADCOperation(1, adc_working);
	m_pcoData->wNowADC= (WORD) adc_working;

		// -- Initialise size, bin, roi
	unsigned int maxWidth, maxHeight,maxwidth_step, maxheight_step; 
	getMaxWidthHeight(maxWidth, maxHeight);
	getXYsteps(maxwidth_step, maxheight_step);


	_get_MaxRoi(m_RoiLima);
	_get_MaxRoi(m_RoiLimaRequested);
	
	WORD bitsPerPix;
	getBitsPerPixel(bitsPerPix);

	sprintf_s(msg, MSG_SIZE, "* CCD Size = X[%d] * Y[%d] (%d bits)\n", maxWidth, maxHeight, bitsPerPix);
	DEB_TRACE() <<   msg;
	m_log.append(msg);
	
	sprintf_s(msg, MSG_SIZE, "* ROI Steps = x:%d, y:%d\n", maxwidth_step, maxheight_step);
	DEB_TRACE() <<   msg;
	m_log.append(msg);

	errMsg = _pco_GetTemperatureInfo(error);
	PCO_THROW_OR_TRACE(error, errMsg) ;

	_pco_SetRecordingState(0, error);
	
	if(_isCameraType(Dimax)) _init_dimax();
	else if(_isCameraType(Pco2k)) _init_dimax();
	else if(_isCameraType(Pco4k)) _init_dimax();
	else if(_isCameraType(Edge)) _init_edge();
	else {
		char msg[MSG_SIZE+1];
		sprintf_s(msg, MSG_SIZE, "Camera type not supported! [x%04x]", _getCameraType());
		DEB_ALWAYS() <<  msg;

		throw LIMA_HW_EXC(Error, msg);
	}


	m_cam_connected = true;
	error = 0;

	if(!m_cam_connected)
		throw LIMA_HW_EXC(Error, "Camera not found!");

	_pco_initHWIOSignal(0, error);
#endif

  DEB_TRACE() << m_log;
  DEB_TRACE() << "END OF CAMERA";
	DEB_ALWAYS() << fnId << " [exit]";

}

//=========================================================================================================
//=========================================================================================================
void  Camera::_init_dimax() {

	DEB_CONSTRUCTOR();
	char msg[MSG_SIZE + 1];
	const char *pcoFn;

	int error=0;
	DWORD _dwValidImageCnt, _dwMaxImageCnt;

	
	// block #1 -- Get RAM size
	{
		int segmentPco, segmentArr;

		DWORD ramSize;
		WORD pageSize;

		WORD bitsPerPix;
		getBitsPerPixel(bitsPerPix);

		PCO_FN3(error, pcoFn,PCO_GetCameraRamSize, m_handle, &ramSize, &pageSize);
		PCO_THROW_OR_TRACE(error, pcoFn) ;

		m_pcoData->dwRamSize = ramSize;     // nr of pages of the ram
		m_pcoData->wPixPerPage = pageSize;    // nr of pixels of the page

		sprintf_s(msg, MSG_SIZE, "* ramPages[%d] pixPerPage[%d] bitsPerPix[%d]\n",  
				m_pcoData->dwRamSize, m_pcoData->wPixPerPage, bitsPerPix);
		DEB_TRACE() <<   msg;
		m_log.append(msg);
		
		double nrBytes = (double) m_pcoData->dwRamSize  * (double) m_pcoData->wPixPerPage * 
			(double)bitsPerPix / 9.; // 8 bits data + 1 bit CRC -> 9
		
		sprintf_s(msg, MSG_SIZE, "* camMemorySize [%lld B] [%g GB]\n",  
				(long long int) nrBytes, nrBytes/GIGABYTE);
		DEB_TRACE() <<   msg;
		m_log.append(msg);

		// ----------------- get initial seg Size - images & print

		// ---- get the size in pages of each of the 4 segments

		DWORD   segSize[4];
		PCO_FN2(error, pcoFn,PCO_GetCameraRamSegmentSize, m_handle, segSize);
		PCO_THROW_OR_TRACE(error, pcoFn) ;

		for(segmentArr=0; segmentArr < PCO_MAXSEGMENTS ; segmentArr++) {
			segmentPco = segmentArr +1;		// PCO segment (1 ... 4)
			m_pcoData->dwSegmentSize[segmentArr] = segSize[segmentArr];

			sprintf_s(msg, MSG_SIZE, "* segment[%d] number of pages[%d]\n", segmentPco,m_pcoData->dwSegmentSize[segmentArr]);
			DEB_TRACE() <<   msg;
			m_log.append(msg);

		}

		//---- get nr de images in each segment & nr max of img on each segmente
		for(segmentArr=0;  segmentArr< PCO_MAXSEGMENTS ; segmentArr++) {
			segmentPco = segmentArr +1;

			PCO_FN4(error, pcoFn,PCO_GetNumberOfImagesInSegment, m_handle, segmentPco, &_dwValidImageCnt, &_dwMaxImageCnt);
			PCO_THROW_OR_TRACE(error, pcoFn) ;

			m_pcoData->dwValidImageCnt[segmentArr] = _dwValidImageCnt;
			m_pcoData->dwMaxImageCnt[segmentArr] = _dwMaxImageCnt;

			sprintf_s(msg, MSG_SIZE, "* segment[%d] nr images [%d]  max imag [%d]\n", segmentPco, _dwValidImageCnt,  _dwMaxImageCnt);
			DEB_TRACE() <<   msg;
			m_log.append(msg);

		} // for	


		// set the first segment to the max ram size, the others = 0
		// This function will result in all segments being cleared. 
		// All previously recorded images will be lost!

		//m_pcoData->dwSegmentSize[0] = m_pcoData->dwRamSize;
		for(segmentArr=1;  segmentArr < PCO_MAXSEGMENTS ; segmentArr++) {
			m_pcoData->dwSegmentSize[0] += m_pcoData->dwSegmentSize[segmentArr];
			m_pcoData->dwSegmentSize[segmentArr] = 0;
		}
		sprintf_s(msg, MSG_SIZE, "* m_pcoData->dwSegmentSize0 [%d]  m_pcoData->dwRamSize [%d]\n", m_pcoData->dwSegmentSize[0], m_pcoData->dwRamSize);
		DEB_TRACE() <<   msg;
		m_log.append(msg);


		PCO_FN2(error, pcoFn,PCO_SetCameraRamSegmentSize, m_handle, &m_pcoData->dwSegmentSize[0]);
		PCO_THROW_OR_TRACE(error, pcoFn) ;
	}  // block #1 

	DEB_TRACE() <<  "end block 1 / get initial seg Size - images";

	{
		UNUSED int segmentPco, segmentArr;
		
		unsigned int maxWidth, maxHeight; 
		getMaxWidthHeight(maxWidth, maxHeight);

		
		DWORD pages_per_image = maxWidth * maxHeight / m_pcoData->wPixPerPage;

		///------------------------------------------------------------------------TODO ?????
		for(segmentArr=0; segmentArr < PCO_MAXSEGMENTS ; segmentArr++) {
			segmentPco = segmentArr +1;
			if(m_pcoData->dwMaxImageCnt[segmentArr] == 0){
				m_pcoData->dwMaxImageCnt[segmentArr] = m_pcoData->dwSegmentSize[segmentArr] / pages_per_image;
				if(m_pcoData->dwMaxImageCnt[segmentArr] > 4) m_pcoData->dwMaxImageCnt[segmentArr] -= 2;
			}
		}	
	} // block


	// -- Get Active RAM segment 

		PCO_FN2(error, pcoFn,PCO_GetActiveRamSegment, m_handle, &m_pcoData->wActiveRamSegment);
		PCO_THROW_OR_TRACE(error, pcoFn) ;

		PCO_FN4(error, pcoFn,PCO_GetNumberOfImagesInSegment, m_handle, m_pcoData->wActiveRamSegment, &_dwValidImageCnt, &_dwMaxImageCnt);
		PCO_THROW_OR_TRACE(error, pcoFn) ;



	DEB_TRACE() <<  "original DONE";


}


//=========================================================================================================
//=========================================================================================================
void Camera::_init_edge() {

	m_pcoData->fTransferRateMHzMax = 550.;



}



//=========================================================================================================
//=========================================================================================================
Camera::~Camera()
{
	DEB_DESTRUCTOR();
	DEB_ALWAYS() << "DESTRUCTOR ...................." ;

    delete m_acq_thread;
    m_acq_thread = NULL;


	m_cam_connected = false;

	reset(RESET_CLOSE_INTERFACE);
}



//=================================================================================================
//=================================================================================================
void Camera::getCameraName(std::string& name)
{
  DEB_MEMBER_FUNCT();
  DEB_RETURN() << DEB_VAR1(m_pcoData->camera_name);

  name = m_pcoData->camera_name;
}

//=================================================================================================
//=================================================================================================
void Camera::prepareAcq()
{
    DEB_MEMBER_FUNCT();
	DEF_FNID;

	DEB_ALWAYS() << fnId << " [ENTRY]" ;

	int error;
	const char *msg;
    int iRequestedFrames;
    WORD state;

    m_sync->getNbFrames(iRequestedFrames);

	//SetBinning, SetROI, ARM, GetSizes, AllocateBuffer.

    //------------------------------------------------- set binning if needed
    _pco_SetBinning(error);

    //------------------------------------------------- set roi if needed
    _pco_SetROI(error);

	//------------------------------------------------- triggering mode 
    //------------------------------------- acquire mode : ignore or not ext. signal
	msg = _pco_SetTriggerMode_SetAcquireMode(error);
    PCO_THROW_OR_TRACE(error, msg) ;

    // ----------------------------------------- storage mode (recorder + sequence)
    if(_isCameraType(Dimax)) {
		
			// live video requested frames = 0
		enumPcoStorageMode mode = (iRequestedFrames > 0) ? RecSeq : Fifo;

		msg = _pco_SetStorageMode_SetRecorderSubmode(mode, error);
		PCO_THROW_OR_TRACE(error, msg) ;
	}

	if(_isCameraType(Pco4k | Pco2k)) {
			// live video requested frames = 0
		enumPcoStorageMode mode = Fifo;
		DEB_ALWAYS() << "PcoStorageMode mode - PCO2K / 4K: " << DEB_VAR1(mode);

		msg = _pco_SetStorageMode_SetRecorderSubmode(mode, error);
		PCO_THROW_OR_TRACE(error, msg) ;
	}

    //----------------------------------- set exposure time & delay time
	_pco_SetDelayExposureTime(error,0);   // initial set of delay (phase = 0)
	PCO_THROW_OR_TRACE(error, "_pco_SetDelayExposureTime") ;


    //------------------------------------------------- check recording state
    state = _pco_GetRecordingState(error);
    PCO_THROW_OR_TRACE(error, "PCO_GetRecordingState") ;

    if (state>0) {
        DEB_TRACE() << "Force recording state to 0x0000" ;

		_pco_SetRecordingState(0, error);
        PCO_THROW_OR_TRACE(error, "PCO_SetRecordingState") ;
	}


	msg = _pco_SetMetaDataMode(0, error); PCO_THROW_OR_TRACE(error, msg) ;
 
	//--------------------------- PREPARE / pixel rate - ARM required 
	_pco_SetPixelRate(error); PCO_THROW_OR_TRACE(error, "_pco_SetPixelRate") ;
		
	//--------------------------- PREPARE / clXferParam, LUT - ARM required 
	_pco_SetTransferParameter_SetActiveLookupTable(error); 
	PCO_THROW_OR_TRACE(error, "_pco_SetTransferParameter_SetActiveLookupTable") ;

    
	//--------------------------- PREPARE / cocruntime (valid after PCO_SetDelayExposureTime and ARM)
	_pco_GetCOCRuntime(error); PCO_THROW_OR_TRACE(error, "_pco_GetCOCRuntime") ;


	DEB_ALWAYS() << fnId << " [EXIT]" ;
    return;
}


//=================================================================================================
//=================================================================================================
void Camera::startAcq()
{
    DEB_MEMBER_FUNCT();

	m_acq_frame_nb = -1;
	m_pcoData->pcoError = 0;
	m_pcoData->pcoErrorMsg[0] = 0;

	traceAcq.traceAcqClean();

	TIME_USEC tStart;
	msElapsedTimeSet(tStart);

	m_sync->getExpTime(traceAcq.sExposure);
	m_sync->getLatTime(traceAcq.sDelay);
	traceAcq.msImgCoc = pcoGetCocRunTime() * 1000.;

//=====================================================================
	DEF_FNID;
    //WORD state;
    //HANDLE hEvent= NULL;

	DEB_ALWAYS() << fnId << " [ENTRY]" ;

	//int error;
	//const char *msg;

    int iRequestedFrames;

			// live video requested frames = 0
    m_sync->getNbFrames(iRequestedFrames);



//-----------------------------------------------------------------------------------------------
//	5. Arm the camera.
//	6. Get the sizes and allocate a buffer:
//		PCO_GETSIZES(hCam, &actualsizex, &actualsizey, &ccdsizex, &ccdsizey)
//		PCO_ALLOCATEBUFFER(hCam, &bufferNr, actualsizex * actualsizey * sizeof(WORD), &data, &hEvent)
//		In case of CamLink and GigE interface: PCO_CamLinkSetImageParameters(actualsizex, actualsizey)
//		PCO_ArmCamera(hCam)
//-----------------------------------------------------------------------------------------------
	

#if 0
	//--------------------------- PREPARE / ARM  
	DEB_ALWAYS() << "\n   ARM the camera / PCO_ArmCamera (1)";
	PCO_FN1(error, msg,PCO_ArmCamera, m_handle); 
	PCO_THROW_OR_TRACE(error, msg) ;
	
	
	//--------------------------- PREPARE / getSizes (valid after ARM) alloc buffer
	
	
	_pco_GetSizes( &m_pcoData->wXResActual, &m_pcoData->wYResActual, &m_pcoData->wXResMax, &m_pcoData->wYResMax, error); 

	//PCO_FN5(error, msg,PCO_GetSizes , m_handle, &m_pcoData->wXResActual, &m_pcoData->wYResActual, &m_pcoData->wXResMax, &m_pcoData->wYResMax) ;
	PCO_THROW_OR_TRACE(error, msg) ;

	m_buffer->_pcoAllocBuffers(false);

	//--------------------------- PREPARE / img parameters
	msg = _pco_SetCamLinkSetImageParameters(error); PCO_THROW_OR_TRACE(error, msg) ;

	//--------------------------- PREPARE / cocruntime (valid after PCO_SetDelayExposureTime and ARM)
	msg = _pco_GetCOCRuntime(error); PCO_THROW_OR_TRACE(error, msg) ;


    //------------------------------------------------- checking nr of frames
    if(_isCameraType(Dimax)){
        unsigned long framesMax;
        framesMax = pcoGetFramesMax(m_pcoData->wActiveRamSegment);

        if ((((unsigned long) iRequestedFrames) > framesMax)) {
            throw LIMA_HW_EXC(Error, "frames OUT OF RANGE");
        }
    } 
	
#endif
	//------------------------------------------------- start acquisition

    DEB_ALWAYS() << "[... starting]";
	traceAcq.msStartAcqStart = msElapsedTime(tStart);

	m_sync->setStarted(true);
	m_sync->setExposing(pcoAcqRecordStart);

    _setStatus(Camera::Exposure, false);
    
	//Start acqusition thread
	AutoMutex aLock(m_cond.mutex());
    m_wait_flag = false;
    m_cond.broadcast();
    DEB_ALWAYS() << "[... starting after mutex]";


#if 0
	if(_isCameraType(Edge)){
		_beginthread( _pco_acq_thread_edge, 0, (void*) this);
		m_cam->traceAcq.msStartAcqEnd = msElapsedTime(tStart);
		return;
	}

	if(_isCameraType(Pco2k | Pco4k)){
		_pco_SetRecordingState(1, error);
		_beginthread( _pco_acq_thread_ringBuffer, 0, (void*) this);
		m_cam->traceAcq.msStartAcqEnd = msElapsedTime(tStart);
		return;
	}

	if(_isCameraType(Dimax)){
		_pco_SetRecordingState(1, error);
		if(iRequestedFrames > 0 ) {
			_beginthread( _pco_acq_thread_dimax, 0, (void*) this);
		} else {
			_beginthread( _pco_acq_thread_dimax_live, 0, (void*) this);
		}
		m_cam->traceAcq.msStartAcqEnd = msElapsedTime(tStart);
		return;
	}

	throw LIMA_HW_EXC(Error, "unkown camera type");
#endif
    DEB_ALWAYS() << "[exit]";
	return;
}


//==========================================================================================================
//==========================================================================================================

long msElapsedTime(TIME_USEC &t0) {
    long msDiff;
	TIME_USEC tNow;


#ifdef __linux__
    long seconds, useconds;
    gettimeofday(&tNow, NULL);

    seconds  = tNow.tv_sec  - t0.tv_sec;
    useconds = tNow.tv_usec - t0.tv_usec;

    msDiff = long ( ((seconds) * 1000 + useconds/1000.0) + 0.5 );
#else
	_ftime64_s(&tNow);

	msDiff = (long)((tNow.time - t0.time)*1000) + (tNow.millitm - t0.millitm);
#endif

	return msDiff;
}

void msElapsedTimeSet(TIME_USEC &t0) {

#ifdef __linux__
    gettimeofday(&t0, NULL);
#else
	_ftime64_s(&t0);
#endif

}


void usElapsedTimeSet(TIME_UTICKS &tick0) {

#ifdef __linux__
    clock_gettime(CLOCK_REALTIME, &tick0); 

#else
	QueryPerformanceCounter(&tick0);
#endif
}

long long usElapsedTime(TIME_UTICKS &tick0) {
	TIME_UTICKS tick;   // A point in time
	long long usDiff;

#ifdef __linux__
    clock_gettime(CLOCK_REALTIME, &tick); 

    usDiff = (long long int)   ((tick.tv_sec - tick0.tv_sec) * 1000000. +
            (tick.tv_nsec - tick0.tv_nsec) / 1000. );

#else
	TIME_UTICKS ticksPerSecond;
	long long uS, uS0;
	QueryPerformanceFrequency(&ticksPerSecond); 
	QueryPerformanceCounter(&tick);

	double ticsPerUSecond = ticksPerSecond.QuadPart/1.0e6;
	uS = (long long) (tick.QuadPart/ticsPerUSecond);
	uS0 = (long long) (tick0.QuadPart/ticsPerUSecond);
    usDiff = uS - uS0;
#endif

	return usDiff;

}

double usElapsedTimeTicsPerSec() {
    double ticks = 0;
#ifdef __linux__
	TIME_UTICKS tick;   // A point in time
    clock_gettime(CLOCK_REALTIME, &tick); 

#else
	TIME_UTICKS ticksPerSecond;
	QueryPerformanceFrequency(&ticksPerSecond); 
    ticks = (double) ticksPerSecond.QuadPart;
#endif
	return ticks;

}



//==========================================================================================================
//==========================================================================================================

const char *sPcoAcqStatus[] ={
	"pcoAcqIdle", 
	"pcoAcqStart", 
	"pcoAcqRecordStart", 
	"pcoAcqRecordEnd",  
	"pcoAcqTransferStart", 
	"pcoAcqTransferEnd", 
	"pcoAcqStop", 
	"pcoAcqTransferStop", 
	"pcoAcqRecordTimeout",
	"pcoAcqWaitTimeout",
	"pcoAcqWaitError",
	"pcoAcqError",
	"pcoAcqPcoError",
};



//=====================================================================
//=====================================================================
void Camera::reset(int reset_level)
{
	DEB_MEMBER_FUNCT();
	int error;
	//const char *msg;


	switch(reset_level) 
	{
	case RESET_CLOSE_INTERFACE: 
		DEB_ALWAYS() << "\n... RESET - freeBuff, closeCam, resetLib  " << DEB_VAR1(reset_level) ;

		//if(m_buffer) m_buffer->_pcoAllocBuffersFree();

		_pco_Close_Cam(error);
		
		//PCO_FN0(error, msg,PCO_ResetLib); PCO_PRINT_ERR(error, msg); 
		break;

	default:
		DEB_ALWAYS() << "\n... RESET -  " << DEB_VAR1(reset_level);
		//_init();
		break;
	}

}


//=========================================================================================================
//=========================================================================================================
#define LEN_TMP_MSG 256
int Camera::PcoCheckError(int line, const char *file, int err, const char *fn, const char *comments) {
	DEB_MEMBER_FUNCT();
	//DEF_FNID;


	//static char lastErrorMsg[500];
	static char tmpMsg[LEN_TMP_MSG+1];
	char *msg;
	const char *title;
    const char *titleErr  ="\n------------------- PCO SKD ERROR -------------------" ;
    const char *titleWarn ="\n------------- PCO SKD WARNING - IGNORED -------------" ;
    const char *titleEnd  ="\n-----------------------------------------------------" ;
	//size_t lg;

	sprintf_s(tmpMsg,LEN_TMP_MSG,"%s [%s][%d] %s", fn, file, line, comments);
	m_msgLog->add(tmpMsg);
	if (err != 0) {
		DWORD dwErr = err;
		m_pcoData->pcoError = err;
		msg = m_pcoData->pcoErrorMsg;

		PCO_GetErrorText(dwErr, msg, ERR_SIZE-14);
        

		if(err & PCO_ERROR_IS_WARNING) {
            title = titleWarn;
			DEB_WARNING()
			    << title 
		        << "\n   " << tmpMsg  
		        << "\n   " << msg
                << titleEnd 
                ;
			return 0;
		}
		
		
        title = titleErr;
		DEB_ALWAYS() 
            << title 
		    << "\n   " << tmpMsg  
		    << "\n   " << msg
            << titleEnd 
        ;
		return (err);
	}
	return (err);
}


//=========================================================================================================
//=========================================================================================================
char* Camera::_PcoCheckError(int line, const char *file, int err, int &error, const char *fn) {
	static char lastErrorMsg[ERR_SIZE];
	static char tmpMsg[LEN_TMP_MSG+1];
	char *msg;
	size_t lg;

	sprintf_s(tmpMsg,LEN_TMP_MSG,"%s (%d)", fn, line);
	m_msgLog->add(tmpMsg);

	error = m_pcoData->pcoError = err;
	msg = m_pcoData->pcoErrorMsg;

	if (err != 0) {
		PCO_GetErrorText(err, lastErrorMsg, ERR_SIZE-14);
		//strncpy_s(msg, ERR_SIZE, lastErrorMsg, _TRUNCATE); 
		strcpy_s(msg, ERR_SIZE, lastErrorMsg); 

		lg = strlen(msg);
		sprintf_s(msg+lg,ERR_SIZE - lg, " [%s][%d]", file, line);

		return lastErrorMsg;
	}
	return NULL;
}

//=========================================================================================================
//=========================================================================================================
unsigned long Camera::pcoGetFramesMax(int segmentPco){
	DEF_FNID;

		int segmentArr = segmentPco-1;
		unsigned long framesMax;
		unsigned long xroisize,yroisize;
		unsigned long long pixPerFrame, pagesPerFrame;

		if(_isCameraType(Edge)) {
			return LONG_MAX;
		}



		if(!_isCameraType(Dimax | Pco2k | Pco4k)) {
			printf("=== %s> unknown camera type [%d]\n", fnId, _getCameraType());
			return 0;
		}

		if((segmentPco <1) ||(segmentPco > PCO_MAXSEGMENTS)) {
			printf("=== %s> ERROR segmentPco[%d]\n", fnId, segmentPco);
			return 0;
		}

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

		return framesMax;
	}






//=================================================================================================
//=================================================================================================
void Camera::getArmWidthHeight(WORD& width,WORD& height)
{
	DEB_MEMBER_FUNCT();
	//DEF_FNID;

	width = m_pcoData->wXResActual;
	height = m_pcoData->wYResActual;
}


		const char *ptr;







//=================================================================================================
//=================================================================================================
int Camera::dumpRecordedImages(int &nrImages, int &error){
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg;

	HANDLE m_handle __attribute__((unused)) = getHandle();
	WORD wSegment = _pco_GetActiveRamSegment(); 
	DWORD _dwValidImageCnt, _dwMaxImageCnt;


	WORD wRecState_actual;

	nrImages = -1;

	if(!_isCameraType(Dimax)) return -2;

	PCO_FN2(error, msg,PCO_GetRecordingState, m_handle, &wRecState_actual);
	PCO_PRINT_ERR(error, msg); 	
	
	if (error) return -100;
	if(wRecState_actual != 0) return -1;


	msg = _PcoCheckError(__LINE__, __FILE__, camera->PCO_GetNumberOfImagesInSegment( wSegment, &_dwValidImageCnt, &_dwMaxImageCnt), error);
	if(error) {
		printf("=== %s [%d]> ERROR %s\n", fnId, __LINE__, msg);
		throw LIMA_HW_EXC(Error, "PCO_GetNumberOfImagesInSegment");
	}

	nrImages = _dwValidImageCnt;

	return 0;

}





//=================================================================================================
//=================================================================================================



#if 0
//=================================================================================================
//=================================================================================================
void Camera::_pco_set_shutter_rolling_edge(int &error){
		
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg;
	char msgBuff[MSG_SIZE+1];

	DWORD _dwSetup;
	DWORD m_dwSetup[10];
	WORD m_wLen = 10;
	WORD m_wType;

	// PCO recommended timing values
	int ts[3] = {2000, 3000, 250}; // command, image, channel timeout
	DWORD sleepMs = 10000;  // sleep time after reboot

	if(!_isCameraType(Edge)) {
		return ;
	}

	DEB_ALWAYS() << fnId << " [entry - edge] ";

	m_config = TRUE;

	// DWORD m_dwSetup[10];
	// WORD m_wLen = 10;
	// WORD m_wType;
	// int ts[3] = { 2000, 3000, 250}; // command, image, channel timeout
	// PCO_OpenCamera(&m_hCam,0);
	// PCO_GetCameraSetup(m_hCam, &m_wType, &m_dwSetup[0], &m_wLen);

	// m_dwSetup[0] = PCO_EDGE_SETUP_GLOBAL_SHUTTER;
	// PCO_SetTimeouts(m_hCam, &ts[0], sizeof(ts));
	// PCO_SetCameraSetup(m_hCam, m_wType, &m_dwSetup[0], m_wLen);
	// PCO_RebootCamera(m_hCam);
	// PCO_CloseCamera(m_hCam);
	// Camera setup parameter for pco.edge:
	// #define PCO_EDGE_SETUP_ROLLING_SHUTTER 0x00000001         // rolling shutter
	// #define PCO_EDGE_SETUP_GLOBAL_SHUTTER  0x00000002         // global shutter

	
	_dwSetup = m_pcoData->bRollingShutter ? PCO_EDGE_SETUP_ROLLING_SHUTTER : PCO_EDGE_SETUP_GLOBAL_SHUTTER;

    PCO_FN4(error, msg,PCO_GetCameraSetup, m_handle, &m_wType, &m_dwSetup[0], &m_wLen);
    PCO_PRINT_ERR(error, msg); 	if(error) return;

	if(m_dwSetup[0] == _dwSetup) { 
		DEB_ALWAYS() << fnId << " [exit - no change] ";
		m_config = FALSE;
		return;
	}

	msg = msgBuff;
	sprintf_s((char *) msg, MSG_SIZE, "[Change ROLLING SHUTTER from [%d] to [%d]]", 
		m_dwSetup[0]==PCO_EDGE_SETUP_ROLLING_SHUTTER, _dwSetup==PCO_EDGE_SETUP_ROLLING_SHUTTER);
	DEB_ALWAYS() << fnId << " " << msg;

	m_dwSetup[0] = _dwSetup;

    PCO_FN3(error, msg,PCO_SetTimeouts, m_handle, &ts[0], sizeof(ts));
    PCO_PRINT_ERR(error, msg); 	if(error) return;

	msg = "[PCO_SetCameraSetup]";
	DEB_ALWAYS() << fnId << " " << msg;
    PCO_FN4(error, msg,PCO_SetCameraSetup, m_handle, m_wType, &m_dwSetup[0], m_wLen);
    PCO_PRINT_ERR(error, msg); 	if(error) return;

	msg = "[PCO_RebootCamera]";
	DEB_ALWAYS() << fnId << " " << msg;
    PCO_FN1(error, msg,PCO_RebootCamera, m_handle);
    PCO_PRINT_ERR(error, msg); 	if(error) return;

	//m_sync->_getBufferCtrlObj()->_pcoAllocBuffersFree();
	m_buffer->_pcoAllocBuffersFree();

	msg = "[PCO_CloseCamera]";
	DEB_ALWAYS() << fnId << " " << msg;
    PCO_FN1(error, msg,PCO_CloseCamera, m_handle);
    PCO_PRINT_ERR(error, msg); 	if(error) return;
	m_handle = 0;
	
	msg = msgBuff;
	sprintf_s((char *) msg, MSG_SIZE, "[Sleep %u ms]", sleepMs);
	DEB_ALWAYS() << fnId << " " << msg;
	::Sleep(sleepMs);

	_init();

	DEB_ALWAYS() << fnId << " [exit] ";

	m_config = FALSE;
	return;

}
#endif
//=================================================================================================
//=================================================================================================
bool Camera::_isValid_pixelRate(DWORD dwPixelRate){
		
	DEB_MEMBER_FUNCT();
	//DEF_FNID;

	// pixelrate 1     (long word; frequency in Hz)
	// pixelrate 2,3,4 (long word; frequency in Hz; if not available, then value = 0)

	if(dwPixelRate > 0) 
		for(int i = 0; i < 4; i++) {			
			if(dwPixelRate == m_pcoData->stcPcoDesc1.dwPixelRateDESC[i]) return TRUE;
		}

	return FALSE;
}



//=================================================================================================
//=================================================================================================

void Camera::getXYsteps(unsigned int &xSteps, unsigned int &ySteps){
	DEB_MEMBER_FUNCT();
	//DEF_FNID;
	
	xSteps = m_pcoData->stcPcoDesc1.wRoiHorStepsDESC;
	ySteps = m_pcoData->stcPcoDesc1.wRoiVertStepsDESC;
}
        
/***
void Camera::getMaxWidthHeight(unsigned int &xMax, unsigned int &yMax){
	DEB_MEMBER_FUNCT();
	//DEF_FNID;
	xMax = m_pcoData->stcPcoDesc1.wMaxHorzResStdDESC;
	yMax = m_pcoData->stcPcoDesc1.wMaxVertResStdDESC;
}
****/
	
void Camera::getMaxWidthHeight(DWORD &xMax, DWORD &yMax){
	DEB_MEMBER_FUNCT();
	//DEF_FNID;
	xMax = m_pcoData->stcPcoDesc1.wMaxHorzResStdDESC;
	yMax = m_pcoData->stcPcoDesc1.wMaxVertResStdDESC;
}


void Camera::getBytesPerPixel(unsigned int& pixbytes){
	pixbytes = (m_pcoData->stcPcoDesc1.wDynResDESC <= 8)?1:2;
}

void Camera::getBitsPerPixel(WORD& pixbits){
	pixbits = m_pcoData->stcPcoDesc1.wDynResDESC;
}


/****************************************************************************************
 Some sensors have a ROI stepping. See the camera description and check the parameters
 wRoiHorStepsDESC and/or wRoiVertStepsDESC.

 For dual ADC mode the horizontal ROI must be symmetrical. For a pco.dimax the horizontal and
 vertical ROI must be symmetrical. For a pco.edge the vertical ROI must be symmetrical.
****************************************************************************************/

int Camera::_checkValidRoi(const Roi &roi_new, Roi &roi_fixed){
		
	DEB_MEMBER_FUNCT();
	//DEF_FNID;

	int iInvalid;
	unsigned int x0, x1, y0, y1;
	unsigned int x0org, x1org, y0org, y1org;
	unsigned int diff0, diff1, tmp;

	unsigned int xMax, yMax, xSteps, ySteps;
	getMaxWidthHeight(xMax, yMax);
	getXYsteps(xSteps, ySteps);

	x0org = x0 = roi_new.getTopLeft().x+1;
	x1org = x1 = roi_new.getBottomRight().x+1;
	y0org = y0 = roi_new.getTopLeft().y+1;
	y1org = y1 = roi_new.getBottomRight().y+1;

	// lima roi [0,2047]
	//  pco roi [1,2048]

	iInvalid = 0;

	if(x0 < 1) {x0 = 1 ; iInvalid |= Xrange;}
	if(x1 > xMax) {x1 = xMax ; iInvalid |= Xrange;}
	if(x0 > x1) { tmp = x0 ; x0 = x1 ; x1 = tmp;  iInvalid |= Xrange; }

	if ( (diff0 = (x0 - 1) % xSteps) != 0 ) { x0 -= diff0; iInvalid |= Xsteps; }
	if ( (diff1 = x1 % xSteps) != 0 ) { x1 += xSteps - diff1; iInvalid |= Xsteps; }

	if(y0 < 1) {y0 = 1 ; iInvalid |= Yrange;}
	if(y1 > yMax) {y1 = yMax ; iInvalid |= Yrange;}
	if(y0 > y1) { tmp = y0 ; y0 = y1 ; y1 = tmp;  iInvalid |= Yrange; }

	if ( (diff0 = (y0 - 1) % ySteps) != 0 ) { y0 -= diff0; iInvalid |= Ysteps; }
	if ( (diff1 = y1 % ySteps) != 0 ) { y1 += ySteps - diff1; iInvalid |= Ysteps; }


	bool bSymX = false, bSymY = false;
	if(_isCameraType(Dimax)){ bSymX = bSymY = true; }
	if(_isCameraType(Edge)) { bSymY = true; }

	int adc_working, adc_max;
	_pco_GetADCOperation(adc_working, adc_max);
	if(adc_working != 1) { bSymX = true; }

	if(bSymY){
		if( (diff0 = y0 - 1) != (diff1 = yMax - y1) ){
			if(diff0 > diff1) 
				y0 -= diff0 - diff1;
			else
				y1 += diff1 - diff0;

			iInvalid |= Ysym;
		}
	}

	if(bSymX){
		if( (diff0 = x0 - 1) != (diff1 = xMax - x1) ){
			if(diff0 > diff1) 
				x0 -= diff0 - diff1;
			else
				x1 += diff1 - diff0;

			iInvalid |= Xsym;
		}
	}

	roi_fixed.setTopLeft(Point(x0-1, y0-1));
	roi_fixed.setSize(Size(x1 -x0+1, y1-y0+1));

	if(_getDebug(DBG_ROI) || iInvalid) {
		DEB_ALWAYS()  
			<< "\nREQUESTED roiX " << DEB_VAR4(x0org, x1org, xSteps, xMax)   
			<< "\nREQUESTED roiY " << DEB_VAR4(y0org, y1org, ySteps, yMax) 
			<< "\n     FIXED roi " << DEB_VAR4(x0, x1, y0, y1)
			<< "\n       STATUS  " << DEB_VAR3(iInvalid, bSymX, bSymY);
	}

	return iInvalid ;

}


//=================================================================================================
//=================================================================================================
void Camera::_set_Roi(const Roi &new_roi, const Roi &requested_roi, int &error){
	
	Size roi_size;
	Roi fixed_roi;
	DEB_MEMBER_FUNCT();
	//DEF_FNID;

	if(_checkValidRoi(new_roi, fixed_roi)){
		error = -1;
		return;
	}

	    // pco roi [1,max] ---- lima Roi [0, max-1]


		m_RoiLima = new_roi;
		m_RoiLimaRequested = requested_roi;

	if(_getDebug(DBG_ROI)) {
		DEB_ALWAYS() << DEB_VAR1(m_RoiLima);
	}	
		
	error = 0;
	return ;
}

//=================================================================================================
//=================================================================================================


void Camera::_get_Roi(Roi &roi){
		
	DEB_MEMBER_FUNCT();
	//DEF_FNID;

	roi = m_RoiLima;


	if(_getDebug(DBG_ROI)) {
		DEB_ALWAYS() << DEB_VAR1(m_RoiLima);
	}	
}

void Camera::_get_Roi(unsigned int &x0, unsigned int &x1, unsigned int &y0, unsigned int &y1){
		
	DEB_MEMBER_FUNCT();
	//DEF_FNID;

	Point top_left = m_RoiLima.getTopLeft();
	UNUSED Point bot_right = m_RoiLima.getBottomRight();
	UNUSED Size size = m_RoiLima.getSize();

	x0 = top_left.x + 1;
	y0 = top_left.y + 1;
	x1 = bot_right.x + 1; 
	y1 = bot_right.y + 1;

	if(_getDebug(DBG_ROI)) {
		DEB_ALWAYS() << DEB_VAR5(m_RoiLima, x0, x1, y0, y1);
	}	
}

void Camera::_get_MaxRoi(Roi &roi){
		
	DEB_MEMBER_FUNCT();
	//DEF_FNID;

	unsigned int xMax, yMax;

	getMaxWidthHeight(xMax, yMax);

	roi.setTopLeft(Point(0, 0));
	roi.setSize(Size(xMax, yMax));
}


//=========================================================================================================
//=========================================================================================================
void Camera::_get_RoiSize(Size& roi_size)
{

	roi_size = m_RoiLima.getSize();
}

//=========================================================================================================
//=========================================================================================================
void Camera::_get_ImageType(ImageType& image_type)
{
  unsigned int pixbytes;
  getBytesPerPixel(pixbytes);
  image_type = (pixbytes == 2) ? Bpp16 : Bpp8;
}

//=================================================================================================
//=================================================================================================

// 31/10/2013 PCO Support Team <support@pco.de>
// Pixelsize is not implemented in the complete SW- and HW-stack.

void Camera::_get_PixelSize(double& x_size,double &y_size)
{  

	// pixel size in micrometer 

	if( _isCameraType(Pco2k)) {
		x_size = y_size = 7.4;	// um / BR_pco_2000_105.pdf	
		return;
	}

	if( _isCameraType(Pco4k)) {
		x_size = y_size = 9.0;	// um / BR_pco_4000_105.pdf	
		return;
	}

	if( _isCameraType(Edge)) {
		x_size = y_size = 6.5;	// um / pco.edge User Manual V1.01, page 34	
		return;
	}

	if( _isCameraType(Dimax)) {
		x_size = y_size = 11;	// um / pco.dimax User\92s Manual V1.01	
		return;
	}

	x_size = y_size = -1.;		

}

//=================================================================================================
//=================================================================================================
void Camera::_set_ImageType(ImageType curr_image_type)
{
    // ---- DONE
	// only check if it valid, BUT don't set it ????
  switch(curr_image_type)
    {
    case Bpp16:
    case Bpp8:
      break;

    default:
      throw LIMA_HW_EXC(InvalidValue,"This image type is not Managed");
    }

}
//=========================================================================================================
//=========================================================================================================
void Camera::_get_DetectorType(std::string& det_type)
{
    // ---- DONE
   det_type = "Pco";
}

//=========================================================================================================
//=========================================================================================================
void Camera::_get_MaxImageSize(Size& max_image_size)
{

  // ---- DONE
  DWORD width,height;

  getMaxWidthHeight(width,height);
  max_image_size = Size(int(width),int(height));

}

//=================================================================================================
//=================================================================================================
bool Camera::_isCameraType(int tp)
{
	DEB_MEMBER_FUNCT();

	switch(_getCameraType()) 
	{
		case CAMERATYPE_PCO_DIMAX_STD: 
			return !!(tp & Dimax) ;
		
		case CAMERATYPE_PCO_EDGE_GL:
			return !!(tp & (EdgeGL | Edge));

		case CAMERATYPE_PCO_EDGE:
			return !!(tp & (EdgeRolling | Edge));

		case CAMERATYPE_PCO2000:
			return !!(tp & Pco2k) ;

		case CAMERATYPE_PCO4000:
			return !!(tp & Pco4k) ;

		default:
			return FALSE;
	}
}

//=================================================================================================
//=================================================================================================
bool Camera::_isInterfaceType(int tp)
{
	DEB_MEMBER_FUNCT();

	switch(_getInterfaceType()) 
	{
		case INTERFACE_FIREWIRE: 
			return !!(tp & Fw) ;
		
		case INTERFACE_CAMERALINK:
			return !!(tp & Cl);

		case INTERFACE_USB:
			return !!(tp & Usb);

		case INTERFACE_ETHERNET:
			return !!(tp & Eth) ;

		case INTERFACE_SERIAL:
			return !!(tp & Serial) ;

		case INTERFACE_USB3:
			return !!(tp & Usb3);

		case INTERFACE_CAMERALINKHS:
			return !!(tp & ClHs);

		case INTERFACE_COAXPRESS:
			return !!(tp & Coaxpress);

		default:
			return FALSE;
    }
}

//=================================================================================================
//=================================================================================================
void Camera::_get_XYsteps(Point &xy_steps)
{
	DEB_MEMBER_FUNCT();

		unsigned int xSteps, ySteps;

		getXYsteps(xSteps, ySteps);

		xy_steps.x = xSteps;
		xy_steps.y = ySteps;
}

//=================================================================================================
//=================================================================================================
void Camera::_presetPixelRate(DWORD &pixRate, int &error)
{
	DEB_MEMBER_FUNCT();
		if(!_isCameraType(Edge) || !_isValid_pixelRate(pixRate)) {
			pixRate = 0;
			error = -1;
			return;
		}

		m_pcoData->dwPixelRateRequested = pixRate;
		error = 0;
}


//=================================================================================================
//=================================================================================================
void Camera::msgLog(const char *s) 
{
	m_msgLog->add(s); 
}




//=================================================================================================
//=================================================================================================

//=================================================================================================
//=================================================================================================
DWORD Camera::_getCameraSerialNumber()
{
	DEB_MEMBER_FUNCT();
	return m_pcoData->stcPcoCamType.dwSerialNumber;
}

WORD Camera::_getInterfaceType()
{
	DEB_MEMBER_FUNCT();
	return m_pcoData->stcPcoCamType.wInterfaceType;
}

const char *Camera::_getInterfaceTypeStr()
{
	DEB_MEMBER_FUNCT();
	return m_pcoData->iface;
}



//=================================================================================================
//=================================================================================================
WORD Camera::_getCameraType()
{
	DEB_MEMBER_FUNCT();
	return m_pcoData->stcPcoCamType.wCamType;
}

const char *Camera::_getCameraTypeStr()
{
	DEB_MEMBER_FUNCT();
	return m_pcoData->model;
}



//=================================================================================================
//=================================================================================================
WORD Camera::_getCameraSubType()
{
	DEB_MEMBER_FUNCT();
	return m_pcoData->stcPcoCamType.wCamSubType;
}

const char *Camera::_getCameraSubTypeStr()
{
	DEB_MEMBER_FUNCT();
	return m_pcoData->modelSubType;
}


//=================================================================================================
//=================================================================================================

bool Camera::_getCameraState(long long flag)
{
	return !!(m_state & flag);
}



//=================================================================================================
//=================================================================================================

void Camera::_setCameraState(long long flag, bool val)
{
	if(val) 
	{
		m_state |= flag;
	} 
	else
	{
		m_state |= flag;
		m_state ^= flag;
	}
	return;
}


//=================================================================================================
//=================================================================================================
void Camera::stopAcq()
{
  _stopAcq(true);
}
//=================================================================================================
void Camera::_stopAcq(bool waitForThread)
{
    DEB_MEMBER_FUNCT();

    DEB_ALWAYS() << "[entry]" << DEB_VAR1(waitForThread);

    AutoMutex aLock(m_cond.mutex());
    if(m_status != Camera::Ready)
    {
        while(waitForThread && m_thread_running)
        {
            m_wait_flag = true;
            //WaitObject_.Signal();
            m_cond.wait();
        }
        aLock.unlock();

        //Let the acq thread stop the acquisition
        if(waitForThread) 
        {
              DEB_ALWAYS() << "[return]" << DEB_VAR1(waitForThread);
             return;
        }

        // Stop acquisition
        DEB_TRACE() << "Stop acquisition";
        //Camera_->AcquisitionStop.Execute();

          DEB_ALWAYS() << "[set Ready]" << DEB_VAR1(waitForThread);
         _setStatus(Camera::Ready,false);
    }
}
//=================================================================================================
//=================================================================================================

enum traceAcqId {traceAcq_execTimeTot, };

//=================================================================================================
//=================================================================================================
//---------------------------
//- Camera::_AcqThread::threadFunction()
//---------------------------
void Camera::_AcqThread::threadFunction()
{
    DEB_MEMBER_FUNCT();
    DEF_FNID;

	TIME_USEC tStart;
	TIME_UTICKS usStart, usStartTot;

    m_cam.traceAcq.fnId = fnId;
    DEB_ALWAYS() << "[entry]" ;

    int err;
    int pcoBuffIdx, pcoFrameNr, pcoFrameNrTimestamp;
    void *limaBuffPtr;
    void *pcoBuffPtr;
    DWORD width, height;

    AutoMutex aLock(m_cam.m_cond.mutex());
    //StdBufferCbMgr& buffer_mgr = m_cam.m_buffer_ctrl_obj.getBuffer();

    //StdBufferCbMgr& buffer_mgr = m_buffer->getBuffer();

    int nb_allocated_buffers;

    int _nb_frames, limaFrameNr;
    
    limaFrameNr = 0;
    if(m_cam.m_sync) 
    {
        m_cam.m_sync->getNbFrames(_nb_frames);
    }
    else
    {
        _nb_frames = -1;
    }
    
     DEB_ALWAYS() << DEB_VAR3(m_cam.m_wait_flag, m_cam.m_quit, _nb_frames);

    while(!m_cam.m_quit)
    {
        while(m_cam.m_wait_flag && !m_cam.m_quit)
        {
          DEB_ALWAYS() << "++++++++++++++++++++++++++++++++++Wait";
          m_cam.m_thread_running = false;
          m_cam.m_cond.broadcast();
          m_cam.m_cond.wait();
        } // while wait
        
        DEB_ALWAYS() << "++++++++++++++++++++++++++++++++++Run";
        m_cam.m_thread_running = true;
        if(m_cam.m_quit) return;

        Camera::Status _statusReturn = Camera::Ready;
        bool continueAcq = true;

        
    	m_cam.traceAcq.usTicks[traceAcq_execTimeTot].desc = "total execTime";
    	m_cam.traceAcq.usTicks[traceAcq_Lima].desc = "Lima execTime";
    	m_cam.traceAcq.usTicks[traceAcq_pcoSdk].desc = "SDK execTime";

    	msElapsedTimeSet(tStart);
    	usElapsedTimeSet(usStart);
    	usElapsedTimeSet(usStartTot);
        m_cam.traceAcq.fnId = fnId;
        m_cam.traceAcq.fnTimestampEntry = getTimestamp();
	

    	m_cam.traceAcq.msStartAcqStart = msElapsedTime(tStart);
        
    	//m_cam.traceAcq.usTicks[traceAcq_Lima].desc = "xfer to lima / total execTime";

        m_cam.m_sync->setStarted(true);        

        m_cam.m_sync->getNbFrames(_nb_frames);
        limaFrameNr = 0;            // 0 ..... N-1
    
    	m_cam.traceAcq.nrImgRequested = _nb_frames;

        m_cam.m_status = Camera::Exposure;
        m_cam.m_cond.broadcast();
        aLock.unlock();
        
        
        err = m_cam.grabber->Get_actual_size(&width,&height,NULL);
        PCO_CHECK_ERROR1(err, "Get_actual_size");
        if(err)  m_cam.traceAcq.nrErrors++;

        DEB_ALWAYS() << DEB_VAR3(width, height, _nb_frames);

        pcoBuffIdx=1;

        bool acquireFirst = m_cam._isCameraType(Edge) && m_cam._isInterfaceType(Cl);

        if(acquireFirst)
        {
            err = m_cam.grabber->Start_Acquire(_nb_frames);
            PCO_CHECK_ERROR1(err, "Start_Acquire");
            if(err)  m_cam.traceAcq.nrErrors++;

            m_cam._pco_SetRecordingState(1, err);
            PCO_CHECK_ERROR1(err, "SetRecordingState(1)");
            if(err)  m_cam.traceAcq.nrErrors++;
        }
        else
        {
            m_cam._pco_SetRecordingState(1, err);
            PCO_CHECK_ERROR1(err, "SetRecordingState(1)");
            if(err)  m_cam.traceAcq.nrErrors++;

            err = m_cam.grabber->Start_Acquire(_nb_frames);
            PCO_CHECK_ERROR1(err, "Start_Acquire");
            if(err)  m_cam.traceAcq.nrErrors++;
        }
        

        if(err!=PCO_NOERROR) 
        {
            m_cam.traceAcq.nrErrors++;
            //m_cam._setStatus(Camera::Fault,false);        
            _statusReturn = Camera::Fault;
            continueAcq = false;        
            aLock.lock();
            m_cam.m_wait_flag = true;
        }       
        
        while(  !m_cam.m_wait_flag && 
                continueAcq && 
                ((_nb_frames == 0) || limaFrameNr < _nb_frames))
        {

            m_cam.traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
       		usElapsedTimeSet(usStart);

            pcoFrameNr = limaFrameNr +1;
            limaBuffPtr =  m_cam.m_buffer->_getFrameBufferPtr(limaFrameNr, nb_allocated_buffers);
           //B_ALWAYS() << DEB_VAR4(nb_allocated_buffers, _nb_frames, limaFrameNr, limaBuffPtr);

            m_cam.traceAcq.usTicks[traceAcq_GetImageEx].value += usElapsedTime(usStart);
            		usElapsedTimeSet(usStart);

            m_cam._setStatus(Camera::Readout,false);

            m_cam.traceAcq.usTicks[traceAcq_Lima].value += usElapsedTime(usStart);
       		usElapsedTimeSet(usStart);

    		usElapsedTimeSet(usStart);
            err=m_cam.grabber->Wait_For_Next_Image(&pcoBuffIdx,10);
            PCO_CHECK_ERROR1(err, "Wait_For_Next_Image");
            if(err!=PCO_NOERROR)
            {
                m_cam.traceAcq.nrErrors++;
                printf("\ngrab_loop Error while waiting for image number %d",pcoFrameNr);
            }
            
            if(err==PCO_NOERROR)
            {
                err=m_cam.grabber->Check_DMA_Length(pcoBuffIdx);
                PCO_CHECK_ERROR1(err, "Check_DMA_Length");
                if(err!=PCO_NOERROR)
                {
                    m_cam.traceAcq.nrErrors++;
                    printf("\ngrab_loop Check_DMA_Length error 0x%x",err);
                }
            }

            if(err!=PCO_NOERROR)
            {
                printf("\ngrab_loop Error break loop at image number %d",pcoFrameNr);
                _statusReturn = Camera::Fault;
                //m_cam._setStatus(Camera::Fault,false);
                continueAcq = false;        
                aLock.lock();
                m_cam.m_wait_flag = true;
                goto whileend;
            }
            
            //DEB_ALWAYS()  << "lima image#  " << DEB_VAR1(limaFrameNr) <<" acquired !";

            err=m_cam.grabber->Get_Framebuffer_adr(pcoBuffIdx,&pcoBuffPtr);
            PCO_CHECK_ERROR1(err, "Get_Framebuffer_adr");
            if(err!=PCO_NOERROR)
            {
                m_cam.traceAcq.nrErrors++;
                printf("\ngrab_loop Get_Framebuffer_adr(%d,) error",pcoBuffIdx);
            }
            if(err==PCO_NOERROR)
            {
                m_cam.grabber->Extract_Image(limaBuffPtr,pcoBuffPtr,width,height);
                pcoFrameNrTimestamp=image_nr_from_timestamp(limaBuffPtr,0);
                
	            m_cam.traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
        		usElapsedTimeSet(usStart);

                m_cam.traceAcq.checkImgNrPcoTimestamp = pcoFrameNrTimestamp;
                m_cam.traceAcq.checkImgNrPco = pcoFrameNr;
                m_cam.traceAcq.checkImgNrLima = limaFrameNr;
                m_cam.traceAcq.checkImgNrLima = limaFrameNr;
                m_cam.traceAcq.msStartAcqNow = msElapsedTime(tStart);


                HwFrameInfoType frame_info;
	            frame_info.acq_frame_nb = limaFrameNr;
	            continueAcq = m_cam.m_buffer->m_buffer_cb_mgr.newFrameReady(frame_info);

	            m_cam.traceAcq.usTicks[traceAcq_Lima].value += usElapsedTime(usStart);
        		usElapsedTimeSet(usStart);
            }

            err=m_cam.grabber->Unblock_buffer(pcoBuffIdx); 
            PCO_CHECK_ERROR1(err, "Unblock_buffer");
            if(err!=PCO_NOERROR)
            {
                m_cam.traceAcq.nrErrors++;
                printf("\ngrab_loop Unblock_buffer error 0x%x\n",err);
            }
            m_cam.traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
       		usElapsedTimeSet(usStart);

            if((limaFrameNr % 100) == 0)
                printf("pcoFrameNr [%d] diff[%d]\r",pcoFrameNr,pcoFrameNrTimestamp-pcoFrameNr);
            //    printf("\n");

            ++limaFrameNr;

            if(0) // TODO - FAULT
            {
                //m_cam._setStatus(Camera::Fault,false);
                _statusReturn = Camera::Fault;
                continueAcq = false;
                m_cam.m_wait_flag = true;
            }
whileend:;
        } // while  nb_frames, continue, wait

        m_cam._stopAcq(false);

        printf("\n");
        
    	m_cam.traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
        usElapsedTimeSet(usStart);
        m_cam._pco_SetRecordingState(0, err);
        PCO_CHECK_ERROR1(err, "SetRecordingState(0)");
        if(err) m_cam.traceAcq.nrErrors++;
 
        err=m_cam.grabber->Stop_Acquire(); 
        PCO_CHECK_ERROR1(err, "Stop_Acquire");
        if(err!=PCO_NOERROR)
        {
            m_cam.traceAcq.nrErrors++;
            printf("\ngrab_loop Stop_Acquire error \n");
        }

        err=m_cam.grabber->Free_Framebuffer(); 
        PCO_CHECK_ERROR1(err, "Free_Framebuffer");
        if(err!=PCO_NOERROR)
        {
            m_cam.traceAcq.nrErrors++;
            printf("\ngrab_loop Free_Framebuffer error \n");
        }
    	m_cam.traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
        usElapsedTimeSet(usStart);

        m_cam.traceAcq.fnTimestampExit = getTimestamp();
        m_cam.traceAcq.msStartAcqEnd = msElapsedTime(tStart);
        m_cam.traceAcq.usTicks[traceAcq_execTimeTot].value = usElapsedTime(usStartTot);      
        m_cam._setStatus(_statusReturn,false);        

        aLock.lock();
        m_cam.m_wait_flag = true;
        m_cam.m_sync->setStarted(false);        


    } // while quit
    DEB_ALWAYS() << "[exit]" ;
}

//=================================================================================================
//=================================================================================================
Camera::_AcqThread::_AcqThread(Camera &aCam) :
                    m_cam(aCam)
{
	DEB_CONSTRUCTOR();
	DEB_ALWAYS() << "[entry]" ;
    pthread_attr_setscope(&m_thread_attr,PTHREAD_SCOPE_PROCESS);
	DEB_ALWAYS() << "[exit]" ;
}
//=================================================================================================
//=================================================================================================

Camera::_AcqThread::~_AcqThread()
{
	DEB_DESTRUCTOR();
	DEB_ALWAYS() << "[entry]" ;

    AutoMutex aLock(m_cam.m_cond.mutex());
    m_cam.m_quit = true;
    //m_cam.WaitObject_.Signal();
    m_cam.m_cond.broadcast();
    aLock.unlock();
    
    join();
	DEB_ALWAYS() << "[exit]" ;
}

//=================================================================================================
//=================================================================================================
void Camera::getStatus(Camera::Status& status)
{
    DEB_MEMBER_FUNCT();
    AutoMutex aLock(m_cond.mutex());
    status = m_status;
    DEB_RETURN() << DEB_VAR1(DEB_HEX(status));
}

//=================================================================================================
//=================================================================================================
void Camera::_setStatus(Camera::Status status,bool force)
{
    DEB_MEMBER_FUNCT();
    AutoMutex aLock(m_cond.mutex());
    if(force || m_status != Camera::Fault)
        m_status = status;
    m_cond.broadcast();
    aLock.unlock();
}
