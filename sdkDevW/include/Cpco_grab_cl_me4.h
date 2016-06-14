//-----------------------------------------------------------------//
// Name        | CPco_grab_cl_me4.h          | Type: ( ) source    //
//-------------------------------------------|       (*) header    //
// Project     | pco.camera                  |       ( ) others    //
//-----------------------------------------------------------------//
// Platform    | WINDOWS 2000/XP                                   //
//-----------------------------------------------------------------//
// Environment | Microsoft Visual C++ 6.0                          //
//             |                                                   //
//-----------------------------------------------------------------//
// Purpose     | pco.camera - CameraLink ME4 API                   //
//-----------------------------------------------------------------//
// Author      | MBL, PCO AG                                       //
//-----------------------------------------------------------------//
// Revision    | rev. 1.01 rel. 0.00                               //
//-----------------------------------------------------------------//
// Notes       | In this file are all functions and definitions,   //
//             | for communication with CameraLink grabbers        //
//             |                                                   //
//             |                                                   //
//-----------------------------------------------------------------//
// (c) 2010 PCO AG * Donaupark 11 *                                //
// D-93309      Kelheim / Germany * Phone: +49 (0)9441 / 2005-0 *  //
// Fax: +49 (0)9441 / 2005-20 * Email: info@pco.de                 //
//-----------------------------------------------------------------//

#ifndef CPCO_GRAB_CL_ME4_H
#define CPCO_GRAB_CL_ME4_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#include "fgrab_prototyp.h"
#include "fgrab_struct.h"
#include "fgrab_define.h"
#pragma GCC diagnostic pop


#ifdef WIN32
#include <io.h>
#endif

#include "Cpco_com_cl_me4.h"
#include "sc2_cl_defs.h"

#define ID_DOUBLEMODE      0x01
#define ID_TIMESTAMP       0x02
#define ID_NOISEFILTER     0x03
#define ID_COLORSENSOR     0x04
#define ID_PIXELRATE       0x05
#define ID_ALIGNMENT       0x06



///
/// \brief The CPco_grab_cl_me4 class
///
/// This is the grabber class for CameraLink ME4. It it responsible for starting image acquisitions and the actual transfer.
///
class CPco_grab_cl_me4
{

public:


protected:
  PCO_HANDLE hgrabber;
  CPco_com_cl_me4 *cam;
  CPco_Log *clog;

  Fg_Struct *fg;
  int pco_hap_loaded;
  int port;
  int me_boardnr;

  int buf_manager;
  dma_mem*  pMem0;
  dma_mem*  pMemInt;
  unsigned char** padr;

  int nr_of_buffer;
  DWORD size_alloc;

  int aquire_status;
  int aquire_flag;
  int last_picnr;
  DWORD act_dmalength;
  int ImageTimeout;

  DWORD act_width,act_height;
  DWORD act_linewidth;
  unsigned char* adr_line;
  DWORD line_alloc;

  DWORD DataFormat;
  PCO_SC2_CL_TRANSFER_PARAM clpar;
  SC2_Camera_Description_Response description;

  WORD  camtype;
  DWORD serialnumber;
  DWORD cam_pixelrate;
  WORD cam_timestampmode;  
  WORD cam_doublemode;
  WORD cam_align;
  WORD cam_noisefilter;
  WORD cam_colorsensor;
  DWORD cam_width,cam_height;


public:
  CPco_grab_cl_me4(CPco_com_cl_me4* camera=NULL);

  virtual ~CPco_grab_cl_me4(){}
  virtual DWORD Open_Grabber(int board)=0;
  virtual DWORD Set_Grabber_Size(DWORD width,DWORD height)=0;
  virtual DWORD Set_DataFormat(DWORD format)=0;
  virtual DWORD PostArm(int userset=0)=0; 

  virtual void Extract_Image(void *bufout,void *bufin,int width,int height)=0;
  virtual void Get_Image_Line(void *bufout,void *bufin,int linenumber,int width,int height)=0;

