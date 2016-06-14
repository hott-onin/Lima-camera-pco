//-----------------------------------------------------------------//
// Name        | Cpco_com_func.cpp           | Type: (*) source    //
//-------------------------------------------|       ( ) header    //
// Project     | pco.camera                  |       ( ) others    //
//-----------------------------------------------------------------//
// Platform    | Linux                                             //
//-----------------------------------------------------------------//
// Environment |                                                   //
//             |                                                   //
//-----------------------------------------------------------------//
// Purpose     | pco.camera - Communication                        //
//-----------------------------------------------------------------//
// Author      | MBL, PCO AG                                       //
//-----------------------------------------------------------------//
// Revision    | rev. 1.03                                         //
//-----------------------------------------------------------------//
// Notes       | In this file are all functions which use the      //
//             | telegram structures for communication with        //
//             | pco devices                                       //
//             |                                                   //
//             |                                                   //
//-----------------------------------------------------------------//
// (c) 2010 - 2014 PCO AG                                          //
// Donaupark 11 D-93309  Kelheim / Germany                         //
// Phone: +49 (0)9441 / 2005-0   Fax: +49 (0)9441 / 2005-20        //
// Email: info@pco.de                                              //
//-----------------------------------------------------------------//

//-----------------------------------------------------------------//
// This program is free software; you can redistribute it and/or   //
// modify it under the terms of the GNU General Public License as  //
// published by the Free Software Foundation; either version 2 of  //
// the License, or (at your option) any later version.             //
//                                                                 //
// This program is distributed in the hope that it will be useful, //
// but WITHOUT ANY WARRANTY; without even the implied warranty of  //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the    //
// GNU General Public License for more details.                    //
//                                                                 //
// You should have received a copy of the GNU General Public       //
// License along with this program; if not, write to the           //
// Free Software Foundation, Inc., 59 Temple Place- Suite 330,     //
// Boston, MA 02111-1307, USA                                      //
//-----------------------------------------------------------------//

//-----------------------------------------------------------------//
// Revision History:                                               //
//-----------------------------------------------------------------//
// Rev.:     | Date:      | Changed:                               //
// --------- | ---------- | ---------------------------------------//
//  1.01     | 08.10.2010 | from Me4 class                         //
//           |            |                                        //
// --------- | ---------- | ---------------------------------------//
//  1.02     | 25.02.2014 | changes for usb cameras                //
//           |            | new functions                          //
// --------- | ---------- | ---------------------------------------//
//  1.03     | 20.06.2015 | bugfixes                               //
//           |            | comments in header for doxygen         //
//-----------------------------------------------------------------//
//  0.0x     | xx.xx.2009 |                                        //
//           |            |                                        //
//-----------------------------------------------------------------//


#include "Cpco_com.h"
#include "Cpco_com_func.h"

DWORD CPco_com::PCO_GetRecordingState(WORD *recstate)
{
  SC2_Recording_State_Response resp;
  SC2_Simple_Telegram com;
  DWORD err=PCO_NOERROR;

  com.wCode=GET_RECORDING_STATE;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(SC2_Simple_Telegram),
    &resp,sizeof(SC2_Recording_State_Response));


  *recstate=resp.wState;
  return err;

}

#define REC_WAIT_TIME 500
DWORD CPco_com::PCO_SetRecordingState(WORD recstate)
{
  WORD g_state,x;
  DWORD err=PCO_NOERROR;

  PCO_GetRecordingState(&g_state);

  if(g_state!=recstate)
  {
    DWORD s,ns;
    SC2_Set_Recording_State com;
    SC2_Recording_State_Response resp;

    com.wCode=SET_RECORDING_STATE;
    com.wState=recstate;
    com.wSize=sizeof(SC2_Set_Recording_State);
    err=Control_Command(&com,sizeof(SC2_Set_Recording_State),
      &resp,sizeof(SC2_Recording_State_Response));

    if(err!=0)
      return err;

    PCO_GetCOCRuntime(&s,&ns);

    ns/=1000000;
    ns+=1;
    ns+=s*1000;

    ns+=REC_WAIT_TIME;
    ns/=50;

    for(x=0;x<ns;x++)
    {
      PCO_GetRecordingState(&g_state);
      if(g_state==recstate)
        break;

      Sleep_ms(50);
    }
    if(x>=ns)
      err=PCO_ERROR_TIMEOUT;
  }
  return err;
}


