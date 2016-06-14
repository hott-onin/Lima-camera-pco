//-----------------------------------------------------------------//
// Name        | Cpco_com_func.h             | Type: ( ) source    //
//-------------------------------------------|       (*) header    //
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
// Revision    |                                                   //
//-----------------------------------------------------------------//
// Notes       | Telegram functions                                //
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
//  see Cpco_com_func.cpp                                          //
//-----------------------------------------------------------------//

#ifndef CPCO_COM_FUNC_H
#define CPCO_COM_FUNC_H

///
/// \file Cpco_com_func.h
///
/// \brief Basic camera communication
///
/// \author PCO AG
///

/// @name Camera Control Functions
///
/// These functions are used to communicate with the camera.
/// They always return an Error message, 0 in case of success else less than 0.
///


///
/// \brief Requests the current recording state.
///
/// The recording state controls the status of the camera. If the recording status is [run], images can
/// be started by exposure trigger and \<acq enbl\>. If the recording status is [clear]'ed or [stop]'ped,
/// all image readout or exposure sequences are stopped and the sensors (CCD or CMOS) are running
/// in a special idle mode to prevent dark charge accumulation.
///
/// The recording status has the highest priority compared to functions like \<acq enbl\> or exposure trigger.
///
/// The recording status is controlled by:
/// - software command: set recording status = [run]
///
/// The recording status is cleared by:
/// - powering ON the camera
/// - software command: set recording status = [stop]
/// - software command: reset all settings to default values
///
/// \param recstate Pointer to a WORD variable to receive the recording state.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetRecordingState(WORD *recstate);

///
/// \brief Sets the current recording status and waits until the status is valid. If the state can't be set the function will return an error.
///
/// The recording state controls the status of the camera. If the recording status is [run], images can
/// be started by exposure trigger and \<acq enbl\>. If the recording status is [clear]'ed or [stop]'ped,
/// all image readout or exposure sequences are stopped and the sensors (CCD or CMOS) are running
/// in a special idle mode to prevent dark charge accumulation.
///
/// The recording status has the highest priority compared to functions like \<acq enbl\> or exposure trigger.
///
/// The recording status is controlled by:
/// - software command: set recording status = [run]
///
/// The recording status is cleared by:
/// - powering ON the camera
/// - software command: set recording status = [stop]
/// - software command: reset all settings to default values
///
/// \param recstate WORD variable to hold the recording state.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetRecordingState(WORD recstate);

///
/// \brief Arms the camera and validates the settings
///
/// Arms, i.e. prepares the camera for a consecutive set recording status = [run] command. All
/// configurations and settings made up to this moment are accepted and the internal settings of the
/// camera are prepared. Thus the camera is able to start immediately when the set recording status = [run] command is performed.
///
/// \b Note: It is required to issue an arm camera command before every set recording state = [run]
/// command in order to ensure that all settings are accepted correctly. Do not change settings
/// between arm camera command and set recording status command. It is possible to
/// change the timing by calling PCO_SetDelayExposureTime() after the recording state = [run].
/// \ref PCO_SetDelayExposureTime
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_ArmCamera();

///
/// \brief Set mode of the timestamp function.
///
/// To obtain information about the recording time of images this command can be useful. It writes a
/// continuous image number and date / time information with a resolution of 10 \f$\mu\f$s direct into the
/// raw image data. The first 14 pixels (top left corner) are used to hold this information. The
/// numbers are coded in BCD with one byte per pixel, which means that every pixel can hold 2
/// digits. If the pixels have more resolution as 8 bits, then the BCD digits are left bound adjusted and
/// the lower bits are zero. Additionally to this 14 pixels, the information can be written in ASCII text
/// for direct inspection. An 8 by 8 pixel array is used per ASCII digit. The digits are displayed below the BCD coded line.
///
/// The input data should be filled with the following parameter:
/// - 0x0000 = no stamp in image
/// - 0x0001 = BCD coded stamp in the first 14 pixel
/// - 0x0002 = BCD coded stamp in the first 14 pixel + ASCII text
/// - 0x0003 = ASCII text only (see descriptor for availability)
///
/// \b Note:
/// - the image number is set to value = [1], when an arm command is performed
/// - using this command without setting the [date] / [time] results in an error message
///
/// Format of BCD coded pixels:
/// Pixel 1 | Pixel 2 | Pixel 3 | Pixel 4 | Pixel 5 | Pixel 6 | Pixel 7 |
/// --------|---------|---------|---------|---------|---------|---------|
/// image counter (MSB) (00...99)  | image counter (00...99)  | image counter (00...99) | image counter (LSB) (00...99) | year (MSB) (20) | year (LSB) (15...99) | month (01...12) |
///
/// Pixel 8 | Pixel 9 | Pixel 10 | Pixel 11 | Pixel 12 | Pixel 13 | Pixel 14 |
/// --------|---------|---------|---------|---------|---------|---------|
/// day (01...31) | h (00...23) | min (00...59) | s (00...59) | \f$\mu\f$s * 10000 (00...99) | \f$\mu\f$s * 100 (00...99) | \f$\mu\f$s (00...99) |
/// \param mode WORD variable to hold the time stamp mode.
/// \return Error message, 0 in case of success else less than 0
DWORD PCO_SetTimestampMode(WORD mode);

