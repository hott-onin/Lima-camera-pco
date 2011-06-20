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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Exceptions.h"

#include "PcoCamera.h"
#include "PcoSyncCtrlObj.h"
#include "PcoVideoCtrlObj.h"

using namespace lima;
using namespace lima::Pco;


Camera::Camera(const char *ip_addr) :
  m_cam_connected(false),
  m_sync(NULL),
  m_video(NULL)
{
  DEB_CONSTRUCTOR();
  // Init Frames
  m_frame[0].ImageBuffer = NULL;
  m_frame[0].Context[0] = this;
  m_frame[1].ImageBuffer = NULL;
  m_frame[1].Context[0] = this;
  
  m_camera_name[0] = m_sensor_type[0] = '\0';
  unsigned long ip = inet_addr(ip_addr);
  tPvErr error = PvInitialize();
  if(error)
    throw LIMA_HW_EXC(Error, "could not initialize Pco API");

  m_cam_connected = !PvCameraOpenByAddr(ip,ePvAccessMaster,&m_handle);
  if(!m_cam_connected)
    throw LIMA_HW_EXC(Error, "Camera not found!");

  unsigned long psize;
  PvAttrStringGet(m_handle, "CameraName", m_camera_name, 128, &psize);
  PvAttrUint32Get(m_handle, "UniqueId", &m_uid);
  PvAttrUint32Get(m_handle, "FirmwareVerMajor", &m_ufirmware_maj);
  PvAttrUint32Get(m_handle, "FirmwareVerMinor", &m_ufirmware_min);
  PvAttrEnumGet(m_handle, "SensorType", m_sensor_type, 
		sizeof(m_sensor_type), &psize);

  DEB_TRACE() << DEB_VAR3(m_camera_name,m_sensor_type,m_uid);

  PvAttrUint32Get(m_handle, "SensorWidth", &m_maxwidth);
  PvAttrUint32Get(m_handle, "SensorHeight", &m_maxheight);

  DEB_TRACE() << DEB_VAR2(m_maxwidth,m_maxheight);

  error = PvAttrUint32Set(m_handle,"Width",m_maxwidth);
  if(error)
    throw LIMA_HW_EXC(Error,"Can't set image width");
  
  error = PvAttrUint32Set(m_handle,"Height",m_maxheight);
  if(error)
    throw LIMA_HW_EXC(Error,"Can't set image height");
  
  VideoMode localVideoMode;
  if(isMonochrome())
    {
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Mono16");
      localVideoMode = Y16;
    }
  else
    {
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Bayer16");
      localVideoMode = BAYER_RG16;
    }

  if(error)
    throw LIMA_HW_EXC(Error,"Can't set image format");
  
  m_video_mode = localVideoMode;

  error = PvAttrEnumSet(m_handle, "AcquisitionMode", "Continuous");
  if(error)
    throw LIMA_HW_EXC(Error,"Can't set acquisition mode to continuous");
}

Camera::~Camera()
{
  DEB_DESTRUCTOR();

  if(m_cam_connected)
    {
      PvCommandRun(m_handle,"AcquisitionStop");
      PvCaptureEnd(m_handle);
      PvCameraClose(m_handle);
    }
  PvUnInitialize();
  if(m_frame[0].ImageBuffer)
    free(m_frame[0].ImageBuffer);
  if(m_frame[1].ImageBuffer)
    free(m_frame[1].ImageBuffer);
}

/** @brief test if the camera is monochrome
 */
bool Camera::isMonochrome() const
{
  DEB_MEMBER_FUNCT();

  return !strcmp(m_sensor_type,"Mono");
}

VideoMode Camera::getVideoMode() const
{
  DEB_MEMBER_FUNCT();
  DEB_RETURN() << DEB_VAR1(m_video_mode);

  return m_video_mode;
}