  virtual DWORD Check_DRAM_Status(char *mess ATTRIBUTE_UNUSED,int mess_size ATTRIBUTE_UNUSED,int *fill ATTRIBUTE_UNUSED){return PCO_NOERROR;}


  ///
  /// \brief Sets the logging file for the grabber
  ///
  /// \param elog Pointer to a logging class object
  ///
    void SetLog(CPco_Log *elog);

  ///
  /// \brief Closes the grabber. This should be done before calling Close_Cam().
  /// \anchor Close_Grabber
  /// \return Error or 0 in case of success
  ///
  DWORD Close_Grabber();
  ///
  ///
  /// \brief Sets the grabber timeout
  /// \anchor Set_Grabber_Timeout
  /// \param timeout new timeout in ms
  /// \return always 0
  ///
  DWORD Set_Grabber_Timeout(int timeout);
  ///
  /// \overload Allocate_Framebuffer(int,void*)
  ///
  DWORD Allocate_Framebuffer(int nr_of_buffer);
  ///
  /// \brief Allocates the specified number of framebuffers.
  /// \anchor Allocate_Framebuffer
  /// \param nr_of_buffer Number of buffers to allocate
  /// \param adr_in Address to memory where the buffers should be allocated. If set to NULL the function will allocate the neccessary memory. (recommended)
  /// \return Error code or or 0 in case of success
  ///
  DWORD Allocate_Framebuffer(int nr_of_buffer,void *adr_in);

  ///
  /// \overload Free_Framebuffer
  ///
  DWORD Free_Framebuffer();
  ///
  /// \brief Frees all framebuffers
  /// \anchor Free_Framebuffer
  /// \param adr_in Address to the memory where the buffers should be cleared. If set to NULL the function will free all available buffers. (recommended)
  /// \return Error code or or 0 in case of success
  ///
  DWORD Free_Framebuffer(void *adr_in);
  ///
  /// \brief Gets the number of the current framebuffer
  /// \anchor Get_Framebuffer
  /// \return Number of the current framebuffer
  ///
  DWORD Get_Framebuffer();
  ///
  /// \brief Unblocks the specified buffer. The buffer may be overwritten at any point after this function has been called.
  /// \anchor Unblock_buffer
  /// \param num Number of the buffer to unblock.
  /// \return Error code or or 0 in case of success
  ///
  DWORD Unblock_buffer(int num);
  ///
  /// \brief A simple image acquisition function.
  /// \anchor Acquire_Image
  /// This function calls Start_Acquire(), then waits until a single image is in the grabber buffer.
  //  The image is then extracted to the specified address. Then Stop_Acquire() is called.
  /// This function should be used when timing isn't critical.
  /// \param address The address where the image is extracted to.
  /// \return Error code or or 0 in case of success
  ///
  DWORD Acquire_Image(void *address);

