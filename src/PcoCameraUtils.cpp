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

#include "Pco.h"
#include "PcoCameraUtils.h"
#include "PcoCamera.h"
#include "PcoSyncCtrlObj.h"
#include "PcoBufferCtrlObj.h"

using namespace lima;
using namespace lima::Pco;


#define BUFF_INFO_SIZE 5000


void print_hex_dump_buff(void *ptr_buff, size_t len);
int __xlat_date(char *s1, char &ptrTo, int lenTo) ;
char *_xlat_date(char *s1, char *s2, char *s3) ;
	
//=========================================================================================================
const char* _timestamp_pcocamerautils() {return ID_TIMESTAMP_M ;}
//=========================================================================================================

//=========================================================================================================
// dummy comments for test 02ccc
//=========================================================================================================


//=========================================================================================================
//=========================================================================================================
char *getTimestamp(timestampFmt fmtIdx, time_t xtime) {
   static char timeline[128];
   //errno_t err;
	time_t ltime;
	struct tm today;
	const char *fmt;

  switch(fmtIdx) {
    case Iso: fmt = "%Y/%m/%d %H:%M:%S"; break;
    case IsoHMS: fmt = "%H:%M:%S"; break;
    case FnDate: fmt = "%Y-%m-%d"; break;

    default:
    case FnFull: fmt = "%Y-%m-%d-%H%M%S"; break;
  }

	if(xtime == 0) 
		time( &ltime );
	else
		ltime = xtime;
	//err = localtime_s( &today, &ltime );
	localtime_s( &today, &ltime );
	strftime(timeline, 128, fmt, &today );
      
	return timeline;
}

time_t getTimestamp() { return time(NULL); }

//====================================================================
//====================================================================
//$Id: [Oct  8 2013 15:21:07] [Tue Oct  8 15:21:07 2013] (..\..\..\..\src\PcoCamera.cpp) $

#define LEN_BUFF_DATE 128
#define TOKNR_DT 5


int __xlat_date(char *s1, char &ptrTo, int lenTo) {
	const char *tok[TOKNR_DT];
	char *tokNext = NULL;
	int tokNr, iM, iD, iY, i;
	const char *ptr;
	const char *months = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec";
	const char *sM;
	const char *sT;
	char buff[LEN_BUFF_DATE+1];

	strcpy_s(buff, LEN_BUFF_DATE, s1);
	ptr = buff;

	for(tokNr = i = 0; i < TOKNR_DT; i++) {
		if( (tok[i] = strtok_s((char *)ptr, " ", &tokNext)) == NULL) break;
		ptr = NULL;
		tokNr = i+1;
	}

	if(tokNr == 4) {
		// Oct  8 2013 15:21:07
		sM = tok[0]; iD = atoi(tok[1]); iY = atoi(tok[2]); sT = tok[3]; 
	} else if(tokNr == 5) {
		// Tue Oct  8 15:21:07 2013
		sM = tok[1]; iD = atoi(tok[2]); iY = atoi(tok[4]); sT = tok[3]; 
	} else {
		sM = "xxx"; iD = 99; iY = 9999; sT = "99:99:99"; 
	}
	
	ptr = strstr(months,sM);
	iM = (ptr != NULL) ? ( int(ptr - months) / 4) + 1 : 99;


	return sprintf_s(&ptrTo, lenTo, "%04d/%02d/%02d %s", iY, iM, iD, sT);
}

char *_xlat_date(char *s1, char *s2, char *s3) {
	static char buff[LEN_BUFF_DATE+1];
	char *ptr = buff;
	char *ptrMax = buff + LEN_BUFF_DATE;

	ptr += sprintf_s(ptr, ptrMax - ptr,  "$Id: comp[");
	ptr += __xlat_date(s1, *ptr, (int) (ptrMax - ptr));
	ptr += sprintf_s(ptr, ptrMax - ptr,  "] file[");
	ptr += __xlat_date(s2, *ptr, (int) (ptrMax - ptr));
	ptr += sprintf_s(ptr, ptrMax - ptr, "] [%s] $", s3);
	return buff;
	
}
char *_split_date(const char *_s) {
	static char s1[LEN_BUFF_DATE+1];
	static char s2[LEN_BUFF_DATE+1];
	static char s3[LEN_BUFF_DATE+1];
	char s[3*LEN_BUFF_DATE+1];
	char *ptr1, *ptr2;
	strcpy_s(s, 3*LEN_BUFF_DATE, _s);
	*s1 = *s2 = *s3 = 0;

	ptr1 = strchr(s,'[');
	if(ptr1) {
		ptr2 = strchr(ptr1,']');
		if(ptr2) {
			*ptr2 = 0;
			strcpy_s(s1, LEN_BUFF_DATE, ptr1+1);
		}
	}

	ptr1 = strchr(ptr2+1,'[');
	if(ptr1) {
		ptr2 = strchr(ptr1,']');
		if(ptr2) {
			*ptr2 = 0;
			strcpy_s(s2, LEN_BUFF_DATE, ptr1+1);
		}
	}

	ptr1 = strchr(ptr2+1,'[');
	if(ptr1) {
		ptr2 = strchr(ptr1,']');
		if(ptr2) {
			*ptr2 = 0;
			strcpy_s(s3, LEN_BUFF_DATE, ptr1+1);
		}
	}

	return _xlat_date(s1, s2, s3);
}

//====================================================================
//====================================================================

char *str_trim_left(char *s) {
	char c;
	if(s == NULL) return NULL;
	while((c = *s) != 0) {
		if(!isspace(c)) break;
		s++;		
	}
	return s;
}

char *str_trim_right(char *s) {
	char *ptr;
	if(s == NULL) return NULL;
	ptr = s + strlen(s) - 1;
	while(s <= ptr) {
		if(!isspace(*ptr)) break;
		*ptr-- = 0;
	}
	return s;
}
char *str_trim(char *s) {
	return str_trim_left(str_trim_right(s));
}

char *str_toupper(char *s) {
	char *ptr = s;
	while(*ptr) { 
		*ptr = toupper(*ptr);
		ptr++;
	}
	return s;
}

char *str_printable(char *s) {
	char *ptr = s;
	char c;
	while(*ptr) {
	    c = *ptr;
	    if((c <= 0x20) || (c >= 0x7f))  
		    *ptr = 0x20;
		ptr++;
	}
	return s;
}

//=========================================================================================================
//=========================================================================================================

void Camera::stcTraceAcq::traceAcqClean(){
	void *ptr = this;
	memset(ptr, 0, sizeof(struct stcTraceAcq));
}

void Camera::stcTraceAcq::traceMsg(char *s){
	char *ptr = msg;
	char *dt = getTimestamp(IsoHMS);
	size_t lg = strlen(ptr);
	ptr += lg;
	snprintf(ptr, LEN_TRACEACQ_MSG - lg,"%s> %s", dt, s);
}

char *Camera::talk(char *cmd){
	//static char buff[BUFF_INFO_SIZE +1];
	sprintf_s(mytalk, mytalkmax - mytalk, "talk> %s", cmd);
	m_msgLog->add(mytalk);
	return _talk(cmd, mytalk, mytalkmax - mytalk);
}