///
/// \brief Get mode of the timestamp function.
///
/// The input pointer will be filled with the following parameter:
/// - 0x0000 = no stamp in image
/// - 0x0001 = BCD coded stamp in the first 14 pixel
/// - 0x0002 = BCD coded stamp in the first 14 pixel + ASCII text
/// - 0x0003 = ASCII text only (see descriptor for availability)
///
/// \param mode Pointer to a WORD variable to receive the time stamp mode.
/// See PCO_SetTimestampMode() for a detailed explanation.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetTimestampMode(WORD *mode);

///
/// \brief Gets the exposure runtime for one image of the camera.
///
/// Get and split the 'camera operation code' runtime into two DWORD. One will hold the longer
/// part, in seconds, and the other will hold the shorter part, in nanoseconds. This function can be
/// used to calculate the FPS. The sum of dwTime_s and dwTime_ns covers the delay, exposure and
/// readout time. If external exposure is active, it returns only the readout time.
///
/// The input pointer will be filled with the following parameter:
/// - current coc runtime in seconds and nanoseconds (0s .. 136years(maybe enough), 0ns .. 999.999.999ns - all above it will show up in the seconds part)
///
/// \param s Pointer to a DWORD variable to receive the time part in seconds of the COC.
/// \param ns Pointer to a DWORD variable to receive the time part in nanoseconds of the COC (range: 0ns-999.999.999ns).
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetCOCRuntime(DWORD *s,DWORD *ns);

///
/// \brief Request the current camera and power supply temperatures.
/// 
/// Power supply temperature is not available with all cameras. If it is not available, the temperature will show 0. In case the sensor
/// temperature is not available it will show 0x8000.
///
/// The input pointers will be filled with the following parameters:
/// - CCD temperature as signed word in \f$^\circ\f$C*10.
/// - Camera temperature as signed word in \f$^\circ\f$C.
/// - Power Supply temperature as signed word in \f$^\circ\f$C.
///
/// \param sCCDTemp Pointer to a SHORT variable, to receive the CCD temp. value.
/// \param sCAMTemp Pointer to a SHORT variable, to receive the camera temp. value.
/// \param sExtTemp Pointer to a SHORT variable, to receive the power device temp. value.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetTemperature(SHORT *sCCDTemp,SHORT *sCAMTemp,SHORT *sExtTemp);

///
/// \brief Requests a single image from the camera.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_RequestImage();

///
/// \brief Gets the actual bit alignment of the raw image data.
///
/// Since the image data is less than a WORD, which is 16 bit, the data can be placed in two reasonable ways.
/// Either you set the LSB of the image data to the LSB of the transferred data or
/// you set the MSB of the image data to the MSB of the transferred data.
///
/// \param align Pointer to a WORD variable to receive the bit alignment.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetBitAlignment(WORD *align);

///
/// \brief Sets the actual bit alignment of the raw image data. See PCO_GetBitAlignment() for details.
/// 
/// Set the following parameter:
/// - wBitAlignment:
///   - 0x0000 = [MSB aligned]; all raw image data will be aligned to the MSB. This is the default setting.
///   - 0x0001 = [LSB aligned]; all raw image data will be aligned to the LSB.
///
/// \param align WORD variable which holds the bit alignment.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetBitAlignment(WORD align);