DWORD CPco_com::PCO_ArmCamera(void)
{
  SC2_Arm_Camera_Response resp;
  SC2_Simple_Telegram com;
  DWORD err=PCO_NOERROR;

  //must increase timeout for arm command
  tab_timeout.command=15000;
  com.wCode=ARM_CAMERA;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(SC2_Simple_Telegram),
    &resp,sizeof(SC2_Arm_Camera_Response));

  tab_timeout.command=PCO_SC2_COMMAND_TIMEOUT;

  return err;
}


DWORD CPco_com::PCO_SetTimestampMode(WORD mode)
{
  SC2_Timestamp_Mode_Response resp;
  SC2_Set_Timestamp_Mode com;
  DWORD err=PCO_NOERROR;

  com.wMode=mode;

  com.wCode=SET_TIMESTAMP_MODE;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  return err;
}


DWORD CPco_com::PCO_GetTimestampMode(WORD *mode)
{
  SC2_Timestamp_Mode_Response resp;
  SC2_Simple_Telegram com;
  DWORD err=PCO_NOERROR;

  com.wCode=GET_TIMESTAMP_MODE;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  *mode=resp.wMode;

  return err;
}


DWORD CPco_com::PCO_GetCOCRuntime(DWORD *s,DWORD *ns)
{
  SC2_COC_Runtime_Response resp;
  SC2_Simple_Telegram com;
  DWORD err=PCO_NOERROR;

  com.wCode=GET_COC_RUNTIME;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    *s=resp.dwtime_s;
    *ns=resp.dwtime_ns;
  }

  return err;
}


DWORD CPco_com::PCO_GetTemperature(SHORT *sCCDTemp,SHORT *sCAMTemp,SHORT *sExtTemp)
{
  SC2_Temperatures_Response resp;
  SC2_Simple_Telegram com;
  DWORD err=PCO_NOERROR;

  com.wCode=GET_TEMPERATURE;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    *sCCDTemp=resp.sCCDtemp;
    *sCAMTemp=resp.sCamtemp;
    *sExtTemp=resp.sPStemp;
  }

  return err;
}


DWORD CPco_com::PCO_RequestImage()
{
  SC2_Request_Image com;
  SC2_Request_Image_Response resp;
  DWORD err;

  //send request to camera
  com.wCode=REQUEST_IMAGE;
  com.wSize=sizeof(SC2_Request_Image);
  err=Control_Command(&com,sizeof(SC2_Request_Image),
    &resp,sizeof(SC2_Request_Image_Response));


  if(err!=PCO_NOERROR)
    writelog(ERROR_M,hdriver,"Send_Request_Image: err 0x%x com 0x%x",err,com.wCode);

  return err;
}


DWORD CPco_com::PCO_GetBitAlignment(WORD *align)
{
  SC2_Bit_Alignment_Response resp;
  SC2_Simple_Telegram com;
  DWORD err=PCO_NOERROR;

  com.wCode=GET_BIT_ALIGNMENT;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  *align=resp.wAlignment;
  return err;

}

DWORD CPco_com::PCO_SetBitAlignment(WORD align)
{
  DWORD err;

  SC2_Set_Bit_Alignment com;
  SC2_Bit_Alignment_Response resp;

  com.wCode=SET_BIT_ALIGNMENT;
  com.wAlignment=align;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}


DWORD CPco_com::PCO_SetDelayExposure(DWORD delay,DWORD expos)
{
  DWORD err;

  SC2_Set_Delay_Exposure com;
  SC2_Delay_Exposure_Response resp;

  com.wCode=SET_DELAY_EXPOSURE_TIME;
  com.wSize=sizeof(SC2_Set_Delay_Exposure);
  com.dwDelay=delay;
  com.dwExposure=expos;
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}

DWORD CPco_com::PCO_GetDelayExposure(DWORD *delay,DWORD *expos)
{
  DWORD err;

  SC2_Simple_Telegram com;
  SC2_Delay_Exposure_Response resp;

  com.wCode=GET_DELAY_EXPOSURE_TIME;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    *delay=resp.dwDelay;
    *expos=resp.dwExposure;
  }

  return err;
}


DWORD CPco_com::PCO_SetTimebase(WORD delay,WORD expos)
{
  DWORD err;

  SC2_Set_Timebase com;
  SC2_Timebase_Response resp;

  com.wCode=SET_TIMEBASE;
  com.wSize=sizeof(SC2_Set_Timebase);
  com.wTimebaseDelay=delay;
  com.wTimebaseExposure=expos;
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}