  ///
  /// \brief Transfers an image from the recorder buffer of the camera.
  /// \anchor Acquire_Image
  /// This is a synchronous transfer and should be used in time-insensitive cases.
  /// \param adr Pointer to address where the image gets stored
  /// \param Segment select segment of recorder buffer
  /// \param ImageNr select image in recorder buffer
  /// \return Error or 0 in case of success
  ///
  DWORD Get_Image(WORD Segment,DWORD ImageNr,void *adr);
  ///
  /// \overload Start_Acquire(int,int)
  ///
  DWORD Start_Acquire(int num);
  ///
  /// \brief Starts image acquisition.
  ///
  /// This should be the first function called when starting a new image acquisition series.
  /// After that, only calls to Wait_For_Image and Extract_Image are neccessary. Don't forget
  /// to allocate any neccessary framebuffers before calling this function!
  ///
  /// \param nr_of_ima number of images to grab
  /// \param flag Blocking or non-blocking buffer. (PCO_SC2_CL_NON_BLOCKING_BUFFER or PCO_SC2_CL_BLOCKING_BUFFER)
  /// \return Error code, or or 0 in case of success
  ///
  DWORD Start_Acquire(int nr_of_ima,int flag);
  ///
  /// \brief As Start_Acquire(), but with nonblocking buffer
  /// \param nr_of_ima number of images to grab
  /// \return Error code or or 0 in case of success
  ///
  DWORD Start_Acquire_NonBlock(int nr_of_ima);
  ///
  /// \brief Stops image acquisition.
  /// \anchor Stop_Acquire
  /// \return Error code or 0 in case of success
  ///
  DWORD Stop_Acquire();
  ///
  /// \brief Returns the status of the image acquisition.
  /// \return Acquisition status: TRUE = running, FALSE = stopped.
  ///
  BOOL started();
  ///
  /// \brief Waits until the next image is completely transferred.
  /// The input pointer will be filled with a number that can then be passed on to Get_Framebuffer_adr to get the memory address
  /// where the picture is stored.
  /// \param nr_of_pic Pointer to int variable that receives the picture number (or 0 in case of an error!)
  /// \param timeout Sets a timeout in seconds. After this the function returns with an error.
  /// \return Error code or 0 in case of success
  ///
  DWORD Wait_For_Next_Image(int *nr_of_pic,int timeout);
  ///
  /// \brief Waits until all images are completely transferred to the preallocated buffers.
  /// \param nr_of_pic number of picture to wait for
  /// \param timeout Sets a timeout in seconds. After this the function returns with an error.
  /// \return Error code or 0 in case of success
  ///
  DWORD Wait_For_Images(int nr_of_pic,int timeout);

  DWORD Get_last_Image(int *nr_of_pic);
  ///
  /// \brief Gets the framebuffer address where the picture with number "nr_of_pic" is stored.
  /// \param nr_of_pic Number of the picture to get the address.
  /// \param adr Pointer to Pointer to memory where the image is stored.
  /// \return Error code or 0 in case of success
  ///
  DWORD Get_Framebuffer_adr(int nr_of_pic,void **adr);
  ///
  /// \brief Returns the current grabber sizes
  /// \param width Current width
  /// \param height Current height
  /// \param bitpix current bits per pixel
  /// \return Returns always 0.
  ///
  DWORD Get_actual_size(unsigned int *width,unsigned int *height,unsigned int *bitpix);
  ///
  /// \brief Returns the current grabber format
  /// \return Returns always the actual DataFormat
  ///
  DWORD Get_DataFormat(){return DataFormat;}
  ///
  /// \brief Set camera parameters to the grabber class, which are needed for correct handling of image data
  /// \param id see ID_ defines at top of this file
  /// \param value correct value for the appropriate id, retrieved from camera after last PCO_ArmCamera() call
  /// \return Error code or 0 in case of success
  ///
  DWORD Set_Grabber_Param(int id, int value);
  ///
  /// \brief Check if DMA transfer was successful
  /// \param num number of the picture to check
  /// \return Error code or 0 in case of success
  ///
  DWORD Check_DMA_Length(int num);
  ///
  /// \brief Set BitAlignment parameter to the grabber class, which is needed for correct handling of image data
  /// \param act_align value retrieved from camera after last PCO_ArmCamera() call
  /// \return Error code or 0 in case of success
  ///
  DWORD SetBitAlignment(int act_align);

protected:

  void writelog(DWORD lev,PCO_HANDLE hdriver,const char *str,...);
  ///
  /// \brief Gets the last error message and writes it to the log
  /// \anchor Fg_Error
  /// \param fg The Fg_Struct member variable.
  ///
  void Fg_Error(Fg_Struct *fg);
  ///
  /// \brief Get the actual camera parameters, which are needed for correct handling of image data
  ///
  DWORD Get_Camera_Settings(); 

};

