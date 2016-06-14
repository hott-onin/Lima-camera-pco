//-----------------------------------------------------------------//
// Name        | Cpco_log.cpp                | Type: (*) source    //
//-------------------------------------------|       ( ) header    //
// Project     | pco.camera                  |       ( ) others    //
//-----------------------------------------------------------------//
// Platform    | LINUX                                             //
//-----------------------------------------------------------------//
// Environment |                                                   //
//-----------------------------------------------------------------//
// Purpose     | pco.camera - Logging class                        //
//-----------------------------------------------------------------//
// Author      | MBL, PCO AG                                       //
//-----------------------------------------------------------------//
// Revision    | rev. 0.01 rel. 0.00                               //
//-----------------------------------------------------------------//
// Notes       | Common functions                                  //
//             |                                                   //
//             |                                                   //
//-----------------------------------------------------------------//
// (c) 2010 PCO AG * Donaupark 11 *                                //
// D-93309      Kelheim / Germany * Phone: +49 (0)9441 / 2005-0 *  //
// Fax: +49 (0)9441 / 2005-20 * Email: info@pco.de                 //
//-----------------------------------------------------------------//


//-----------------------------------------------------------------//
// Revision History:                                               //
//-----------------------------------------------------------------//
// Rev.:     | Date:      | Changed:                               //
// --------- | ---------- | ---------------------------------------//
//  0.01     | 16.06.2010 |  new file                              //
//-----------------------------------------------------------------//
//  0.0x     | xx.xx.200x |                                        //
//-----------------------------------------------------------------//

//#include "stdafx.h"

//#include "pco_includes.h"
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <ctype.h>
#include <time.h>

#include <sys/time.h>


#include "Cpco_log.h"

const char crlf[3]={0x0d,0x0a,0x00};

/*
CPco_Log::CPco_Log(int logbits)
{
hflog=-1;
log_bits=logbits;
}
*/

CPco_Log::CPco_Log(const char *name)
{

#if defined _DEBUG
  log_bits=0x0003FFFF; //ERROR_M|INIT_M|INTERNAL_1_M|INTERNAL_2_M|TIME_M|TIME_MD;
#else
  log_bits=ERROR_M|INIT_M;
#endif

  hflog=(PCO_HANDLE)NULL;
  QueryPerformanceFrequency((LARGE_INTEGER*)&lpFrequency);
  lpPCount1=lpPCount2=0;
  QueryPerformanceCounter((LARGE_INTEGER*)&lpPCount1);


  if(name==NULL)
    sprintf(logname,"pco_log.log");
  else
    strcpy(logname,name);

  hflog=open(logname,O_CREAT|O_WRONLY|O_TRUNC,0666);
  if(hflog!=-1)
  {
    char fname[MAX_PATH+100];
    SYSTEMTIME  st;
    DWORD z;

    GetLocalTime(&st);
    sprintf(fname,"%s logfile started\r\n"
      "%02d:%02d:%02d %02d.%02d.%04d",logname,st.wHour,st.wMinute,st.wSecond,st.wDay,st.wMonth,st.wYear);

    strcat(fname,crlf);
    strcat(fname,crlf);

    lseek(hflog,0,SEEK_END);
    z=(DWORD)strlen(fname);
    write(hflog,fname,z);
  }
}

CPco_Log::~CPco_Log()
{
  if(hflog)
  {
    char fname[MAX_PATH+100];
    SYSTEMTIME  st;
    DWORD z;

    GetLocalTime(&st);
    sprintf(fname,"Log ended %02d:%02d.%02d %02d.%02d.%04d"
      ,st.wHour,st.wMinute,st.wSecond,st.wDay,st.wMonth,st.wYear);
    strcat(fname,crlf);
    strcat(fname,crlf);

    lseek(hflog,0,SEEK_END);
    z=(DWORD)strlen(fname);
    write(hflog,fname,z);

    close(hflog);
    hflog=-1;
  }
}


void CPco_Log::writelog(DWORD lev,const char *str,...)
{
  va_list arg;

  QueryPerformanceCounter((LARGE_INTEGER*)&lpPCount2);

  if(lev==0)
    lev+=ERROR_M;
  if(lev&log_bits)
  {
    char buf[300];
    SYSTEMTIME  st;
    DWORD z;

    z=0;
    memset(buf,0,300);

    if(log_bits&TIME_M)
    {
      GetLocalTime(&st);
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"%02d:%02d:%02d.%03d ",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
    }
    if(log_bits&TIME_MD)
    {
      double time;
      time=(double)(lpPCount2-lpPCount1);
      time=time/lpFrequency;
      time*=1000; //ms
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"%6d.%03dms ",(int)time%1000000,(int)((time-(int)time)*1000));
    }

    if(lev&ERROR_M)
    {
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"ERROR ");
    }

    va_start(arg,str);
    z=(DWORD)strlen(buf);
    vsprintf(buf+z,str,arg);
    va_end(arg);

    if(hflog)
    {
      strcat(buf,crlf);
      lseek(hflog,0,SEEK_END);
      z=(DWORD)strlen(buf);
      write(hflog,buf,z);
    }
    lpPCount1=lpPCount2;
 }
}