DWORD CPco_com::PCO_GetTimebase(WORD *delay,WORD *expos)
{
  DWORD err;

  SC2_Simple_Telegram com;
  SC2_Timebase_Response resp;

  com.wCode=GET_TIMEBASE;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    *delay=resp.wTimebaseDelay;
    *expos=resp.wTimebaseExposure;
  }

  return err;
}

DWORD CPco_com::PCO_GetCameraName(void* buf,int length)
{
  DWORD err;
  int len;
  SC2_Camera_Name_Response resp;
  SC2_Simple_Telegram com;

  com.wCode=GET_CAMERA_NAME;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(com),
    &resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    len=(int)strlen(resp.szName);
    if(len<length)
    {
      memcpy(buf,resp.szName,len);
      memset((char*)buf+len,0,length-len);
    }
    else
      memset(buf,0,length);
  }
  return err;
}

DWORD CPco_com::PCO_GetInfo(DWORD typ,void* buf,int length)
{
  DWORD err;
  int len;

  SC2_Get_Info_String_Response resp;
  SC2_Get_Info_String com;

  com.wCode=GET_INFO_STRING;
  com.wSize=sizeof(SC2_Get_Info_String);
  com.dwType=typ;
  err=Control_Command(&com,sizeof(SC2_Get_Info_String),
    &resp,sizeof(SC2_Get_Info_String_Response));


  if(err==PCO_NOERROR)
  {
    len=(int)strlen(resp.szName);
    if(len<length)
    {
      memcpy(buf,resp.szName,len);
      memset((char*)buf+len,0,length-len);
    }
    else
      memset(buf,0,length);
  }
  else
  {
    if(((err&~PCO_ERROR_DEVICE_MASK)==PCO_ERROR_FIRMWARE_NOT_SUPPORTED)&&(typ==1))
    {
      err=PCO_GetCameraName(buf,length);
    }
  }

  return err;
}

DWORD CPco_com::PCO_GetActualSize(DWORD *width,DWORD *height)
{
  DWORD err;

  SC2_Simple_Telegram com;
  SC2_ROI_Response resp;

  com.wCode=GET_ROI;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    *width=resp.wROI_x1-resp.wROI_x0+1;
    *height=resp.wROI_y1-resp.wROI_y0+1;
  }

  return err;
}

DWORD CPco_com::PCO_GetCameraDescriptor(SC2_Camera_Description_Response *descript)
{
  memcpy(descript,&description,sizeof(SC2_Camera_Description_Response));
  return PCO_NOERROR;
}

DWORD CPco_com::PCO_SetADCOperation(WORD num)
{
  DWORD err;

  SC2_Set_ADC_Operation com;
  SC2_ADC_Operation_Response resp;

  com.wCode=SET_ADC_OPERATION;
  com.wMode=num;
  com.wSize=sizeof(SC2_Set_ADC_Operation);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;

}


DWORD CPco_com::PCO_GetPixelRate(DWORD *PixelRate)
{
  DWORD err;

  SC2_Simple_Telegram com;
  SC2_Pixelrate_Response resp;

  com.wCode=GET_PIXELRATE;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  if(err==PCO_NOERROR)
    *PixelRate=resp.dwPixelrate;

  return err;
}


DWORD CPco_com::PCO_SetPixelRate(DWORD PixelRate)
{
  DWORD err;

  SC2_Set_Pixelrate com;
  SC2_Pixelrate_Response resp;

  com.wCode=SET_PIXELRATE;
  com.dwPixelrate=PixelRate;
  com.wSize=sizeof(SC2_Set_Pixelrate);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}


DWORD CPco_com::PCO_SetLut(WORD Identifier,WORD Parameter)
{
  DWORD err;

  SC2_Set_Lookuptable com;
  SC2_Set_Lookuptable_Response resp;

  com.wCode=SET_LOOKUPTABLE;
  com.wIdentifier=Identifier;
  com.wParameter=Parameter;

  com.wSize=sizeof(SC2_Set_Lookuptable);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}


DWORD CPco_com::PCO_GetLut(WORD *Identifier,WORD *Parameter)
{
  SC2_Simple_Telegram com;
  SC2_Set_Lookuptable_Response resp;
  DWORD err;

  com.wCode=GET_LOOKUPTABLE;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    *Identifier=resp.wIdentifier;
    *Parameter=resp.wParameter;
  }
  return err;
}