///
/// \brief  Sets the exposure and delay time of the camera without changing the timebase.
///
/// If the recording state is on, it is possible to change the timing without calling PCO_ArmCamera.
///
/// See PCO_SetDelayExposureTime() for a detailed description.
/// \param delay DWORD variable to hold the delay time.
/// \param expos DWORD variable to hold the exposure time.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetDelayExposure(DWORD delay,DWORD expos);

///
/// \brief Gets the exposure and delay time table of the camera.
/// See PCO_SetDelayExposureTime() for a detailed description.
/// 
/// \param delay Pointer to a DWORD array to receive the delay time.
/// \param expos Pointer to a DWORD array to receive the exposure time.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetDelayExposure(DWORD *delay,DWORD *expos);


///
/// \brief Sets the exposure and delay time bases of the camera.
/// See PCO_SetDelayExposureTime() for a detailed description.
/// \param delay WORD variable to hold the delay time base.
/// \param expos WORD variable to hold the exposure time base.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetTimebase(WORD delay,WORD expos);

///
/// \brief Gets the exposure and delay time bases of the camera.
/// See PCO_SetDelayExposureTime() for a detailed description
/// \param delay Pointer to WORD variable to receive the delay timebase.
/// \param expos Pointer to WORD variable to receive the exposure timebase.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetTimebase(WORD *delay,WORD *expos);

///
/// \brief Gets the name of the camera. Not applicable to all cameras.
/// 
/// The string buf has to be long enough to get the camera name. Maximum length will be 40 characters including a terminating zero.
///
/// The input pointers will be filled with the following parameters:
/// - Camera name as it is stored inside the camera (e.g. "pco.4000").
///
/// \param buf Pointer to a string to receive the camera name.
/// \param length WORD variable which holds the maximum length of the string.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetCameraName(void* buf,int length);

///
/// \brief Gets the basic information of the camera.
/// 
/// The string buf has to be long enough to get the information.
///
/// The input pointers will be filled with one of the following parameters:
/// - Camera name plus interface name as it is stored inside the camera (e.g."pco.edge 5.5 USB").
/// - Camera name as it is stored inside the camera (e.g. "pco.4000").
/// - Sensor name as it is stored inside the camera
///
/// \param typ - 0: Camera and interface name
///            - 1: Camera name only
///            - 2: Sensor name
/// \param buf Pointer to a string, to receive the info string.
/// \param length int variable which holds the length of buf.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetInfo(DWORD typ,void* buf,int length);

///
/// \brief Gets the actual armed image size of the camera. This accounts for binning and ROI.
/// 
/// If the user recently changed size influencing values without issuing an ARM, the GetSizes function will return the sizes from the last
/// recording. If no recording occurred, it will return the last ROI settings.
///
/// \param width Pointer to a DWORD to receive the actual width.
/// \param height Pointer to a DWORD to receive the actual height.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetActualSize(DWORD *width,DWORD *height);

///
/// \brief Gets the camera description data structure.
/// 
/// This is a cached value that is retrieved once when Open_Cam is called.
/// Request camera description (sensor type, horizontal / vertical / dynamic resolution/ binning/ delay/ exposure ...).
/// The response message describes the sensor type, the readout hardware and its
/// possible operating range. This set of information can be used to verify the settings before the user calls the PCO_Setxxx - commands.
///
/// \param description Pointer to a PCO description structure.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetCameraDescriptor(SC2_Camera_Description_Response *description);

///
/// \brief Sets the adc operation mode of the camera, if available.
/// 
///
/// Set analog-digital-converter (ADC) operation for reading the image sensor data. Pixel data can be
/// read out using one ADC (better linearity) or in parallel using two ADCs (faster). This option is
/// only available for some camera models. If the user sets 2ADCs he must center and adapt the ROI
/// to symmetrical values, e.g. pco.1600: x1,y1,x2,y2=701,1,900,500 (100,1,200,500 is not possible).
///
/// The input data has to be filled with the following parameter:
/// - operation to be set:
///   - 0x0001 = 1 ADC or
///   - 0x0002 = 2 ADCs should be used...
/// - the existence of the number of ADCs can be checked with the values defined in the camera description
///
/// \param num
/// WORD variable to hold the adc operation mode.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetADCOperation(WORD num);


