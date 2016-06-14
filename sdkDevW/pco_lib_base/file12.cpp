//-----------------------------------------------------------------//
// Name        | file12.cpp                  | Type: (*) source    //
//-------------------------------------------|       ( ) header    //
// Project     | pco.camera                  |       ( ) others    //
//-----------------------------------------------------------------//
// Platform    | Linux                                             //
//-----------------------------------------------------------------//
// Environment |                                                   //
//             |                                                   //
//-----------------------------------------------------------------//
// Purpose     | library for b16 and tif files                     //
//-----------------------------------------------------------------//
// Author      | MBL, PCO AG                                       //
//-----------------------------------------------------------------//
// Revision    | rev. 1.01 rel. 0.00                               //
//-----------------------------------------------------------------//
// Notes       | must be linked together with libpcolog            //
//             |                                                   //
//             |                                                   //
//-----------------------------------------------------------------//
// (c) 2012 PCO AG * Donaupark 11 *                                //
// D-93309      Kelheim / Germany * Phone: +49 (0)9441 / 2005-0 *  //
// Fax: +49 (0)9441 / 2005-20 * Email: info@pco.de                 //
//-----------------------------------------------------------------//

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include <unistd.h>

#include "defs.h"
#include "PCO_err.h"
#include "Cpco_log.h"

#include "file12.h"

#define FPVERS "1.20"
#define FPVER  120

#define FILEISOK        1
#define FILEISMACFORMAT 2


CPco_Log *file12log=NULL;
FILE *file12log_out=NULL;

char pcotiff_text[70]="";


static void writelog(DWORD lev,const char *str,...)
{
  char *txt;
  txt=new char[1000];
  va_list arg;
  va_start(arg,str);
  vsprintf(txt,str,arg);
  if(file12log)
   file12log->writelog(lev,txt);
  else if(file12log_out)
   fprintf(stderr,"%s\n",txt);

  va_end(arg);
  delete[] txt;
}