void Camera::getCameraName(std::string& name)
{
  DEB_MEMBER_FUNCT();
  DEB_RETURN() << DEB_VAR1(m_camera_name);

  name = m_camera_name;
}
void Camera::setVideoMode(VideoMode aMode)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(aMode);

  ImageType anImageType;
  tPvErr error;
  switch(aMode)
    {
    case Y8:
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Mono8");
      anImageType = Bpp8;
      break;
    case Y16:
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Mono16");
      anImageType = Bpp16;
      break;
    case BAYER_RG8:
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Bayer8");
      anImageType = Bpp8;
      break;
    case BAYER_RG16:
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Bayer16");
      anImageType = Bpp16;
      break;
    default:
      throw LIMA_HW_EXC(InvalidValue,"This video mode is not managed!");
    }
  
  if(error)
    throw LIMA_HW_EXC(Error,"Can't change video mode");
  
  m_video_mode = aMode;
  maxImageSizeChanged(Size(m_maxwidth,m_maxheight),anImageType);
}

void Camera::_allocBuffer()
{
  DEB_MEMBER_FUNCT();

  tPvUint32 imageSize;
  tPvErr error = PvAttrUint32Get(m_handle, "TotalBytesPerFrame", &imageSize);
  if(error)
    throw LIMA_HW_EXC(Error,"Can't get camera image size");

  DEB_TRACE() << DEB_VAR1(imageSize);
  //realloc
  if(!m_frame[0].ImageBuffer || m_frame[0].ImageBufferSize < imageSize)
    {
      //Frame 0
      m_frame[0].ImageBuffer = realloc(m_frame[0].ImageBuffer,
				       imageSize);
      m_frame[0].ImageBufferSize = imageSize;

      //Frame 1
      m_frame[1].ImageBuffer = realloc(m_frame[1].ImageBuffer,
				       imageSize);

      m_frame[1].ImageBufferSize = imageSize;
    }
}
/** @brief start the acquisition.
    must have m_video != NULL and previously call _allocBuffer
*/
void Camera::startAcq()
{
  DEB_MEMBER_FUNCT();

  m_continue_acq = true;
  m_acq_frame_nb = 0;
  tPvErr error = PvCaptureQueueFrame(m_handle,&m_frame[0],_newFrameCBK);

  int requested_nb_frames;
  m_sync->getNbFrames(requested_nb_frames);
  bool isLive;
  m_video->getLive(isLive);

  if(!requested_nb_frames || requested_nb_frames > 1 || isLive)
    error = PvCaptureQueueFrame(m_handle,&m_frame[1],_newFrameCBK);
}

void Camera::reset()
{
  DEB_MEMBER_FUNCT();
  //@todo maybe something to do!
}

void Camera::_newFrameCBK(tPvFrame* aFrame)
{
  DEB_STATIC_FUNCT();
  Camera *aCamera = (Camera*)aFrame->Context[0];
  aCamera->_newFrame(aFrame);
}

void Camera::_newFrame(tPvFrame* aFrame)
{
  DEB_MEMBER_FUNCT();

  if(!m_continue_acq) return;

  if(aFrame->Status != ePvErrSuccess)
    {
      if(aFrame->Status != ePvErrCancelled)
	{
	  DEB_WARNING() << DEB_VAR1(aFrame->Status);
	  PvCaptureQueueFrame(m_handle,aFrame,_newFrameCBK);
	}
      return;
    }
  
  int requested_nb_frames;
  m_sync->getNbFrames(requested_nb_frames);
  bool isLive;
  m_video->getLive(isLive);
  ++m_acq_frame_nb;

  bool stopAcq = false;
  if(isLive || !requested_nb_frames || m_acq_frame_nb < (requested_nb_frames - 1))
    {
      if(isLive || requested_nb_frames ||
	 m_acq_frame_nb < (requested_nb_frames - 2))
	tPvErr error = PvCaptureQueueFrame(m_handle,aFrame,_newFrameCBK);
    }
  else
    stopAcq = true;
  
  VideoMode mode;
  switch(aFrame->Format)
    {
    case ePvFmtMono8: 	mode = Y8;		break;
    case ePvFmtMono16: 	mode = Y16;		break;
    case ePvFmtBayer8: 	mode = BAYER_RG8;	break;
    case ePvFmtBayer16: mode = BAYER_RG16;	break;
    default:
      DEB_ERROR() << "Format not supported: " << DEB_VAR1(aFrame->Format);
      m_sync->stopAcq();
      return;
    }

  m_continue_acq =  m_video->callNewImage((char*)aFrame->ImageBuffer,
					  aFrame->Width,
					  aFrame->Height,
					  mode);
  if(stopAcq || !m_continue_acq)
    m_sync->stopAcq(false);
}