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
#include <sys/stat.h>
#include <sys/timeb.h>
#else
#include <sys/stat.h>
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
        virtual void threadFunctionDimax();
        virtual void threadFunctionEdge();
    
    private:
        Camera&    m_cam;
};


//=================================================================
//=================================================================================================
//---------------------------
//- Camera::_AcqThread::threadFunction()
//---------------------------
void Camera::_AcqThread::threadFunction()
{
    DEB_MEMBER_FUNCT();
    DEF_FNID;


        if(m_cam._isCameraType(Dimax))
        {
            //m_cam.threadFunctionDimax();
            threadFunctionDimax();
        }
        else if(m_cam._isCameraType(Edge))
        {
            //m_cam.threadFunctionEdge();
            threadFunctionEdge();
        }
        else
        {
            DEB_ALWAYS() << "ABORT - camera not supported!!!";
            throw LIMA_HW_EXC(Error, "camera not supported");
        }
 
}


//=================================================================================================
// FOR DIMAX - BEGIN
//=================================================================================================
void Camera::_AcqThread::threadFunctionDimax()
{
    DEB_MEMBER_FUNCT();
    DEF_FNID;

    const char *_msgAbort;
	TIME_USEC tStart;
	long long usStart, usStartTot;

    //Camera &m_cam = *this;
    m_cam.m_pcoData->traceAcq.fnId = fnId;
    //m_cam->traceAcq.fnId = fnId;
    DEB_ALWAYS() << "[entry]" ;

    int err, errTot;
    int pcoBuffIdx, pcoFrameNr, pcoFrameNrTimestamp;
    void *limaBuffPtr;
    //void *pcoBuffPtr;
    DWORD width, height;
    
    
    
    AutoMutex aLock(m_cam.m_cond.mutex());
    //StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_obj.getBuffer();

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
          DEB_ALWAYS() << "++++++++++++++++++++++++++++++++++Wait " << getTimestamp(Iso);
          m_cam.m_thread_running = false;
          m_cam.m_cond.broadcast();
          m_cam.m_cond.wait();
        } // while wait
        
        DEB_ALWAYS() << "++++++++++++++++++++++++++++++++++Run " << getTimestamp(Iso);
        m_cam.m_thread_running = true;
        if(m_cam.m_quit) return;

        Camera::Status _statusReturn = Camera::Ready;
        bool continueAcq = true;

        bool bNoTimestamp;
        WORD wTimestampMode;
        m_cam._pco_GetTimestampMode(wTimestampMode, err);
        bNoTimestamp = err || !wTimestampMode; 
        
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_execTimeTot].desc = "total execTime";
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_Lima].desc = "Lima execTime";
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].desc = "SDK execTime";

    	msElapsedTimeSet(tStart);
    	usElapsedTimeSet(usStart);
    	usElapsedTimeSet(usStartTot);
        m_cam.m_pcoData->traceAcq.fnId = fnId;
        m_cam.m_pcoData->traceAcq.fnTimestampEntry = getTimestamp();
	

    	m_cam.m_pcoData->traceAcq.msStartAcqStart = msElapsedTime(tStart);
        
    	//m_cam.m_pcoData->traceAcq.usTicks[traceAcq_Lima].desc = "xfer to lima / total execTime";

        m_cam.m_sync->setStarted(true);        

        m_cam.m_sync->getNbFrames(_nb_frames);
        limaFrameNr = 0;            // 0 ..... N-1
    
    	m_cam.m_pcoData->traceAcq.nrImgRequested = _nb_frames;

        m_cam.m_status = Camera::Exposure;
        m_cam.m_cond.broadcast();
        aLock.unlock();
        
        
        err = m_cam.grabber->Get_actual_size(&width,&height,NULL);
        PCO_CHECK_ERROR1(err, "Get_actual_size");
        if(err)  m_cam.m_pcoData->traceAcq.nrErrors++;

        DEB_ALWAYS() << DEB_VAR3(width, height, _nb_frames);

        pcoBuffIdx=1;

        bool acquireFirst = m_cam._isCameraType(Edge) && m_cam._isInterfaceType(INTERFACE_CAMERALINK);

        
        errTot = 0;
        if(acquireFirst)
        {
            //err = grabber->Start_Acquire(_nb_frames);
            //PCO_CHECK_ERROR1(err, "Start_Acquire");
            //if(err)  {errTot ++; m_cam.m_pcoData->traceAcq.nrErrors++;}

            DEB_ALWAYS() << fnId << " _pco_SetRecordingState AFTER grabber->Start_Acquire" ;
            m_cam._pco_SetRecordingState(1, err);
            PCO_CHECK_ERROR1(err, "SetRecordingState(1)");
            if(err)  {errTot ++; m_cam.m_pcoData->traceAcq.nrErrors++;}
        }
        else
        {
            DEB_ALWAYS() << fnId << " _pco_SetRecordingState BEFORE grabber->Start_Acquire" ;
            m_cam._pco_SetRecordingState(1, err);
            PCO_CHECK_ERROR1(err, "SetRecordingState(1)");
            if(err)  {errTot ++; m_cam.m_pcoData->traceAcq.nrErrors++;}

            //err = grabber->Start_Acquire(_nb_frames);
            //PCO_CHECK_ERROR1(err, "Start_Acquire");
            //if(err)  {errTot ++; m_cam.m_pcoData->traceAcq.nrErrors++;}
        }
        
        bool bAbort = false;
        
        if(errTot) 
        {
            m_cam.m_pcoData->traceAcq.nrErrors++;
            //_setStatus(Camera::Fault,false);        
            //_statusReturn = Camera::Fault;
            continueAcq = false;        
            m_cam.m_wait_flag = true;
            bAbort = true;
        }       
        


        if(m_cam._isCameraType(Dimax) && continueAcq)
        {
            DWORD validCount, maxCount;
            int error;
            
            DEB_ALWAYS() << "INFO - recording [begin]";
            while(1)
            {
                m_cam._waitForRecording(_nb_frames, validCount, maxCount, error);
        
                if(error || ((DWORD) _nb_frames > maxCount))
                {
                    continueAcq = false;        
                    m_cam.m_wait_flag = true;
                    bAbort = true;
                    _msgAbort = "ABORT - _waitForRecording "; 
                    DEB_ALWAYS() << _msgAbort << DEB_VAR4(_nb_frames, validCount, maxCount, error) ;
                    break;
                }
                
                if(validCount >= (DWORD) _nb_frames)
                {
                    m_cam._pco_SetRecordingState(0, err);
                    PCO_CHECK_ERROR1(err, "SetRecordingState(0)");
                    if(err)
                    {
                        continueAcq = false;        
                        m_cam.m_wait_flag = true;
                        bAbort = true;
                        _msgAbort = "ABORT - _SetRecordingState(0)"; DEB_ALWAYS() << _msgAbort;
                        break;
                    } 
                    else
                    {
                        DEB_ALWAYS() << "INFO - recording [end]";
                        break;                 
                    }
                }
             }
        }

        // WORD wActSeg = m_cam._pco_GetActiveRamSegment();
        WORD wActSeg; m_cam._pco_GetActiveRamSegment(wActSeg, err);
        
        while(  !m_cam.m_wait_flag && 
                continueAcq && 
                ((_nb_frames == 0) || limaFrameNr < _nb_frames))
        {

            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
       		usElapsedTimeSet(usStart);

            pcoFrameNr = limaFrameNr +1;
            limaBuffPtr =  m_cam.m_buffer->_getFrameBufferPtr(limaFrameNr, nb_allocated_buffers);
           //DEB_ALWAYS() << DEB_VAR4(nb_allocated_buffers, _nb_frames, limaFrameNr, limaBuffPtr);

            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_GetImageEx].value += usElapsedTime(usStart);
            		usElapsedTimeSet(usStart);

            m_cam._setStatus(Camera::Readout,false);

            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_Lima].value += usElapsedTime(usStart);
       		usElapsedTimeSet(usStart);

    		usElapsedTimeSet(usStart);
    		                        
            //err = camera-> pco _ReadImagesFromSegment(wActSeg,pcoFrameNr,pcoFrameNr);
            //err=grabber->Get_Framebuffer_adr(pcoBuffIdx,&pcoBuffPtr);
 
            {
                //grabber->Extract_Image(limaBuffPtr,pcoBuffPtr,width,height);
                //DWORD Get_Image ( WORD Segment, DWORD ImageNr, void * adr )
                //DEB_ALWAYS() << DEB_VAR3(wActSeg, pcoFrameNr, limaBuffPtr);

                m_cam.grabber->Get_Image(wActSeg, pcoFrameNr, limaBuffPtr );
                PCO_CHECK_ERROR1(err, "Get_Image");
                if(err!=PCO_NOERROR)
                {
                    m_cam.m_pcoData->traceAcq.nrErrors++;
                    _msgAbort = "ABORT - Get_Image "; 
                    DEB_ALWAYS()  << _msgAbort << DEB_VAR3(wActSeg, pcoFrameNr, limaBuffPtr) ;
                    continueAcq = false;        
                    m_cam.m_wait_flag = true;
                    bAbort = true;
                    break;
 
                }
                
 
                pcoFrameNrTimestamp=image_nr_from_timestamp(limaBuffPtr,0, bNoTimestamp);
                
	            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
        		usElapsedTimeSet(usStart);

                m_cam.m_pcoData->traceAcq.checkImgNrPcoTimestamp = pcoFrameNrTimestamp;
                m_cam.m_pcoData->traceAcq.checkImgNrPco = pcoFrameNr;
                m_cam.m_pcoData->traceAcq.checkImgNrLima = limaFrameNr;
                m_cam.m_pcoData->traceAcq.checkImgNrLima = limaFrameNr;
                m_cam.m_pcoData->traceAcq.msStartAcqNow = msElapsedTime(tStart);


                HwFrameInfoType frame_info;
	            frame_info.acq_frame_nb = limaFrameNr;
	            continueAcq = m_cam.m_buffer->m_buffer_cb_mgr.newFrameReady(frame_info);

	            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_Lima].value += usElapsedTime(usStart);
        		usElapsedTimeSet(usStart);
            }

            err=m_cam.grabber->Unblock_buffer(pcoBuffIdx); 
            PCO_CHECK_ERROR1(err, "Unblock_buffer");
            if(err!=PCO_NOERROR)
            {
                m_cam.m_pcoData->traceAcq.nrErrors++;
                _msgAbort = "ABORT - Unblock_buffer "; 
                DEB_ALWAYS()  << _msgAbort << DEB_VAR1(err) ;
                continueAcq = false;        
                m_cam.m_wait_flag = true;
                bAbort = true;
                break; // while frames
            }
            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
       		usElapsedTimeSet(usStart);

            if((limaFrameNr % 100) == 0)
            {
                printf("pcoFrameNr [%d] diff[%d]\r",pcoFrameNr,pcoFrameNrTimestamp-pcoFrameNr);
                //printf("\n");
            }
            ++limaFrameNr;

            int _nrStop;
            if((m_cam.m_sync->_getRequestStop(_nrStop))  == stopRequest) 
            {
                _msgAbort = "STOP REQUEST"; DEB_ALWAYS() << _msgAbort;
                //m_sync->_setRequestStop(stopNone);
                continueAcq = false;        
                m_cam.m_wait_flag = true;
                bAbort = true;
            }

        } // while  nb_frames, continue, wait

        m_cam._stopAcq(false);

        printf("\n");
        
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
        usElapsedTimeSet(usStart);
        m_cam._pco_SetRecordingState(0, err);
        PCO_CHECK_ERROR1(err, "SetRecordingState(0)");
        if(err) 
        {
            m_cam.m_pcoData->traceAcq.nrErrors++;
            _msgAbort = "ABORT - SetRecordingState(0)"; DEB_ALWAYS() << _msgAbort;
            bAbort = true;
        }
        
        err=m_cam.grabber->Stop_Acquire(); 
        PCO_CHECK_ERROR1(err, "Stop_Acquire");
        if(err!=PCO_NOERROR)
        {
            m_cam.m_pcoData->traceAcq.nrErrors++;
            _msgAbort = "ABORT - Stop_Acquire()"; DEB_ALWAYS() << _msgAbort;
            bAbort = true;
        }

        err=m_cam.grabber->Free_Framebuffer(); 
        PCO_CHECK_ERROR1(err, "Free_Framebuffer");
        if(err!=PCO_NOERROR)
        {
            m_cam.m_pcoData->traceAcq.nrErrors++;
            _msgAbort = "ABORT - Free_Framebuffer()"; DEB_ALWAYS() << _msgAbort;
            bAbort = true;
        }
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
        usElapsedTimeSet(usStart);

        m_cam.m_pcoData->traceAcq.fnTimestampExit = getTimestamp();
        m_cam.m_pcoData->traceAcq.msStartAcqEnd = msElapsedTime(tStart);
        m_cam.m_pcoData->traceAcq.usTicks[traceAcq_execTimeTot].value = usElapsedTime(usStartTot);      
        m_cam._setStatus(_statusReturn,false);
        
        aLock.lock();
        m_cam.m_wait_flag = true;
        m_cam.m_sync->setStarted(false);        

        if(bAbort)
        {
            DEB_ALWAYS() << _msgAbort;
            {
                Event *ev = new Event(Hardware,Event::Error,Event::Camera,Event::Default, _msgAbort);
                m_cam._getPcoHwEventCtrlObj()->reportEvent(ev);
			}
        }


    } // while quit
    DEB_ALWAYS() << "[exit]" ;
}