extern "C" int get_b16_fileparams(char *filename,int *width,int *height,int *colormode)
{
  unsigned char *cptr;
  unsigned char *c1;
  unsigned int *b1;
  int hfread;
  int e;

  cptr=(unsigned char *)malloc(200);

  hfread = open(filename,O_RDONLY);
  if(hfread == -1)
  {
   free(cptr);
   writelog(ERROR_M,"get_b16_fileparams() Open file %s failed",filename);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  e=read(hfread,cptr,200);

  if(e<200)
  {
   close(hfread);
   free(cptr);
   writelog(ERROR_M,"get_b16_fileparams() read header from %s failed",filename);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  if((*c1 != 'P')||(*(c1+1) != 'C')||(*(c1+2) != 'O')||(*(c1+3) != '-'))
  if(e<200)
  {
   close(hfread);
   free(cptr);
   writelog(ERROR_M,"get_b16_fileparams() Missing b16 intro 'PCO-' in %s",filename);
   return PCO_ERROR_NOFILE;
  }

/* read FILEHEADER width and height
*/
  c1=cptr+12;                          /*Width*/
  b1=(unsigned int*)c1;
  *width   = *b1;

  c1=cptr+16;                          /*Height*/
  b1=(unsigned int*)c1;
  *height  = *b1;

  c1=cptr+20;

  if(*c1!='C')
  {
   switch(*b1)
   {
      case 0:
      case 1:
       *colormode  = *b1;
       break;

      case -1:
       b1++;
       *colormode  = *b1;
       break;

      default:
       *colormode=0;
       break;
   }
  }
  else
  {
    c1=cptr+35;
    if(*c1=='5')
     *colormode=1;
    else
     *colormode=0;
  }

  close(hfread);
  free(cptr);

  writelog(INFO_M,"get_b16_fileparams() %s done",filename);

  return PCO_NOERROR;
}

int get_b16_lutset(char *filename)
{
  unsigned char *cptr;
  unsigned char *c1;
  unsigned int *b1;
  int hfread;
  int e;

  cptr=(unsigned char *)malloc(400);

  hfread = open(filename,O_RDONLY);
  if(hfread == -1)
  {
   free(cptr);
   writelog(ERROR_M,"get_b16_lutset() Open file %s failed",filename);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  e=read(hfread,cptr,400);

  if(e<400)
  {
   close(hfread);
   free(cptr);
   writelog(ERROR_M,"get_b16_lutset() read header from %s failed",filename);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  if((*c1 != 'P')||(*(c1+1) != 'C')||(*(c1+2) != 'O')||(*(c1+3) != '-'))
  {
   close(hfread);
   free(cptr);
   writelog(ERROR_M,"get_b16_lutset() Missing b16 intro 'PCO-' in %s",filename);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr+20;                          /* colormode */
  b1=(unsigned int*)c1;
  if(*b1!=0xFFFFFFFF)
   return PCO_NOERROR;
  else
   {
    b1++;
    b1++;   //colormode

/*
    err=setdialogbw(*b1   ,*(b1+1),*(b1+2));
//              bwmin,bwmax ,bwlin

    if(err==NOERR)
     err=setdialogcol(*(b1+3),*(b1+4),*(b1+5),*(b1+6),*(b1+7),*(b1+8),*(b1+9));
//                        rmin  ,rmax  ,gmin  ,gmax  ,bmin  ,bmax  ,collin
*/
   }

  close(hfread);
  free(cptr);

  return PCO_NOERROR;
}

extern "C" int read_b16(char *filename, void *buf)
{
  unsigned char *cptr;
  unsigned char *c1;
  unsigned int *b1;
  int hfread;
  int e,z;
  int fsize;
  int headerl;
  int err;

  cptr=(unsigned char *)malloc(200);

  hfread = open(filename,O_RDONLY);
  if(hfread == -1)
  {
   free(cptr);
   writelog(ERROR_M,"read_b16() Open file %s failed",filename);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  e=read(hfread,cptr,200);

  if(e<200)
  {
   close(hfread);
   free(cptr);
   writelog(ERROR_M,"read_b16() File read header from %s failed",filename);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  if((*c1 != 'P')||(*(c1+1) != 'C')||(*(c1+2) != 'O')||(*(c1+3) != '-'))
  {
   close(hfread);
   free(cptr);
   writelog(ERROR_M,"read_b16() Missing b16 intro 'PCO-' in %s",filename);
   return PCO_ERROR_NOFILE;
  }

/*read FILEHEADER*/
  c1=cptr+4;                  /* filesize */
  b1=(unsigned int*)c1;
  fsize=*b1;

  c1=cptr+8;                  /* headerlength */
  b1=(unsigned int*)c1;
  headerl =*b1;

  z=fsize-headerl;

  err=PCO_NOERROR;
  
/* read data */
  lseek(hfread,headerl,SEEK_SET);

  e=read(hfread,buf,z);

  if(e<z)
  {
   writelog(ERROR_M,"read_b16() read %d bytes from %s failed",z,filename);
   err= PCO_ERROR_NOFILE;
  }

  close(hfread);
  free(cptr);

  writelog(INFO_M,"read_b16() %s done",filename);
  return err;
}

extern "C" int store_b16(char *filename,int width,int height,
              int colormode,void *buf)
{
  unsigned char *cptr;
  unsigned char *c1;
  unsigned int *b1;
  int hfstore;
  int e,z;
  int headerl;

//  printf("store %s w%d h%d size %d buf%p\n",filename,width,height,width*height*2,buf);

  cptr=(unsigned char *)malloc(2000);

  if(cptr==NULL)
  {
   writelog(ERROR_M,"store_b16() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }

  hfstore = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0666);//S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if(hfstore == -1)
  {
   writelog(ERROR_M,"store_b16() create file %s failed",filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  headerl = 128;

  c1=cptr;
  *c1++ = 'P';           //Begin PCO-Header PCO-
  *c1++ = 'C';
  *c1++ = 'O';
  *c1++ = '-';

  b1 = (unsigned int*)c1;

/* Daten fuer Header */
  *b1++ = (width*height*2)+headerl;
  *b1++ = headerl;
  *b1++ = width;
  *b1++ = height;
  *b1++ = 0;

  *b1++ = colormode;
/*
  getdialogbw(b1   ,(b1+1),(b1+2));
//              bwmin,bwmax ,bwlin
  getdialogcol((b1+3),(b1+4),(b1+5),(b1+6),(b1+7),(b1+8),(b1+9));
//               rmin  ,rmax  ,gmin  ,gmax  ,bmin  ,bmax  ,collin

  b1+=10;
*/
/* Fill Header */
  c1=(unsigned char *)b1;
  for(;c1<cptr+128;)
   *c1++=0;

  z=headerl;
  e=write(hfstore,(void *)cptr,z);
  if(e==-1)
  {
   writelog(ERROR_M,"store_b16() write header to %s failed",filename);
   close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  z=width*height*2;

  e=write(hfstore,(void *)buf,z);
  if(e == -1)
  {
   writelog(ERROR_M,"store_b16() write %d bytes to %s failed",z,filename);
   close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }


  close(hfstore);
  free(cptr);

  writelog(INFO_M,"store_b16() %s done",filename);
  return PCO_NOERROR;
}

extern "C" int store_tif_v(char *filename,int width,int height,
               int colormode,void *bufadr,char *apptext)
{
  unsigned short *cptr;
  unsigned short *c1;
  unsigned int *b1;
  int hfstore;
  int e,z,x;
  int headerl;
  int slen,txtlen;
  char *ch;


  cptr=(unsigned short *)malloc(65536);

  if(cptr==NULL)
  {
   writelog(ERROR_M,"store_tiff() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }

  hfstore = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0666);//S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if(hfstore == -1)
  {
   writelog(ERROR_M,"store_tiff() create file %s failed",filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }


  slen=strlen(apptext);
  txtlen=slen+1;
  txtlen=(txtlen/16)*16+16;


  c1=cptr;
  *c1++ = 0x4949;           /* Begin TIFF-Header II */
  *c1++ = 0x002A;

  *c1++ = 0x0010;           /* Pointer to IFD  */
  *c1++ = 0;


  *c1++ = 0;
  *c1++ = 0;
  *c1 = 0;

/* create  IFD */
  c1=cptr+8;

  *c1++ = 0x000F;             /* Entry Count */

  *c1++ = 0x00FE;             /* NewSubfileType */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000000;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0100;             /* ImageWidth */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = width;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0101;             /* ImageHeight */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = height;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0102;             /* BitsPerPixel */
  *c1++ = 0x0003;             /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0010;             /* 16 */
  *c1++ = 0x0000;

  *c1++ = 0x0103;             /* Compression */
  *c1++ = 0x0003;             /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             /* 1 */
  *c1++ = 0x0000;

  *c1++ = 0x0106;             /* PhotometricInterpretation */
  *c1++ = 0x0003;             /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             /* 1 */
  *c1++ = 0x0000;


  *c1++ = 0x0111;             /* StripOffset */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = height;             /* 1 Zeilen pro */
  *b1++ = 0x0E0;              /* pointer */
  c1 = (unsigned short *)b1;

  *c1++ = 0x0115;             /* SamplePerPixel */
  *c1++ = 0x0003;             /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             /* 1 */
  *c1++ = 0x0000;

  *c1++ = 0x0116;             /* RowsPerStrip */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000001;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0117;              /* StripByteCounts */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = height;
  *b1++ = 0x0E0+height*4;     /* pointer */
  c1 = (unsigned short *)b1;

  *c1++ = 0x011A;              /* X-Resolution */
  *c1++ = 0x0005;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x0E0+height*8;      /* pointer */
  c1 = (unsigned short *)b1;

  *c1++ = 0x011B;              /* Y-Resolution */
  *c1++ = 0x0005;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x0E0+height*8+8;    /* pointer */
  c1 = (unsigned short *)b1;

  *c1++ = 0x011C;              /* PlanarConfiguration */
  *c1++ = 0x0003;              /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              /* 1 */
  *c1++ = 0x0000;

  *c1++ = 0x0128;              /* ResolutionUnit */
  *c1++ = 0x0003;              /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              /* 1 */
  *c1++ = 0x0000;

  *c1++ = 0x0131;              /* Software */
  *c1++ = 0x0002;
  b1 = (unsigned int *)c1;
  *b1++ = slen+1;
  *b1++ = 0x0E0+height*8+16;   /* pointer */


  c1 = (unsigned short *)b1;
  for(;c1<cptr+0xE0/2;)
   *c1++ = 0;                   /* ende */

/* beginn der stripadressen 224byte vom Fileanfang */
  z=0x0E0+height*8+16+txtlen;     /*  textlength */

  c1=cptr+0x70;
  b1 = (unsigned int *)c1;
  for(x=0;x<height;x++)
   *b1++=z+x*width*2;

  for(x=0;x<height;x++)
   *b1++=width*2;

  *b1++=0x00000004;
  *b1++=width;
  *b1++=0x00000004;
  *b1++=height;

  ch = (char*)b1;  //12345678901234567890123456789012345678901234567890

  strcpy((char*)b1,apptext);
  ch=(char*)b1;
  ch+=slen;
  for(;ch<(char*)cptr+z;)
   *ch++=0;

  headerl = (int)(ch-(char*)cptr);

  e=write(hfstore,(void *)cptr,headerl);
  if(e== -1)
  {
   writelog(ERROR_M,"store_tiff() write header to %s failed",filename);
   close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  z=width*height*2;
  e=write(hfstore,(void *)bufadr,z);
  if(e==-1)
  {
   writelog(ERROR_M,"store_tiff() write %d bytes to %s failed",z,filename);
   close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  close(hfstore);
  free(cptr);
  writelog(INFO_M,"store_tiff() %s done",filename);

  return PCO_NOERROR;
}

extern "C" int store_tif(char *filename,int width,int height,int colormode,void *bufadr)
{
  if(strlen(pcotiff_text)==0)
  {
   int x;
   sprintf(pcotiff_text,"PCO File_R/W-Library %s Copyright (C)2012 PCO ",FPVERS);
   if(colormode==0)
    strcat(pcotiff_text,"InputImage: B/W  ");
   else
    strcat(pcotiff_text,"InputImage: COLOR");
   for(x=strlen(pcotiff_text);x<70-1;x++)
    pcotiff_text[x]=0x20;
   pcotiff_text[x]=0;
  }
  return store_tif_v(filename,width,height,colormode,bufadr,pcotiff_text);
}


static void chbytes(int W,int H,unsigned short* array)
{
  int x=0,y=0,pos=0;
  unsigned char buf1=0,buf2=0;
  for (y=0;y<H;y++)
  {
   for (x=0;x<W;x++)
   {
    pos=y*W+x;
    buf1=array[pos]%256;
    buf2=array[pos]/256;
    array[pos]=buf1*256;
    array[pos]+=buf2;
   }
  }
}

static void swapbytes(int W,int H,unsigned short* outbuf,unsigned short* inbuf)
{
  int x=0,y=0;
  for (y=0;y<H;y++)
  {
   for (x=0;x<W;x++)
   {
    *outbuf=((*inbuf&0xFF)<<8) + ((*inbuf&0xFF00)>>8);
    outbuf++;
    inbuf++;
   }
  }
}



/********************************************************************************************************************
*                                            FITS-Datei schreiben                                                   *
*********************************************************************************************************************/

#define FITSBLOCK 2880

extern "C" int store_fits_exp(char *filename,int W, int H, void *img_data,int _exp_time_ms)
{
  int z=0,e=0,i=0;
  FILE *fitsfile;
  char *headerline,*buffer;
  unsigned short *imgbuf;

  fitsfile=fopen(filename,"w+");
  if (fitsfile==NULL)
  {
   writelog(ERROR_M,"store_fits_exp() create file %s failed",filename);
   remove(filename);
   return PCO_ERROR_NOFILE;
  }

  imgbuf=(unsigned short *)malloc(W*H*sizeof(unsigned short));
  if(imgbuf==NULL)
  {
   writelog(ERROR_M,"store_fits_exp() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }

  buffer=(char *)malloc(81);
  if(buffer==NULL)
  {
   free(imgbuf);
   writelog(ERROR_M,"store_fits_exp() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }

  headerline =(char *)malloc(2881);
  if(headerline==NULL)
  {
   free(imgbuf);
   free(buffer);
   writelog(ERROR_M,"store_fits_exp() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }

  memcpy(imgbuf,img_data,W*H*sizeof(unsigned short));

  chbytes(W,H,(unsigned short*)imgbuf);

  memset(headerline,0,2881);//to avoid confusion

//now we can insert the keywords
//don't forget to provide a line length of 80 characters
  sprintf(buffer,"SIMPLE  =                    T                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"BITPIX  =                   16                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS   =                    2                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS1  =                 %4d                                                  ",W);
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS2  =                 %4d                                                  ",H);
  strcat(headerline,buffer);
 //end of header:
  sprintf(buffer,"END                                                                             ");
  strcat(headerline,buffer);
  for (i=strlen(headerline);i<FITSBLOCK;i++)
  {
   sprintf(buffer," ");
   strcat(headerline,buffer);
  }

//write the header to fitsfile
  e=0;
  e=fputs(headerline,fitsfile);
  if (e==EOF)
  {
   writelog(ERROR_M,"store_fits_exp() write header to %s failed",filename);
   fclose(fitsfile);
   free(imgbuf);
   free(buffer);
   free(headerline);
   return PCO_ERROR_NOFILE;
  }

//write the data to 'fitsfile'
  e=fwrite(imgbuf,sizeof(unsigned short),W*H,fitsfile);
  if (e==0)
  {
   writelog(ERROR_M,"store_fits_exp() write %d bytes to %s failed",z,filename);
   fclose(fitsfile);
   free(imgbuf);
   free(buffer);
   free(headerline);
   return PCO_ERROR_NOFILE;
  }

//we have to fill up the last 2880 byte block with blanks
  memset(headerline,' ',2881);

  z=W*H;
  z=(H+4)*W*2;
  z%=FITSBLOCK;
  z=FITSBLOCK-z;


  e=fwrite(headerline,sizeof(char),z,fitsfile);
  if (e==0)
  {
   writelog(ERROR_M,"store_fits_exp() write %d bytes to %s failed",z,filename);
   fclose(fitsfile);
   free(imgbuf);
   free(buffer);
   free(headerline);
   return PCO_ERROR_NOFILE;
  }

  fclose(fitsfile);
  free(imgbuf);
  free(buffer);
  free(headerline);

  writelog(INFO_M,"store_fits_exp() %s done",filename);

  return PCO_NOERROR;
}

extern "C" int store_fits(char *filename,int W, int H,int colormode, void *img_data)
{
  int z=0,e=0,i=0;
  FILE *fitsfile;
  char *headerline,*buffer;
  unsigned short *imgbuf;


  fitsfile=fopen(filename,"w+");
  if (fitsfile==NULL)
  {
   writelog(ERROR_M,"store_fits() create file %s failed",filename);
   remove(filename);
   return PCO_ERROR_NOFILE;
  }

  imgbuf=(unsigned short *)malloc(W*H*sizeof(unsigned short));
  if(imgbuf==NULL)
  {
   writelog(ERROR_M,"store_fits() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }

  buffer=(char *)malloc(81);
  if(buffer==NULL)
  {
   free(imgbuf);
   writelog(ERROR_M,"store_fits() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }

  headerline =(char *)malloc(2881);
  if(headerline==NULL)
  {
   free(imgbuf);
   free(buffer);
   writelog(ERROR_M,"store_fits() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }


  swapbytes(W,H,imgbuf,(unsigned short*)img_data);

//  memcpy(imgbuf,img_data,W*H*sizeof(unsigned short));
//  chbytes(W,H,(unsigned short*)imgbuf);



  memset(headerline,0,2881);//to avoid confusion
//now we can insert the keywords
//don't forget to provide a line length of 80 characters
  sprintf(buffer,"SIMPLE  =                    T                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"BITPIX  =                   16                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS   =                    2                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS1  =                 %4d                                                  ",W);
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS2  =                 %4d                                                  ",H);
  strcat(headerline,buffer);
 //end of header:
  sprintf(buffer,"END                                                                             ");
  strcat(headerline,buffer);
  for (i=strlen(headerline);i<FITSBLOCK;i++)
  {
  sprintf(buffer," ");
  strcat(headerline,buffer);
  }

  //write the header to fitsfile
  e=0;
  e=fputs(headerline,fitsfile);
  if (e==EOF)
  {
   writelog(ERROR_M,"store_fits() write header to %s failed",filename);
   fclose(fitsfile);
   free(imgbuf);
   free(buffer);
   free(headerline);
   return PCO_ERROR_NOFILE;
  }

  e=0;
  //write the data to 'fitsfile'
  e=fwrite(imgbuf,sizeof(unsigned short),W*H,fitsfile);
  if (e==0)
  {
   writelog(ERROR_M,"store_fits() write %d bytes to %s failed",z,filename);
   fclose(fitsfile);
   free(imgbuf);
   free(buffer);
   free(headerline);
   return PCO_ERROR_NOFILE;
  }

  //we have to fill up the last 2880 byte block with blanks
  memset(headerline,' ',2881);

  z=W*H;
  z=(H+4)*W*2;
  z%=FITSBLOCK;
  z=FITSBLOCK-z;

  e=fwrite(headerline,sizeof(unsigned short),W*H,fitsfile);
  if (e==0)
  {
   writelog(ERROR_M,"store_fits() write %d bytes to %s failed",z,filename);
   fclose(fitsfile);
   free(imgbuf);
   free(buffer);
   free(headerline);
   return PCO_ERROR_NOFILE;
  }

  fclose(fitsfile);
  free(imgbuf);
  free(buffer);
  free(headerline);

  writelog(INFO_M,"store_fits() %s done",filename);
  return PCO_NOERROR;
}


extern "C" int store_bmp(char *filename,int width,int height,int colormode,void *buf)
{
  unsigned char *cptr;
  unsigned char *c1;
  unsigned int *b1;
  int hfstore;
  int e,z,x;
  int headerl;


  if(colormode>2)
  {
   writelog(ERROR_M,"store_bmp() wrong colormode value");
   return PCO_ERROR_WRONGVALUE;
  }

  cptr=(unsigned char *)malloc(2000L);

  if(cptr==NULL)
  {
   writelog(ERROR_M,"store_bmp() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }

  hfstore = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0666);//S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH); |O_SYNC
  if(hfstore == -1)
  {
   writelog(ERROR_M,"store_bmp() create file %s failed",filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

//calculate size of LUT-Table
  if(colormode==0)
   x=256;
  else
   x=0;

  headerl=x*4 + 0x28 + 0x0A;

  c1=cptr;
  *c1++ = 'B';           //Begin BMP-Header BM
  *c1++ = 'M';

  b1 = (unsigned int *)c1;

// Data for header
  if(colormode==0)
   *b1++ = width*height+headerl;
  if(colormode==1)
   *b1++ = width*height*3+headerl;
  if(colormode==2)
   *b1++ = width*height*4+headerl;
  *b1++ = 0L;
  *b1++ = headerl;
  *b1++ = 0x028;               //sizeof(BITMAPAPINFOHEADER);
  *b1++ = width;
  *b1++ = height;
  if(colormode==0)
   *b1++ = 1+(8<<16);
  if(colormode==1)
   *b1++ = 1+(24<<16);
  if(colormode==2)
   *b1++ = 1+(32<<16);
  *b1++ = 0;                  // BI_RGB;
  *b1++ = width*height;
  *b1++ = 0;
  *b1++ = 0;
  *b1++ = 0;
  *b1++ = 0;

  c1 = (unsigned char *)b1;

//write  LUT-Table
  if(colormode==0)
  {
   for(z=0;z<x;z++)
   {
    *c1++ = (unsigned char)z;
    *c1++ = (unsigned char)z;
    *c1++ = (unsigned char)z;
    *c1++ = 0;
   }
  }


  z=headerl;
  e=write(hfstore,(void *)cptr,z);
  if(e==-1)
  {
   writelog(ERROR_M,"store_bmp() write header to %s failed",filename);
   close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

//write data to file
  switch(colormode)
   {
    case 0:
     z=width*height;
     break;

    case 1:
     z=width*height*3;
     break;

    case 2:
     z=width*height*4;
     break;

    default:
     break;
   }

  e=write(hfstore,(void *)buf,z);
  if(e==-1)
  {
   writelog(ERROR_M,"store_bmp() write %d bytes to %s failed",z,filename);
   close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  close(hfstore);
  free(cptr);

  writelog(INFO_M,"store_bmp() %s done",filename);
  return PCO_NOERROR;
}

extern "C"  int store_tif8bw_v(char *filename,int width,int height,int colormode,void *bufadr,char* apptext)
{
  unsigned short *cptr;
  unsigned short *c1;
  unsigned int *b1;
  int hfstore;
  int e,z,x;
  int headerl;
  int slen,txtlen;
  char *ch;

  cptr=(unsigned short *)malloc(65536);

  if(cptr==NULL)
  {
   writelog(ERROR_M,"store_tif8bw() memory allocation failed");
   return PCO_ERROR_NOMEMORY;
  }

  hfstore = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0666);//S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if(hfstore == -1)
  {
   writelog(ERROR_M,"store_btif8bw() create file %s failed",filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  slen=strlen(apptext);
  txtlen=slen+1;
  txtlen=(txtlen/16)*16+16;


  c1=cptr;
  *c1++ = 0x4949;           //Begin TIFF-Header II
  *c1++ = 0x002A;

  *c1++ = 0x0010;            //Pointer to IFD
  *c1++ = 0;


  *c1++ = 0;
  *c1++ = 0;
  *c1 = 0;

// create  IFD
  c1=cptr+8;                 //8 Words

  *c1++ = 0x000F;             //Entry Count

  *c1++ = 0x00FE;             //NewSubfileType
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000000;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0100;             //ImageWidth
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = width;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0101;             //ImageHeight
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = height;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0102;             //BitsPerPixel
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0008;             //8
  *c1++ = 0x0000;

  *c1++ = 0x0103;             //Compression
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             //1
  *c1++ = 0x0000;

  *c1++ = 0x0106;             //PhotometricInterpretation
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             //1 min is black
  *c1++ = 0x0000;

  *c1++ = 0x0111;             //StripOffset
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = height;              //4 Zeilen pro
  *b1++ = 0x0E0;               //pointer
  c1 = (unsigned short *)b1;

  *c1++ = 0x0115;             //SamplePerPixel
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             //1
  *c1++ = 0x0000;

  *c1++ = 0x0116;             //RowsPerStrip
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000001;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0117;              //StripByteCounts
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = height;
  *b1++ = 0x0E0+height*4;     //pointer;
  c1 = (unsigned short *)b1;

  *c1++ = 0x011A;              //X-Resolution
  *c1++ = 0x0005;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x0E0+height*8;   //pointer;
  c1 = (unsigned short *)b1;

  *c1++ = 0x011B;              //Y-Resolution
  *c1++ = 0x0005;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x0E0+height*8+8; //pointer;
  c1 = (unsigned short *)b1;

  *c1++ = 0x011C;              //PlanarConfiguration
  *c1++ = 0x0003;              //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              //1
  *c1++ = 0x0000;

  *c1++ = 0x0128;              //ResolutionUnit
  *c1++ = 0x0003;              //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              //1
  *c1++ = 0x0000;


  *c1++ = 0x0131;             //Software
  *c1++ = 0x0002;
  b1 = (unsigned int *)c1;
  *b1++ = slen+1;
  *b1++ = 0x0E0+height*8+16; //pointer;

  c1 = (unsigned short *)b1;
  for(;c1<cptr+0xE0/2;)
   *c1++ = 0;                   /* ende */

//beginn der stripadressen 224byte vom Fileanfang
  z=0x0E0+height*8+16+txtlen;     //  txtlength

  c1=cptr+0x70; //0x70=0xE0/2
  b1 = (unsigned int *)c1;     //write line offsets
  for(x=0;x<height;x++)
   *b1++=z+x*width;

  for(x=0;x<height;x++)        //write line bytes
   *b1++=width;

  *b1++=0x00000004;
  *b1++=width;
  *b1++=0x00000004;
  *b1++=height;

  ch=(char*)b1;
  strcpy(ch,apptext);
  ch+=slen;
  for(;ch<(char*)cptr+z;)
   *ch++=0;

  headerl = (int)(ch-(char*)cptr);

  e=write(hfstore,(void *)cptr,headerl);
  if(e== -1)
  {
   writelog(ERROR_M,"store_tif8bw() write header to %s failed",filename);
   close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  z=width*height;
  e=write(hfstore,(void *)bufadr,z);
  if(e==-1)
  {
   writelog(ERROR_M,"store_tif8bw() write %d bytes to %s failed",z,filename);
   close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  close(hfstore);
  free(cptr);

  writelog(INFO_M,"store_tif8bw() %s done",filename);
  return PCO_NOERROR;
}

extern "C" int store_tif8bw(char *filename,int width,int height,int colormode,void *bufadr)
{
  if(strlen(pcotiff_text)==0)
  {
   int x;
   sprintf(pcotiff_text,"PCO File_R/W-Library %s Copyright (C)2012 PCO ",FPVERS);
   if(colormode==0)
    strcat(pcotiff_text,"InputImage: B/W  ");
   else
    strcat(pcotiff_text,"InputImage: COLOR");
   for(x=strlen(pcotiff_text);x<70-1;x++)
    pcotiff_text[x]=0x20;
   pcotiff_text[x]=0;
  }
  return store_tif8bw_v(filename,width,height,colormode,bufadr,pcotiff_text);
}



/*
int store_tif8rgb(char *filename,int width,int height,
                int colormode,void *bufadr)
{
  unsigned short *cptr;
  unsigned short *c1;
  unsigned int *b1;
  int x;
  int e;

  HANDLE hfstore;
  HANDLE hb1;
  long z,zz;
  unsigned int headerl;

  hb1=GlobalAlloc(GHND,8000L);
  cptr=(unsigned short *)GlobalLock(hb1);

  c1=cptr;
  *c1++ = 0x4949;           //Begin TIFF-Header II
  *c1++ = 0x002A;

  *c1++ = 0x0010;            //Pointer to IFD
  *c1++ = 0;


  *c1++ = 0x0008;
  *c1++ = 0x0008;
  *c1 =   0x0008;

// create  IFD
  c1=cptr+8;                 //8 W�ter

  *c1++ = 0x000F;             //Entry Count

  *c1++ = 0x00FE;             //NewSubfileType
  *c1++ = 0x0004;
  b1 = (void *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000000;
  c1 = (void *)b1;

  *c1++ = 0x0100;             //ImageWidth
  *c1++ = 0x0004;
  b1 = (void *)c1;
  *b1++ = 0x00000001;
  *b1++ = width;
  c1 = (void *)b1;

  *c1++ = 0x0101;             //ImageHeight
  *c1++ = 0x0004;
  b1 = (void *)c1;
  *b1++ = 0x00000001;
  *b1++ = height;
  c1 = (void *)b1;

  *c1++ = 0x0102;             //BitsPerPixel
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0003;
  *c1++ = 0x0000;
  *c1++ = 0x0008;             //pointer uf adr 00000008
  *c1++ = 0x0000;

  *c1++ = 0x0103;             //Compression
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             //1
  *c1++ = 0x0000;

  *c1++ = 0x0106;             //PhotometricInterpretation
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0002;             //RGB
  *c1++ = 0x0000;

  *c1++ = 0x0111;             //StripOffset
  *c1++ = 0x0004;
  b1 = (void *)c1;
  *b1++ = height/4;            //4 Zeilen pro
  *b1++ = 0x0E0;               //pointer
  c1 = (void *)b1;

  *c1++ = 0x0115;             //SamplePerPixel
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0003;             //3
  *c1++ = 0x0000;

  *c1++ = 0x0116;             //RowsPerStrip
  *c1++ = 0x0004;
  b1 = (void *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000004;
  c1 = (void *)b1;

  *c1++ = 0x0117;              //StripByteCounts
  *c1++ = 0x0004;
  b1 = (void *)c1;
  *b1++ = height/4;
  *b1++ = 0x0E0+height;     //pointer;
  c1 = (void *)b1;

  *c1++ = 0x011C;              //PlanarConfiguration
  *c1++ = 0x0003;              //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              //1
  *c1++ = 0x0000;

  *c1++ = 0x0128;              //ResolutionUnit
  *c1++ = 0x0003;              //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              //1
  *c1++ = 0x0000;

  *c1++ = 0x011A;              //X-Resolution
  *c1++ = 0x0005;
  b1 = (void *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x0E0+height*2;   //pointer;
  c1 = (void *)b1;

  *c1++ = 0x011B;              //Y-Resolution
  *c1++ = 0x0005;
  b1 = (void *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x0E0+height*2+8; //pointer;
  c1 = (void *)b1;

  *c1++ = 0x0131;             //Software
  *c1++ = 0x0002;
  b1 = (void *)c1;
  *b1++ = 0x00000032;
  *b1++ = 0x0E0+height*2+16; //pointer;


  c1 = (void *)b1;
  *c1++ = 0;                  //ende
  *c1 = 0;

  //beginn der stripadressen 224byte vom Fileanfang
//@ver102a
  z=0x0E0+height*2+16+70;     //  70 is textlength
  c1=cptr+0x70;
  b1 = (void *)c1;
  for(x=0;x<height/4;x++)
	*b1++=z+x*width*4*3;

  for(x=0;x<height/4;x++)
	*b1++=width*4*3;

  *b1++=0x00000004;
  *b1++=width;
  *b1++=0x00000004;
  *b1++=height;

  c1 = (void *)b1;  //12345678901234567890123456789012345678901234567890


  sprintf((char *)c1,"SensiCam File-Program %s Copyright (C) 1997 PCO ",FPVERS);
//  sprintf((char *)c1,"SensiCam   Utility-Program Copyright (C) 1996 PCO ");
  c1+=25;
  if(colormode==0)
   sprintf((char *)c1,"Version: B/W    ");
  else
   sprintf((char *)c1,"Version: COLOR  ");
  c1+=8;

//@ver3.04  fehler beseitigt
  *c1++=0x2020;    // 4 x blank
  *c1++=0x2020;

//@ver102a brauchts nicht mehr
//  for(;c1<(cptr+0x0500);)
//   *c1++=0x0000;

  headerl = (c1-cptr)*2;           //W�ter
  hfstore = CreateFile(filename,
	                   GENERIC_WRITE,
					   0,
					   NULL,
					   CREATE_ALWAYS, 
					   FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
					   0);
  if(hfstore== INVALID_HANDLE_VALUE)
  {
	 GlobalUnlock(hb1);
	 GlobalFree(hb1);
	 return PCO_ERROR_NOFILE;
  }
  

  z=headerl;
  e=WriteFile(hfstore,(void *)cptr,z,&zz,NULL);
  if((e==0)||(z!=zz))
  {
   CloseHandle(hfstore);
   DeleteFile(filename);
   GlobalUnlock(hb1);
   GlobalFree(hb1);
   return PCO_ERROR_NOFILE;
  }

  z=width*height*3;
  e=WriteFile(hfstore,(void *)bufadr,z,&zz,NULL);
  if((e==0)||(z!=zz))
  {
   CloseHandle(hfstore);
   DeleteFile(filename);
   GlobalUnlock(hb1);
   GlobalFree(hb1);
   return PCO_ERROR_NOFILE;
  }

  CloseHandle(hfstore);
  GlobalUnlock(hb1);
  GlobalFree(hb1);
  return PCO_NOERROR;
}
*/



/*
int store_bmp24(char *filename,int width,int height,
              int colormode,void *buf)
{
  unsigned char  *cptr;
  unsigned char  *c1;
  unsigned char  *c2;
  unsigned long  *b1;

  HFILE hfstore;
  HANDLE hb1;
  long x,z;
  unsigned int headerl;
  char of[20];
  int bufanz,bufrest,i;

  if(colormode==0)
   return PCO_ERROR_WRONGVALUE;

  hb1=GlobalAlloc(GHND,64000L);
  cptr=(unsigned char  *)GlobalLock(hb1);

  headerl=sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);

  c1=cptr;
  *c1++ = 'B';           //Begin BMP-Header BM
  *c1++ = 'M';

  (void *)b1 = c1;

// Data for header
  *b1++ = width*height*3+headerl;
  *b1++ = 0L;
  *b1++ = headerl;
  *b1++ = sizeof(BITMAPINFOHEADER);
  *b1++ = width;
  *b1++ = height;
  *b1++ = 1+(24<<16);
  *b1++ = BI_RGB;
  *b1++ = width*height;
  *b1++ = 0;
  *b1++ = 0;
  *b1++ = 0;
  *b1 = 0;



  hfstore = _lcreat(filename,0);

  if(hfstore==HFILE_ERROR)
	{
	 GlobalUnlock(hb1);
	 GlobalFree(hb1);
	 return PCO_ERROR_NOFILE;
	}

//write header to file
  z=headerl;
  if((x=_hwrite(hfstore,(void  *)cptr,z))!=z)
	{
	 _lclose(hfstore);
	 OpenFile(filename,(OFSTRUCT *)&of,OF_DELETE);
	 GlobalUnlock(hb1);
	 GlobalFree(hb1);
	 return PCO_ERROR_NOFILE;
	}


//write data to file
  z=width*height*3;
  bufanz=z/63000L;
  bufrest=z%63000L;
  (void *)c2 = buf;
  for(i=0;i<bufanz;i++)
	{
	 (void *)c1=cptr;
	 for(x=0;x<21000U;x++)
	  {
		*c1++ = *c2++;
		*c1++ = *c2++;
		*c1++ = *c2++;
		c2++;
	  }

    if((x=_hwrite(hfstore,(void *)cptr,63000U))!=63000U)
	  {
	   _lclose(hfstore);
	   OpenFile(filename,(OFSTRUCT *)&of,OF_DELETE);
	   GlobalUnlock(hb1);
	   GlobalFree(hb1);
	   return PCO_ERROR_NOFILE;
	  }
   }

  c1=cptr;
  for(x=0;x<bufrest/3;x++)
	{
	 *c1++ = *c2++;
	 *c1++ = *c2++;
	 *c1++ = *c2++;
	 c2++;
	}

  if(_hwrite(hfstore,(void *)cptr,bufrest)!=bufrest)
	{
	 _lclose(hfstore);
	 OpenFile(filename,(OFSTRUCT far *)&of,OF_DELETE);   //delete File
	 GlobalUnlock(hb1);
	 GlobalFree(hb1);
	 return PCO_ERROR_NOFILE;
	}


  _lclose(hfstore);
  GlobalUnlock(hb1);
  GlobalFree(hb1);
  return PCO_NOERROR;
}
*/



/********************************************************************************************************/
/* added TIFF - Reader... Franz Reitner/24.06.1999  mbl 2012                                            */
/********************************************************************************************************/
const short ImageWidth      = 256;     // IFD - Einträge (Image File Descriptor)
const short ImageHeight     = 257;
const short BitsPerPixel    = 258;
const short Compression     = 259;
const short PhotoInterp     = 262;
const short StripOffsets    = 273;
const short SamplesPerPixel = 277;
const short RowsPerStrip    = 278;
const short StripByteCnt    = 279;
const short XResolution     = 282;
const short YResolution     = 283;
const short PlanarConfig    = 284;
const short ResolutionUnits = 296;
const short ColorMap        = 320;

/* Aufbau TIF - File: Header 0x49 0x49 0x2A 0x2A(Kennung mit Version)
                      XXXXYYYY(Offset für IFD)
                      DATEN(entweder vor oder nach IFD)
                      bei Adresse XXXXYYYY: ZZZZ Anzahl Einträge in IFD
                      IFDs
                      DATEN(entweder vor oder nach IFD) */

typedef struct                         // IFD - Struktur
{
  unsigned short TagField;             // IFD ID
  unsigned short ftype;                // Type (Byte,String,Short,Long,Float)
  unsigned long length;                // Anzahl Einträge
  unsigned long Offset;                // Datum, falls Anzahl Einträge=1; ansonsten Offset im File
}TE;


TE TIFFEntry;
bool bIs217 = FALSE;


int is_tif(int hfread,int *type)
{
  char bfr[64];
  long currPos;
  int z;

  currPos = lseek( hfread, 0L, SEEK_CUR );
  lseek(hfread,0,SEEK_SET);

  z=read(hfread,bfr,64);
  if(z<64)
  {
   lseek(hfread,currPos,SEEK_SET);
   writelog(ERROR_M,"is_tif() read header failed");
   return PCO_ERROR_NOFILE;
  }

  if ((bfr[0] == 'I') &&(bfr[1] == 'I') &&(bfr[2] == 0x2A) &&(bfr[3] == 0))
  {
   writelog(INFO_M,"is_tif() Intel tif file found");
   *type= FILEISOK;

/*  const char* pdois;
    ok = FILEISOK;                   // nur INTEL-Format wird gelesen, sonst Drehwurm

    bIs217 = FALSE;
    pdois = strstr((const char *)&bfr[8], "CamWare File-Program");
    if(pdois != NULL)
    {
     pdois = strstr(pdois, "2.");
        if(pdois != NULL)
        {
          int ivmj, ivmi;

          ivmj = atoi(&pdois[0]);
          ivmi = atoi(&pdois[2]);
          if(ivmj < 1)
          {
            bIs217 = TRUE;
          }
          if(ivmj == 2)
          {
            if(ivmi < 20)
              bIs217 = TRUE;
          }
        }
      }
*/
  }
  else if ((bfr[0] == 'M') &&(bfr[1] == 'M') &&(bfr[3] == 0x2A) &&(bfr[2] == 0))
  {
   writelog(INFO_M,"is_tif() MAC tif file found");
   *type = FILEISOK | FILEISMACFORMAT;                   // nur INTEL-Format wird gelesen, sonst Drehwurm
  }
  else
  {
   writelog(ERROR_M,"is_tif() wrong header no tif-file");
   *type = 0;
   lseek(hfread,currPos,SEEK_SET);
   return PCO_ERROR_NOFILE;
  }
  lseek(hfread,currPos,SEEK_SET);

  return PCO_NOERROR;
}                                      // IsTiffFile

int is_tif_file(char *filename,int *type)
{
  int hfread;
  int err=PCO_NOERROR;

  hfread = open(filename,O_RDONLY);
  if(hfread == -1)
  {
   writelog(ERROR_M,"is_tif_file() Open file %s failed",filename);
   return PCO_ERROR_NOFILE;
  }

  err=is_tif(hfread,type);

  close(hfread);
  return err;
}

void mswab(unsigned char* src, unsigned char* dest, int isize)
{
  dest+= isize;
  dest--;
  for(int i = 0; i < isize; i++)
  {
    *dest = *src;
    src++;
    dest--;
  }
}



int getsize_tif(int hfread, int *width, int *height, int *bitpix)
{
  int w = 0, h = 0, bp= 0, ifdcnt = 0;
  long offset = 0, lh;
  unsigned short ush, ush2;
  bool bmacformat = FALSE;
  int err;
  unsigned int x;

  err= PCO_NOERROR;

  err = is_tif(hfread,&ifdcnt);
  if(err!=PCO_NOERROR)
   return err;

  if((ifdcnt & FILEISMACFORMAT) == FILEISMACFORMAT)
   bmacformat = TRUE;

  lseek(hfread,4,SEEK_SET);
  x=read(hfread,&offset,sizeof(offset));
  if(x<sizeof(offset))
   writelog(ERROR_M,"read_tif() read offset failed");

  if(bmacformat)
  {
    mswab((unsigned char*)&offset, (unsigned char*)&lh, sizeof(lh));
    offset = lh;
  }
  lseek(hfread,offset,SEEK_SET);

  x=read(hfread,&ush,2);
  if(x<2)
   writelog(ERROR_M,"read_tif() read ifdcnt failed");

  if(bmacformat)
  {
   mswab((unsigned char*)&ush, (unsigned char*)&ush2, sizeof(ush2));
   ifdcnt = ush2;
  }
  else
   ifdcnt = ush;

  offset += 2;

  for (int i = 0; i < ifdcnt; i++)
  {
   lseek(hfread,offset+i * sizeof(TIFFEntry),SEEK_SET);
   read(hfread,&TIFFEntry,sizeof(TIFFEntry));
   if(bmacformat)
   {
     mswab((unsigned char*)&TIFFEntry.ftype, (unsigned char*)&ush, sizeof(ush));
     TIFFEntry.ftype = ush;
     mswab((unsigned char*)&TIFFEntry.TagField, (unsigned char*)&ush, sizeof(ush));
     TIFFEntry.TagField = ush;
     mswab((unsigned char*)&TIFFEntry.length, (unsigned char*)&lh, sizeof(lh));
     TIFFEntry.length = lh;
     if(TIFFEntry.length == 1)
     {
      if(TIFFEntry.ftype == 3)
      {
       mswab((unsigned char*)&TIFFEntry.Offset, (unsigned char*)&ush, sizeof(ush));
       TIFFEntry.Offset = ush;
      }
      if(TIFFEntry.ftype == 4)
      {
       mswab((unsigned char*)&TIFFEntry.Offset, (unsigned char*)&lh, sizeof(lh));
       TIFFEntry.Offset = lh;
      }
    }
   }
   if (TIFFEntry.TagField == ImageWidth)
    w = TIFFEntry.Offset;
   if (TIFFEntry.TagField == ImageHeight)
    h = TIFFEntry.Offset;
   if (TIFFEntry.TagField == BitsPerPixel)
    bp = TIFFEntry.Offset;
  }

  if (!((w != 0)&&(h != 0)&&(bp != 0)))
  {
    *width = 0;
    *height= 0;
    *bitpix= 0;
    writelog(ERROR_M,"getsize_tife() no imagewidth,imageheight or bitperpixel value found");
    return PCO_ERROR_NOFILE;
  }

  *width=w;
  *height=h;
  *bitpix=bp;
  return PCO_NOERROR;
}

int getsize_tif_file(char *filename, int *width, int *height, int *bitpix)
{
  int hfread;
  int err=PCO_NOERROR;

  hfread = open(filename,O_RDONLY);
  if(hfread == -1)
  {
   writelog(ERROR_M,"getsize_tif_file() Open file %s failed",filename);
   return PCO_ERROR_NOFILE;
  }

  err=getsize_tif(hfread,width,height,bitpix);

  close(hfread);
  return err;
}



//noch zu testen
int read_tif(char *filename,void* buf, int iNoShifting)
{
  int hfread;
//  bool bcheckalignment = FALSE;         // 2.17 macht einen Fehler beim Abspeichern
//  bool bmulti = FALSE;

  int width,height,ifdcnt,iImageType;
  int iBitsR,iBitsG,iBitsB,iRowsPerStrip;
  unsigned long offset,DataOffset,lFileLengthL;
  long lh;
  unsigned short ush, ush2;

  bool bmacformat = FALSE;
  int    SDataBytes = 0, SDataCnt = 0, SBytes = 0, SCnt = 0;
  unsigned int x;

  offset=DataOffset=iBitsR=iBitsG=iBitsB=iRowsPerStrip=0;
  width=height=ifdcnt=iImageType = 0;

  DWORD *SDataOffset = NULL, *SByteCnt = NULL;
  unsigned long bread = 0;
  WORD  *p = NULL;
  int err = PCO_NOERROR;

  hfread = open(filename,O_RDONLY);
  if(hfread == -1)
  {
   writelog(ERROR_M,"read_tif() Open file %s failed",filename);
   return PCO_ERROR_NOFILE;
  }

  lFileLengthL = lseek(hfread,0,SEEK_END);

  err= is_tif(hfread,&SCnt);
  if(err!=PCO_NOERROR)
  {
   close(hfread);
   writelog(ERROR_M,"read_tif() is_tif of file %s failed",filename);
   return err;
  }

  if((SCnt & FILEISMACFORMAT) == FILEISMACFORMAT)
   bmacformat = TRUE;


  lseek(hfread,4,SEEK_SET);

  x=read(hfread,&offset,sizeof(offset));
  if(x<sizeof(offset))
   writelog(ERROR_M,"read_tif() read offset failed");

  if(bmacformat)
  {
    mswab((unsigned char*)&offset, (unsigned char*)&lh, sizeof(lh));
    offset = lh;
  }
  lseek(hfread,offset,SEEK_SET);

  x=read(hfread,&ush,2);
  if(x<2)
   writelog(ERROR_M,"read_tif() read ifdcnt failed");

  if(bmacformat)
  {
    mswab((unsigned char*)&ush, (unsigned char*)&ush2, sizeof(ush2));
    ifdcnt = ush2;
  }
  else
    ifdcnt = ush;
  offset += 2;

  writelog(INFO_M,"read_tif() ifdcnt is %d file length %d",ifdcnt,lFileLengthL);

  for (int i = 0; i < ifdcnt; i++)      // Einträge siehe TIFF - Beschreibung (tiff.pdf vom 03.06.1992)
  {                                    //                       (ist aktuelle Vers.! 24.06.1999/Franz)
    //                       (http:// www.adobe.com)
    lseek(hfread, + i * sizeof(TIFFEntry),SEEK_SET);
    read(hfread,&TIFFEntry, sizeof(TIFFEntry));
    if(bmacformat)
    {
      mswab((unsigned char*)&TIFFEntry.ftype, (unsigned char*)&ush, sizeof(ush));
      TIFFEntry.ftype = ush;
      mswab((unsigned char*)&TIFFEntry.TagField, (unsigned char*)&ush, sizeof(ush));
      TIFFEntry.TagField = ush;
      mswab((unsigned char*)&TIFFEntry.length, (unsigned char*)&lh, sizeof(lh));
      TIFFEntry.length = lh;
      if(TIFFEntry.length == 1)
      {
        if(TIFFEntry.ftype == 3)
        {
          mswab((unsigned char*)&TIFFEntry.Offset, (unsigned char*)&ush, sizeof(ush));
          TIFFEntry.Offset = ush;
        }
        if(TIFFEntry.ftype == 4)
        {
          mswab((unsigned char*)&TIFFEntry.Offset, (unsigned char*)&lh, sizeof(lh));
          TIFFEntry.Offset = lh;
        }
      }
      else
      {
        mswab((unsigned char*)&TIFFEntry.Offset, (unsigned char*)&lh, sizeof(lh));
        TIFFEntry.Offset = lh;
      }
    }

    switch (TIFFEntry.TagField)
    {
      case ImageWidth:
      {
        if (TIFFEntry.ftype == 3)
          width = TIFFEntry.Offset;// word
        if (TIFFEntry.ftype == 4)
          width = TIFFEntry.Offset;// dword
        writelog(INFO_M,"read_tif() ImageWidth is %d ",width);
        break;
      }
      case ImageHeight:
      {
        if (TIFFEntry.ftype == 3)
          height = TIFFEntry.Offset;// word
        if (TIFFEntry.ftype == 4)
          height = TIFFEntry.Offset;// dword
        writelog(INFO_M,"read_tif() ImageHeight is %d ",height);
        break;
      }
      case BitsPerPixel:               // Auflösung in bit je Pixel (8bit BW, 16bit BW oder 24bit RGB)
      {
        short sValue;

        sValue =(short)TIFFEntry.Offset;// word
        if (TIFFEntry.length == 1)
        {
          if ((sValue > 16) || (sValue < 8))
            err = PCO_ERROR_NOFILE;
          if (sValue <= 8)
            iImageType = 1;
          else
            iImageType = 2;            // 1 -> 8 bit BW; 2 -> 9...16 bit BW
          writelog(INFO_M,"read_tif() BitsPerPixel is %d ",sValue);
        }
        else if((TIFFEntry.length == 3) ||(TIFFEntry.length == 4))
        {
          short buf[5];

          if(TIFFEntry.ftype == 3)
          {
            lseek(hfread,TIFFEntry.Offset,SEEK_SET);
            read(hfread, buf, 6);
            if(bmacformat)
            {
              mswab((unsigned char*)&buf[0], (unsigned char*)&ush, sizeof(ush));
              buf[0] = ush;
              mswab((unsigned char*)&buf[1], (unsigned char*)&ush, sizeof(ush));
              buf[1] = ush;
              mswab((unsigned char*)&buf[2], (unsigned char*)&ush, sizeof(ush));
              buf[2] = ush;
            }
            iBitsR = buf[0];
            iBitsG = buf[1];
            iBitsB = buf[2];
            if ((iBitsR > 8) ||(iBitsG > 8) ||(iBitsB > 8))
            {
              err = PCO_ERROR_WRONGVALUE;
              writelog(INFO_M,"read_tif() BitsPerPixel cannot handle format 3:%d%d%d ",iBitsR,iBitsG,iBitsB);
            }
            else
              iImageType = TIFFEntry.length;          // 3 -> RGB Bild

            writelog(INFO_M,"read_tif() BitsPerPixel is %d%d%d ",iBitsR,iBitsG,iBitsB);
          }
          else
          {
            err = PCO_ERROR_WRONGVALUE;
            writelog(ERROR_M,"read_tif() BitsPerPixel cannot handle type %d",TIFFEntry.ftype);
           }
        }
        break;
      }
      case Compression:                // 1: kein Packen, 2: Huffman, 3: Fax G3,
      {                               // 4: Fax G4, 5: LZW, 32773: PackBits
        if (TIFFEntry.Offset != 1)// word
        {
          err = PCO_ERROR_WRONGVALUE;
          writelog(ERROR_M,"read_tif() Compression cannot handle format %d",TIFFEntry.Offset);
        }
        break;
      }
      case PhotoInterp:                // 0: bilevel u. Graustufen, 0 ist weis, 1: bilevel u. Gr., 0 ist schwarz
      {                               // 2: RGB 3: RGB über Palette (nicht ausgeführt)
        if ((TIFFEntry.Offset != 1) &&(TIFFEntry.Offset != 2))// word
        {
          err = PCO_ERROR_WRONGVALUE;
          writelog(ERROR_M,"read_tif() PhotometricInterpetation cannot handle format %d",TIFFEntry.Offset);
        }
        break;
      }
/*
      case 0xC53F:
      {
        unsigned char* pucdat;
        int ilen = -1;
        Bild *strbildl;

        DataOffset = TIFFEntry.Offset;
        if(TIFFEntry.ftype == 1)
        {
          ilen = TIFFEntry.length;
        }
        if(TIFFEntry.ftype == 3)
        {
          ilen = TIFFEntry.length * sizeof(short);
        }
        if(TIFFEntry.ftype == 4)
        {
          ilen = TIFFEntry.length * sizeof(long);
          bmulti = TRUE;
        }
        if (ilen == -1)
        {
          CloseHandle(hfread);
          return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
        }
        if(ilen > sizeof(Bild))
          ilen = sizeof(Bild);
        pucdat = (unsigned char*) malloc(ilen + 0x10);
        strbildl = (Bild*)&pucdat[0];
        if((TIFFEntry.ftype == 4)||(TIFFEntry.ftype == 1))// 4: Multi(alt) 1: tif und multi (neu, ab 303)
        {
          strbildl = (Bild*)&pucdat[2];
        }

        SetFilePointer(hfread, DataOffset, 0, FILE_BEGIN);
        ReadFile(hfread, &pucdat[2], ilen, &read, 0);

        if((strbildl->iVersion == FILEVERSION200) ||
           (strbildl->iVersion == FILEVERSION300) ||
           (strbildl->iVersion == FILEVERSION301) ||
           (strbildl->iVersion == FILEVERSION302) ||
           (strbildl->iVersion == FILEVERSION303) ||
           (strbildl->iVersion == FILEVERSION304) ||
           (strbildl->iVersion == FILEVERSION305))
        {
          memcpy(&strBild->sTime, &strbildl->sTime, sizeof(Bild) - sizeof(word*));
          RebuildBildStruct(strBild);
          // close filehandle
        }
        if(bIs217 && strBild->bAlignUpper)
          bcheckalignment = TRUE;
        else
          bcheckalignment = FALSE;
        free(pucdat);

        break;
      }
*/
      case StripOffsets:               // Adresse der einzelnen Strips in der Datei
      {
        DataOffset = TIFFEntry.Offset;
        if (TIFFEntry.length > 1)
        {
          char cSize[6] = { 0, 1, 0, 2, 4, 8 };
          int iBytes;

          if ((TIFFEntry.ftype != 3) &&(TIFFEntry.ftype != 4))
          {
            err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
            writelog(ERROR_M,"read_tif() StripOffsets cannot handle type %d",TIFFEntry.ftype);
            break;
          }
          iBytes = cSize[TIFFEntry.ftype];

          SDataOffset =(DWORD*)malloc(TIFFEntry.length * iBytes);

          lseek(hfread,DataOffset,SEEK_SET);
          bread=read(hfread,SDataOffset, TIFFEntry.length * iBytes);
          if(bmacformat)
          {
            for(unsigned int i = 0; i < TIFFEntry.length; i++)
            {
              if(iBytes == 2)
              {
                mswab((unsigned char*)&SDataOffset[i], (unsigned char*)&ush, sizeof(ush));
                SDataOffset[i] = ush;
              }
              else
              {
                mswab((unsigned char*)&SDataOffset, (unsigned char*)&lh, sizeof(lh));
                SDataOffset[i] = lh;
              }
            }
          }
          SDataBytes = iBytes;
          SDataCnt   = TIFFEntry.length;
          if (bread != (DWORD)(SDataCnt * SDataBytes))
          {
           err = PCO_ERROR_WRONGVALUE;
           writelog(ERROR_M,"read_tif() StripOffsets wrong count %d",bread);
           break;
          }
        }
        else if (TIFFEntry.length == 1)
        {
          char cSize[6] = { 0, 1, 0, 2, 4, 8 };
          int iBytes;

          if ((TIFFEntry.ftype != 3) &&(TIFFEntry.ftype != 4))
          {
           err = PCO_ERROR_WRONGVALUE;
           writelog(ERROR_M,"read_tif() StripOffsets cannot handle type %d",TIFFEntry.ftype);
           break;
          }
          iBytes = cSize[TIFFEntry.ftype];

          SDataOffset =(DWORD*)malloc(TIFFEntry.length * iBytes);
          *SDataOffset = DataOffset;
          SDataCnt   = TIFFEntry.length;
        }
        else
        {
         SDataOffset = NULL;
        }
        break;
      }
      case SamplesPerPixel:            // 1: BW, 3: RGB (24bit), 4: RGB (32bit)
      {
        if ((TIFFEntry.Offset != 1) &&(TIFFEntry.Offset != 3) &&(TIFFEntry.Offset != 4))// word
        {
          err = PCO_ERROR_WRONGVALUE;
          writelog(ERROR_M,"read_tif() SamplesPerPixel cannot handle format %d",TIFFEntry.Offset);
        }
        break;
      }
      case RowsPerStrip:               // Anzahl Pixel je Strip
      {
        if (TIFFEntry.ftype == 3)
          iRowsPerStrip = TIFFEntry.Offset;
        else if (TIFFEntry.ftype == 4)
          iRowsPerStrip = TIFFEntry.Offset;
        else
        {
         err = PCO_ERROR_WRONGVALUE;
         writelog(ERROR_M,"read_tif() RowsPerStrip cannot handle this type %d",TIFFEntry.ftype);
        }
        writelog(NONE_M,"read_tif() RowsPerStrip %d",iRowsPerStrip);
        break;
      }
      case StripByteCnt:               // Anzahl Bytes der jeweiligen Strips
      {
        DataOffset = TIFFEntry.Offset;
        char cSize[6] = { 0, 1, 0, 2, 4, 8 };
        int iBytes;

        if (TIFFEntry.length > 1)
        {
          if ((TIFFEntry.ftype != 3) &&(TIFFEntry.ftype != 4))
          {
            err = PCO_ERROR_WRONGVALUE;
            writelog(ERROR_M,"read_tif() StripByteCount cannot handle this type %d",TIFFEntry.ftype);
            break;
          }
          iBytes = cSize[TIFFEntry.ftype];

          SByteCnt =(DWORD*)malloc(TIFFEntry.length * iBytes);

          lseek(hfread,DataOffset,SEEK_SET);
          read(hfread,SByteCnt, TIFFEntry.length * iBytes);
          if(bmacformat)
          {
            for(unsigned int i = 0; i < TIFFEntry.length; i++)
            {
              if(iBytes == 2)
              {
                mswab((unsigned char*)&SByteCnt[i], (unsigned char*)&ush, sizeof(ush));
                SByteCnt[i] = ush;
              }
              else
              {
                mswab((unsigned char*)&SByteCnt, (unsigned char*)&lh, sizeof(lh));
                SByteCnt[i] = lh;
              }
            }
          }
          SBytes = iBytes;
          SCnt   = TIFFEntry.length;

          if (bread !=(unsigned long)(SCnt * SBytes))
          {
           err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
           writelog(ERROR_M,"read_tif() StripByteCount wrong count %d",bread);
          }
        }
        else if (TIFFEntry.length == 1)
        {
         if ((TIFFEntry.ftype != 3) &&(TIFFEntry.ftype != 4))
         {
          err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
          writelog(ERROR_M,"read_tif() StripByteCount cannot handle type %d",TIFFEntry.ftype);
          break;
         }
         iBytes = cSize[TIFFEntry.ftype];
         SBytes = iBytes;
         SByteCnt =(DWORD*)malloc(TIFFEntry.length * iBytes);
         *SByteCnt = TIFFEntry.Offset;
         SCnt   = TIFFEntry.length;
        }
        else
        {
         SByteCnt = NULL;
        }
        break;
      }
      case PlanarConfig:               // 1: BW, normal RGB (RGBRGBRGB...) 2: Planes (nicht ausgeführt)
      {
        if (TIFFEntry.Offset != 1)// word
        {
          err = PCO_ERROR_WRONGVALUE;
          writelog(ERROR_M,"read_tif() PlanarConfiguration cannot handle format %d",TIFFEntry.Offset);
        }
        break;
      }
    }
    if (err != 0)
     break;
  }

  if (SDataCnt != SCnt)
  {
    err = PCO_ERROR_WRONGVALUE;
    writelog(ERROR_M,"read_tif() wrong data count SDataCnt %d != SCnt %d",SDataCnt != SCnt);
  }
/*
  if(bmulti == TRUE)
  {
    if(*SByteCnt != (DWORD)(width * height * iImageType))
     *SByteCnt = (DWORD)(width * height * iImageType);// Korrektur eines Fehlers in mmfilewriter...
  }
*/

  if(err)
  {
   close(hfread);
   if(SDataOffset != NULL)
    free(SDataOffset);
   if(SByteCnt != NULL)
    free(SByteCnt);
   return (err);
  }

  WORD *os, *bc;
  DWORD *dwp;
  WORD  *cp;
  char  *cccp;
  unsigned long lOs, lBc;

  p =(WORD*)malloc(width * height * iImageType);

  os =(WORD*)SDataOffset;
  bc =(WORD*)SByteCnt;
  cp = p;
  for (short j = 0; j < SCnt; j++)      // Stripdaten aus File auslesen und in Zwischenspeicher eintragen
  {
   if (SBytes == 4)                    // Pointer sind DWORDs
   {
    dwp =(DWORD*)os;
    lOs = *dwp;
    dwp =(DWORD*)bc;
    lBc = *dwp;
   }
   else                               // Pointer sind WORDs
   {
    lOs = *os;
    lBc = *bc;
   }

   lseek(hfread,lOs,SEEK_SET);
   bread=read(hfread,cp,lBc);
   if (bread != lBc)                    // gelesen = gefordert ?
   {
    err = PCO_ERROR_WRONGVALUE;
    writelog(ERROR_M,"read_tif() read data failed %d bytes from offest",lOs,lBc);
    break;
   }
   os++;                              // Pointer erhöhen um 2 Byte
   bc++;
   if (SBytes == 4)                    // falls DWORDs nochmals um 2 Byte erhöhen
   {
    os++;
    bc++;
   }
    // cp += (read>>1);
   cccp =(char*) cp;                 // Ziel um Anzahl gelesener Bytes erhöhen 
   cccp += bread;
   cp =(WORD*) cccp;
  }

  close(hfread);                 // Datei schliessen

  if (err != PCO_NOERROR)
  {                                    // bei Fehler Filedatenspeicher freigeben
   if (p != NULL)
   {
    free(p);
    p = NULL;
   }
  }

  if(SDataOffset != NULL)              // Speicher freigeben
   free(SDataOffset);
  if(SByteCnt != NULL)
   free(SByteCnt);
  os = NULL;
  bc = NULL;
  cp = NULL;


/*  
  if ((!((width != 0) &&(height != 0))) ||(err))
  {
    return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
  }
  int ishift;
  ishift = 0;

  
  strBild->iXRes = width;              // Bildgrössen übergeben
  strBild->iYRes = height;
  unsigned short usmaskis[8] = { 0, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};
  unsigned short usmask;

  usmask = usmaskis[16 - strBild->iBitRes];

  if (iImageType == 1)                  // 8bit BW in 16bit Rohformat eintragen
  {
    short *spl;
    char  *cpl;
    long k, l;
    
    spl =(short*)im;                  // Zieldaten
    cpl =(char*) p;                   // Filedaten

    for (k = 0; k < height; k++)
    {
      for (l = 0; l < width; l++)
      {
        *spl = (unsigned char)*cpl;                   // 8bit übernehmen
        *spl <<= ishift;

        spl++;                         // nächstes Pixel
        cpl++;
      }
    }
  }

  if (iImageType == 2)                  // 16bit direkt kopieren
  {
    long m;
    WORD *o;

    o = p;
    memset(im, 0, width * height *2);    // Zieldaten auf 0 setzen
    for (m = 0; m < height; m++)        // zeilenweise umkopieren, da Bytes ausgeblendet
    {                                  // werden, um 4 Byte Grenzen einzuhalten
      if(bmacformat)
      {
        for (int ix = 0; ix < width; ix++)
        {
          ush = o[ix];
          mswab((unsigned char*)&ush, (unsigned char*)&ush2, sizeof(ush2));
          ((WORD*)im)[ix + m * width] = ush2;
        }
      }
      else
      {
        if(bcheckalignment)
        {
          for (int ix = 0; ix < width; ix++)
          {
            ush = o[ix];
            ((WORD*)im)[ix + m * width] = ush;
            if(ush & usmask)           // Behebt CamWare-Fehler beim tif speichern.
            {
              strBild->bAlignUpper = FALSE;
              bcheckalignment = FALSE;
            }
          }
        }
        else
          memcpy((WORD*)im + m * width, (const void*)o, width * 2);
      }
      o += width;                      // nächste Zeile
    }
  }

  if ((iImageType == 3) ||(iImageType == 4))// RGB auf Rohformat filtern
  {
    long k, l;
    bool bRed = TRUE;
    bool bTog = TRUE;
    unsigned char *cpl;
    unsigned short *spl;

    cpl =(unsigned char*) p;                   // Bilddaten aus File
    spl =(unsigned short*) im;                 // Bilddaten (Ziel)
    for (k = 0; k < height; k++)
    {
      bTog = TRUE;
      if (bRed)                         // rote Zeile
      {
        for (l = 0; l < width; l++)
        {
          *spl = (unsigned char)*cpl;   // 8bit Inhalt übernehmen
          *spl <<= ishift;
          if (bTog)                     // zeigt auf roten Pixel
          {
            bTog = FALSE;
            cpl += 4;                  // nächster ist grün
          }
          else                         // zeigt auf grünen Pixel
          {
            bTog = TRUE;
            cpl += 2;                  // nächster ist rot
          }
          if (iImageType == 4)          // 4. Byte ausblenden
            cpl++;
          spl++;
        } 
        if(bTog)
          cpl++;                        // vorbereiten auf blaue Zeile, nächster ist grün
        bRed = FALSE;
      }
      else                             // blaue Zeile
      {
        for (l = 0; l < width; l++)
        {
          *spl = (unsigned char)*cpl;
          *spl <<= ishift;
          if (bTog)                     // zeigt auf grünen Pixel
          {
            bTog = FALSE;
            cpl += 4;                  // nächster ist blau
          }
          else                         // zeigt auf blauen Pixel
          {
            bTog = TRUE;
            cpl += 2;                  // nächster ist grün
          }
          if (iImageType == 4)          // 4. Byte ausblenden
            cpl++;
          spl++;
        } 
        if(!bTog)
          cpl -= 2;                     // vorbereiten auf rote Zeile, nächster ist rot
        else
          cpl--;
        bRed = TRUE;
      }
    }
  }
  RebuildBildStruct(strBild);
  if((iImageType == 2) && (strBild->bAlignUpper) && (iNoShifting == 0))
  {
    Shift((WORD*)im, strBild->iXRes * strBild->iYRes, FALSE, 16 - strBild->iBitRes);
  }

  if (p != NULL)
    free(p);
  return (err);
*/




  return err;
}