#define NRTOK 5
#define NRCMDS 50
char *Camera::_talk(char *_cmd, char *output, int lg){
	DEB_MEMBER_FUNCT();
		char cmdBuff[BUFF_INFO_SIZE +1];
		char cmdBuffAux[BUFF_INFO_SIZE +1];
		const char *cmd;
		const char *keys[NRCMDS], *keys_desc[NRCMDS];
		const char *key;
		int ikey = 0;
		const char *tok[NRTOK];
		int tokNr;
		char *ptr, *ptrMax;
		int segmentPco = m_pcoData->wActiveRamSegment;
		int segmentArr = segmentPco -1;
		int error;
		
		ptr = output; *ptr = 0;
		ptrMax = ptr + lg;

		//int width = +20;
		
		strcpy_s(cmdBuff, BUFF_INFO_SIZE, _cmd);
		cmd = str_trim(cmdBuff);
		strcpy_s(cmdBuffAux, BUFF_INFO_SIZE, cmd);

		if(*cmd){
			char *tokContext;
			for(int i=0; i < NRTOK; i++) {
				if( (tok[i] = strtok_s((char *)cmd, " ", &tokContext)) == NULL) break;
				cmd = NULL;
				tokNr = i;
			}
			cmd = tok[0];
		}

		if(*cmd == 0) {
			Roi limaRoi;
			unsigned int x0,x1,y0,y1;
			_get_Roi(x0, x1, y0, y1);
			_get_Roi(limaRoi);

			ptr += sprintf_s(ptr, ptrMax - ptr,"**** %s [begin]\n", __FUNCTION__);

			ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", "* --- PCO info ---");
			ptr += sprintf_s(ptr, ptrMax - ptr, "* timestamp[%s]\n", getTimestamp(Iso));
			ptr += sprintf_s(ptr, ptrMax - ptr, "* cam_name[%s]\n", m_pcoData->camera_name);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* roi X(%d,%d) Y(%d,%d) size(%d,%d)\n",  
					x0, x1, y0, y1, x1-x0+1, y1-y0+1);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* roiLima XY0(%d,%d) XY1(%d,%d) size(%d,%d)\n",  
					limaRoi.getTopLeft().x, limaRoi.getTopLeft().y,
					limaRoi.getBottomRight().x, limaRoi.getBottomRight().y,
					limaRoi.getSize().getWidth(), limaRoi.getSize().getHeight());

			ptr += sprintf_s(ptr, ptrMax - ptr, "* cocRunTime[%g] (s) frameRate[%g] (fps)\n",  
				pcoGetCocRunTime(), pcoGetFrameRate());
			

			double _exposure, _delay;
			struct lima::HwSyncCtrlObj::ValidRangesType valid_ranges;
			m_sync->getValidRanges(valid_ranges);
			m_sync->getExpTime(_exposure);
			m_sync->getLatTime(_delay);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* exp[%g s] min[%g us] max[%g ms] step[%g us]\n", 
				_exposure, 
				m_pcoData->stcPcoDesc1.dwMinExposureDESC * 1.0e-3,
				m_pcoData->stcPcoDesc1.dwMaxExposureDESC * 1.0,
				m_pcoData->stcPcoDesc1.dwMinExposureStepDESC * 1.0e-3  );

			ptr += sprintf_s(ptr, ptrMax - ptr, "* valid exp min[%g us] max[%g ms]\n", 
				valid_ranges.min_exp_time * 1.0e6, valid_ranges.max_exp_time * 1.0e3);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* delay[%g s] min[%g us] max[%g ms] step[%g us]\n", 
				_delay, 
				m_pcoData->stcPcoDesc1.dwMinDelayDESC * 1.0e-3,
				m_pcoData->stcPcoDesc1.dwMaxDelayDESC * 1.0,

				m_pcoData->stcPcoDesc1.dwMinDelayStepDESC * 1.0e-3  );
			ptr += sprintf_s(ptr, ptrMax - ptr, "* valid delay min[%g us] max[%g ms]\n", 
				valid_ranges.min_lat_time * 1.0e6, valid_ranges.max_lat_time * 1.0e3);

			WORD wXResActual, wYResActual, wXResMax, wYResMax;
			_pco_GetSizes( &wXResActual, &wYResActual, &wXResMax, &wYResMax, error);
			
			ptr += sprintf_s(ptr, ptrMax - ptr, "* wXResActual=[%d] wYResActual=[%d] \n",  wXResActual,  wYResActual);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* wXResMax=[%d] wYResMax=[%d] \n",  wXResMax,  wYResMax);
			ptr += sprintf_s(ptr, ptrMax - ptr, 
			    "* bMetaDataAllowed=[%d] wMetaDataMode=[%d] wMetaDataSize=[%d] wMetaDataVersion=[%d] \n",  
				m_pcoData->bMetaDataAllowed, m_pcoData->wMetaDataMode, m_pcoData->wMetaDataSize,  m_pcoData->wMetaDataVersion);


			unsigned int maxWidth, maxHeight,Xstep, Ystep; 
			getMaxWidthHeight(maxWidth, maxHeight);
			getXYsteps(Xstep, Ystep);
			WORD bitsPerPix; getBitsPerPixel(bitsPerPix);
			unsigned int bytesPerPix; getBytesPerPixel(bytesPerPix);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* maxWidth=[%d] maxHeight=[%d] \n",  maxWidth,  maxHeight);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* Xstep=[%d] Ystep=[%d] \n",  Xstep,  Ystep);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* bitsPerPix=[%d] bytesPerPix=[%d] \n",  bitsPerPix,  bytesPerPix);
			



			ptr += sprintf_s(ptr, ptrMax - ptr, "* dwPixelRate=[%d](%g MHz)\n",  
				m_pcoData->dwPixelRate, m_pcoData->dwPixelRate/1000000.);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* dwPixelRateRequested=[%d](%g MHz) \n",  
				m_pcoData->dwPixelRateRequested, m_pcoData->dwPixelRateRequested/1000000.);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* Valid dwPixelRate=[%d][%d][%d][%d] \n",  
				m_pcoData->stcPcoDesc1.dwPixelRateDESC[0],m_pcoData->stcPcoDesc1.dwPixelRateDESC[1],
				m_pcoData->stcPcoDesc1.dwPixelRateDESC[2],m_pcoData->stcPcoDesc1.dwPixelRateDESC[3]);

			double pixSizeX, pixSizeY;
			_get_PixelSize(pixSizeX, pixSizeY);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* pixelSize (um) [%g,%g] \n",  pixSizeX, pixSizeY);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* wLUT_Identifier[x%04x] wLUT_Parameter [x%04x]\n",
				m_pcoData->wLUT_Identifier, m_pcoData->wLUT_Parameter);

			int iFrames;
			m_sync->getNbFrames(iFrames);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* m_sync->getNbFrames=[%d frames]\n", iFrames);

            unsigned int pixbytes; getBytesPerPixel(pixbytes);
			
			ptr += sprintf_s(ptr, ptrMax - ptr, "* pixBits[%d] pixBytes[%d]\n",  
				bitsPerPix,pixbytes);

			if(_isCameraType(Dimax | Pco2k)){
				
				ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", "* --- DIMAX info ---");
				ptr += sprintf_s(ptr, ptrMax - ptr, "* pagesInRam[%d] pixPerPage[%d] bytesPerPix[%d] ramGB[%g]\n",  
					m_pcoData->dwRamSize, m_pcoData->wPixPerPage,pixbytes,
					(1.0e-9 * m_pcoData->dwRamSize) * m_pcoData->wPixPerPage * pixbytes);

				ptr += sprintf_s(ptr, ptrMax - ptr, "* PcoActiveSegment=[%d]\n", segmentArr+1);
				ptr += sprintf_s(ptr, ptrMax - ptr, "* m_pcoData->dwMaxFramesInSegment[%d]=[%u frames]\n", 
					segmentArr, m_pcoData->dwMaxFramesInSegment[segmentArr]);
				ptr += sprintf_s(ptr, ptrMax - ptr, "* m_pcoData->dwSegmentSize[%d]=[%u pages]\n", 
					segmentArr, m_pcoData->dwSegmentSize[segmentArr]);
				ptr += sprintf_s(ptr, ptrMax - ptr, "* m_pcoData->dwValidImageCnt[%d]=[%u]\n", 
                    segmentArr, m_pcoData->dwValidImageCnt[segmentArr]);
				ptr += sprintf_s(ptr, ptrMax - ptr, "* m_pcoData->dwMaxImageCnt[%d]=[%d]\n", segmentArr, m_pcoData->dwMaxImageCnt[segmentArr]);
				ptr += sprintf_s(ptr, ptrMax - ptr, "* storage_mode[%d] recorder_submode[%d]\n", 
					m_pcoData->storage_mode, m_pcoData->recorder_submode);
				ptr += sprintf_s(ptr, ptrMax - ptr, 
					"* Acq: rec[%ld] xfer[%ld] recNow[%ld] recTout[%ld] (ms) [%s]\n",
					m_pcoData->msAcqRec, m_pcoData->msAcqXfer,  
					m_pcoData->msAcqTnow, m_pcoData->msAcqTout, 
					getTimestamp(Iso, m_pcoData->msAcqRecTimestamp));

			}

			ptr += sprintf_s(ptr, ptrMax - ptr,"**** %s [end]\n", __FUNCTION__);
			return output;
		}


		//----------------------------------------------------------------
		//----------------------------------------------------------------
		key = keys[ikey] = "prepareInfo";     
		keys_desc[ikey++] = "(R) exp, lat, trig, ...";
		if(_stricmp(cmd, key) == 0){
			double _exposure, _delay;

			m_sync->getExpTime(_exposure);
			m_sync->getLatTime(_delay);
			
			ptr += sprintf_s(ptr, ptrMax - ptr, "* exp[%g] delay[%g] (s)\n", _exposure, _delay);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* limaTrigMode[%s]\n",
			    traceAcq.sLimaTriggerMode);
			ptr += sprintf_s(ptr, ptrMax - ptr, "*    pcoTrigMode[%d][%s]\n",
			    traceAcq.iPcoTriggerMode, traceAcq.sPcoTriggerMode);
			ptr += sprintf_s(ptr, ptrMax - ptr, "*     pcoAcqMode[%d][%s]\n",
			    traceAcq.iPcoAcqMode, traceAcq.sPcoAcqMode);
			    
            ptr += _pco_roi_info(ptr, ptrMax - ptr, error);

			return output;
		}



		
		//----------------------------------------------------------------
		//----------------------------------------------------------------
		key = keys[ikey] = "expDelayTime";     
		keys_desc[ikey++] = "(R) exposure and delay time";
		if(_stricmp(cmd, key) == 0){
			double _exposure, _delay;

			m_sync->getExpTime(_exposure);
			m_sync->getLatTime(_delay);
			
			ptr += sprintf_s(ptr, ptrMax - ptr, "* exp[%g] delay[%g] (s)\n", _exposure, _delay);

			return output;
		}


		key = keys[ikey] = "timingInfo";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) timing information (exp, trig delay, ...)";
		if(_stricmp(cmd, key) == 0){
			double frameTime, expTime, sysDelay, sysJitter, trigDelay;

			_pco_GetImageTiming(frameTime, expTime, sysDelay, sysJitter, trigDelay );


			ptr += sprintf_s(ptr, ptrMax - ptr, "frameTime %g  ",  frameTime);
			ptr += sprintf_s(ptr, ptrMax - ptr, "expTime %g  ",  expTime);
			ptr += sprintf_s(ptr, ptrMax - ptr, "sysDelay %g  ",  sysDelay);
			ptr += sprintf_s(ptr, ptrMax - ptr, "sysJitter %g  ",  sysJitter);
			ptr += sprintf_s(ptr, ptrMax - ptr, "trigDelay %g  ",  trigDelay);
			ptr += sprintf_s(ptr, ptrMax - ptr, "%s\n", "");

			return output;
		}

		key = keys[ikey] = "cocRunTime";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) Camera Operation Code runtime covers the delay, exposure and readout time";
		if(_stricmp(cmd, key) == 0){
			ptr += sprintf_s(ptr, ptrMax - ptr, "%g",  pcoGetCocRunTime());
			return output;
		}
		key = keys[ikey] = "frameRate";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) max frame rate (calculated as 1/cocRunTime)";   
		if(_stricmp(cmd, key) == 0){
			ptr += sprintf_s(ptr, ptrMax - ptr, "%g", pcoGetFrameRate());
			return output;
		}

		key = keys[ikey] = "timestamp";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) timestamp of compiled modules";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			ptr += sprintf_s(ptr, ptrMax - ptr,  m_pcoData->version);
			return output;
		}


		key = keys[ikey] = "clTransferParam";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) CameraLink transfer parameters";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			ptr += sprintf_s(ptr, ptrMax - ptr, "      baudrate=[%u] %g Kbps\n", m_pcoData->clTransferParam.baudrate, m_pcoData->clTransferParam.baudrate/1000.);
			ptr += sprintf_s(ptr, ptrMax - ptr, "ClockFrequency=[%u] %g MHz\n", m_pcoData->clTransferParam.ClockFrequency, m_pcoData->clTransferParam.ClockFrequency/1000000.);
			ptr += sprintf_s(ptr, ptrMax - ptr, "        CCline=[%u]\n", m_pcoData->clTransferParam.CCline);
			ptr += sprintf_s(ptr, ptrMax - ptr, "    DataFormat=[x%x]\n", m_pcoData->clTransferParam.DataFormat);
			ptr += sprintf_s(ptr, ptrMax - ptr, "      Transmit=[%u]\n", m_pcoData->clTransferParam.Transmit);
			return output;
		}

		key = keys[ikey] = "maxNbImages";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) for DIMAX only / max number of images in the active ram segment";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			if(!_isCameraType(Dimax)) {
				ptr += sprintf_s(ptr, ptrMax - ptr, "%d", -1);
				return output;
			}

			ptr += sprintf_s(ptr, ptrMax - ptr, "%ld", pcoGetFramesMax(m_pcoData->wActiveRamSegment));
			return output;
		}

		//----------------------------------------------------------------
		key = keys[ikey] = "dumpRecordedImg";     
		keys_desc[ikey++] = "(R) for DIMAX only / TODO"; 
		if(_stricmp(cmd, key) == 0){
			int res, nrImages, error;

			res = dumpRecordedImages(nrImages, error);


			ptr += sprintf_s(ptr, ptrMax - ptr, "res[%d] nrImages[%d] error[0x%x] ", res, nrImages, error);
			return output;
		}

		key = keys[ikey] = "allocatedBuffer";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) TODO";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			unsigned int bytesPerPix; getBytesPerPixel(bytesPerPix);

			int sizeBytes = m_pcoData->wXResActual * m_pcoData->wYResActual * bytesPerPix;
			ptr += sprintf_s(ptr, ptrMax - ptr, "IMAGE info:\n"
			                                    "    X=[%d] Y=[%d] bytesPerPix=[%d] size=[%d B]\n",  
				m_pcoData->wXResActual,  m_pcoData->wYResActual, bytesPerPix, sizeBytes);
			
			ptr += sprintf_s(ptr, ptrMax - ptr, "PCO API allocated buffers:\n"
												"    allocated=[%s] nrBuff=[%d] size=[%d B][%g MB] imgPerBuff[%u]\n", 
				m_pcoData->bAllocatedBufferDone ? "TRUE" : "FALSE", 
				m_pcoData->iAllocatedBufferNumber, 
				m_pcoData->dwAllocatedBufferSize, m_pcoData->dwAllocatedBufferSize/1000000.,
				m_pcoData->dwAllocatedBufferSize/sizeBytes);

			ptr += sprintf_s(ptr, ptrMax - ptr, "LIMA allocated buffers: \n"
												"    nr of buffers=[%d] \n", 
				m_pcoData->iAllocatedBufferNumberLima);


			return output;
		}


		key = keys[ikey] = "timeDimax";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) for DIMAX only / acq time details (record and transfer time)";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){

			if(!(_isCameraType(Dimax | Pco2k | Pco4k))) {
				ptr += sprintf_s(ptr, ptrMax - ptr,  "* ERROR - only for DIMAX / 2K");
				return output;
			}

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* Acq: rec[%ld] xfer[%ld] recLoopTime[%ld] recLoopTout[%ld] acqAll[%ld] (ms) [%s]\n",
				m_pcoData->msAcqRec, m_pcoData->msAcqXfer,  
				m_pcoData->msAcqTnow, m_pcoData->msAcqTout, 
				m_pcoData->msAcqAll, 
				getTimestamp(Iso, m_pcoData->msAcqRecTimestamp));


			return output;
		}

		key = keys[ikey] = "lastImgRecorded";     //----------------------------------------------------------------
		keys_desc[ikey++] = "last image recorded";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			DWORD lastImgRecorded = traceAcq.nrImgRecorded;

			if(!(_isCameraType(Dimax | Pco2k | Pco4k))) {
				lastImgRecorded = 0;
			}

			ptr += sprintf_s(ptr, ptrMax - ptr, "%d\n", lastImgRecorded);
			return output;
		}

		//----------------------------------------------------------------
		key = keys[ikey] = "lastImgAcquired";
		keys_desc[ikey++] = "last image acquired";
		if(_stricmp(cmd, key) == 0){
			DWORD lastImgAcquired = traceAcq.nrImgAcquired;


			ptr += sprintf_s(ptr, ptrMax - ptr, "%d\n", lastImgAcquired);
			return output;
		}

		//----------------------------------------------------------------
		key = keys[ikey] = "traceAcq";     
		keys_desc[ikey++] = "(R) trace details (not all records are filled!)";
		if(_stricmp(cmd, key) == 0){
			time_t _timet;

			if(0 && !(_isCameraType(Dimax | Pco2k | Pco4k))) {
				ptr += sprintf_s(ptr, ptrMax - ptr,  "* ERROR - only for DIMAX / 2K");
				return output;
			}

#ifdef __linux__
			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* fnId[%s]\n",
				traceAcq.fnId);