DWORD CPco_com::PCO_ResetSettingsToDefault()
{
  DWORD err;
  SC2_Simple_Telegram com;
  SC2_Reset_Settings_To_Default_Response resp;

  com.wCode=RESET_SETTINGS_TO_DEFAULT;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}


DWORD CPco_com::PCO_GetTriggerMode(WORD *mode)
{
  DWORD err;
  SC2_Simple_Telegram com;
  SC2_Trigger_Mode_Response resp;

  com.wCode=GET_TRIGGER_MODE;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
    *mode=resp.wMode;

  return err;
}


DWORD CPco_com::PCO_SetTriggerMode(WORD mode)
{
  DWORD err;
  SC2_Set_Trigger_Mode com;
  SC2_Trigger_Mode_Response resp;

  com.wCode=SET_TRIGGER_MODE;
  com.wMode=mode;
  com.wSize=sizeof(SC2_Set_Trigger_Mode);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}


DWORD CPco_com::PCO_ForceTrigger(WORD *trigger)
{
  DWORD err;
  SC2_Simple_Telegram com;
  SC2_Force_Trigger_Response resp;

  com.wCode=FORCE_TRIGGER;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  *trigger=resp.wReturn;
  return err;
}


DWORD CPco_com::PCO_SetCameraToCurrentTime()
{
  DWORD err;
  SC2_Set_Date_Time com;
  SC2_Date_Time_Response resp;

  struct tm st;

#ifdef WIN32
  time_t curtime;
  curtime=time(NULL);
  localtime_s(&st,&curtime);
#else
  struct timeval tv;

  gettimeofday(&tv,NULL);

  localtime_r(&tv.tv_sec,&st);
#endif

  com.wCode=SET_DATE_TIME;
  com.wSize=sizeof(SC2_Set_Date_Time);
  com.bDay=st.tm_mday;
  com.bMonth=st.tm_mon+1;
  com.wYear=st.tm_year+1900;
  com.wHours=st.tm_hour;
  com.bMinutes=st.tm_min;
  com.bSeconds=st.tm_sec;
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}


DWORD CPco_com::PCO_GetSensorSignalStatus(DWORD *status,DWORD *imagecount)
{
  DWORD err;

  SC2_Simple_Telegram com;
  SC2_Camera_Sensor_Signal_Status_Response resp;

  com.wCode=GET_SENSOR_SIGNAL_STATUS;
  com.wSize=sizeof(SC2_Simple_Telegram);

  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  *status=resp.dwStatus;
  *imagecount=resp.dwImageCount;

  return err;
}


DWORD CPco_com::PCO_GetROI(WORD *RoiX0,WORD *RoiY0,WORD *RoiX1,WORD *RoiY1)
{
  DWORD err;

  SC2_Simple_Telegram com;
  SC2_ROI_Response resp;

  com.wCode=GET_ROI;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    *RoiX0=resp.wROI_x0;
    *RoiX1=resp.wROI_x1;
    *RoiY0=resp.wROI_y0;
    *RoiY1=resp.wROI_y1;
  }
  return err;
}


DWORD CPco_com::PCO_SetROI(WORD RoiX0,WORD RoiY0,WORD RoiX1,WORD RoiY1)
{
  DWORD err;

  SC2_Set_ROI com;
  SC2_ROI_Response resp;
  com.wCode=SET_ROI;
  com.wSize=sizeof(SC2_Set_ROI);
  com.wROI_x0=RoiX0;
  com.wROI_x1=RoiX1;
  com.wROI_y0=RoiY0;
  com.wROI_y1=RoiY1;
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}


DWORD CPco_com::PCO_GetBinning(WORD *BinHorz,WORD *BinVert)
{
  DWORD err;

  SC2_Simple_Telegram com;
  SC2_Binning_Response resp;

  com.wCode=GET_BINNING;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    *BinHorz=resp.wBinningx;
    *BinVert=resp.wBinningy;
  }
  return err;
}


DWORD CPco_com::PCO_SetBinning(WORD BinHorz,WORD BinVert)
{
  DWORD err;

  SC2_Set_Binning com;
  SC2_Binning_Response resp;

  com.wCode=SET_BINNING;
  com.wSize=sizeof(SC2_Set_Binning);
  com.wBinningx=BinHorz;
  com.wBinningy=BinVert;

  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}

