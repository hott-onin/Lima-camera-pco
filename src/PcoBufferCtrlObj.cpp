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

#include "lima/Exceptions.h"
#include "PcoBufferCtrlObj.h"
#include "PcoSyncCtrlObj.h"
#include "PcoCamera.h"
#include <sys/timeb.h>
#include <time.h>

#undef DEBUG_XFER_IMAG
#define COMPILE_WAIT_CONDITION
#undef COMPILEIT
#define USING_PCO_ALLOCATED_BUFFERS
#define EVENT_WAIT_TMOUT_MS 10000

#define THROW_LIMA_HW_EXC(e, x)  { \
	printf("========*** LIMA_HW_EXC %s\n", x ); \
			throw LIMA_HW_EXC(e, x); \
} 


using namespace lima;
using namespace lima::Pco;

//=========================================================================================================
const char* _timestamp_pcobufferctrlobj() {return ID_TIMESTAMP_M ;}
//=========================================================================================================

//=========================================================================================================
//=========================================================================================================
BufferCtrlObj::BufferCtrlObj(Camera *cam) :
  m_handle(cam->getHandle()),
  m_cam(cam),
  m_pcoData(m_cam->_getPcoData())
  //m_status(0)
{
  DEB_CONSTRUCTOR();


	//SoftBufferCtrlObj::Sync &m_bufferSync = *getBufferSync(cond);
	m_bufferSync = getBufferSync(cond);

  //----------------------------------------------- initialization buffers & creating events
  for(int i=0; i < PCO_BUFFER_NREVENTS; i++) {
		m_allocBuff.pcoAllocBufferNr[i] = -1;
		m_allocBuff.pcoAllocBufferPtr[i]	= NULL;
		m_allocBuff.dwPcoAllocBufferSize[i]	= 0;

		m_allocBuff.limaAllocBufferPtr[i]	= NULL;
		m_allocBuff.dwLimaAllocBufferSize[i]	= 0;
  }
	m_allocBuff.pcoAllocBufferDone = false;
	m_allocBuff.createEventsDone = false;

}
//=========================================================================================================
//=========================================================================================================
void BufferCtrlObj::prepareAcq()
{
	DEB_MEMBER_FUNCT();

	//_pcoAllocBuffers();

	FrameDim dim;

	getFrameDim(dim);

	m_ImageBufferSize = dim.getMemSize();

	if(m_cam->_getDebug(DBG_BUFF)) {DEB_ALWAYS() << DEB_VAR2(dim, m_ImageBufferSize);}
	DEB_TRACE() << DEB_VAR1(m_ImageBufferSize);

}

//=========================================================================================================
//=========================================================================================================
void BufferCtrlObj::startAcq()
{
	DEB_MEMBER_FUNCT();
	std::string name;

	StdBufferCbMgr& buffer_mgr = m_buffer_cb_mgr;
	buffer_mgr.setStartTimestamp(Timestamp::now());
}




//===================================================================================================================
//===================================================================================================================
void *BufferCtrlObj::_getFrameBufferPtr(int lima_buffer_nb, int &nb_allocated_buffers)
{
	DEB_MEMBER_FUNCT();
	DEF_FNID;
	const char *msg;
	double timeout = 30;
	StdBufferCbMgr& buffer_mgr = m_buffer_cb_mgr;


	Sync::Status status;
	AutoMutex lock(cond.mutex());

    void *myBuffer = NULL;
    
	Timestamp t0 = Timestamp::now();

	for (bool doit = true; doit; ) {
		double wait_timeout = timeout - double(Timestamp::now() - t0);
		if (wait_timeout <= 0) {
				msg = "=== Sync wait INTERRUPTED + TIMEOUT === ";
				DEB_ALWAYS() << msg << DEB_VAR2(lima_buffer_nb , timeout);
				return NULL;
		}

		status = m_bufferSync->wait(lima_buffer_nb, wait_timeout);
		if(m_cam->_getDebug(DBG_LIMABUFF)) {DEB_ALWAYS() << DEB_VAR3(lima_buffer_nb, timeout, status);}

		switch(status){
			case Sync::AVAILABLE:
				myBuffer = buffer_mgr.getFrameBufferPtr(lima_buffer_nb);
				buffer_mgr.getNbBuffers(nb_allocated_buffers);
				doit = false;
				break;
			case Sync::TIMEOUT:
				msg = "=== Sync wait TIMEOUT === ";
				DEB_ALWAYS() << msg << DEB_VAR2(lima_buffer_nb , timeout);
				return NULL;
                break;
                				
			case Sync::INTERRUPTED:
				msg = "=== Sync wait INTERRUPTED === ";
				DEB_ALWAYS() << msg << DEB_VAR2(lima_buffer_nb , timeout);
				break;

			default:
				msg = "=== Sync wait UNKNOWN STATUS === ";
				DEB_ALWAYS() << msg << DEB_VAR2(lima_buffer_nb , timeout);
		}

	}
	
    return myBuffer;
}	