//=================================================================================================
// FOR DIMAX - END
//=================================================================================================


//=================================================================================================
// FOR EDGE - BEGIN
//=================================================================================================
void Camera::_AcqThread::threadFunctionEdge()
{
 
    DEB_MEMBER_FUNCT();
    DEF_FNID;

    const char *_msgAbort;
	TIME_USEC tStart;
	long long usStart, usStartTot;

    m_cam.m_pcoData->traceAcq.fnId = fnId;
    DEB_ALWAYS() << "[entry]" ;

    int err, errTot;
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
          DEB_ALWAYS() << "++++++++++++++++++++++++++++++++++Wait " << getTimestamp(Iso);
          m_cam.m_thread_running = false;
          m_cam.m_cond.broadcast();
          m_cam.m_cond.wait();
        } // while wait
        
        DEB_ALWAYS() << "++++++++++++++++++++++++++++++++++Run " << getTimestamp(Iso);
        m_cam.m_thread_running = true;
        if(m_cam.m_quit) return;

        Camera::Status _statusReturn = Camera::Ready;
        bool continueAcq = true;

        bool bNoTimestamp;
        WORD wTimestampMode;
        m_cam._pco_GetTimestampMode(wTimestampMode, err);
        bNoTimestamp = err || !wTimestampMode; 
        
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_execTimeTot].desc = "total execTime";
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_Lima].desc = "Lima execTime";
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].desc = "SDK execTime";

    	msElapsedTimeSet(tStart);
    	usElapsedTimeSet(usStart);
    	usElapsedTimeSet(usStartTot);
        m_cam.m_pcoData->traceAcq.fnId = fnId;
        m_cam.m_pcoData->traceAcq.fnTimestampEntry = getTimestamp();
	

    	m_cam.m_pcoData->traceAcq.msStartAcqStart = msElapsedTime(tStart);
        
    	//m_cam.m_pcoData->traceAcq.usTicks[traceAcq_Lima].desc = "xfer to lima / total execTime";

        m_cam.m_sync->setStarted(true);        

        m_cam.m_sync->getNbFrames(_nb_frames);
        limaFrameNr = 0;            // 0 ..... N-1
    
    	m_cam.m_pcoData->traceAcq.nrImgRequested = _nb_frames;

        m_cam.m_status = Camera::Exposure;
        m_cam.m_cond.broadcast();
        aLock.unlock();
        
        
        err = m_cam.grabber->Get_actual_size(&width,&height,NULL);
        PCO_CHECK_ERROR1(err, "Get_actual_size");
        if(err)  m_cam.m_pcoData->traceAcq.nrErrors++;

        DEB_ALWAYS() << DEB_VAR3(width, height, _nb_frames);

        pcoBuffIdx=1;

        //bool acquireFirst = m_cam._isCameraType(Edge) && m_cam._isInterfaceType(INTERFACE_CAMERALINK);
		bool acquireFirst = m_cam._isCameraType(Edge) && m_cam._isInterfaceType(ifCameralinkAll);
        
        errTot = 0;
        if(acquireFirst)
        {
            DEB_ALWAYS() << "Start_Acquire";
            err = m_cam.grabber->Start_Acquire(_nb_frames);
            PCO_CHECK_ERROR1(err, "Start_Acquire");
            if(err)  {errTot ++; m_cam.m_pcoData->traceAcq.nrErrors++;}

            DEB_ALWAYS() << "_pco_SetRecordingState(1)";
            m_cam._pco_SetRecordingState(1, err);
            PCO_CHECK_ERROR1(err, "SetRecordingState(1)");
            if(err)  {errTot ++; m_cam.m_pcoData->traceAcq.nrErrors++;}
        }
        else
        {
            DEB_ALWAYS() << "_pco_SetRecordingState(1)";
            m_cam._pco_SetRecordingState(1, err);
            PCO_CHECK_ERROR1(err, "SetRecordingState(1)");
            if(err)  {errTot ++; m_cam.m_pcoData->traceAcq.nrErrors++;}

            DEB_ALWAYS() << "Start_Acquire";
            err = m_cam.grabber->Start_Acquire(_nb_frames);
            PCO_CHECK_ERROR1(err, "Start_Acquire");
            if(err)  {errTot ++; m_cam.m_pcoData->traceAcq.nrErrors++;}
        }
        

        if(errTot) 
        {
            m_cam.m_pcoData->traceAcq.nrErrors++;
            //m_cam._setStatus(Camera::Fault,false);        
            //_statusReturn = Camera::Fault;
            continueAcq = false;        
            m_cam.m_wait_flag = true;
        }       
        
        bool bAbort = false; Event::Severity abortSeverity = Event::Error;

        while(  !m_cam.m_wait_flag && 
                continueAcq && 
                ((_nb_frames == 0) || limaFrameNr < _nb_frames))
        {
            
            //DEB_ALWAYS() << DEB_VAR4(m_cam.m_wait_flag, continueAcq, _nb_frames, limaFrameNr);

            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
       		usElapsedTimeSet(usStart);

            pcoFrameNr = limaFrameNr +1;
            limaBuffPtr =  m_cam.m_buffer->_getFrameBufferPtr(limaFrameNr, nb_allocated_buffers);
           //DEB_ALWAYS() << DEB_VAR4(nb_allocated_buffers, _nb_frames, limaFrameNr, limaBuffPtr);

            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_GetImageEx].value += usElapsedTime(usStart);
            		usElapsedTimeSet(usStart);

            m_cam._setStatus(Camera::Readout,false);

            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_Lima].value += usElapsedTime(usStart);
       		usElapsedTimeSet(usStart);

    		usElapsedTimeSet(usStart);
            err=m_cam.grabber->Wait_For_Next_Image(&pcoBuffIdx,10);
            PCO_CHECK_ERROR1(err, "Wait_For_Next_Image");
            if(err!=PCO_NOERROR)
            {
                m_cam.m_pcoData->traceAcq.nrErrors++;
                _msgAbort = "ABORT - Wait_For_Next_Image ";
                DEB_ALWAYS()  << _msgAbort << DEB_VAR1(pcoFrameNr) ;
                continueAcq = false;        
                m_cam.m_wait_flag = true;
                bAbort = true;
                break; // while frames
        }
            
            if(err==PCO_NOERROR)
            {
                err=m_cam.grabber->Check_DMA_Length(pcoBuffIdx);
                PCO_CHECK_ERROR1(err, "Check_DMA_Length");
                if(err!=PCO_NOERROR)
                {
                    m_cam.m_pcoData->traceAcq.nrErrors++;
                    _msgAbort = "ABORT - Check_DMA_Length ";
                    DEB_ALWAYS()  << _msgAbort << DEB_VAR1(err) ;
                    continueAcq = false;        
                    m_cam.m_wait_flag = true;
                    bAbort = true;
                    break; // while frames
                }
            }

            if(err!=PCO_NOERROR)
            {
                _msgAbort = "ABORT - grab_loop Error break loop at image number ";
                DEB_ALWAYS()  << _msgAbort << DEB_VAR1(pcoFrameNr) ;
                //_statusReturn = Camera::Fault;
                //m_cam._setStatus(Camera::Fault,false);
                continueAcq = false;        
                m_cam.m_wait_flag = true;
                continueAcq = false;        
                m_cam.m_wait_flag = true;
                bAbort = true;
                break; // while frames
                //continue;   // while wait
            }
            
            //DEB_ALWAYS()  << "lima image#  " << DEB_VAR1(limaFrameNr) <<" acquired !";

            err=m_cam.grabber->Get_Framebuffer_adr(pcoBuffIdx,&pcoBuffPtr);
            PCO_CHECK_ERROR1(err, "Get_Framebuffer_adr");
            if(err!=PCO_NOERROR)
            {
                m_cam.m_pcoData->traceAcq.nrErrors++;
                _msgAbort = "ABORT - Get_Framebuffer_adr ";
                DEB_ALWAYS()  << _msgAbort << DEB_VAR1(pcoBuffIdx) ;
                continueAcq = false;        
                m_cam.m_wait_flag = true;
                bAbort = true;
                break; // while frames
            }
            if(err==PCO_NOERROR)
            {
                m_cam.grabber->Extract_Image(limaBuffPtr,pcoBuffPtr,width,height);
                //pcoFrameNrTimestamp=image_nr_from_timestamp(limaBuffPtr,0);
                pcoFrameNrTimestamp=image_nr_from_timestamp(limaBuffPtr,0, bNoTimestamp);
                
	            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
        		usElapsedTimeSet(usStart);

                m_cam.m_pcoData->traceAcq.checkImgNrPcoTimestamp = pcoFrameNrTimestamp;
                m_cam.m_pcoData->traceAcq.checkImgNrPco = pcoFrameNr;
                m_cam.m_pcoData->traceAcq.checkImgNrLima = limaFrameNr;
                m_cam.m_pcoData->traceAcq.checkImgNrLima = limaFrameNr;
                m_cam.m_pcoData->traceAcq.msStartAcqNow = msElapsedTime(tStart);


                HwFrameInfoType frame_info;
	            frame_info.acq_frame_nb = limaFrameNr;
	            continueAcq = m_cam.m_buffer->m_buffer_cb_mgr.newFrameReady(frame_info);

	            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_Lima].value += usElapsedTime(usStart);
        		usElapsedTimeSet(usStart);
            }

            err=m_cam.grabber->Unblock_buffer(pcoBuffIdx); 
            PCO_CHECK_ERROR1(err, "Unblock_buffer");
            if(err!=PCO_NOERROR)
            {
                m_cam.m_pcoData->traceAcq.nrErrors++;
                _msgAbort = "ABORT - Unblock_buffer ";
                DEB_ALWAYS()  << _msgAbort << DEB_VAR1(err) ;
                continueAcq = false;        
                m_cam.m_wait_flag = true;
                bAbort = true;
                break; // while frames
            }
            m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
       		usElapsedTimeSet(usStart);

            if((limaFrameNr % 100) == 0)
            {
                printf("pcoFrameNr [%d] diff[%d]\r",pcoFrameNr,pcoFrameNrTimestamp-pcoFrameNr);
                //printf("\n");
            }
            ++limaFrameNr;
            
            int _nrStop;
            if((m_cam.m_sync->_getRequestStop(_nrStop))  == stopRequest) 
            {
                _msgAbort = "STOP REQUEST"; DEB_ALWAYS() << _msgAbort;
                //m_sync->_setRequestStop(stopNone);
                continueAcq = false;        
                m_cam.m_wait_flag = true;
                bAbort = true; abortSeverity = Event::Warning;
            }
        
        } // while  nb_frames, continue, wait

        m_cam._stopAcq(false);

        printf("\n");
        
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
        usElapsedTimeSet(usStart);
        m_cam._pco_SetRecordingState(0, err);
        PCO_CHECK_ERROR1(err, "SetRecordingState(0)");
        if(err)
        {
            m_cam.m_pcoData->traceAcq.nrErrors++;
            _msgAbort = "ABORT - SetRecordingState(0)"; DEB_ALWAYS() << _msgAbort;
            bAbort = true;
        }
 
        err=m_cam.grabber->Stop_Acquire(); 
        PCO_CHECK_ERROR1(err, "Stop_Acquire");
        if(err!=PCO_NOERROR)
        {
            m_cam.m_pcoData->traceAcq.nrErrors++;
            _msgAbort = "ABORT - Stop_Acquire()"; DEB_ALWAYS() << _msgAbort;
            bAbort = true;
        }

        err=m_cam.grabber->Free_Framebuffer(); 
        PCO_CHECK_ERROR1(err, "Free_Framebuffer");
        if(err!=PCO_NOERROR)
        {
            m_cam.m_pcoData->traceAcq.nrErrors++;
            _msgAbort = "ABORT - Free_Framebuffer()"; DEB_ALWAYS() << _msgAbort;
            bAbort = true;
        }
    	m_cam.m_pcoData->traceAcq.usTicks[traceAcq_pcoSdk].value += usElapsedTime(usStart);
        usElapsedTimeSet(usStart);

        m_cam.m_pcoData->traceAcq.fnTimestampExit = getTimestamp();
        m_cam.m_pcoData->traceAcq.msStartAcqEnd = msElapsedTime(tStart);
        m_cam.m_pcoData->traceAcq.usTicks[traceAcq_execTimeTot].value = usElapsedTime(usStartTot);      
        m_cam._setStatus(_statusReturn,false);
        
        aLock.lock();
        m_cam.m_wait_flag = true;
        m_cam.m_sync->setStarted(false);        


        if(bAbort)
        {
            DEB_ALWAYS() << _msgAbort;
            {
                Event *ev = new Event(Hardware,abortSeverity,Event::Camera,Event::Default, _msgAbort);
                m_cam._getPcoHwEventCtrlObj()->reportEvent(ev);
			}
        }

    } // while quit
    DEB_ALWAYS() << "[exit]" ;
}
//=================================================================================================
// FOR EDGE - END
//=================================================================================================

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