DWORD CPco_com::PCO_SetDateTime(struct tm *st)
{
  DWORD err;
  SC2_Set_Date_Time com;
  SC2_Date_Time_Response resp;

  com.wCode=SET_DATE_TIME;
  com.wSize=sizeof(SC2_Set_Date_Time);
  com.bDay=st->tm_mday;
  com.bMonth=st->tm_mon+1;
  com.wYear=st->tm_year+1900;
  com.wHours=st->tm_hour;
  com.bMinutes=st->tm_min;
  com.bSeconds=st->tm_sec;
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}


DWORD CPco_com::PCO_GetDelayExposureTime(DWORD *delay,DWORD *expos,WORD *tb_delay,WORD *tb_expos)
{
  DWORD err;

  err=PCO_GetTimebase(tb_delay,tb_expos);
  if(err==PCO_NOERROR)
    err=PCO_GetDelayExposure(delay,expos);
  return err;
}

DWORD CPco_com::PCO_SetDelayExposureTime(DWORD delay,DWORD expos,WORD tb_delay,WORD tb_expos)
{
  DWORD err;

  err=PCO_SetTimebase(tb_delay,tb_expos);
  if(err==PCO_NOERROR)
    err=PCO_SetDelayExposure(delay,expos);

  return err;
}

DWORD CPco_com::PCO_GetCameraSetup(WORD *Type,DWORD *Setup,WORD *Len)
{
  DWORD err=PCO_NOERROR;
  SC2_Simple_Telegram com;
  SC2_Get_Camera_Setup_Response resp;
  WORD len;

  if(*Len > NUMSETUPFLAGS)
    len = *Len;
  else
    len = NUMSETUPFLAGS;
  //   len=max(*Len,NUMSETUPFLAGS);


  com.wCode=GET_CAMERA_SETUP;
  com.wSize=sizeof(SC2_Simple_Telegram);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  *Type=resp.wType;
  if(Setup)
    memcpy(Setup,resp.dwSetupFlags,len*sizeof(DWORD));
  *Len=sizeof(resp.dwSetupFlags)/sizeof(DWORD);
  return err;
}

DWORD CPco_com::PCO_SetCameraSetup(WORD Type, DWORD *Setup,WORD Len)
{
  DWORD err=PCO_NOERROR;
  SC2_Set_Camera_Setup com;
  SC2_Get_Camera_Setup_Response resp;
  WORD len;

  if(Len > NUMSETUPFLAGS)
    len = Len;
  else
    len = NUMSETUPFLAGS;

  //   len=max(Len,NUMSETUPFLAGS);

  com.wCode=SET_CAMERA_SETUP;
  com.wSize=sizeof(SC2_Set_Camera_Setup);
  com.wType=Type;
  memcpy(com.dwSetupFlags,Setup,len*sizeof(DWORD));
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  return err;
}


DWORD CPco_com::PCO_GetHWIOSignalCount(WORD *numSignals)
{
  DWORD err;
  SC2_Simple_Telegram com;
  SC2_Get_Num_HW_IO_Signals_Response resp;

  com.wCode=GET_NUMBER_HW_IO_SIGNALS;
  com.wSize=sizeof(SC2_Simple_Telegram);

  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
    *numSignals=resp.wNumOfSignals;

  return err;
}


DWORD CPco_com::PCO_GetHWIOSignalDescriptor(WORD SignalNum,SC2_Get_HW_IO_Signal_Descriptor_Response *SignalDesc)
{
  DWORD err;
  SC2_Get_HW_IO_Signal_Descriptor com;
  SC2_Get_HW_IO_Signal_Descriptor_Response resp;

  com.wCode=GET_HW_IO_SIGNAL_DESCRIPTION;
  com.wSize=sizeof(SC2_Get_HW_IO_Signal_Descriptor);
  com.wNumSignal=SignalNum;

  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  if(err==PCO_NOERROR)
    memcpy(SignalDesc,&resp,sizeof(SC2_Get_HW_IO_Signal_Descriptor_Response));
  else
    memset(SignalDesc,0,sizeof(SC2_Get_HW_IO_Signal_Descriptor_Response));

  return err;
}


