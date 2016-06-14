//-----------------------------------------------------------------//
// Name        | Cpco_com.h                  | Type: ( ) source    //
//-------------------------------------------|       (*) header    //
// Project     | pco.camera                  |       ( ) others    //
//-----------------------------------------------------------------//
// Platform    | LINUX                                             //
//-----------------------------------------------------------------//
// Environment | gcc                                               //
//             |                                                   //
//-----------------------------------------------------------------//
// Purpose     | pco.camera - Communication                        //
//-----------------------------------------------------------------//
// Author      | MBL, PCO AG                                       //
//-----------------------------------------------------------------//
// Revision    | rev. 1.04                                         //
//-----------------------------------------------------------------//
// Notes       | Common functions                                  //
//             |                                                   //
//             |                                                   //
//-----------------------------------------------------------------//
// (c) 2010 - 2015 PCO AG                                          //
// Donaupark 11 D-93309  Kelheim / Germany                         //
// Phone: +49 (0)9441 / 2005-0   Fax: +49 (0)9441 / 2005-20        //
// Email: info@pco.de                                              //
//-----------------------------------------------------------------//


//-----------------------------------------------------------------//
// Revision History:                                               //
//  see Cpco_com.cpp                                               //
//-----------------------------------------------------------------//

#ifndef CPCO_COM_H
#define CPCO_COM_H

// GNU specific __attribute__((unused)) define
#ifdef __GNUC__
#define ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define ATTRIBUTE_UNUSED
#endif

#include "pco_includes.h"

#if !defined (MAX_PATH)
#define MAX_PATH 1024
#endif

///
/// \brief Base interface class
///
/// Derived from this class are all interface specific classes. It includes some common functions and defines the mandatory functions that each subclass has to implement.
///

class CPco_com {
public:
    WORD   num_lut;
    SC2_LUT_DESC cam_lut[10];

protected:
    //common
    CPco_Log *clog;
    PCO_HANDLE hdriver;
    DWORD  initmode;
    DWORD  boardnr;
    DWORD  camerarev;
    bool internal_open;
    DWORD connected;

    PCO_SC2_TIMEOUTS tab_timeout;
    SC2_Camera_Description_Response description;

    //functions
public:
    CPco_com();
    virtual ~CPco_com(){}

    ///
    /// \brief Sets the logging file for the communication class
    /// \anchor SetLog
    /// \param elog Pointer to a CPco_Log
    ///
    void SetLog(CPco_Log *elog);

    ///
    /// \brief Opens a connection to a pco.camera
    /// \anchor Open_Cam
    /// \param num Number of the camera. In most cases, this should be zero.
    /// \return Error code or 0 on success
    ///
    virtual DWORD Open_Cam(DWORD num)=0;

    ///
    /// \brief Opens a connection to a pco.camera
    /// \anchor Open_Cam_Ext
    /// \param num Number of the camera. In most cases, this should be zero.
    /// \param open Unused.
    /// \return Error code or 0 on success
    ///
    virtual DWORD Open_Cam_Ext(DWORD num,SC2_OpenStruct *open)=0;

    ///
    /// \brief Closes a connection with a pco.camera
    /// \anchor Close_Cam
    /// Not all classes derived from this must implement a close function.
    /// \return Error code or 0 on success
    ///
    virtual DWORD Close_Cam(){return PCO_NOERROR;}

    ///
    /// \brief The main function to communicate with the camera via telegrams
    /// \anchor Control_Command
    /// See sc2_telegram.h for a list of telegrams and sc2_command.h for a list of possible commands
    ///
    /// Checksum calculation is done in this function, no need to pre-calculate it.
    /// \param buf_in Pointer to the buffer where the telegram is stored
    /// \param size_in Pointer to a DWORD that holds the size of the buffer
    /// \param buf_out Pointer to the buffer where the response gets stored
    /// \param size_out Pointer to a DWORD that holds the size of the buffer. This is updated with the new size. Maximum telegram size is around 270 bytes.
    /// \return Error code or 0 on success
    ///
    virtual DWORD Control_Command(void *buf_in,DWORD size_in,void *buf_out,DWORD size_out)=0;

    ///
    /// \brief Gets the current timeouts for images and telegrams
    /// \anchor gettimeouts
    /// \param timeouts Pointer to a PCO_SC2_TIMEOUTS structure
    ///
    void gettimeouts(PCO_SC2_TIMEOUTS *timeouts);

    ///
    /// \brief GetConnectionStatus
    /// \anchor GetConnectionStatus
    /// \return Connectionstatus
    /// \retval 1 connected
    /// \retval 0 not connected
    int GetConnectionStatus();

    ///
    /// \brief Sets the connection status
    /// \anchor SetConnectionStatus
    /// \param status
    ///
    void SetConnectionStatus(int status);

    ///
    /// \brief Common sleep function for Linux/Windows
    /// \anchor Sleep_ms
    /// \param time Time to sleep in ms
    ///
    void Sleep_ms(int time);

    ///
    /// \brief Sets the timeouts
    /// \anchor Set_Timeouts
    /// \param timetable Pointer to an unsigned int array. First parameter is the command timeout, second the image timeout, third the transfer timeout.
    /// \param len Length of the array in bytes, a maximum of 12 bytes are used.
    ///
    void Set_Timeouts(void *timetable,DWORD len);

protected:
    ///
    /// \brief Scans the connected camera and gets basic informations
    /// \anchor scan_camera
    /// \return Error code or 0 on success
    ///
    virtual DWORD scan_camera()=0;

    ///
    /// \brief Builds the checksum for Control_Command() telegrams
    /// \anchor build_checksum
    /// \param buf Pointer to buffer that contains the telegram
    /// \param size Pointer to size of the buffer
    /// \return Error code or 0 on success
    ///
    DWORD build_checksum(unsigned char *buf,int *size);

    ///
    /// \brief Verifies the checksum for a telegram
    /// \anchor test_checksum
    /// \param buf Pointer to buffer that contains the telegram
    /// \param size Pointer to size of the buffer
    /// \return Error code or 0 on success
    ///
    DWORD test_checksum(unsigned char *buf,int *size);

    ///
    /// \brief Gets the camera descriptor and caches it
    /// \anchor get_description
    /// \return Error code or 0 on success
    ///
    DWORD get_description();

    ///
    /// \brief Gets camera main processor and main FPGA firmware versions and writes them to the logfile
    /// \anchor get_firmwarerev
    /// \return Error code or 0 on success
    ///
    DWORD get_firmwarerev();

    ///
    /// \brief Gets installed LUTs and writes them to the logfile
    /// \anchor get_lut_info
    /// \return Error code or 0 on success
    ///
    DWORD get_lut_info();

    ///
    /// \brief Writes a log entry
    /// \anchor writelog
    /// \param lev Type of the entry, e.g. Error, Status, Message, ...
    /// \param hdriver The source of the error
    /// \param str The actual error as printf formatted string
    ///
    void writelog(DWORD lev,PCO_HANDLE hdriver,const char *str,...);

public:

#include "Cpco_com_func.h"
#include "Cpco_com_func_2.h"

};

#endif