///
/// \brief The class for a pco.edge 5.5 Rolling Shutter
///
class CPco_grab_cl_me4_edge : public CPco_grab_cl_me4
{

public:
  CPco_grab_cl_me4_edge(CPco_com_cl_me4* camera);
  ///
  /// \brief Opens the grabber and retrieves the neccessary variables from the camera object.
  /// \anchor Open_Grabber
  /// \param board Set to zero if there is only one camera connected.
  /// Open_Cam() on the appropriate class object \b must be called first or this will fail!
  /// \return Error or 0 in case of success
  ///
  DWORD Open_Grabber(int board);
  ///
  /// \brief Sets the grabber size.
  /// \anchor Set_Grabber_Size
  /// It is extremely important to set this before any images are transferred! If any of the settings are changed that influence the image size
  /// Set_Grabber_Size \b must be called again before any images are transferred! If this is not done, memory or segmentation faults will occur!
  /// \param width Actual width of the picture
  /// \param height Actual height of the picture
  /// \return Error or 0 in case of success
  ///
  DWORD Set_Grabber_Size(DWORD width,DWORD height);
  ///
  /// \brief Sets the data format for the grabber
  /// \anchor Set_DataFormat
  /// \param format The new data format.
  /// \return always 0
  ///
  DWORD Set_DataFormat(DWORD format);
  ///
  /// \brief Reorders the image lines and copies the reordered image to the output buffer.
  /// \anchor Extract_Image
  /// \param bufout Output buffer. Must be large enough to hold the image!
  /// \param bufin Input buffer.
  /// \param width Image width.
  /// \param height Image height.
  /// \param line_width Unused.
  ///
  void Extract_Image(void *bufout, void *bufin, int width, int height,int line_width ATTRIBUTE_UNUSED);
  ///
  /// \overload Extract_Image(void*,void*,int,int,int)
  ///
  void Extract_Image(void *bufout,void *bufin,int width,int height);
  ///
  /// \brief Get a single image line
  /// \anchor Get_Image_Line
  /// \param bufout Output buffer. Must be large enough to hold a single image line!
  /// \param bufin Input buffer.
  /// \param linenumber Line number to get.
  /// \param width Image width.
  /// \param height Image height.
  ///
  void Get_Image_Line(void *bufout,void *bufin,int linenumber,int width,int height);
  ///
  /// \brief Get current camera parameter and set camera and grabber parameter if necessary
  /// \anchor PostArm
  /// camera parameters changed: Lut and TransferParameter
  /// grabber parameters changed: DataFormat, width, height, size of allocated framebuffers
  /// \param userset when 0 camera and grabber parameters are changed
  /// \param userset when 1 only camera parameters are changed
  DWORD PostArm(int userset=0); 

  DWORD Check_DRAM_Status(char *mess,int mess_size,int *fill);

  ~CPco_grab_cl_me4_edge(){Close_Grabber();}
protected:
};


///
/// \brief The class for a pco.edge 4.2 Rolling Shutter
///
class CPco_grab_cl_me4_edge42 : public CPco_grab_cl_me4
{

public:
  CPco_grab_cl_me4_edge42(CPco_com_cl_me4* camera);
  ///
  /// \brief Opens the grabber and retrieves the neccessary variables from the camera object.
  /// \anchor Open_Grabber
  /// \param board Set to zero if there is only one camera connected.
  /// Open_Cam() on the appropriate class object \b must be called first or this will fail!
  /// \return Error or 0 in case of success
  ///
  DWORD Open_Grabber(int board);
  ///
  /// \brief Sets the grabber size.
  /// \anchor Set_Grabber_Size
  /// It is extremely important to set this before any images are transferred! If any of the settings are changed that influence the image size
  /// Set_Grabber_Size \b must be called again before any images are transferred! If this is not done, memory or segmentation faults will occur!
  /// \param width Actual width of the picture
  /// \param height Actual height of the picture
  /// \return Error or 0 in case of success
  ///
  DWORD Set_Grabber_Size(DWORD width,DWORD height);
  ///
  /// \brief Sets the data format for the grabber
  /// \anchor Set_DataFormat
  /// \param format The new data format.
  /// \return always 0
  ///
  DWORD Set_DataFormat(DWORD format);
  ///
  /// \brief Reorders the image lines and copies the reordered image to the output buffer.
  /// \anchor Extract_Image
  /// \param bufout Output buffer. Must be large enough to hold the image!
  /// \param bufin Input buffer.
  /// \param width Image width.
  /// \param height Image height.
  /// \param line_width Unused.
  ///
  void Extract_Image(void *bufout, void *bufin, int width, int height,int line_width ATTRIBUTE_UNUSED);
  ///
  /// \overload Extract_Image(void*,void*,int,int,int)
  ///
  void Extract_Image(void *bufout,void *bufin,int width,int height);
  ///
  /// \brief Get a single image line
  /// \anchor Get_Image_Line
  /// \param bufout Output buffer. Must be large enough to hold a single image line!
  /// \param bufin Input buffer.
  /// \param linenumber Line number to get.
  /// \param width Image width.
  /// \param height Image height.
  ///
  void Get_Image_Line(void *bufout,void *bufin,int linenumber,int width,int height);
  ///
  /// \brief Get current camera parameter and set camera and grabber parameter if necessary
  /// \anchor PostArm
  /// camera parameters changed: none
  /// grabber parameters changed: DataFormat, width, height, size of allocated framebuffers
  /// \param userset when 0 camera and grabber parameters are changed
  /// \param userset when 1 only camera parameters are changed
  DWORD PostArm(int userset=0); 