DWORD CPco_com::PCO_GetHWIOSignalDescriptor(WORD SignalNum,char *outbuf,int *size)
{
  DWORD err;
  char buffer[100];
  int done,len,siz,size_in;

  SC2_Get_HW_IO_Signal_Descriptor com;
  SC2_Get_HW_IO_Signal_Descriptor_Response resp;

  com.wCode=GET_HW_IO_SIGNAL_DESCRIPTION;
  com.wSize=sizeof(SC2_Get_HW_IO_Signal_Descriptor);
  com.wNumSignal=SignalNum;

  size_in=0;
  done=0;
  if(outbuf)
  {
    if(size)
      size_in=*size;
    else
      return PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
  }
  else
    siz=1;

  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  if(err==PCO_NOERROR)
  {
    sprintf(buffer,"HWIOSignalDescriptor for Output %d\n",SignalNum);
    len=istrlen(buffer);
    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;

    sprintf(buffer,"possible selections:\n");
    len=istrlen(buffer);
    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;

    if(strlen(resp.szSignalName[0]))
    {
      sprintf(buffer,"0: %s\n",resp.szSignalName[0]);
      len=istrlen(buffer);
      if(outbuf)
      {
        if(done+len+1<=size_in)
        {
          strcat(outbuf,buffer);
          done+=len;
        }
      }
      else
        siz+=len;
    }
    if(strlen(resp.szSignalName[1]))
    {
      sprintf(buffer,"1: %s\n",resp.szSignalName[1]);
      len=istrlen(buffer);
      if(outbuf)
      {
        if(done+len+1<=size_in)
        {
          strcat(outbuf,buffer);
          done+=len;
        }
      }
      else
        siz+=len;
    }
    if(strlen(resp.szSignalName[2]))
    {
      sprintf(buffer,"2: %s\n",resp.szSignalName[2]);
      len=istrlen(buffer);
      if(outbuf)
      {
        if(done+len+1<=size_in)
        {
          strcat(outbuf,buffer);
          done+=len;
        }
      }
      else
        siz+=len;

    }
    if(strlen(resp.szSignalName[3]))
    {
      sprintf(buffer,"3: %s\n",resp.szSignalName[3]);
      len=istrlen(buffer);
      if(outbuf)
      {
        if(done+len+1<=size_in)
        {
          strcat(outbuf,buffer);
          done+=len;
        }
      }
      else
        siz+=len;
    }

    sprintf(buffer,"selectable signal types:\n");
    len=istrlen(buffer);
    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;

    memset(buffer,0,sizeof(buffer));
    if(resp.wSignalTypes&0x01)
      strcat(buffer,"TTL\n");
    if(resp.wSignalTypes&0x02)
      strcat(buffer,"High level TTL\n");
    if(resp.wSignalTypes&0x04)
      strcat(buffer,"Contact mode\n");
    if(resp.wSignalTypes&0x08)
      strcat(buffer,"RS485 differential\n");
    len=istrlen(buffer);
    if(len==0)
      strcat(buffer,"None\n");

    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;

    sprintf(buffer,"selectable signal polarity:\n");
    len=istrlen(buffer);
    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;

    memset(buffer,0,sizeof(buffer));
    if(resp.wSignalPolarity&0x01)
      strcat(buffer,"Low level active\n");
    if(resp.wSignalPolarity&0x02)
      strcat(buffer,"High level active\n");
    if(resp.wSignalPolarity&0x04)
      strcat(buffer,"Rising edge active\n");
    if(resp.wSignalPolarity&0x08)
      strcat(buffer,"Falling edge active\n");
    len=istrlen(buffer);
    if(len==0)
      strcat(buffer,"None\n");

    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;

    sprintf(buffer,"selectable filter settings:\n");
    len=istrlen(buffer);
    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;

    memset(buffer,0,sizeof(buffer));
    if(resp.wSignalPolarity&0x01)
      strcat(buffer,"Off\n");
    if(resp.wSignalPolarity&0x02)
      strcat(buffer,"Medium\n");
    if(resp.wSignalPolarity&0x04)
      strcat(buffer,"High\n");
    len=istrlen(buffer);
    if(len==0)
      strcat(buffer,"None\n");
    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;
  }

  if(size)
    *size=siz;

  return err;
}


DWORD CPco_com::PCO_GetHWIOSignal(WORD SignalNum ATTRIBUTE_UNUSED,WORD *Enabled,WORD *Type,WORD *Polarity,WORD *FilterSetting,WORD *Selected)
{
  DWORD err;
  SC2_Get_HW_IO_Signal com;
  SC2_Get_HW_IO_Signal_Response resp;

  com.wCode=GET_HW_IO_SIGNAL;
  com.wSize=sizeof(SC2_Get_HW_IO_Signal);

  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  if(err==PCO_NOERROR)
  {
    if(Enabled)
      *Enabled=resp.wEnabled;
    if(Type)
      *Type=resp.wType;
    if(Polarity)
      *Polarity=resp.wPolarity;
    if(FilterSetting)
      *FilterSetting=resp.wFilterSetting;
    if(Selected)
      *Selected=resp.wSelected;
  }

  return err;
}