///
/// \brief Gets the pixelrate for reading images from the image sensor.
/// 
/// For the pco.edge the higher pixelrate needs also execution of PCO_SetTransferParameter() and PCO_SetLut() with appropriate parameters.
///
/// \param PixelRate Pointer to a DWORD variable to receive the pixelrate.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetPixelRate(DWORD *PixelRate);

///
/// \brief Sets the pixel rate of the camera.
/// 
/// For the pco.edge the higher pixelrate needs also execution of PCO_SetTransferParameter() and PCO_SetLut() with appropriate parameters.
///
/// \param PixelRate DWORD variable to hold the pixelrate.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetPixelRate(DWORD PixelRate);

///
/// \brief Sets the active lookup table in the camera, if available.
/// 
/// Only available with a pco.edge
/// \param Identifier define LUT to be activated, 0x0000 for no LUT, see PCO_GetLookupTableInfo() for available LUTs
/// \param Parameter Offset: 11 Bit value for fixed offset subtraction before transferring the data via the lookup table
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetLut(WORD Identifier,WORD Parameter);

///
/// \brief Gets the active lookup table in the camera, if available.
/// 
/// Only available with a pco.edge
/// \param Identifier Currently active LUT, 0x0000 for no LUT
/// \param Parameter Offset: 11 Bit value for fixed offset subtraction before transferring the data via the lookup table
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetLut(WORD *Identifier,WORD *Parameter);

///
/// \brief Resets all camera to default values.
/// 
/// This function is executed during a power-up sequence.
///
/// The following are the default settings:
/// | Setting: | Default: |
/// |----------|----------|
/// | Sensor Format | standard |
/// | ROI | full resolution |
/// | Binning | no binning |
/// | Pixel Rate | (depending on image sensor) |
/// | Gain | Normal gain (if setting available due to sensor) |
/// | Double Image Mode | Off |
/// | IR sensitivity | Off (if setting available due to sensor) |
/// | Cooling Set point | -12 \f$^\circ\f$C (if setting available |
/// | ADC mode | Using one ADC (if setting avaialble |
/// | Exposure Time | 20 ms |
/// | Delay Time |0 \f$\mu\f$ |
/// | Trigger Mode | Auto Trigger
/// | Recording state | stopped |
/// | Memory Segmentation | Total memory allocated to first segment |
/// | Storage Mode | Recorder Ring Buffer + Live View on |
/// | Acquire Mode | Auto
///
/// \b Note: If the camera is running when this command is sent, it will be stopped!
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_ResetSettingsToDefault();

///
/// \brief Get image trigger mode (for further explanation see camera manual).
/// 
/// Trigger modes:
/// - 0x0000 = [auto trigger]
///
///   An exposure of a new image is started automatically best possible compared to the
///   readout of an image. If a CCD is used, and images are taken in a sequence, then exposures
///   and sensor readout are started simultaneously. Signals at the trigger input (\<exp trig\>) are irrelevant.
/// - 0x0001 = [software trigger]:
///
///   An exposure can only be started by a force trigger command.
/// - 0x0002 = [extern exposure & software trigger]:
///
///   A delay / exposure sequence is started at the RISING or FALLING edge (depending on
///   the DIP switch setting) of the trigger input (\<exp trig\>).
/// - 0x0003 = [extern exposure control]:
///   The exposure time is defined by the pulse length at the trigger input(\<exp trig\>). The
///   delay and exposure time values defined by the set/request delay and exposure command
///   are ineffective. (Exposure time length control is also possible for double image mode; the
///   exposure time of the second image is given by the readout time of the first image.)
///
/// \b Note: In the [extern exposure & software trigger] and [extern exposure control] modes, it also
/// depends on the selected acquire mode, if a trigger edge at the trigger input (\<exp trig\>)
/// will be effective or not (see also PCO_GetAcquireMode() (Auto / External)). A software
/// trigger however will always be effective independent of the state of the \<acq enbl\> input
/// (concerned trigger modes are: [software trigger] and [extern exposure & software trigger].
///
/// \param mode Pointer to a WORD variable to receive the triggermode.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetTriggerMode(WORD *mode);

///
/// \brief Sets the trigger mode of the camera.
/// 
/// See PCO_GetTriggerMode for a detailed explanation.
///
/// The command will be rejected, if Recording State is [run].
/// \param mode WORD variable to hold the triggermode.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetTriggerMode(WORD mode);