void CPco_Log::writelog(DWORD lev,const char *str,  va_list arg)
{
  QueryPerformanceCounter((LARGE_INTEGER*)&lpPCount2);

  if(lev==0)
    lev+=ERROR_M;
  if(lev&log_bits)
  {
    char buf[300];
    SYSTEMTIME  st;
    DWORD z;

    z=0;
    memset(buf,0,300);

    if(log_bits&TIME_M)
    {
      GetLocalTime(&st);
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"%02d:%02d:%02d.%03d ",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
    }
    if(log_bits&TIME_MD)
    {
      double time;
      time=(double)(lpPCount2-lpPCount1);

      time=time/lpFrequency;
      time*=1000; //ms
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"%6d.%03dms ",(int)time%1000000,(int)((time-(int)time)*1000));
    }

    if(lev&ERROR_M)
    {
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"ERROR ");
    }

    z=(DWORD)strlen(buf);
    vsprintf(buf+z,str,arg);

    if(hflog)
    {
      strcat(buf,crlf);
      lseek(hflog,0,SEEK_END);
      z=(DWORD)strlen(buf);
      write(hflog,buf,z);
    }
    lpPCount1=lpPCount2;
  }
}


void CPco_Log::writelog(DWORD lev,PCO_HANDLE hdriver,const char *str,...)
{
  va_list arg;

  QueryPerformanceCounter((LARGE_INTEGER*)&lpPCount2);

  if(lev==0)
    lev+=ERROR_M;
  if(lev&log_bits)
  {
    char buf[300];
    SYSTEMTIME  st;
    DWORD z;

    z=0;
    memset(buf,0,300);

    if(log_bits&TIME_M)
    {
      GetLocalTime(&st);
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"%02d:%02d:%02d.%03d ",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
    }
    if(log_bits&TIME_MD)
    {
      double time;
      time=(double)(lpPCount2-lpPCount1);

      time=time/lpFrequency;
      time*=1000; //ms
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"%6d.%03dms ",(int)time%1000000,(int)((time-(int)time)*1000));
    }

    z=(DWORD)strlen(buf);
    sprintf(buf+z,"0x%04x ",(DWORD) hdriver);

    if(lev&ERROR_M)
    {
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"ERROR ");
    }

    va_start(arg,str);
    z=(DWORD)strlen(buf);
    vsprintf(buf+z,str,arg);
    va_end(arg);

    if(hflog)
    {
      strcat(buf,crlf);
      lseek(hflog,0,SEEK_END);
      z=(DWORD)strlen(buf);
      write(hflog,buf,z);
    }
    lpPCount1=lpPCount2;
 }
}


void CPco_Log::writelog(DWORD lev,PCO_HANDLE hdriver,const char *str,va_list args)
{
  QueryPerformanceCounter((LARGE_INTEGER*)&lpPCount2);

  if(lev==0)
    lev+=ERROR_M;
  if(lev&log_bits)
  {
    char buf[300];
    SYSTEMTIME  st;
    DWORD z;

    z=0;
    memset(buf,0,300);

    if(log_bits&TIME_M)
    {
      GetLocalTime(&st);
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"%02d:%02d:%02d.%03d ",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
    }
    if(log_bits&TIME_MD)
    {
     double time;
     time=(double)(lpPCount2-lpPCount1);
 
     time=time/lpFrequency;
     time*=1000; //ms
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"%6d.%03dms ",(int)time%1000000,(int)((time-(int)time)*1000));
    }

    z=(DWORD)strlen(buf);
    sprintf(buf + z, "0x%04x ", (DWORD) hdriver);

    if(lev&ERROR_M)
    {
      z=(DWORD)strlen(buf);
      sprintf(buf+z,"ERROR ");
    }

    z=(DWORD)strlen(buf);
    vsprintf(buf+z,str,args);

    if(hflog)
    {
      strcat(buf,crlf);
      lseek(hflog,0,SEEK_END);
      z=(DWORD)strlen(buf);
      write(hflog,buf,z);
    }

    lpPCount1=lpPCount2;
  }
}



void CPco_Log::set_logbits(DWORD logbit)
{
  log_bits=logbit;
}

DWORD CPco_Log::get_logbits(void)
{
  return log_bits;
}


void CPco_Log::start_time_mess(void)
{
  QueryPerformanceCounter((LARGE_INTEGER*)&stamp1);
}

double CPco_Log::stop_time_mess(void)
{
  double time;
  QueryPerformanceCounter((LARGE_INTEGER*)&stamp2);
  time=(double)(stamp2-stamp1);
  time=time/lpFrequency;
  time*=1000; //ms
  return time;
}

#ifndef WIN32
void CPco_Log::GetLocalTime(SYSTEMTIME* st) {
      time_t t;
      struct tm* timeinfo;
      time(&t);
      timeinfo = localtime(&t);
      st->wMilliseconds = 0; //ms is not really needed and not used

      st->wSecond = timeinfo->tm_sec;
      st->wMinute = timeinfo->tm_min;
      st->wHour = timeinfo->tm_hour;
      st->wDay = timeinfo->tm_mday;
      st->wDayOfWeek = timeinfo->tm_wday;
      st->wMonth = timeinfo->tm_mon;
      st->wYear = timeinfo->tm_year+1900;
}

void CPco_Log::QueryPerformanceCounter(__int64* lpCount)
{
    struct timeval t;

    gettimeofday(&t,NULL);
    *lpCount = t.tv_sec*1000000LL + t.tv_usec; //time in useconds
/*
    timespec t;
    long long nanotime;
    clock_gettime(CLOCK_MONOTONIC,&t);

    nanotime = t.tv_sec*1000000000LL + t.tv_nsec; //time in nanoseconds
    *lpCount = nanotime / 1000 + (nanotime % 1000 >= 500); //round up halves
*/
}

void CPco_Log::QueryPerformanceFrequency(__int64* frequency)
{
    *frequency = 1000000;
}

#endif