//=========================================================================================================
//=========================================================================================================

Camera::Camera(const std::string& camPar)
{
	//DEF_FNID;
	DEB_CONSTRUCTOR();

    DEB_ALWAYS() << "... ::Camera [entry]";

	m_cam_connected=false;
	m_acq_frame_nb = 1;
	m_sync = NULL;
	m_buffer= NULL;
	m_handle = 0;

    camera = NULL;
    grabber = NULL;
    
    m_quit = false;
    m_wait_flag = true;
    
	//delay_time = exp_time = 0;
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

   	m_pcoData->traceAcqClean();

	m_checkImgNr = new CheckImgNr(this);

	// properties: params 
	paramsInit(camPar.c_str());

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

    DEB_ALWAYS()  << "setting the log" ;

    char fnLog[PATH_MAX];
    unsigned long debugSdk = 0;
    //unsigned long long debugSdk = 0x0000F0FF;
	key = "logBits"; ret = paramsGet(key, value);
    debugSdk = ret ? strtol(value, NULL, 16) : 0;

	key = "logPath"; ret = paramsGet(key, value);

    if(ret && debugSdk)
    {
        snprintf(fnLog, PATH_MAX,"%s/pco_%s.log", value, getTimestamp(FnFull));
	    mylog = new CPco_Log(fnLog);
        mylog->set_logbits(debugSdk);

	    printf("+++++++++++++++++++++++++++++++++ logFile[%s] logBits[0x%x]\n", 
	                fnLog,mylog->get_logbits());
    }
    else
    {
	    mylog = new CPco_Log(NULL);
        mylog->set_logbits(0);
	    printf("+++++++++++++++++++++++++++++++++ log [NO LOGS]\n");
    }

    m_pcoData->testCmdMode = 0;
    //paramsGet("testMode", m_pcoData->testCmdMode);
    paramsGet("testMode", value);
    m_pcoData->testCmdMode = strtoull(value, NULL, 0);

	DEB_ALWAYS()
		<< ALWAYS_NL << DEB_VAR1(m_pcoData->version) 
		<< ALWAYS_NL << _checkLogFiles()
		;

	//m_bin.changed = Invalid;
	
	_init();
	
    DEB_ALWAYS() << "... new _AcqThread";
    m_acq_thread = new _AcqThread(*this);
    DEB_ALWAYS() << "... _AcqThread start";
    m_acq_thread->start();
    DEB_ALWAYS() << "... exit";
	
	m_pcoData->timestamps.constructor = getTimestamp();
	
	m_config = FALSE;
	_setStatus(Camera::Ready,true);

    DEB_ALWAYS() << "constructor exit";
}