#else
			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* fnId[%s] nrEvents[%d]\n"
				"* fnIdXfer[%s]\n",
				traceAcq.fnId,
				PCO_BUFFER_NREVENTS,
				traceAcq.fnIdXfer);
#endif
            
				
			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* ... entry[%s]\n",
				getTimestamp(Iso, traceAcq.fnTimestampEntry));

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* ...  exit[%s]\n",
				getTimestamp(Iso, traceAcq.fnTimestampExit));

            if(_isInterfaceType(Cl | ClHs)) 
            {
    			ptr += sprintf_s(ptr, ptrMax - ptr, 
    				"* clSettings[%s]\n",
    				m_pcoData->sClTransferParameterSettings);
            }

			Point top_left = m_RoiLima.getTopLeft();
			Point bot_right = m_RoiLima.getBottomRight();

			Size size = m_RoiLima.getSize();			

			unsigned int bytesPerPix; getBytesPerPixel(bytesPerPix);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* roiLima xy0[%d,%d] xy1[%d,%d] size[%d,%d]\n",  
					top_left.x, top_left.y,
					bot_right.x, bot_right.y,
					size.getWidth(), size.getHeight());

			long long imgSize = size.getWidth()* size.getHeight() * bytesPerPix;
			long long totSize = imgSize * traceAcq.nrImgRequested;
			double mbTotSize =  totSize/MBYTEF;
			//double mbNowSize =  imgSize * traceAcq.checkImgNrPco/(1024.*1024.);
			//double xferSpeed = traceAcq.msXfer > 0. ? mbTotSize / traceAcq.msXfer * 1000. : -1.;

			ptr += sprintf_s(ptr, ptrMax - ptr, "* imgSize[%lld]B [%g]MB totSize[%g]MB [%g]GB\n",  
					imgSize, imgSize/MBYTEF,
					 mbTotSize, mbTotSize/KBYTEF);

			double secTotTime = (traceAcq.msStartAcqEnd - traceAcq.msStartAcqStart)/1000.;
			double secNowTime = (traceAcq.msStartAcqNow - traceAcq.msStartAcqStart)/1000.;
			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* ... msStartAcq Start/now/end[%ld][%ld][%ld]ms Time now/tot[%g][%g]s\n",
				        traceAcq.msStartAcqStart, traceAcq.msStartAcqNow, traceAcq.msStartAcqEnd, 
				        secNowTime, secTotTime);
            
            double frameRateNow, speedNow, sizeNow;
            if(secNowTime > 0.) 
            {
                frameRateNow = traceAcq.checkImgNrPco/ secNowTime;
                sizeNow = imgSize * traceAcq.checkImgNrPco ;
                speedNow = sizeNow /secNowTime;
            }
            else
            {
                frameRateNow = speedNow = sizeNow = -1.;
            }
            
            
            if(secTotTime > 0.) 
            {
			    ptr += sprintf_s(ptr, ptrMax - ptr, 
				    "* ... tot speed[%g]MB/s actual/pco[%g][%g]fps\n",
				            mbTotSize / secTotTime, 
				            traceAcq.nrImgRequested/secTotTime,
				            pcoGetFrameRate());
            }
            
			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* nrImgRequested[%d] nrImgRequested0[%d] nrImgAcquired[%d] nrImgRecorded[%u] maxImgCount[%u]\n",
				traceAcq.nrImgRequested,
				traceAcq.nrImgRequested0,
				traceAcq.nrImgAcquired,
				traceAcq.nrImgRecorded,
				traceAcq.maxImgCount);

			
			for(int _i = 0; _i < LEN_TRACEACQ_TRHEAD; _i++){
				const char *desc = traceAcq.usTicks[_i].desc;
				if(desc != NULL) {
					ptr += sprintf_s(ptr, ptrMax - ptr, 
						"* ... usTicks[%d][%5.3f] (ms)   (%s)\n", 
						_i, traceAcq.usTicks[_i].value/1000.,
						desc);
			
				}
			}


			_timet = traceAcq.endRecordTimestamp;
			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* msRecordLoop[%ld] msRecord[%ld] endRecord[%s]\n",
				traceAcq.msRecordLoop,
				traceAcq.msRecord,
				_timet ? getTimestamp(Iso, _timet) : "");

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* msXfer[%ld] endXfer[%s]\n",
				traceAcq.msXfer,
				getTimestamp(Iso, traceAcq.endXferTimestamp));

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* msTout[%ld] msTotal[%ld]\n",
				traceAcq.msTout,
				traceAcq.msTotal);

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* ... cocRuntime[%g]ms frameRate(fps) pco[%g] now[%g]\n",
                    traceAcq.msImgCoc,
                    traceAcq.msImgCoc > 0. ? 1000. / traceAcq.msImgCoc : -1.,
                    frameRateNow
				    );

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* ... sizeNow[%g]GB speedNow[%g]MB/s\n",
                    sizeNow/GBYTEF,
				    speedNow/MBYTEF
				    );

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* ... msExposure[%g] msDelay[%g]\n",
				traceAcq.sExposure * 1000.,
				traceAcq.sDelay * 1000.);


			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* ... nrErrors [%d]\n",  
				traceAcq.nrErrors);

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"* ... checkImgNr  lima[%d] pco[%d][%d] diff[%d]\n",  
				traceAcq.checkImgNrLima,
				traceAcq.checkImgNrPco,
				traceAcq.checkImgNrPcoTimestamp,
				traceAcq.checkImgNrPcoTimestamp - traceAcq.checkImgNrPco);

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"%s\n", traceAcq.msg);

			return output;
		}


		key = keys[ikey] = "testCmd";     //----------------------------------------------------------------
		keys_desc[ikey++] = "DISABLED / debug tool";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){


			if((tokNr >= 1) &&  (_stricmp(tok[1], "mode")==0)){
				ptr += sprintf_s(ptr, ptrMax - ptr, "testCmdMode [0x%llx]",  m_pcoData->testCmdMode);
				if(tokNr >= 2){
					int nr;
					unsigned long long _testCmdMode;

					nr = sscanf_s(tok[2], "0x%llx", &_testCmdMode);
					
					if(nr == 1) {
						m_pcoData->testCmdMode = _testCmdMode;
						ptr += sprintf_s(ptr, ptrMax - ptr,  "   changed OK>  ");
					} else {
						ptr += sprintf_s(ptr, ptrMax - ptr,  "   ERROR - NOT changed>  ");
					}
					ptr += sprintf_s(ptr, ptrMax - ptr, "testCmdMode [0x%llx]",  m_pcoData->testCmdMode);
				}
				return output;
			}
			
			
#if 0
			//--- test of close
			if( (_stricmp(tok[1], "cb")==0)){
				int error;
				//const char *msg;

				m_cam_connected = false;

				//m_sync->_getBufferCtrlObj()->_pcoAllocBuffersFree();
				m_buffer->_pcoAllocBuffersFree();
				_pco_Close_Cam(error);
				//m_handle = NULL;
				m_handle = 0;

				ptr += sprintf_s(ptr, ptrMax - ptr, "%s> closed cam\n", tok[1]);
				return output;
			}
#endif
			
			//--- test of callback
			if( (_stricmp(tok[1], "cb")==0)){
				Event *ev = new Event(Hardware,Event::Error,Event::Camera,Event::Default, "test cb");
				m_HwEventCtrlObj->reportEvent(ev);
				ptr += sprintf_s(ptr, ptrMax - ptr, "%s> done\n", tok[1]);
				return output;
			}

			//--- test of sleep
			if((tokNr == 2) &&  (_stricmp(tok[1], "time")==0)){
				long long us;

				TIME_UTICKS usStart;

				ptr += sprintf_s(ptr, ptrMax - ptr,  "sleeping ...\n"); 

				usElapsedTimeSet(usStart);

				::Sleep(atoi(tok[2])*1000);


				us = usElapsedTime(usStart);
				double ticksPerSec = usElapsedTimeTicsPerSec();
				ptr += sprintf_s(ptr, ptrMax - ptr, "us[%lld] ms[%5.3f] tics/us[%g]\n", us, us/1000., ticksPerSec/1.0e6);

				return output;
			}





			if(tokNr == 0) {
				ptr += sprintf_s(ptr, ptrMax - ptr, "tokNr [%d] cmd [%s] No parameters", tokNr, cmd); 
			} else {
				ptr += sprintf_s(ptr, ptrMax - ptr, "tokNr [%d] cmd [%s]\n", tokNr, cmd); 
				for(int i = 1; i<= tokNr; i++) {
					ptr += sprintf_s(ptr, ptrMax - ptr, "tok [%d] [%s]\n", i, tok[i]); 
				}
			}
			return output;
		}