  ~CPco_grab_cl_me4_edge42(){Close_Grabber();}

protected:

};


///
/// \brief The class for a generic pco camera
///
class CPco_grab_cl_me4_camera : public CPco_grab_cl_me4
{

public:
  CPco_grab_cl_me4_camera(CPco_com_cl_me4* camera);
  ///
  /// \brief Opens the grabber and retrieves the neccessary variables from the camera object.
  ///
  /// \param board Set to zero if there is only one camera connected.
  /// Open_Cam() on the appropriate class object \b must be called first or this will fail!
  /// \return Error or 0 in case of success
  ///
  DWORD Open_Grabber(int board);
  ///
  /// \brief Sets the grabber size.
  ///
  /// It is extremely important to set this before any images are transferred! If any of the settings are changed that influence the image size
  /// Set_Grabber_Size \b must be called again before any images are transferred! If this is not done, memory or segmentation faults will occur!
  /// \param width Actual width of the picture
  /// \param height Actual height of the picture
  /// \return Error or 0 in case of success
  ///
  DWORD Set_Grabber_Size(DWORD width,DWORD height);
  ///
  /// \brief Sets the data format for the grabber
  /// \anchor Set_DataFormat
  /// \param format The new data format.
  /// \return always 0
  ///
  DWORD Set_DataFormat(DWORD format);
  ///
  /// \overload Extract_Image(void*,void*,int,int,int)
  ///
  void Extract_Image(void *bufout,void *bufin,int width,int height);
  ///
  /// \brief Reorders the image lines and copies the reordered image to the output buffer.
  ///
  /// \param bufout Output buffer. Must be large enough to hold the image!
  /// \param bufin Input buffer.
  /// \param width Image width.
  /// \param height Image height.
  /// \param line_width Unused.
  ///
  void Extract_Image(void *bufout, void *bufin, int width, int height,int line_width ATTRIBUTE_UNUSED);
  ///
  /// \brief Get a single image line
  ///
  /// \param bufout Output buffer. Must be large enough to hold a single image line!
  /// \param bufin Input buffer.
  /// \param linenumber Line number to get.
  /// \param width Image width.
  /// \param height Image height.
  ///
  void Get_Image_Line(void *bufout,void *bufin,int linenumber,int width,int height);
  ///
  /// \brief Get current camera parameter and set camera and grabber parameter if necessary
  /// \anchor PostArm
  /// camera parameters changed: none
  /// grabber parameters changed: DataFormat, width, height, alignment, size of allocated framebuffers
  /// \param userset when 0 camera and grabber parameters are changed
  /// \param userset when 1 only camera parameters are changed
  DWORD PostArm(int userset=0); 

  ~CPco_grab_cl_me4_camera(){Close_Grabber();}

};


#endif