///
/// \brief Forces a software trigger to the camera.
/// 
/// This software command starts an exposure if the trigger mode is in the [software trigger]
/// (0x0001) state or in the [extern exposure & software trigger] (0x0002) state. If the trigger mode is
/// in the [extern exposure control] (0x0003) state, nothing happens. A ForceTrigger should not be used to generate a distinct timing.
/// To accept a force trigger command the camera must be recording and ready: (recording = [start])
/// and [not busy]. If a trigger fails it will not trigger future exposures.
///
/// Result:
/// - 0x0000 = trigger command was unsuccessful because the camera is busy
/// - 0x0001 = a new image exposure has been triggered by the command
///
/// \param trigger
/// Pointer to a WORD variable to receive whether a trigger occurred or not.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_ForceTrigger(WORD *trigger);

///
/// \brief Sets the camera time to current system time for the timestamp function.
/// 
/// The date and time is updated automatically, as long as the camera is supplied with power.
/// When powering up the camera, then this command should be done once.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetCameraToCurrentTime();

///
/// \brief Gets the signal state of the camera sensor. Edge only!
/// 
/// The signals must not be deemed to be a real time response of the sensor, since the command path adds a system dependent delay. Sending a command
/// and getting the camera response lasts about 2ms (+/- 1ms; for 'simple' commands). In case you need a closer synchronization use hardware signals.
/// \param status DWORD pointer to receive the status flags of the sensor (can be NULL).
///  - Bit0: SIGNAL_STATE_BUSY  0x0001
///  - Bit1: SIGNAL_STATE_IDLE  0x0002
///  - Bit2: SIGNAL_STATE_EXP   0x0004
///  - Bit3: SIGNAL_STATE_READ  0x0008
/// \param imagecount DWORD pointer to receive the # of the last finished image(can be NULL).
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetSensorSignalStatus(DWORD *status,DWORD *imagecount);

///
/// \brief Gets the region of interest of the camera.
/// 
/// Get ROI (region or area of interest) window. The ROI is equal to or smaller than the absolute
/// image area, which is defined by the settings of format and binning.
///
/// Some sensors have a ROI stepping. See the camera description and check the parameters
/// wRoiHorStepsDESC and/or wRoiVertStepsDESC. In case stepping is zero ROI setting other than x0=1, x1=max/bin, y0=1, y1=max/bin it not possible.
///
/// For dual ADC mode the horizontal ROI must be symmetrical. For a pco.dimax the horizontal and
/// vertical ROI must be symmetrical. For a pco.edge the vertical ROI must be symmetrical.
///
/// X0, Y0 start at 1. X1, Y1 end with max. sensor size.
///
/// \latexonly \begin{tabular}{| l c r | } \hline x0,y0 & & \\ & ROI & \\ & & x1,y1 \\ \hline \end{tabular} \endlatexonly
///
/// \param RoiX0
/// Pointer to a WORD variable to receive the x value for the upper left corner.
/// \param RoiY0
/// Pointer to a WORD variable to receive the y value for the upper left corner.
/// \param RoiX1
/// Pointer to a WORD variable to receive the x value for the lower right corner.
/// \param RoiY1
/// Pointer to a WORD variable to receive the y value for the lower right corner.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetROI(WORD *RoiX0,WORD *RoiY0,WORD *RoiX1,WORD *RoiY1);

///
/// \brief Sets the region of interest of the camera.
///
/// Set ROI (region or area of interest) window. The ROI must be equal to or smaller than the
/// absolute image area, which is defined by the settings of format and binning. If the binning
/// settings are changed, the user must adapt the ROI, before PCO_ArmCamera() is accessed. The
/// binning setting sets the limits for the ROI. For example, a sensor with 1600x1200 and binning 2x2
/// will result in a maximum ROI of 800x600.
///
/// Some sensors have a ROI stepping. See the camera description and check the parameters
/// wRoiHorStepsDESC and/or wRoiVertStepsDESC. In case stepping is zero ROI setting other than
/// x0=1, x1=max/bin, y0=1, y1=max/bin it not possible (max depends on the selected sensor format;
/// bin depends on the current binning settings).
///
/// For dual ADC mode the horizontal ROI must be symmetrical. For a pco.dimax the horizontal and
/// vertical ROI must be symmetrical. For a pco.edge the vertical ROI must be symmetrical.
///
/// X0, Y0 start at 1. X1, Y1 end with max. sensor size.
///
/// \latexonly \begin{tabular}{| l c r | } \hline x0,y0 & & \\ & ROI & \\ & & x1,y1 \\ \hline \end{tabular} \endlatexonly
///
/// \param RoiX0
/// WORD variable to hold the x value for the upper left corner.
/// \param RoiY0
/// WORD variable to hold the y value for the upper left corner.
/// \param RoiX1
/// WORD variable to hold the x value for the lower right corner.
/// \param RoiY1
/// WORD variable to hold the y value for the lower right corner.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetROI(WORD RoiX0,WORD RoiY0,WORD RoiX1,WORD RoiY1);