#if 0
		key = keys[ikey] = "rollingShutter";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(RW) for EDGE only / rolling shutter mode";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			int error;
			bool rolling, rollingNew;

			if(!_isCameraType(Edge)) {
				ptr += sprintf_s(ptr, ptrMax - ptr, "%d", -1);
				return output;
			}
			
			rolling = _get_shutter_rolling_edge(error);
			if(tokNr == 0) {
				ptr += sprintf_s(ptr, ptrMax - ptr, "%d", rolling);
				return output;
			}

			if((tokNr != 1) || ((strcmp(tok[1],"0") != 0) && (strcmp(tok[1],"1") != 0))){
				ptr += sprintf_s(ptr, ptrMax - ptr, "syntax ERROR - %s <0 | 1>", cmd);
				return output;
			}
			
			rollingNew = atoi(tok[1]) != 0;

			if(rollingNew != rolling){
				_set_shutter_rolling_edge(rollingNew, error);
			}
			ptr += sprintf_s(ptr, ptrMax - ptr, "%d", rollingNew);
			return output;
		}

#endif

		key = keys[ikey] = "pixelRate";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(RW) pixelrate (Hz) for reading images from the image sensor";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			DWORD pixRate, pixRateNext; int error;

			if(_isCameraType(Dimax) ) tokNr = 0;
			
			if( (tokNr < 0) || (tokNr > 1)) {
				ptr += sprintf_s(ptr, ptrMax - ptr,  "-1.0");
				return output;
			}
			
			if(tokNr == 0) {
				_pco_GetPixelRate(pixRate, pixRateNext, error);
				ptr += sprintf_s(ptr, ptrMax - ptr, "%d", pixRateNext);
				return output;
			}

			pixRate = atoi(tok[1]);
			_presetPixelRate(pixRate, error);
			
			if(error){
				ptr += sprintf_s(ptr, ptrMax - ptr,  "-1.0");
				return output;
			}

			ptr += sprintf_s(ptr, ptrMax - ptr, "%d", m_pcoData->dwPixelRateRequested);
			return output;
		}

		key = keys[ikey] = "pixelRateInfo";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) pixelrate (Hz) for reading images from the image sensor (actual & valid values)";
		if(_stricmp(cmd, key) == 0){
			DWORD dwPixRate, dwPixRateNext ; int error, i;

		    _pco_GetPixelRate(dwPixRate, dwPixRateNext, error);
			ptr += sprintf_s(ptr, ptrMax - ptr, "actualRate(Hz):  %d  (requested %d)  validRates:", dwPixRate, dwPixRateNext);

			for(i=0; i<4; i++) {
				dwPixRate = m_pcoData->stcPcoDesc1.dwPixelRateDESC[i];
				if(dwPixRate){
					ptr += sprintf_s(ptr, ptrMax - ptr, "  %d",dwPixRate);
				}  
			}	
			
			
			return output;

		}


		//----------------------------------------------------------------------------------------------------------
		key = keys[ikey] = "pixelRateValidValues";
		keys_desc[ikey++] = "(R) valid values of pixelrate (Hz) for reading images from the image sensor";
		if(_stricmp(cmd, key) == 0){
			DWORD dwPixRate ; int  i, nr;

			for(nr = i = 0; i<4; i++) 
			{
				dwPixRate = m_pcoData->stcPcoDesc1.dwPixelRateDESC[i];
				if(dwPixRate){
					nr++;
					ptr += sprintf_s(ptr, ptrMax - ptr, "%u  ",dwPixRate);
				}  
			}	

            if(nr == 0)			
                ptr += sprintf_s(ptr, ptrMax - ptr, "%d  ",nr);

            return output;
		}



		//----------------------------------------------------------------------------------------------------------
		key = keys[ikey] = "roi";
		keys_desc[ikey++] = "get actual (fixec) last ROI requested (unfixed) ROIs";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			//unsigned int x0, x1, y0, y1;
			//Roi new_roi;


			if((tokNr != 0) ){
					ptr += sprintf_s(ptr, ptrMax - ptr, "syntax ERROR - %s ", cmd);
					return output;
			}
				
            ptr += _pco_roi_info(ptr, ptrMax - ptr, error);
			return output;

		}



		key = keys[ikey] = "debug";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(RW) pco debug level [<new value in hex format (0x123)>]";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			int nr;

			ptr += sprintf_s(ptr, ptrMax - ptr, "0x%llx",  m_pcoData->debugLevel);

			if((tokNr == 1)){
					nr = sscanf_s(tok[1], "0x%llx",  &m_pcoData->debugLevel);
					ptr += sprintf_s(ptr, ptrMax - ptr, "   %s>  ",  (nr == 1) ? "changed OK": "NOT changed");
					ptr += sprintf_s(ptr, ptrMax - ptr, "0x%llx",  m_pcoData->debugLevel);
			
					DEB_ALWAYS() << output ;
			}
			