//=========================================================================================================
//=========================================================================================================
Camera::~Camera()
{
	DEB_DESTRUCTOR();
	DEB_TRACE() << "DESTRUCTOR ...................." ;

    delete m_acq_thread;
    m_acq_thread = NULL;

	m_cam_connected = false;

	reset(RESET_CLOSE_INTERFACE);
}



//==========================================================================================================
// 2017/05/16
//==========================================================================================================

//==========================================================================================================
//==========================================================================================================
void Camera::_waitForRecording(int nrFrames, DWORD &_dwValidImageCnt, DWORD &_dwMaxImageCnt, int &error) 
{
    DEB_MEMBER_FUNCT();
	DEF_FNID;

	static char msgErr[LEN_ERROR_MSG+1];

	int _nrStop;


	char _msg[LEN_MSG + 1];
    __sprintfSExt(_msg, LEN_MSG, "%s> [ENTRY]", fnId);
	_traceMsg(_msg);

	m_pcoData->traceAcq.fnId = fnId;

	const char *msg;
	
	TIME_USEC tStart, tStart0;
	//TIME_UTICKS usStart, usStartTot;

	msElapsedTimeSet(tStart);
	tStart0 = tStart;

	long timeout, timeout0, msNowRecordLoop, msRecord, msXfer, msTotal;
	int nb_acq_frames;
	int requestStop = stopNone;


	//WORD wSegment = _pco_GetActiveRamSegment(); 
	WORD wSegment;  _pco_GetActiveRamSegment(wSegment, error); 
	double msPerFrame = (pcoGetCocRunTime() * 1000.);
	m_pcoData->traceAcq.msImgCoc = msPerFrame;

	DWORD dwMsSleepOneFrame = (DWORD) (msPerFrame/5.0);	// 4/5 rounding
	if(dwMsSleepOneFrame == 0) dwMsSleepOneFrame = 1;		// min sleep

	bool nb_frames_fixed = false;
	int nb_frames; 	m_sync->getNbFrames(nb_frames);
	m_pcoData->traceAcq.nrImgRequested0 = nb_frames;

	m_sync->setAcqFrames(0);

	timeout = timeout0 = (long) (msPerFrame * (nb_frames * 1.3));	// 30% guard
	if(timeout < TOUT_MIN_DIMAX) timeout = TOUT_MIN_DIMAX;
    
	m_pcoData->traceAcq.msTout = m_pcoData->msAcqTout = timeout;
	_dwValidImageCnt = 0;

	m_sync->getExpTime(m_pcoData->traceAcq.sExposure);
	m_sync->getLatTime(m_pcoData->traceAcq.sDelay);

	m_sync->setExposing(pcoAcqRecordStart);

	DEB_ALWAYS() << "waiting for the recording ...";
	while(true) {

		_pco_GetNumberOfImagesInSegment(wSegment, _dwValidImageCnt, _dwMaxImageCnt, error);

		if(error) {
			printf("=== %s [%d]> ERROR %s\n", fnId, __LINE__, "_pco_GetNumberOfImagesInSegment");
			throw LIMA_HW_EXC(Error, "PCO_GetNumberOfImagesInSegment");
		}
		
		//DEB_ALWAYS() << DEB_VAR3(wSegment, _dwValidImageCnt, _dwMaxImageCnt);

		m_pcoData->dwValidImageCnt[wSegment-1] = 
			m_pcoData->traceAcq.nrImgRecorded = _dwValidImageCnt;
		m_pcoData->dwMaxImageCnt[wSegment-1] =
			m_pcoData->traceAcq.maxImgCount = _dwMaxImageCnt;

		m_pcoData->msAcqTnow = msNowRecordLoop = msElapsedTime(tStart);
		m_pcoData->traceAcq.msRecordLoop = msNowRecordLoop;
		
		if( ((DWORD) nb_frames > _dwMaxImageCnt) ){
			nb_frames_fixed = true;
			
			__sprintfSExt(msgErr,LEN_ERROR_MSG, 
				"=== %s [%d]> ERROR INVALID NR FRAMES fixed nb_frames[%d] _dwMaxImageCnt[%d]", 
				fnId, __LINE__, nb_frames, _dwMaxImageCnt);
			printf("%s\n", msgErr);

			m_sync->setExposing(pcoAcqError);
			break;
		}

		if(  (_dwValidImageCnt >= (DWORD) nb_frames)) break;

		if((timeout < msNowRecordLoop) && !m_pcoData->bExtTrigEnabled) { 
			//m_sync->setExposing(pcoAcqRecordTimeout);
			//m_sync->stopAcq();
			m_sync->setExposing(pcoAcqStop);
			printf("=== %s [%d]> TIMEOUT!!! tout[(%ld) 0(%ld)] recLoopTime[%ld ms] lastImgRecorded[%d] nrImgRequested[%d]\n", 
				fnId, __LINE__, timeout, timeout0, msNowRecordLoop, _dwValidImageCnt, nb_frames);
			break;
		}
	
		if((requestStop = m_sync->_getRequestStop(_nrStop))  == stopRequest) {
			m_sync->_setRequestStop(stopNone);
		
			char msg[LEN_TRACEACQ_MSG+1];
				//m_buffer->_setRequestStop(stopProcessing);
				//m_sync->setExposing(pcoAcqStop);
				
			snprintf(msg,LEN_TRACEACQ_MSG, "=== %s> STOP REQ (recording). lastImgRec[%d]\n", fnId, _dwValidImageCnt);
				printf(msg);
				//m_pcoData->traceMsg(msg);
				_traceMsg(msg);
				break;
		}
		//DEB_ALWAYS() << DEB_VAR1(dwMsSleepOneFrame);
        usleep(dwMsSleepOneFrame * 1000);
		//Sleep(dwMsSleepOneFrame);	// sleep 1 frame
	} // while(true)

	m_pcoData->msAcqTnow = msNowRecordLoop = msElapsedTime(tStart);
	m_pcoData->traceAcq.msRecordLoop = msNowRecordLoop;

	msg = _pco_SetRecordingState(0, error);
	if(error) {
		printf("=== %s [%d]> ERROR %s\n", fnId, __LINE__, msg);
		throw LIMA_HW_EXC(Error, "_pco_SetRecordingState");
	}

                      
                        

	if( (requestStop != stopRequest) && (!nb_frames_fixed)) {
		if(m_sync->getExposing() == pcoAcqRecordStart) m_sync->setExposing(pcoAcqRecordEnd);


		_pco_GetNumberOfImagesInSegment(wSegment, _dwValidImageCnt, _dwMaxImageCnt, error);

		if(error) {
			printf("=== %s [%d]> ERROR %s\n", fnId, __LINE__, msg);
			throw LIMA_HW_EXC(Error, "PCO_GetNumberOfImagesInSegment");
		}

		m_pcoData->dwValidImageCnt[wSegment-1] = 
			m_pcoData->traceAcq.nrImgRecorded = _dwValidImageCnt;

		nb_acq_frames = (_dwValidImageCnt < (DWORD) nb_frames) ? _dwValidImageCnt : nb_frames;
		//m_sync->setAcqFrames(nb_acq_frames);

		// dimax recording time
		m_pcoData->msAcqRec = msRecord = msElapsedTime(tStart);
		m_pcoData->traceAcq.msRecord = msRecord;    // loop & stop record
		
		m_pcoData->traceAcq.endRecordTimestamp = m_pcoData->msAcqRecTimestamp = getTimestamp();
		
		m_pcoData->traceAcq.nrImgAcquired = nb_acq_frames;
		m_pcoData->traceAcq.nrImgRequested = nb_frames;

		msElapsedTimeSet(tStart);  // reset for xfer

		if(nb_acq_frames < nb_frames) m_sync->setNbFrames(nb_acq_frames);

//		if(m_buffer->_getRequestStop()) {
//			m_sync->setExposing(pcoAcqStop);
//		} else 
		
		// --- in case of stop request during the record phase, the transfer
		// --- is made to avoid lose the image recorded
		{
			pcoAcqStatus status;

			if(nb_frames_fixed) status = pcoAcqError;
			m_sync->setExposing(status);

		}

	} // if nb_frames_fixed && no stopRequested
	

	//m_sync->setExposing(status);
	m_pcoData->dwMaxImageCnt[wSegment-1] =
			m_pcoData->traceAcq.maxImgCount = _dwMaxImageCnt;

	// traceAcq info - dimax xfer time
	m_pcoData->msAcqXfer = msXfer = msElapsedTime(tStart);
	m_pcoData->traceAcq.msXfer = msXfer;

	m_pcoData->msAcqAll = msTotal = msElapsedTime(tStart0);
	m_pcoData->traceAcq.msTotal= msTotal;

	m_pcoData->traceAcq.endXferTimestamp = m_pcoData->msAcqXferTimestamp = getTimestamp();


	__sprintfSExt(_msg, LEN_MSG, "%s [%d]> [EXIT] imgRecorded[%d] coc[%g] recLoopTime[%ld] "
			"tout[(%ld) 0(%ld)] rec[%ld] xfer[%ld] all[%ld](ms)\n", 
			fnId, __LINE__, _dwValidImageCnt, msPerFrame, msNowRecordLoop, timeout, timeout0, msRecord, msXfer, msTotal);
	_traceMsg(_msg);

	// included in 34a8fb6723594919f08cf66759fe5dbd6dc4287e only for dimax (to check for others)
	m_sync->setStarted(false);


#if 0
	if(requestStop == stopRequest) 
	{
		Event *ev = new Event(Hardware,Event::Error,Event::Camera,Event::CamFault, errMsg);
		m_cam->_getPcoHwEventCtrlObj()->reportEvent(ev);
	}

#endif

}