///
/// \brief Gets the binning values of the camera.
/// 
/// \param BinHorz Pointer to a WORD variable to hold the horizontal binning value.
/// \param BinVert Pointer to a WORD variable to hold the vertikal binning value.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetBinning(WORD *BinHorz,WORD *BinVert);

///
/// \brief Sets the binning values of the camera.
/// 
/// Set binning. If the binning settings are changed, the user must adapt the ROI, before
/// PCO_ArmCamera() is accessed. The binning setting sets the limits for the ROI. E.g. a sensor with
/// 1600x1200 and binning 2x2 will result in a maximum ROI of 800x600.
/// \param BinHorz WORD variable to hold the horizontal binning value.
/// \param BinVert WORD variable to hold the vertikal binning value.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetBinning(WORD BinHorz,WORD BinVert);

///
/// \brief Sets the time and date to the camera.
/// 
/// Note:
/// - [ms] and [\f$\mu\f$s] values are set to zero, when this command is executed
/// - this command should be performed, when powering up the camera
///
/// \param st Pointer to a tm structure.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetDateTime(struct tm *st);

///
/// \brief Gets the exposure and delay time and the time bases of the camera.
/// 
/// Timebase:
/// - 0 -> value is in ns: exp. time of 100 means 0.0000001s.
/// - 1 -> value is in \f$\mu\f$s: exp. time of 100 means 0.0001s.
/// - 2 -> value is in ms: exp. time of 100 means 0.1s.
///
/// Note:
/// - delay and exposure values are multiplied with the configured timebase unit values
/// - the range of possible values can be checked with the values defined in the camera description.
///
/// \param delay Pointer to a DWORD variable to receive the delay time.
/// \param expos Pointer to a DWORD variable to receive the exposure time.
/// \param tb_delay Pointer to a WORD variable to receive the del. timebase.
/// \param tb_expos Pointer to a WORD variable to receive the exp. timebase.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetDelayExposureTime(DWORD *delay,DWORD *expos,WORD *tb_delay,WORD *tb_expos);

///
/// \brief Sets the exposure and delay time and the time bases of the camera.
/// 
/// If the recording state is on, it is possible to change the timing without calling PCO_ArmCamera().
///
/// Timebase:
/// - 0 -> value is in ns: exp. time of 100 means 0.0000001s.
/// - 1 -> value is in \f$\mu\f$s: exp. time of 100 means 0.0001s.
/// - 2 -> value is in ms: exp. time of 100 means 0.1s.
///
/// Note: - delay and exposure values are multiplied with the configured timebase unit values
///       - the range of possible values can be checked with the values defined in the camera description.
///       - can be used to alter the timing in case the recording state is on. In this case it is not necessary to call PCO_ArmCamera(). If the recording state is off calling PCO_ArmCamera() is mandatory.
///
/// \param delay DWORD variable to hold the delay time.
/// \param expos DWORD variable to hold the exposure time.
/// \param tb_delay WORD variable to hold the del. timebase.
/// \param tb_expos WORD variable to hold the exp. timebase.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetDelayExposureTime(DWORD delay,DWORD expos,WORD tb_delay,WORD tb_expos);

///
/// \brief Sets the sensor format.
/// 
/// The [standard] format uses only effective pixels, while the [extended] format shows all pixels inclusive effective, dark, reference and dummy.
///
/// - 0x0000 = [standard]
/// - 0x0001 = [extended]
///
/// \param wSensor WORD variable which holds the sensor format.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetSensorFormat(WORD wSensor);

///
/// \brief Gets the sensor format.
/// 
/// The [standard] format uses only effective pixels, while the [extended] format shows all pixels inclusive effective, dark, reference and dummy.
///
/// - 0x0000 = [standard]
/// - 0x0001 = [extended]
///
/// \param wSensor Pointer to a WORD variable to receive the sensor format.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetSensorFormat(WORD *wSensor);