DWORD CPco_com::PCO_SetHWIOSignal(WORD SignalNum,WORD Enabled,WORD Type,WORD Polarity,WORD FilterSetting,WORD Selected)
{
  DWORD err;
  SC2_Set_HW_IO_Signal com;
  SC2_Set_HW_IO_Signal_Response resp;

  com.wCode=SET_HW_IO_SIGNAL;
  com.wSize=sizeof(SC2_Set_HW_IO_Signal);
  com.wNumSignal=SignalNum;
  com.wEnabled=Enabled;
  com.wType=Type;
  com.wPolarity=Polarity;
  com.wFilterSetting=FilterSetting;
  com.wSelected=Selected;

  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
  {
    if(Enabled!=resp.wEnabled)
    {
      err=PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
      printf("returned wrong Enable status %d!=%d\n",Enabled,resp.wEnabled);
    }
    if(Polarity!=resp.wPolarity)
    {
      err=PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
      printf("returned wrong Polarity %d!=%d\n",Polarity,resp.wPolarity);
    }
    if(FilterSetting!=resp.wFilterSetting)
    {
      err=PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
      printf("returned wrong FilterSetting %d!=%d\n",FilterSetting,resp.wFilterSetting);
    }
    if(Selected!=resp.wSelected)
    {
      err=PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
      printf("returned wrong Selected %d!=%d\n",Selected,resp.wSelected);
    }
  }

  return err;
}



DWORD CPco_com::PCO_GetHWIOSignalTiming(WORD SignalNum,WORD Selection,DWORD *type,DWORD *Parameter)
{
  DWORD err;
  SC2_Get_HW_IO_Signal_Timing com;
  SC2_Get_HW_IO_Signal_Timing_Response resp;

  com.wCode=GET_HW_IO_SIGNAL_TIMING;
  com.wSize=sizeof(SC2_Get_HW_IO_Signal_Timing);
  com.wNumSignal=SignalNum;
  com.wSelected=Selection;

  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  if(err==PCO_NOERROR)
  {
    if((SignalNum!=resp.wNumSignal)||(Selection!=resp.wSelected))
      err=PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
    else
    {
      if(type)
        *type=resp.dwType;
      if(Parameter)
        *Parameter=resp.dwParameter;
    }
  }

  return err;
}


DWORD CPco_com::PCO_SetHWIOSignalTiming(WORD SignalNum,WORD Selection,DWORD Parameter)
{
  DWORD err;
  SC2_Set_HW_IO_Signal_Timing com;
  SC2_Set_HW_IO_Signal_Timing_Response resp;


  com.wCode=SET_HW_IO_SIGNAL_TIMING;
  com.wSize=sizeof(SC2_Set_HW_IO_Signal_Timing);
  com.wNumSignal=SignalNum;
  com.wSelected=Selection;
  com.dwParameter=Parameter;
  com.dwReserved[0]=0;
  com.dwReserved[1]=0;
  com.dwReserved[2]=0;
  com.dwReserved[3]=0;
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));
  if(err==PCO_NOERROR)
  {
    if(SignalNum!=resp.wNumSignal)
      err=PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
    if(Selection!=resp.wSelected)
      err=PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
    if(Parameter!=resp.dwParameter)
      err=PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
  }

  return err;
}


DWORD CPco_com::PCO_GetFirmwareVersion(char *outbuf,int *size)
{
  SC2_Simple_Telegram com;
  SC2_Firmware_Versions_Response resp;
  DWORD err;
  char buffer[100];
  int done,len,siz,size_in;
  DWORD old_timeout;

  old_timeout=tab_timeout.command;
  if(tab_timeout.command<400)
    tab_timeout.command=400;

  com.wCode=GET_FIRMWARE_VERSIONS;
  com.wSize=sizeof(SC2_Simple_Telegram);

  size_in=0;
  done=0;
  if(outbuf)
  {
    if(size)
      size_in=*size;
    else
      return PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
  }
  else
    siz=1;

  err=Control_Command(&com,sizeof(SC2_Simple_Telegram),
    &resp,sizeof(SC2_Firmware_Versions_Response));

  if(err==PCO_NOERROR)
  {
    int x;
    sprintf(buffer,"Firmware versions:\n");
    len=istrlen(buffer);
    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;

    for(x=0;x<resp.DeviceNum;x++)
    {
      sprintf(buffer,"%-10s : %03d %03d %04d\n",resp.Device[x].szName,resp.Device[x].bMajorRev,resp.Device[x].bMinorRev,resp.Device[x].wVariant);
      len=istrlen(buffer);
      if(outbuf)
      {
        if(done+len+1<=size_in)
        {
          strcat(outbuf,buffer);
          done+=len;
        }
      }
      else
        siz+=len;
    }
  }
  else
    writelog(ERROR_M,hdriver,"GET_FIRMWARE_VERSIONS: err 0x%x",err);

  if(size)
    *size=siz;

  tab_timeout.command=old_timeout;

  return err;
}