#define _PRINT_DBG( x )	ptr += sprintf_s(ptr, ptrMax - ptr, "%15s  0x%08x\n", #x, x ) 	

			if((tokNr == 0)){
				ptr += sprintf_s(ptr, ptrMax - ptr,  "\n");
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
			}

			return output;
		}



		key = keys[ikey] = "ADC";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(RW) ADC working ADC [<new value>]";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			int error;
			const char *valid;
			int adc_new, adc_working, adc_max;

			error = _pco_GetADCOperation(adc_working, adc_max);
			valid = error ? "NO" : "YES";
			ptr += sprintf_s(ptr, ptrMax - ptr, "working[%d] max[%d] config[%s]\n", adc_working, adc_max, valid);
			
			if(error) return output;

			if((tokNr >= 1)){
				adc_new = atoi(tok[1]);
				error = _pco_SetADCOperation(adc_new, adc_working);
				ptr += sprintf_s(ptr, ptrMax - ptr, "working[%d] requested[%d] error[0x%x]\n", adc_working, adc_new, error);
			}
			

			return output;
		}


		//----------------------------------------------------------------
		key = keys[ikey] = "camInfo";
		keys_desc[ikey++] = "(R) detailed cam info (type, if, sn, hw & fw ver, ...)";     
		if(_stricmp(cmd, key) == 0){
			ptr += sprintf_s(ptr, ptrMax - ptr,  "\n");
			ptr += sprintf_s(ptr, ptrMax - ptr, "* dwSerialNumber[%u]\n", 
				m_pcoData->stcPcoCamType.dwSerialNumber);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* wCamType[%x] wCamSubType[%x] [%s]\n", 
				m_pcoData->stcPcoCamType.wCamType, 
				m_pcoData->stcPcoCamType.wCamSubType,
				m_pcoData->model); 
			ptr += sprintf_s(ptr, ptrMax - ptr, "* nameCam[%s]\n", m_pcoData->nameCam);
			ptr += sprintf_s(ptr, ptrMax - ptr, "* nameCamIf[%s]\n", m_pcoData->nameCamIf);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* wInterfaceType[%x]  [%s]\n", 
				m_pcoData->stcPcoCamType.wInterfaceType,
				m_pcoData->iface);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* nameSensor[%s]\n", m_pcoData->nameSensor);
			
			ptr += sprintf_s(ptr, ptrMax - ptr, "* GigE IP [%d.%d.%d.%d]\n", 
				m_pcoData->ipField[0], m_pcoData->ipField[1], m_pcoData->ipField[2], m_pcoData->ipField[3]);
			
			ptr += sprintf_s(ptr, ptrMax - ptr, "* dwHWVersion[%x]  dwFWVersion[%x] <- not used\n", 
				m_pcoData->stcPcoCamType.dwHWVersion, 
				m_pcoData->stcPcoCamType.dwFWVersion);

            ptr += _pco_GetHardwareVersion_Firmware(ptr, ptrMax - ptr, error);
            


			int adc_working, adc_max;
			error = _pco_GetADCOperation(adc_working, adc_max);

			ptr += sprintf_s(ptr, ptrMax - ptr, "* ADC wADCOperation[%d] wNumADCsDESC[%d]\n", 
					adc_working, adc_max);

			ptr += sprintf_s(ptr, ptrMax - ptr,"%s\n", m_log.c_str());
			return output;
		}

		key = keys[ikey] = "camType";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) cam type, interface, serial number, hardware & firmware version";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			ptr += sprintf_s(ptr, ptrMax - ptr, "ty[%s] if[%s] sn[%u] hw[%x] fw[%x]", 
				m_pcoData->model, 
				m_pcoData->iface,
				m_pcoData->stcPcoCamType.dwSerialNumber,
				m_pcoData->stcPcoCamType.dwHWVersion, 
				m_pcoData->stcPcoCamType.dwFWVersion
				);
			return output;
		}

		key = keys[ikey] = "lastError";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) last PCO SDK error";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			m_pcoData->pcoErrorMsg[ERR_SIZE] = 0;
			ptr += sprintf_s(ptr, ptrMax - ptr, "[x%08x] [%s]\n", 
				m_pcoData->pcoError, m_pcoData->pcoErrorMsg
				);
			
			return output;
		}

		key = keys[ikey] = "msgLog";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) log of last cmds executed ";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){

			ptr += m_msgLog->dump(ptr, (int)(ptrMax - ptr), 0);
			m_msgLog->dumpPrint(true);			
			return output;
		}


		key = keys[ikey] = "msgLogFlush";     //----------------------------------------------------------------
		keys_desc[ikey++] = "flush log of last cmds executed ";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			m_msgLog->flush(-1);			
			ptr += sprintf_s(ptr, ptrMax - ptr,  "flushed ...");
			return output;
		}


		key = keys[ikey] = "dumpData";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) hex dump of the stcPcoData";     
		if(_stricmp(cmd, key) == 0){

			print_hex_dump_buff(m_pcoData, sizeof(stcPcoData));

			ptr += sprintf_s(ptr, ptrMax - ptr,  "dumped\n" );
			
			return output;
		}


		key = keys[ikey] = "acqEnable";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) acq enable signal status (BNC acq enbl in)";     
		if(_stricmp(cmd, key) == 0){
			UNUSED int error;
			WORD wAcquEnableState = 0;

			_pco_GetAcqEnblSignalStatus(wAcquEnableState, error);
			
			ptr += sprintf_s(ptr, ptrMax - ptr, "%d", wAcquEnableState);
			
			return output;
		}


