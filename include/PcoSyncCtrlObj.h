#ifndef PCOSYNCCTRLOBJ_H
#define PCOSYNCCTRLOBJ_H

#include "Pco.h"

#include "HwSyncCtrlObj.h"
#include "HwInterface.h"

namespace lima
{
  namespace Pco
  {
    class Camera;
    class BufferCtrlObj;

    class SyncCtrlObj : public HwSyncCtrlObj
    {
      DEB_CLASS_NAMESPC(DebModCamera,"SyncCtrlObj","Pco");
    public:
      SyncCtrlObj(Camera*,BufferCtrlObj*);
      virtual ~SyncCtrlObj();

      virtual bool checkTrigMode(TrigMode trig_mode);
      virtual void setTrigMode(TrigMode  trig_mode);
      virtual void getTrigMode(TrigMode& trig_mode);

      virtual void setExpTime(double  exp_time);
      virtual void getExpTime(double& exp_time);

      virtual void setLatTime(double  lat_time);
      virtual void getLatTime(double& lat_time);

      virtual void setNbFrames(int  nb_frames);
      virtual void getNbFrames(int& nb_frames);

      virtual void setNbHwFrames(int  nb_frames);
      virtual void getNbHwFrames(int& nb_frames);

      virtual void getValidRanges(ValidRangesType& valid_ranges);

      void startAcq();
      void stopAcq(bool clearQueue = true);
      
      void getStatus(HwInterface::StatusType&);

    private:
      Camera*		m_cam;
      //tPvHandle&	m_handle;
      TrigMode		m_trig_mode;
      BufferCtrlObj*	m_buffer;
      int		m_nb_frames;
      bool		m_started;
    };

  } // namespace Pco
} // namespace lima

#endif // PCOSYNCCTRLOBJ_H