///
/// \brief Gets the camera setup structure (see camera specific structures)
/// 
/// Not applicable to all cameras. See sc2_defs.h for valid flags: -- Defines for Get / Set Camera Setup.
///
/// This command can be used to get the shutter mode of the pco.edge.
/// \param Type Pointer to a word to get the actual setup type.
/// \param Setup Pointer to a dword array to get the actual setup.
/// \param Len WORD Pointer to get the length of the array.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetCameraSetup(WORD *Type,DWORD *Setup,WORD *Len);

///
/// \brief Sets the camera setup structure (see camera specific structures)
/// 
/// Not applicable to all cameras. See sc2_defs.h for valid flags: -- Defines for Get / Set Camera Setup
///
/// This command can be used to set the shutter mode of the pco.edge. Camera must be reinitialized to activate new setup: Reboot(optional)-Close-Open
/// \param Type WORD to set the actual setup type. Do not change this value.
/// \param Setup Pointer to DWORD array to set the actual setup.
/// \param Len WORD to indicate the number of valid DWORDs in Setup.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetCameraSetup(WORD Type, DWORD *Setup,WORD Len);

///
/// \brief Gets the number of available HW signals. Not applicable to all cameras.
/// 
/// Get the number of hardware IO signals, which are available with the camera. To set and get the
/// single signals use PCO_GetHWIOSignal() (dimax, edge only) and PCO_SetHWIOSignal() (dimax,
/// edge only). This functions is not available with all cameras. Actually it is implemented in the pco.dimax.
/// \param numSignals WORD variable to get the number of signals
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetHWIOSignalCount(WORD *numSignals);

///
/// \brief Gets the signal descriptor of the requested hardware IO signal. Not applicable to all cameras.
/// 
/// To get the number of available hardware IO signals, please call PCO_GetHWIOSignalCount() (dimax edge only). To set and get the single
/// signals use PCO_GetHWIOSignal() (dimax, edge only) and PCO_SetHWIOSignal() (dimax, edge only).
/// This functions is not available with all cameras. Actually it is implemented in the pco.dimax.
///
/// The output structure has the following parameters:
/// - wSignalDefinitions: Flags showing signal options:
///   - 0x01: Signal can be enabled/disabled
///   - 0x02: Signal is a status output
///   - 0x10: Signal function 1 has got parameter value
///   - 0x20: Signal function 2 has got parameter value
///   - 0x40: Signal function 3 has got parameter value
///   - 0x80: Signal function 4 has got parameter value
/// - wSignalTypes: Flags showing which signal type is available:
///   - 0x01: TTL
///   - 0x02: High Level TTL
///   - 0x04: Contact Mode
///   - 0x08: RS485 differential
/// - wSignalPolarity: Flags showing which signal polarity can be selected:
///   - 0x01: Low level active
///   - 0x02: High Level active
///   - 0x04: Rising edge active
///   - 0x08: Falling edge active
/// - wSignalFilter: Flags showing the filter option:
///   - 0x01: Filter can be switched off (t > ~65ns)
///   - 0x02: Filter can be switched to medium (t > ~1\f$\mu\f$s)
///   - 0x04: Filter can be switched to high (t > ~100ms)
///
/// \param SignalNum WORD variable to query the signal
/// \param SignalDesc Pointer to a SIGNAL_DESC structure to get the signal description
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetHWIOSignalDescriptor(WORD SignalNum,SC2_Get_HW_IO_Signal_Descriptor_Response *SignalDesc);

///
/// \brief Gets the signal descriptor of the requested signal number as a string for console output.
/// \param SignalNum WORD variable to query the signal
/// \param outbuf Pointer to string to hold the signal description.
/// \param size Pointer to size of input string
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetHWIOSignalDescriptor(WORD SignalNum,char *outbuf,int *size);