/***************************************************************
wSignalFilter: Flags showing the filter option:
- 0x01: Filter can be switched off (t > ~65ns)
- 0x02: Filter can be switched to medium (t > ~1us)
- 0x04: Filter can be switched to high (t > ~100ms)
Notes: the command will be rejected, if Recording State is [run]
***************************************************************/



		key = keys[ikey] = "hwioSignals";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) for DIMAX/EDGE only / get hw io signals";     
		if(_stricmp(cmd, key) == 0){
			int error, i;

			_pco_GetHWIOSignal(error);
			if(error) {
				ptr += sprintf_s(ptr, ptrMax - ptr, "ERROR [%d]\n", error);
				//return output;
			}
			//ptr += sprintf_s(ptr, ptrMax - ptr, "signals [%d] [%d]\n", m_pcoData->wNrPcoHWIOSignal0, m_pcoData->wNrPcoHWIOSignal);
			
			for(i=0; i< m_pcoData->wNrPcoHWIOSignal; i++) {
			    ptr += sprintf_s(ptr, ptrMax - ptr,"\n\n#======================== descriptor [%d]\n",i);
			    
			    WORD wSelected = 0;
				ptr += sprintf_s(ptr, ptrMax - ptr, 
					"sigNames[%s] [%s] [%s] [%s] idx[%d]/[%d] sigNum[%d] \n"
					"-def:     def[0x%x] type[0x%x] pol[0x%x] filt[0x%x]\n"
#if 0
					"-sig:    enab[0x%x] type[0x%x] pol[0x%x] filt[0x%x] signalSelected[0x%x]\n" 
#endif
					"-sig:    name[%s]\n\n", 
					m_pcoData->stcPcoHWIOSignalDesc[i].szSignalName[0],
					m_pcoData->stcPcoHWIOSignalDesc[i].szSignalName[1],
					m_pcoData->stcPcoHWIOSignalDesc[i].szSignalName[2],
					m_pcoData->stcPcoHWIOSignalDesc[i].szSignalName[3],
					i, m_pcoData->wNrPcoHWIOSignal,
					m_pcoData->stcPcoHWIOSignal[i][wSelected].wSignalNum,

					m_pcoData->stcPcoHWIOSignalDesc[i].wSignalDefinitions,
					m_pcoData->stcPcoHWIOSignalDesc[i].wSignalTypes,
					m_pcoData->stcPcoHWIOSignalDesc[i].wSignalPolarity,
					m_pcoData->stcPcoHWIOSignalDesc[i].wSignalFilter,

#if 0
					m_pcoData->stcPcoHWIOSignal[i][wSelected].wEnabled,
					m_pcoData->stcPcoHWIOSignal[i][wSelected].wType,
					m_pcoData->stcPcoHWIOSignal[i][wSelected].wPolarity,
					m_pcoData->stcPcoHWIOSignal[i][wSelected].wFilterSetting,
					m_pcoData->stcPcoHWIOSignal[i][wSelected].wSelected,
#endif

					&m_pcoData->sPcoHWIOSignalDesc[i][0]
					//str_printable(&m_pcoData->sPcoHWIOSignalDesc[i][0])
					//&(m_pcoData->sPcoHWIOSignalDesc[iSignal][0])
					);


				WORD val;


/***************************************************************
wSignalDefinitions: Flags showing signal options:
- 0x01: Signal can be enabled/disabled
- 0x02: Signal is a status output
- 0x10: Signal function 1 has got parameter value
- 0x20: Signal function 2 has got parameter value
- 0x40: Signal function 3 has got parameter value
- 0x80: Signal function 4 has got parameter value
****************************************************************/
				val = m_pcoData->stcPcoHWIOSignalDesc[i].wSignalDefinitions;
				ptr += sprintf_s(ptr, ptrMax - ptr, "def[0x%x]: ", val); 
                if(val & 0x01) ptr += sprintf_s(ptr, ptrMax - ptr, "[Signal can be enabled/disabled]");				
                if(val & 0x02) ptr += sprintf_s(ptr, ptrMax - ptr, "[Signal is a status output]");				
                if(val & 0x10) ptr += sprintf_s(ptr, ptrMax - ptr, "[Signal function 1 has got parameter value]");				
                if(val & 0x20) ptr += sprintf_s(ptr, ptrMax - ptr, "[Signal function 2 has got parameter value]");				
                if(val & 0x40) ptr += sprintf_s(ptr, ptrMax - ptr, "[Signal function 3 has got parameter value]");				
                if(val & 0x80) ptr += sprintf_s(ptr, ptrMax - ptr, "[Signal function 4 has got parameter value]");				
				ptr += sprintf_s(ptr, ptrMax - ptr, "\n"); 
					

/***************************************************************
wSignalTypes: Flags showing which signal type is available:
- 0x01: TTL
- 0x02: High Level TTL
- 0x04: Contact Mode
- 0x08: RS485 differential
***************************************************************/
				val = m_pcoData->stcPcoHWIOSignalDesc[i].wSignalTypes;
				ptr += sprintf_s(ptr, ptrMax - ptr, "type[0x%x]: ", val); 
                if(val & 0x01) ptr += sprintf_s(ptr, ptrMax - ptr, "[TTL]");				
                if(val & 0x02) ptr += sprintf_s(ptr, ptrMax - ptr, "[High Level TTL]");				
                if(val & 0x04) ptr += sprintf_s(ptr, ptrMax - ptr, "[Contact Mode]");				
                if(val & 0x08) ptr += sprintf_s(ptr, ptrMax - ptr, "[RS485 differential]");				
				ptr += sprintf_s(ptr, ptrMax - ptr, "\n"); 


/***************************************************************
wSignalPolarity: Flags showing which signal polarity can be selected:
- 0x01: Low level active
- 0x02: High Level active
- 0x04: Rising edge active
- 0x08: Falling edge active
***************************************************************/
				val = m_pcoData->stcPcoHWIOSignalDesc[i].wSignalPolarity;
				ptr += sprintf_s(ptr, ptrMax - ptr, "pol[0x%x]: ", val); 
                if(val & 0x01) ptr += sprintf_s(ptr, ptrMax - ptr, "[Low level active]");				
                if(val & 0x02) ptr += sprintf_s(ptr, ptrMax - ptr, "[High Level active]");				
                if(val & 0x04) ptr += sprintf_s(ptr, ptrMax - ptr, "[Rising edge active]");				
                if(val & 0x08) ptr += sprintf_s(ptr, ptrMax - ptr, "[Falling edge active]");				
				ptr += sprintf_s(ptr, ptrMax - ptr, "\n"); 

/***************************************************************
wSignalFilter: Flags showing the filter option:
- 0x01: Filter can be switched off (t > ~65ns)
- 0x02: Filter can be switched to medium (t > ~1us)
- 0x04: Filter can be switched to high (t > ~100ms)
Notes: the command will be rejected, if Recording State is [run]
***************************************************************/

				val = m_pcoData->stcPcoHWIOSignalDesc[i].wSignalFilter;
				ptr += sprintf_s(ptr, ptrMax - ptr, "filter[0x%x]: ", val); 
                if(val & 0x01) ptr += sprintf_s(ptr, ptrMax - ptr, "[Filter can be switched off (t > ~65ns)]");				
                if(val & 0x02) ptr += sprintf_s(ptr, ptrMax - ptr, "[Filter can be switched to medium (t > ~1us)]");				
                if(val & 0x04) ptr += sprintf_s(ptr, ptrMax - ptr, "[Filter can be switched to high (t > ~100ms)]");				
				ptr += sprintf_s(ptr, ptrMax - ptr, "\n"); 



                for(wSelected = 0; wSelected < 3 ; wSelected++)
                { 

//                    if(m_pcoData->stcPcoHWIOSignal[i][wSelected].wSelected <4)
                    if(m_pcoData->stcPcoHWIOSignalDesc[i].szSignalName[wSelected][0])
                    {
    			    ptr += sprintf_s(ptr, ptrMax - ptr,"\n   .......... descriptor[%d] wSelected[%d]\n",i, wSelected);

				        //val = m_pcoData->stcPcoHWIOSignal[i][wSelected].wSelected;
				        //val = wSelected;
				        ptr += sprintf_s(ptr, ptrMax - ptr, "   signalSelected[%d] [%s]\n", 
				            wSelected, m_pcoData->stcPcoHWIOSignalDesc[i].szSignalName[wSelected]);				

        /***************************************************************
        Enabled Flags showing enable state of the signal
         0x00: Signal is off
         0x01: Signal is active
        ***************************************************************/

				        val = m_pcoData->stcPcoHWIOSignal[i][wSelected].wEnabled;
				        ptr += sprintf_s(ptr, ptrMax - ptr, "   enabled[0x%x]: ", val); 
                        if(val) ptr += sprintf_s(ptr, ptrMax - ptr, "[Signal is active]");
                        else ptr += sprintf_s(ptr, ptrMax - ptr, "[Signal is off]");				
				        ptr += sprintf_s(ptr, ptrMax - ptr, "\n"); 


        /***************************************************************
        Type Flags showing which signal type is selected
         0x01: TTL
         0x02: High Level TTL
         0x04: Contact Mode
         0x08: RS485 differential
        ***************************************************************/
				        val = m_pcoData->stcPcoHWIOSignal[i][wSelected].wType;
				        ptr += sprintf_s(ptr, ptrMax - ptr, "   type[0x%x]: ", val); 
                        if(val & 0x01) ptr += sprintf_s(ptr, ptrMax - ptr, "[TTL]");				
                        if(val & 0x02) ptr += sprintf_s(ptr, ptrMax - ptr, "[High Level TTL]");				
                        if(val & 0x04) ptr += sprintf_s(ptr, ptrMax - ptr, "[Contact Mode]");				
                        if(val & 0x08) ptr += sprintf_s(ptr, ptrMax - ptr, "[RS485 differential]");				
				        ptr += sprintf_s(ptr, ptrMax - ptr, "\n"); 


        /***************************************************************
        Polarity Flags showing which signal polarity is selected
         0x01: High level active
         0x02: Low level active
         0x04: Rising edge active
         0x08: Falling edge active
        ***************************************************************/

				        val = m_pcoData->stcPcoHWIOSignal[i][wSelected].wPolarity;
				        ptr += sprintf_s(ptr, ptrMax - ptr, "   pol[0x%x]: ", val); 
                        if(val & 0x01) ptr += sprintf_s(ptr, ptrMax - ptr, "[Low level active]");				
                        if(val & 0x02) ptr += sprintf_s(ptr, ptrMax - ptr, "[High Level active]");				
                        if(val & 0x04) ptr += sprintf_s(ptr, ptrMax - ptr, "[Rising edge active]");				
                        if(val & 0x08) ptr += sprintf_s(ptr, ptrMax - ptr, "[Falling edge active]");				
				        ptr += sprintf_s(ptr, ptrMax - ptr, "\n"); 


        /***************************************************************
        FilterSetting Flags showing the filter option which is selected
         0x01: Filter can be switched off (t > ~65ns)
         0x02: Filter can be switched to medium (t > ~1 u)
         0x04: Filter can be switched to high (t > ~100ms)
        Selected In case the HWIOSignaldescription shows more than one SignalNames, this parameter can be
        used to select a different signal, e.g. ’Status Busy’ or ’Status Exposure’.
        ***************************************************************/

				        val = m_pcoData->stcPcoHWIOSignal[i][wSelected].wFilterSetting;
				        ptr += sprintf_s(ptr, ptrMax - ptr, "   filter[0x%x]: ", val); 
                        if(val & 0x01) ptr += sprintf_s(ptr, ptrMax - ptr, "[Filter can be switched off (t > ~65ns)]");				
                        if(val & 0x02) ptr += sprintf_s(ptr, ptrMax - ptr, "[Filter can be switched to medium (t > ~1us)]");				
                        if(val & 0x04) ptr += sprintf_s(ptr, ptrMax - ptr, "[Filter can be switched to high (t > ~100ms)]");				
				        ptr += sprintf_s(ptr, ptrMax - ptr, "\n"); 


                    }
				}
					//print_hex_dump_buff(&m_pcoData->stcPcoHWIOSignalDesc[i].szSignalName[0][0], 24*4);
			}

			
			return output;
		}


		key = keys[ikey] = "sethwioExtTrigPol";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(W) for DIMAX / EDGE only / set hw io signals";     
		if(_stricmp(cmd, key) == 0){
			int error, idx;
			WORD val, wSelected;

            wSelected = 0;
            
            if(!( _isCameraType(Dimax |Edge))  ) 
            {
	    	    ptr += sprintf_s(ptr, ptrMax - ptr, "ERROR function only for Dimax and Edge");;
		        return output;
	        }

                        
			if(tokNr != 1){
				ptr += sprintf_s(ptr, ptrMax - ptr, "Usage: sethwioExtTrigPol <pol>\n");
				ptr += sprintf_s(ptr, ptrMax - ptr, "   1: Low level active\n");
				ptr += sprintf_s(ptr, ptrMax - ptr, "   2: High Level active\n");
				ptr += sprintf_s(ptr, ptrMax - ptr, "   4: Rising edge active\n");
				ptr += sprintf_s(ptr, ptrMax - ptr, "   8: Falling edge active\n");
				return output;
			}

    		val = atoi(tok[1]);
				

            _pco_initHWIOSignal(0, val, error);
			
			ptr += sprintf_s(ptr, ptrMax - ptr, "val[0x%x] error[%d]", val, error);

			
			return output;
		}


		key = keys[ikey] = "dumpData";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) hex dump of the stcPcoData";     
		if(_stricmp(cmd, key) == 0){

			print_hex_dump_buff(m_pcoData, sizeof(stcPcoData));
			ptr += sprintf_s(ptr, ptrMax - ptr,  "dumped\n");
			
			return output;
		}


		key = keys[ikey] = "comment";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(W) print timestamp & comment in the screen";     
		if(_stricmp(cmd, key) == 0){
			char *comment = str_trim(cmdBuffAux + strlen(cmd));

			ptr += sprintf_s(ptr, ptrMax - ptr, 
				"\n"
				"=================================================\n"
				"--- %s [%s]\n"
				"=================================================\n"
				, getTimestamp(Iso), comment);

			DEB_ALWAYS() << output ;
			return output;
		}

		key = keys[ikey] = "?";     //----------------------------------------------------------------
		keys_desc[ikey++] = "(R) this help / list of the talk cmds";     //----------------------------------------------------------------
		if(_stricmp(cmd, key) == 0){
			for(int i = 0; i < ikey; i++) {
				ptr += sprintf_s(ptr, ptrMax - ptr, "%18s - %s\n", keys[i], keys_desc[i]);
			}
			ptr += sprintf_s(ptr, ptrMax - ptr, "--- nrCmds[%d][%d]\n", ikey, NRCMDS);
			return output;
		}

		sprintf_s(ptr, ptrMax - ptr, "ERROR unknown cmd [%s]", cmd);
		return output;
}