//=========================================================================================================
// 2018/03/18
//=========================================================================================================

//=========================================================================================================
//=========================================================================================================
int image_nr_from_timestamp(void *buf,int shift, bool bDisable)
{
    unsigned short *b;
    int y;
    int image_nr=0;
    
    if(bDisable) return -1;
    
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

//=================================================================================================
//=================================================================================================
void Camera::_stopAcq(bool waitForThread)
{
    DEB_MEMBER_FUNCT();

    DEB_ALWAYS() << "[entry]" << DEB_VAR2(waitForThread, m_status);

    m_wait_flag = true;
    return;
    
    AutoMutex aLock(m_cond.mutex());
    if(m_status != Camera::Ready)
    {
        // waitForThread == true / while thread is running
        while(waitForThread && m_thread_running) 
        {
            m_wait_flag = true;
            //WaitObject_.Signal();
            m_cond.wait();
        }
        aLock.unlock();

        //  waitForThread == true / thread is not running - Let the acq thread stop the acquisition
        if(waitForThread) 
        {
              DEB_ALWAYS() << "[return]" << DEB_VAR2(waitForThread, m_status);
             return;
        }

        // waitForThread == false / Stop acquisition here
        DEB_TRACE() << "Stop acquisition";
        //Camera_->AcquisitionStop.Execute();

        DEB_ALWAYS() << "[set Ready]" << DEB_VAR1(waitForThread);
       _setStatus(Camera::Ready,false);
    }
    DEB_ALWAYS() << "[return]" << DEB_VAR2(waitForThread, m_status);
}


//=================================================================================================
//=================================================================================================
void Camera::startAcq()
{
    DEB_MEMBER_FUNCT();

	m_acq_frame_nb = -1;
	m_pcoData->pcoError = 0;
	m_pcoData->pcoErrorMsg[0] = 0;

	m_pcoData->traceAcqClean();

	TIME_USEC tStart;
	msElapsedTimeSet(tStart);

	m_sync->getExpTime(m_pcoData->traceAcq.sExposure);
	m_sync->getLatTime(m_pcoData->traceAcq.sDelay);
	m_pcoData->traceAcq.msImgCoc = pcoGetCocRunTime() * 1000.;

	m_pcoData->timestamps.startAcq = getTimestamp();

//=====================================================================
	DEF_FNID;
    //WORD state;
    //HANDLE hEvent= NULL;

	DEB_ALWAYS() << _sprintComment(false, fnId, "[ENTRY]") << _checkLogFiles();

	//int error;
	//const char *msg;

    int iRequestedFrames;

			// live video requested frames = 0
    m_sync->getNbFrames(iRequestedFrames);


	

	//------------------------------------------------- start acquisition

    DEB_ALWAYS() << "[... starting]";
	m_pcoData->traceAcq.msStartAcqStart = msElapsedTime(tStart);

	m_sync->setStarted(true);
	m_sync->setExposing(pcoAcqRecordStart);

    _setStatus(Camera::Exposure, false);
    
	//Start acqusition thread
	AutoMutex aLock(m_cond.mutex());
    m_wait_flag = false;
    m_cond.broadcast();
    DEB_ALWAYS() << "[... starting after mutex]";


    DEB_ALWAYS() << "[exit]";
	return;
}


//=================================================================================================
//=================================================================================================
void Camera::dummySip()
{
	unsigned int ui, width, height;
	unsigned long ul;
	double doub;
	
	int i;
	std::string s;
	if(0) 
	{
        	Camera("");
        	//~Camera();

		startAcq();
		reset(i);

		//talk("");       

		getNbAcquiredFrames();

		getMaxWidthHeight(width,height);


	        getBytesPerPixel(ui); // bytesPerPixel

		getAcqTimeoutRetry(i);   // acqTimeoutRetry
		setAcqTimeoutRetry(i);

        	getAdc(i);    // adc
        	setAdc(i);

        	getAdcMax(i);    // adcMax

		getClTransferParam(s) ;    // clXferPar
		
		getLastError(s) ;   // last error
		
		getTraceAcq(s) ;   // traceAcq
		

		getLastImgRecorded(ul);   // lastImgRecorded
		
		getLastImgAcquired(ul);   // lastImgAcquired
		
		getMaxNbImages(ul);     // maxNbImages
		

		getCocRunTime(doub);    // CocRunTime
		
		getFrameRate(doub);  // frameRate

		getCamInfo(s) ;    // camInfo
		
		getCamType(s) ;   // camType
		
		getVersion(s) ;   // version


		getPcoLogsEnabled(i);      // pcoLogsEnabled

		getPixelRate(i);   // pixelRate
		setPixelRate(i);

		getPixelRateValidValues(s) ;  // pixelRateValidValues
		
		getPixelRateInfo(s) ;       // pixelRateInfo

		getRollingShutter(i);    // rollingShutter
		setRollingShutter(i);

		getRollingShutterInfo(s) ;       // rollingShutterInfo

		getLastFixedRoi(s) ;   // lastFixedRoi

		getCDIMode(i);     // cdiMode
		setCDIMode(i);

}
}

//==========================================================================================================
//==========================================================================================================

void Camera::_set_shutter_rolling_edge(DWORD dwRolling, int &error)
{
	DEB_MEMBER_FUNCT();
	DEB_ALWAYS() << NOT_IMPLEMENTED ;
}
void _pco_shutter_thread_edge(void *argin) { ;}

//=================================================================================================
//=================================================================================================