///
/// \brief Gets the signal options of the requested signal number.
/// 
/// Gets the settings of the requested hardware IO signal. This functions is not available with all cameras. Actually it is implemented in the pco.dimax.
/// \param SignalNum Index of the signal
/// \param Enabled Flags showing enable state of the signal
///                - 0x00: Signal is off
///                - 0x01: Signal is active
/// \param Type Flags showing which signal type is selected
///             - 0x01: TTL
///             - 0x02: High Level TTL
///             - 0x04: Contact Mode
///             - 0x08: RS485 differential
/// \param Polarity Flags showing which signal polarity is selected
///                 - 0x01: High level active
///                 - 0x02: Low level active
///                 - 0x04: Rising edge active
///                 - 0x08: Falling edge active
/// \param FilterSetting Flags showing the filter option which is selected
///                      - 0x01: Filter can be switched off (t > ~65ns)
///                      - 0x02: Filter can be switched to medium (t > ~1\f$\mu\f$)
///                      - 0x04: Filter can be switched to high (t > ~100ms)
/// \param Selected In case the HWIOSignaldescription shows more than one SignalNames, this parameter can be used to select a different signal, e.g. 'Status Busy' or 'Status Exposure'.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetHWIOSignal(WORD SignalNum,WORD *Enabled,WORD *Type,WORD *Polarity,WORD *FilterSetting,WORD *Selected);

///
/// \brief Sets the signal options of the requested signal number.
/// 
/// Sets the settings of the requested hardware IO signal. This functions is not available with all cameras. Actually it is implemented in the pco.dimax.
/// \param SignalNum Index of the signal
/// \param Enabled Flags showing enable state of the signal
///                - 0x00: Signal is off
///                - 0x01: Signal is active
/// \param Type Flags showing which signal type is selected
///             - 0x01: TTL
///             - 0x02: High Level TTL
///             - 0x04: Contact Mode
///             - 0x08: RS485 differential
/// \param Polarity Flags showing which signal polarity is selected
///                 - 0x01: High level active
///                 - 0x02: Low level active
///                 - 0x04: Rising edge active
///                 - 0x08: Falling edge active
/// \param FilterSetting Flags showing the filter option which is selected
///                      - 0x01: Filter can be switched off (t > ~65ns)
///                      - 0x02: Filter can be switched to medium (t > ~1\f$\mu\f$s)
///                      - 0x04: Filter can be switched to high (t > ~100ms)
/// \param Selected In case the HWIOSignaldescription shows more than one SignalNames, this parameter can be used to select a different signal, e.g. 'Status Busy' or 'Status Exposure'.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_SetHWIOSignal(WORD SignalNum,WORD Enabled,WORD Type,WORD Polarity,WORD FilterSetting,WORD Selected);


DWORD PCO_GetHWIOSignalTiming(WORD SignalNum,WORD Selection,DWORD *type,DWORD *Parameter);
DWORD PCO_SetHWIOSignalTiming(WORD SignalNum,WORD Selection,DWORD Parameter);

///
/// \brief Gets the firmware versions as string for console output.
/// 
/// If the string size is not large enough, not all firmware strings will be shown.
/// \param outbuf Pointer to string to hold the firmware version output.
/// \param size Pointer int that holds the size of the string.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetFirmwareVersion(char *outbuf,int *size);

///
/// \brief Gets the hardware versions as string for console output.
/// 
/// If the string size is not large enough, not all hardware strings will be shown.
/// \param outbuf Pointer to string to hold the firmware version output.
/// \param size Pointer to size of the string.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetHardwareVersion(char *outbuf,int *size);

///
/// \brief Request the current camera health status: warnings, errors.
/// 
/// It is recommended to call this function frequently (e.g. every 5s, or after calling ARM) in order to recognize camera internal problems,
/// like electronics temperature error (Taking a shot in the desert at high noon.). This will enable users to prevent the camera hardware from damage.
/// - Warnings are encoded as bits of a long word. Bit set indicates warning, bit cleared indicates that the corresponding parameter is OK.
/// - System errors encoded as bits of a long word. Bit set indicates error, bit cleared indicates that the corresponding status is OK.
/// - System Status encoded as bits of a long word.
///
/// \param dwWarnings Pointer to a DWORD variable, to receive the warning value.
/// \param dwErrors Pointer to a DWORD variable, to receive the error value.
/// \param dwStatus Pointer to a DWORD variable, to receive the error value.
/// \return Error message, 0 in case of success else less than 0
///
DWORD PCO_GetHealthStatus(unsigned int *dwWarnings,unsigned int *dwErrors,unsigned int *dwStatus);

//  void  PCO_SetVerbose(int iNewState);
//  DWORD PCO_GetImageTiming(void);

#endif