//====================================================================
// utils
//====================================================================

#define LEN_LINE_BUFF	128
#define BYTES_PER_LINE		16
#define OFFSET_COL_HEX1	(4 + 1 + 3)
#define OFFSET_COL_HEX2	(OFFSET_COL_HEX1 + (8 * 3) + 2)
#define OFFSET_COL_ASC1	(OFFSET_COL_HEX2 + (8 * 3) + 3)
#define OFFSET_COL_ASC2	(OFFSET_COL_ASC1 + 8 + 1)

//--------------------------------------------------------------------
//--------------------------------------------------------------------
char *nibble_to_hex(char *s, BYTE nibble)
{
  nibble &= 0x0f;
  *s = (nibble <= 9) ?  '0' + nibble : 'a' + nibble - 10;
  return s+1;
}



//--------------------------------------------------------------------
//--------------------------------------------------------------------
char *byte_to_hex(char *s, BYTE byte)
{
  s = nibble_to_hex(s, byte >> 4);
  return nibble_to_hex(s, byte);;
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
char *word_to_hex(char *s, WORD word)
{
  s = byte_to_hex(s, word >> 8);
  return byte_to_hex(s, (BYTE) word);
  
}


//--------------------------------------------------------------------
//--------------------------------------------------------------------
char *dword_to_hex(char *s, DWORD dword)
{
  s = word_to_hex(s, dword >> 16);
  return word_to_hex(s, (WORD) dword);
  
}


//--------------------------------------------------------------------
//--------------------------------------------------------------------
char *_hex_dump_bytes(void *obj, size_t lenObj, char *buff, size_t lenBuff) {
	char *ptr = buff;
	char *ptrMax = buff + lenBuff;
	char *ptrObj = (char *) obj;

	memset(buff, ' ', lenBuff);

	for(unsigned int i=0; i< lenObj; i++) {

		if(ptr+3 > ptrMax) break;
		ptr = byte_to_hex(ptr, *ptrObj ++);
		if(ptr+1 == ptrMax) break;
		ptr++;

	}

	*ptr = 0; 
	return buff;

}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
const char *hex_dump_line(void *buff, size_t len, size_t *nr, WORD *offset) {

	static char line_buff[LEN_LINE_BUFF];

	char *s;
	BYTE *ptr_buff;
	BYTE *ptr_buff0;
	int i, nr1, nr2;

	ptr_buff = (BYTE *) buff;
	ptr_buff0 = ptr_buff;

	s = line_buff;


	if(len <= 0) {
		strcpy_s(s,LEN_LINE_BUFF,"<empty>");
		return line_buff;
	}

	memset(line_buff, ' ', LEN_LINE_BUFF);

	*nr = (len < BYTES_PER_LINE) ? len : BYTES_PER_LINE;

	s = word_to_hex(s, *offset);
	*offset += (WORD) *nr;
	*s = ':';
	s = line_buff + OFFSET_COL_HEX1;

	if(*nr <= BYTES_PER_LINE / 2) {
		nr1 = (int) *nr;
		nr2 = -1;
	} else {
		nr1 = BYTES_PER_LINE / 2;
		nr2 = ((int) *nr) - nr1;
	}

	for(i = 0; i < nr1; i++) {
		s = byte_to_hex(s, *ptr_buff++);
		s++;
	}

	if(nr2 >0 ){
		*s = '-';
		s = line_buff + OFFSET_COL_HEX2;
		for(i = 0; i < nr2; i++) {
			s = byte_to_hex(s, *ptr_buff++);
			s++;
		}
	}	

	ptr_buff = ptr_buff0;
	s = line_buff + OFFSET_COL_ASC1;
	for(i = 0; i < nr1; i++) {
		*s++ = (isprint(*ptr_buff)) ? *ptr_buff : '.';
		ptr_buff++;
	}

	if(nr2 >0 ){
		s = line_buff + OFFSET_COL_ASC2;
		for(i = 0; i < nr2; i++) {
			*s++ = (isprint(*ptr_buff)) ? *ptr_buff : '.';
			ptr_buff++;
		}
	}	
	
	*s = 0;

	return line_buff;

}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
void print_hex_dump_buff(void *ptr_buff, size_t len) {
	WORD offset = 0;
	size_t nr = 0;
	BYTE * ptr = (BYTE *) ptr_buff;
	;
	
	printf("dump buff / len: %lu\n", len);
	
	while(len > 0) {
		printf("%s\n", hex_dump_line(ptr, len, &nr, &offset));
		len -= nr;
		ptr += nr;
	}

}


//--------------------------------------------------------------------
//--------------------------------------------------------------------


ringLog::ringLog(int size) {
        buffer = new struct data [size];
        m_capacity = m_capacity_max = size;
        m_size = 0;
        m_head = 0;
}
void ringLog::flush(int capacity) {
		if((capacity > 0) &&(capacity <= m_capacity_max))
			m_capacity = capacity;
        m_size = 0;
        m_head = 0;
}

ringLog::~ringLog() {
        delete buffer;
}

int ringLog::add(const char *s) {

        struct data *ptr;
        int offset;
        
        if (m_size < m_capacity){
                offset = (m_head + m_size) % m_capacity;
                ptr = buffer + offset;
                m_size++;
        } else {
                ptr = buffer + m_head;
                m_head = (m_head + 1) % m_capacity;
                m_size = m_capacity;
        }
        
        ptr->timestamp = getTimestamp();
        strcpy_s(ptr->str,RING_LOG_BUFFER_SIZE, s);
        return m_size;

}


void ringLog::dumpPrint(bool direction) {

        static char timeline[128];
        struct data *ptr;
        int offset;
        time_t ltime;
        struct tm today;
        const char *fmt = "%Y/%m/%d %H:%M:%S";
        int i;
        //errno_t err;
        
        for(i=0; i< m_size; i++) {
        
                offset = direction ? i : m_size -1 -i;
                
                ptr = buffer + (m_head + offset) % m_capacity;
                ltime = ptr->timestamp;

				//err = localtime_s( &today, &ltime );
				localtime_s( &today, &ltime );

                strftime(timeline, 128, fmt, &today);
                
                printf("%s> %s\n", timeline, ptr->str);
        }
        
}

int ringLog::dump(char *s, int lgMax, bool direction) {

        //static char timeline[128];
        struct data *ptr;
        int offset;
        time_t ltime;
        struct tm today;
        const char *fmt = "%Y/%m/%d %H:%M:%S";
		int linMax = 25 + RING_LOG_BUFFER_SIZE;
        int i;
		//char *ptrOut = s;
        //errno_t err;
        int lg = 0;

        for(i=0; (i< m_size) && ((lgMax - lg) > linMax); i++) {
        
                offset = direction ? i : m_size -1 -i;
                
                ptr = buffer + (m_head + offset) % m_capacity;
                ltime = ptr->timestamp;

				//err = localtime_s( &today, &ltime );
				localtime_s( &today, &ltime );

                lg += (int) strftime(s + lg, lgMax - lg, fmt, &today);
                lg += sprintf_s(s + lg, lgMax - lg, "> %s\n", ptr->str);
        }
        
		return lg;
}

//=========================================================================================================
//=========================================================================================================

unsigned long long Camera::_getDebug(unsigned long long mask = ULLONG_MAX){

		return m_pcoData->debugLevel & mask;

}
//=========================================================================================================
//=========================================================================================================

const char *_checkLogFiles() {
	const char *logFiles[] = {
		"C:\\ProgramData\\pco\\SC2_Cam.log", 
		"C:\\ProgramData\\pco\\PCO_CDlg.log", 
		"C:\\ProgramData\\pco\\PCO_Conv.log",
		NULL};
	const char **ptr = logFiles;
	const char *logOn = "\n\n"		
		"###############################################################################\n"
		"###############################################################################\n"
		"###############################################################################\n"
		"###                                                                         ###\n"
		"###                           !!!  ATTENTION !!!                            ###\n"
		"###                                                                         ###\n"
		"###                     THE PCO LOG FILES ARE ENABLED                       ###\n"
		"###                                                                         ###\n"
		"###                 this option is ONLY for DEBUG & TESTS                   ###\n"
		"###                                                                         ###\n"
		"###                   it downgrades the acquisition time                    ###\n"
		"###                                                                         ###\n"
		"###                                                                         ###\n"
		"###     to DISABLE it:                                                      ###\n"
		"###          * stop the device server                                       ###\n"
		"###          * open the directory C:\\ProgramData\\pco\\                       ###\n"
		"###          * rename all the .log files as .txt                            ###\n"
		"###          * start again the device server                                ###\n"
		"###                                                                         ###\n"
		"###############################################################################\n"
		"###############################################################################\n"
		"###############################################################################\n\n\n";

	const char *logOff = "";
	struct stat fileStat;
	int error;
	bool found = false;

	while(*ptr != NULL) {
		error = stat(*ptr, &fileStat);
		//printf("----------- [%d][%s]\n", error, *ptr);
 		found |= !error;
		ptr++;
	}
	return found ? logOn : logOff;	
};

//====================================================================
//====================================================================

#define INFO_BUFFER_SIZE 1024

char * _getComputerName(char *infoBuff, DWORD  bufCharCount  )
{
    int err;
#ifdef __linux__
    err = gethostname(infoBuff, bufCharCount);
#else
    err =  !GetComputerName( infoBuff, &bufCharCount ) ;
#endif

    if(err) 
        sprintf_s(infoBuff, bufCharCount, "ERROR: GetComputerName" ); 

    return infoBuff ;
}

char * _getUserName(char *infoBuff, DWORD  bufCharCount  )
{
 
    int err;
#ifdef __linux__
    err = getlogin_r(infoBuff, bufCharCount);
#else
    err =  !GetUserName( infoBuff, &bufCharCount ) ;
#endif

    if(err)
	   sprintf_s(infoBuff, bufCharCount,  "ERROR: GetUserName" ); 
    return infoBuff ;
}


#define x64					"x64"
#define Release_Win7_Sync	"Release_Win7_Sync"
#define Release	"Release"
#define osLinux				"linux"

char * _getVSconfiguration(char *infoBuff, DWORD  bufCharCount  )
{
	sprintf_s(infoBuff, bufCharCount, "platform[%s] configuration[%s]",  
		VS_PLATFORM,
		VS_CONFIGURATION); 
  return infoBuff ;
}
//====================================================================
//====================================================================

void Camera::_traceMsg(char *msg)
{
	DEB_MEMBER_FUNCT();
	DEB_ALWAYS() << "\n>>>  " << msg ;		

}