DWORD CPco_com::PCO_GetHardwareVersion(char *outbuf,int *size)
{
  SC2_Simple_Telegram com;
  SC2_Hardware_Versions_Response resp;
  DWORD err;
  char buffer[100];
  int done,len,siz,size_in;
  DWORD old_timeout;


  old_timeout=tab_timeout.command;
  if(tab_timeout.command<400)
    tab_timeout.command=400;


  com.wCode=GET_HARDWARE_VERSIONS;
  com.wSize=sizeof(SC2_Simple_Telegram);

  size_in=0;
  done=0;
  if(outbuf)
  {
    if(size)
      size_in=*size;
    else
      return PCO_ERROR_SDKDLL|SC2_ERROR_SDKDLL|PCO_ERROR_WRONGVALUE;
  }
  else
    siz=1;

  err=Control_Command(&com,sizeof(SC2_Simple_Telegram),
    &resp,sizeof(SC2_Hardware_Versions_Response));

  if(err==PCO_NOERROR)
  {
    int x;
    sprintf(buffer,"Hardware versions:\n");
    len=istrlen(buffer);
    if(outbuf)
    {
      if(done+len+1<=size_in)
      {
        strcat(outbuf,buffer);
        done+=len;
      }
    }
    else
      siz+=len;

    for(x=0;x<resp.BoardNum;x++)
    {
      sprintf(buffer,"%-10s : %04d %04d %04d\n",resp.Board[x].szName,resp.Board[x].wBatchNo,resp.Board[x].wRevision,resp.Board[x].wVariant);
      len=istrlen(buffer);
      if(outbuf)
      {
        if(done+len+1<=size_in)
        {
          strcat(outbuf,buffer);
          done+=len;
        }
      }
      else
        siz+=len;
    }
  }
  else
    writelog(ERROR_M,hdriver,"GET_HARDWARE_VERSIONS: err 0x%x",err);

  if(size)
    *size=siz;

  tab_timeout.command=old_timeout;

  return err;
}


DWORD CPco_com::PCO_GetHealthStatus(unsigned int *dwWarnings,unsigned int *dwErrors,unsigned int *dwStatus)
{
  DWORD err;

  SC2_Simple_Telegram com;
  SC2_Camera_Health_Status_Response resp;

  com.wCode=GET_CAMERA_HEALTH_STATUS;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  // look at the response somehow
  if (err == PCO_NOERROR)
  {
    if (dwWarnings) *dwWarnings = resp.dwWarnings;
    if (dwErrors  ) *dwErrors   = resp.dwErrors;
    if (dwStatus  ) *dwStatus   = resp.dwStatus;
  }
  else
  {
    // clear the bits? Would it be better to leave them alone?
    if (dwWarnings) *dwWarnings = 0;
    if (dwErrors  ) *dwErrors   = 0;
    if (dwStatus  ) *dwStatus   = 0;
  }
  return err;
} // PCO_GetHealthStatus


DWORD CPco_com::PCO_SetSensorFormat(WORD wSensor)
{
  DWORD err;
  SC2_Set_Sensor_Format com;
  SC2_Sensor_Format_Response resp;

  com.wCode=SET_SENSOR_FORMAT;
  com.wSize=sizeof(com);
  com.wFormat=wSensor;
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  return err;
}

DWORD CPco_com::PCO_GetSensorFormat(WORD *wSensor)
{
  DWORD err;
  SC2_Simple_Telegram com;
  SC2_Sensor_Format_Response resp;

  com.wCode=GET_SENSOR_FORMAT;
  com.wSize=sizeof(com);
  err=Control_Command(&com,sizeof(com),&resp,sizeof(resp));

  if(err==PCO_NOERROR)
    *wSensor=resp.wFormat;
  return err;
}

