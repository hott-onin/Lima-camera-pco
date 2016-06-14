//-----------------------------------------------------------------//
// Name        | Cpco_com_func_2.h           | Type: ( ) source    //
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
//  see Cpco_com_func_2.cpp                                        //
//-----------------------------------------------------------------//

#ifndef CPCO_COM_FUNC_2_H
#define CPCO_COM_FUNC_2_H

///
/// \file Cpco_com_func_2.h
///
/// \brief Advanced camera communication
///
/// \author PCO AG
///


///
/// \brief Gets the camera type, serial number and inteface type.
///
/// \param wCamType Pointer to WORD variable to receive the camera type.
/// \param dwSerialNumber Pointer to DWORD variable to receive the serial number.
/// \param wIfType Pointer to WORD variable to receive connected Interface type
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetCameraType(WORD* wCamType,DWORD* dwSerialNumber,WORD* wIfType=NULL);
///
/// \brief Starts a self test procedure.
///
/// See PCO_GetCameraHealthStatus().
/// \param dwWarn Pointer to DWORD variable to receive warnings.
/// \param dwErr Pointer to DWORD variable to receive errors.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_InitiateSelftestProcedure(DWORD* dwWarn, DWORD* dwErr);

DWORD PCO_GetFanControlStatus(WORD* wFanMode,WORD* wFanMin,WORD* wFanMax,WORD* wStepSize,WORD* wSetValue,WORD* wActualValue);
DWORD PCO_SetFanControlStatus(WORD wFanMode,WORD wSetValue);

///
/// \brief Gets the firmware versions of all devices in the camera.
///
/// Not applicable to all cameras.
/// \param response Pointer to a the firmware version structure
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetFirmwareVersion(SC2_Firmware_Versions_Response* response);

DWORD PCO_GetFirmwareVersionExt(BYTE* bNum);

DWORD PCO_WriteMailbox(WORD wMailboxNo,BYTE* bData,WORD len);

DWORD PCO_ReadMailbox(WORD wMailboxNo,BYTE* bData,WORD* len);

DWORD PCO_GetMailboxStatus(WORD* wNumberOfMailboxes,WORD* wMailboxStatus,WORD *len);

///
/// \brief Gets the camera power save mode.
///
/// Not applicable to all cameras.
/// \param wMode WORD pointer to get the actual power save mode. (0-off,default; 1-on)
/// \param wDelayMinutes WORD pointer to get the delay until the camera enters power save mode after main power loss. The actual switching delay is between wDelayMinutes and wDelayMinutes + 1. Possible range is 1 - 60.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetPowersaveMode(WORD* wMode,WORD* wDelayMinutes);

///
/// \brief Sets the camera power save mode.
///
/// Not applicable to all cameras.
/// \param wMode WORD to set the actual power save mode. (0-off,default; 1-on)
/// \param wDelayMinutes WORD to set the delay until the camera enters power save mode after main power loss. The actual switching delay is between wDelayMinutes and wDelayMinutes + 1. Possible range is 1 - 60.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetPowersaveMode(WORD wMode,WORD wDelayMinutes);

///
/// \brief Gets the camera battery status.
///
/// Not applicable to all cameras.
/// \param wBatteryType WORD pointer to get the battery type.
/// - 0x0000 = no battery mounted
/// - 0x0001 = nickel metal hydride type
/// - 0x0002 = lithium ion type
/// - 0x0003 = lithium iron phosphate type
/// - 0xFFFF = unknown battery type
/// \param wBatteryLevel WORD pointer to get the battery level in percent.
/// \param wPowerStatus WORD pointer to get the power status.
/// - 0x0001 = power supply is available
/// - 0x0002 = battery mounted and detected
/// - 0x0004 = battery is charged
///
/// Bits can be combined e.g. 0x0003 means that camera has a battery and is running on external power, 0x0002: camera runs on battery.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetBatteryStatus(WORD* wBatteryType,WORD* wBatteryLevel,WORD* wPowerStatus);

///
/// \brief Gets the value of an external register.
///
/// \param wID WORD pointer to hold the number of the register
/// \param bData Pointer to a BYTE variable to recieve the value of the register.
/// \param len Pointer to a WORD variable to recieve the length of the register.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetExternalRegister(WORD* wID,BYTE* bData,WORD *len);

///
/// \brief Sets the value of an external register.
///
/// Values: 0x0000 - 0x0003
///
/// \param wID WORD pointer to hold the number of the register.
/// \param bData Pointer to a BYTE variable to hold the value of the register.
/// \param wDataSize Pointer to a WORD variable to hold the length of the value to be written.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetExternalRegister(WORD* wID,BYTE* bData,WORD* wDataSize);

///
/// \brief Gets the camera description data structure
///
/// The input structure will be filled with the following parameters:
///
/// \f$ $
/// \begin{TabularC}{9} \hline
/// \rowcolor{lightgray} & & \textbf{Sensor Type} & \textbf{Sensor Subtype} & \textbf{Hor. Res. std} & \textbf{Vert. Res. std} & \textbf{Hor. Res. ext} & \textbf{Vert. Res. ext} & \textbf{Dyn. Res.} \\\cline{1-9}
/// & & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# \\\cline{1-9}
/// \rowcolor{lightgray} \textbf{Max Binn hor} & \textbf{Binn hor steps} & \textbf{Max Binn vert} & \textbf{Binn vert steps} & \textbf{ROI hor steps} & \textbf{ROI vert steps} & \textbf{ADCs} & \textbf{Pixelrate 1} & \textbf{Pixelrate 2} \\\cline{1-9}
/// 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# \\\cline{1-9}
/// \rowcolor{lightgray} \textbf{Pixelrate 3} & \textbf{Pixelrate 4} & \textbf{Convers. Factor 1} & \textbf{Convers. Factor 2} & \textbf{Convers. Factor 3} & \textbf{Convers. Factor 4} & \textbf{IR sens} & \textbf{Min Del. Time (nsec)} & \textbf{Max Del. Time (msec)} \\\cline{1-9}
/// 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\#  & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# \\\cline{1-9}
/// \end{TabularC}
/// \begin{TabularC}{9} \hline
/// \rowcolor{lightgray} \textbf{Min Del Step} (nsec) & \textbf{Min Exp Time (nsec)} & \textbf{Max Exp Time (Msec)} & \textbf{Min Exp. Step (nsec)} & \textbf{Min Del Time IR (nsec)} & \textbf{Max Del Time IR (msec)} & \textbf{Min Exp Time IR (nsec)} & \textbf{Max Exp Time IR (msec)} & \textbf{Time Table} \\\cline{1-9}
/// 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \\\cline{1-9}
/// \rowcolor{lightgray} \textbf{Double Image Mode} & \textbf{Min Cooling Setpoint} & \textbf{Max Cooling Setpoint} & \textbf{Default Cooling Setpoint} & \textbf{Power Down Mode} & \textbf{Offset Regulation} & \textbf{Color Pattern} & \textbf{Color Pattern Type} & \textbf{Reserved 1} \\\cline{1-9}
/// 0x\#\#\#\# 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# & 0x\#\#\#\# \\\cline{1-9}
/// \rowcolor{lightgray} \textbf{General Caps 1} & \textbf{Reserved 2} & \textbf{Reserved 3} & \textbf{Reserved 4} & \textbf{Reserved 5} & \textbf{Reserved 6} & \textbf{Reserved 7} & \textbf{Reserved 8} & \textbf{Reserved 9} \\\cline{1-9}
/// 0x\#\#\#\# \#\#\#\# & 0x\#\#\#\# \#\#\#\#& 0x\#\#\#\# \#\#\#\#& 0x\#\#\#\# \#\#\#\#& 0x\#\#\#\# \#\#\#\#& 0x\#\#\#\# \#\#\#\#& 0x\#\#\#\# \#\#\#\#& 0x\#\#\#\# \#\#\#\#& 0x\#\#\#\# \#\#\#\#\\\cline{1-9}
/// \end{TabularC}
/// $ \f$
///
/// - image sensor type as word, see table "Sensor Type codes" below.
/// - image sensor sub type as word.
/// - horizontal resolution standard in pixels (all effective pixels).
/// - vertical resolution standard in pixels (all effective pixels).
/// - horizontal resolution extended in pixels (all pixels; dummy + dark + eff.).
/// - vertical resolution extended in pixels (all pixels; dummy + dark + eff.).
/// - dynamic resolution in bits/pixel. (i.e. 12, 14 ...)
/// - max. binning value horizontal (allowed values from 1 to max. resolution)
/// - binning steps horizontal
///   1 = linear step (binning from 1 to max i.e. 1,2,3...max is possible)
///   0 = binary step (binning from 1 to max i.e. 1,2,4,8,16...max is possible)
/// - max. binning value vertical (allowed values from 1 to max. resolution)
/// - binning steps vertical
///   1 = linear step (binning from 1 to max i.e. 1,2,3...max is possible)
///   0 = binary step (binning from 1 to max i.e. 1,2,4,8,16...max is possible)
/// - ROI steps horizontal (e.g. 10, => ROI right = 1, 11, 21, 31 ...)
/// - ROI steps vertical (in case steps = 0 ROI < max is not possible)
/// - ADCs (number of ADCs inside camera; i.e. 1..8)
/// - pixelrate 1 (long word; frequency in Hz)
/// - pixelrate 2 (long word; frequency in Hz; if not available, then value = 0)
/// - pixelrate 3 (long word; frequency in Hz; if not available, then value = 0)
/// - pixelrate 4 (long word; frequency in Hz; if not available, then value = 0)
/// - conversion factor 1 (in electron / counts)
///   (the value 100 corresponds to 1; i.e. 610 = 6.1 electron/counts)
/// - conversion factor 2 (in electron / counts; if not available, then value = 0)
///   (the value 100 corresponds to 1; i.e. 610 = 6.1 electron/counts)
/// - conversion factor 3 (in electron / counts; if not available, then value = 0)
///   (the value 100 corresponds to 1; i.e. 610 = 6.1 electron/counts)
/// - conversion factor 4 (in electron / counts; if not available, then value = 0)
///   (the value 100 corresponds to 1; i.e. 610 = 6.1 electron/counts)
/// - soft ROI steps horizontal (API internal value; no camera value)
/// - soft ROI steps vertical (API internal value; no camera value)
/// - IR-sensitivity; sensor can switch to improved IR sensitivity
///   (0 = function not supplied; 1 = possible)
/// - min. delay time in nsec (long word; non IR-sensitivity mode)
/// - max. delay time in msec (long word; non IR-sensitivity mode)
/// - min. delay time step in nsec (long word)
///   Note: Applies both to non IR-sensitivity mode and IR-sensitivity mode
/// - min. exposure time in nsec (long word; non IR-sensitivity mode)
/// - max. exposure time in msec (long word; non IR-sensitivity mode)
/// - min. exposure time step in nsec (long word)
///   Note: Applies both to non IR-sensitivity mode and IR-sensitivity mode
/// - min. delay time in nsec (long word; IR-sensitivity mode)
/// - max. delay time in msec (long word; IR-sensitivity mode)
/// - min. exposure time in nsec (long word; IR-sensitivity mode)
/// - max. exposure time in msec (long word; IR-sensitivity mode)
/// - time table; camera can perform a timetable with several delay/ exposures
///   (0 = function not supplied; 1 = possible)
/// - double image mode; camera can perform a double image with a short interleave time between exposures (0 = function not supplied; 1 = possible)
/// - min. cooling set point (in \f$^\circ\f$C) (if all set points are 0, then cooling is not available)
/// - max. cooling set point (in \f$^\circ\f$C) (if all set points are 0, then cooling is not available)
/// - default cooling set point (in \f$^\circ\f$C) (if all set points are 0, then cooling is not available)
/// - power down mode; switch sensor into power down mode for reduced dark current (0 = function not supplied; 1 = possible)
/// - offset regulation; automatic offset regulation with reference Pixels (0 = function not supplied; 1 = possible)
/// - color pattern; four nibbles are describing the colors of a color chip. (see below)
/// - color pattern type; 0: RGB Bayer Pattern
/// - general caps 1; describes special features of the camera, whether they are available
/// - reserved 1 - 9 (for future use)
///
///Color Pattern description (2x2 matrix):
///
/// The color pattern is declared by four nibbles. Each nibble holds the value of the corresponding color.
///
/// Color defines:
/// - RED = 0x01
/// - GREEN (RED LINE) = 0x02
/// - GREEN (BLUE LINE) = 0x03
/// - BLUE = 0x04
///
/// The four nibbles are arranged in the following way:
///
/// \f$
/// \begin{tabular}{ | c | c | c | c | c | c | }
/// \hline
/// MSB & 3 & 2 & 1 & 0 & LSB \\ \hline
/// \end{tabular}
/// \f$
///
/// For the sample this would result in:
///
/// 0x04030201 (Nibble3: BLUE, Nibble2: GREENB, Nibble1: GREENR, Nibble0: RED)
///
/// The color description is necessary for determining the color of the upper left corner of a color image. The resulting value is a parameter for the de-mosaicking algorithm.
///
/// \f[
/// \begin{tabular}{| p{4.5cm} | c | p{4.5cm} | c |}
/// \hline
/// \rowcolor{lightgray} \multicolumn{4}{|l|}{\textbf{Sensor Type codes:}} \\ \rowcolor{lightgray} \multicolumn{2}{|l}{\textbf{monochrome sensors:}} & \multicolumn{2}{l|}{\textbf{color sensors:}} \\ \hline
/// Sony ICX285AL & 0x0010 & Sony ICX285AK & 0x0011 \\ Sony ICX263AL & 0x0020 & Sony ICX263AK & 0x0021 \\ Sony ICX274AL & 0x0030 & Sony ICX274AK & 0x0031 \\ Sony ICX407AL & 0x0040 & Sony ICX407AK & 0x0041 \\ Sony ICX414AL & 0x0050 & Sony ICX414AK & 0x0051 \\ Kodak KAI-2000M & 0x0110 & Kodak KAI-2000CM & 0x0111 \\ Kodak KAI-2001M & 0x0120 & Kodak KAI-2001CM & 0x0121 \\ Kodak KAI-4010M & 0x0130 & Kodak KAI-4010CM & 0x0131 \\ Kodak KAI-4011M & 0x0132 & Kodak KAI-4011CM & 0x0133 \\ Kodak KAI-4020M & 0x0140 & Kodak KAI-4020CM & 0x0141 \\ Kodak KAI-4021M & 0x0142 & Kodak KAI-4021CM & 0x0143 \\ Kodak KAI-11000M & 0x0150 & Kodak KAI-11000CM & 0x0151 \\ Kodak KAI-11002M & 0x0152 & Kodak KAI-11002CM & 0x0153 \\ Kodak KAI-16000AXA & 0x0160 & Kodak KAI-16000CXA & 0x0161 \\ Micron MV13 bw & 0x1010 & Micron MV13 col & 0x1011 \\ Fairchild CIS2051 V1 FI & 0x2000 & - & - \\ Fairchild CIS2051 V1 BI & 0x2010 & - & - \\ Cypress RR V1 bw & 0x3000 & Cypress RR V1 col & 0x3001 \\ \hline
/// \end{tabular}
/// \f]
/// \b Note: This list will be updated with new entries.
///
/// Some new firmware features have been implemented after the release of the camera. The general
/// caps dword describes the availability of new functionality.
/// The \b GENERALCAPS1 dword holds the following flags:
///
/// \f$
/// \begin{tabular}{| p{6cm} | c | p{6cm} |}
/// \hline
/// \rowcolor{lightgray} \textbf{Flag name} & \textbf{value} & \textbf{Short description} \\ \hline
/// NOISE\_FILTER & 0x00000001  & Noise filter is available \\ \hline
/// HOTPIX\_FILTER & 0x00000002  & Hot pixel correction is available \\ \hline
/// HOTPIX\_ONLY\_WITH\_NOISE\_FILTER & 0x00000004  & Hot pixel corr. does not work without noise filter \\ \hline
/// TIMESTAMP\_ASCII\_ONLY & 0x00000008 & Time stamp without binary is available \\ \hline
/// DATAFORMAT2X12 & 0x00000010 & Camlink (1200hs) transfer can be done by 2x12  \\ \hline
/// RECORD\_STOP & 0x00000020 & Record stop event mode is available  \\ \hline
/// HOT\_PIXEL\_CORRECTION & 0x00000040 & Hot pixel correction is available \\ \hline
/// NO\_EXTEXPCTRL & 0x00000080 & External exposure control is not available \\ \hline
/// NO\_TIMESTAMP & 0x00000100 & Time stamp is not available \\ \hline
/// NO\_ACQUIREMODE & 0x00000200 & Acquire mode is not available \\ \hline
/// DATAFORMAT4X16 & 0x00000400 & Data format 4x16 is available \\ \hline
/// DATAFORMAT5X16 & 0x00000800 & Data format 5x16 is available \\ \hline
/// NO\_RECORDER & 0x00001000 & Camera has no  internal recorder \\ \hline
/// FAST\_TIMING & 0x00002000 & Camera supports fast timing mode \\ \hline
/// METADATA & 0x00004000 & Camera can produce meta data \\ \hline
/// SETFRAMERATE\_ENABLED & 0x00008000 & Camera supports Set/GetFrameRate \\ \hline
/// CDI\_MODE & 0x00010000 & Supports correlated double image mode \\ \hline
/// CCM & 0x00020000 & Has got internal color correction matrix \\ \hline
/// EXTERNAL\_SYNC & 0x00040000 & Can be synced externally \\ \hline
/// NO\_GLOBAL\_SHUTTER & 0x00080000 & Global shutter setting not supported \\ \hline
/// GLOBAL\_RESET\_MODE & 0x00100000 & Global reset rolling readout supported \\ \hline
/// EXT\_ACQUIRE & 0x00200000 & extended acquire command \\ \hline
/// FAN\_CONTROL & 0x00400000 & fan control command \\ \hline
/// ROI\_VERT\_SYMM\_TO\_HORZ\_AXIS & 0x00800000 & vert.ROI must be symmetrical to horizontal axis \\ \hline
/// ROI\_VERT\_SYMM\_TO\_VERT\_AXIS & 0x01000000 & horz.ROI must be symmetrical to vertical axis \\ \hline
/// HW\_IO\_SIGNAL\_DESCRIPTOR & 0x40000000 & Hardware IO description is available \\ \hline
/// ENHANCED\_DESCRIPTOR\_2 & 0x80000000 & Enhanced description 2 is available \\ \hline
/// \end{tabular}
/// \f$
///
///See sc2_defs.h for more information.
///
/// \param resp Pointer to a SC2_Camera_Description_Response structure.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetCameraDescription(SC2_Camera_Description_Response* resp);

///
/// \brief Get image sensor gain setting
///
/// Current conversion factor in electrons/count (the variable must be divided by 100 to get the real value)
///
/// i.e. 0x01B3 (hex) = 435 (decimal) = 4.35 electrons/count conversion factor must be valid as defined in the camera description
/// \param wConvFact Pointer to a WORD variable to receive the conversion factor.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetConversionFactor(WORD* wConvFact);

///
/// \brief Set image sensor gain
///
/// Conversion factor to be set in electrons/count (the variable must be divided by 100 to get the real value)
///
/// i.e. 0x01B3 (hex) = 435 (decimal) = 4.35 electrons/count
///
/// Conversion factor must be valid as defined in the camera description.
/// \param wConvFact WORD to set the conversion factor.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetConversionFactor(WORD wConvFact);

///
/// \brief Gets the double image mode of the camera.
///
/// Not applicable to all cameras.
/// \param wDoubleImage Pointer to a WORD variable to receive the double image mode.
/// - 0x0001 = double image mode ON
/// - 0x0000 = double image mode OFF
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetDoubleImageMode(WORD* wDoubleImage);

///
/// \brief Sets the double image mode of the camera.
///
/// Some cameras (defined in the camera description) allow the user to make
/// a double image with two exposures separated by a short interleaving time. A double image is
/// transferred as one frame, that is the two images resulting from the two/double exposures are
/// stitched together as one and are counted as one. Thus the buffer size has to be doubled. The first
/// half of the buffer will be filled with image 'A', the first exposed frame. The second exposure
/// (image 'B') will be transferred to the second half of the buffer.
///
/// Not applicable to all cameras.
/// \param wDoubleImage WORD variable to hold the double image mode.
/// - 0x0001 = double image mode ON
/// - 0x0000 = double image mode OFF
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetDoubleImageMode(WORD wDoubleImage);

///
/// \brief Get analog-digital-converter (ADC) operation for reading the image sensor data.
///
/// Pixel data can be read out using one ADC (better linearity), or in parallel using two ADCs (faster). This option is
/// only available for some camera models (defined in the camera description). If the user sets 2ADCs he must center and
/// adapt the ROI to symmetrical values, e.g. pco.1600: x1,y1,x2,y2=701,1,900,500 (100,1,200,500 is not possible).
/// \param wADCOperation Pointer to a WORD variable to receive the adc operation mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetADCOperation(WORD* wADCOperation);

///
/// \brief Gets the IR sensitivity mode of the camera.
///
/// This option is only available for special camera models with image sensors that have improved IR sensitivity.
/// \param wIR Pointer to a WORD variable to receive the IR sensitivity mode.
/// - 0x0000 IR sensitivity OFF
/// - 0x0001 IR sensitivity ON
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetIRSensitivity(WORD* wIR);

///
/// \brief Gets the IR sensitivity mode of the camera.
///
/// Set IR sensitivity for the image sensor. This option is only available for special camera models with image sensors that have improved IR sensitivity.
/// \param wIR WORD variable to set the IR sensitivity mode.
/// - 0x0000 IR sensitivity OFF
/// - 0x0001 IR sensitivity ON
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetIRSensitivity(WORD wIR);

///
/// \brief Get the temperature set point for cooling the image sensor (only available for cooled cameras).
///
/// If min. cooling set point (in \f$^\circ\f$C) and max. cooling set point (in \f$^\circ\f$C) are zero, then cooling is not available.
/// \param sCoolSet Pointer to a SHORT variable to receive the cooling setpoint temperature.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetCoolingSetpointTemperature(SHORT* sCoolSet);

///
/// \brief Set the temperature set point for cooling the image sensor (only available for cooled cameras).
///
/// If min. cooling set point (in \f$^\circ\f$C) and max. cooling set point (in \f$^\circ\f$C) are zero, then cooling is not available.
/// \param sCoolSet SHORT variable to hold the cooling setpoint temperature in \f$^\circ\f$C units.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetCoolingSetpointTemperature(SHORT sCoolSet);

///
/// \brief Get the mode for the offset regulation with reference pixels (see camera manual for further explanations).
///
/// Mode:
/// - 0x0000 = [auto]
/// - 0x0001 = [OFF]
///
/// \param wOffsetRegulation Pointer to a WORD variable to receive the offset mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetOffsetMode (WORD* wOffsetRegulation);

///
/// \brief Set the mode for the offset regulation with reference pixels (see the camera manual for further explanations).
///
/// Mode:
/// - 0x0000 = [auto]
/// - 0x0001 = [OFF]
///
/// \param wOffsetRegulation WORD variable to hold the offset mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetOffsetMode (WORD wOffsetRegulation);

DWORD PCO_GetCameraDescriptionEx(SC2_Camera_Description_Response* descript1,SC2_Camera_Description_2_Response* descript2,WORD wType);

///
/// \brief Get the actual noise filter mode. See the camera descriptor for availability of this feature.
///
/// Parameter:
/// - 0x0000 = [OFF]
/// - 0x0001 = [ON]
/// - 0x0101 = [ON + Hot Pixel correction]
///
/// \param wNoiseFilterMode Pointer to a WORD variable to receive the noise filter mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetNoiseFilterMode (WORD* wNoiseFilterMode);

///
/// \brief Sets the actual noise filter mode. See the camera descriptor for availability of this feature.
///
/// Parameter:
/// - 0x0000 = [OFF]
/// - 0x0001 = [ON]
/// - 0x0101 = [ON + Hot Pixel correction]
///
/// \param wNoiseFilterMode WORD variable to hold the noise filter mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetNoiseFilterMode (WORD wNoiseFilterMode);

///
/// \brief Get the Hot Pixel correction mode.
///
/// This command is optional and depends on the hardware and firmware. Check the availability according to the camera descriptor (HOT_PIXEL_CORRECTION).
/// Mode:
/// - 0x0000 = [OFF]
/// - 0x0001 = [ON]
///
/// \param wHotPixelCorrectionMode Pointer to a WORD variable to receive the hot pixel correction mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetHotPixelCorrectionMode (WORD* wHotPixelCorrectionMode);

///
/// \brief Get the Hot Pixel correction mode.
///
/// This command is optional and depends on the hardware and firmware. Check the availability according to the camera descriptor (HOT_PIXEL_CORRECTION).
/// Mode:
/// - 0x0000 = [OFF]
/// - 0x0001 = [ON]
///
/// \param wHotPixelCorrectionMode Pointer to a WORD variable to receive the hot pixel correction mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetHotPixelCorrectionMode (WORD wHotPixelCorrectionMode);

///
/// \brief Writes a hot pixel list to the camera.
///
/// This command is optional and depends on the hardware and firmware. Check the availability
/// according to the camera descriptor (HOT_PIXEL_CORRECTION). To change the hot pixel list
/// inside the camera, please call PCO_ReadHotPixelList() first, then modify the list and write it back
/// with this command. We recommend doing a backup of the list after readout. An invalid list will break the hot pixel correction!
///
/// The x and y coordinates have to be consistent, that means corresponding coordinate pairs must have the same index!
/// \param wListNo WORD variable which holds the number of the list (zero based).
/// \param wNumValid WORD variable which holds the number of valid members
/// \param wHotPixX WORD array which holds the x coordinates of a hotpixel list
/// \param wHotPixY WORD array which holds the y coordinates of a hotpixel list
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_WriteHotPixelList(WORD wListNo, WORD wNumValid,WORD* wHotPixX, WORD* wHotPixY);

///
/// \brief Reads a hot pixel list from the camera.
///
/// This command is optional and depends on the hardware and firmware. Check the availability
/// according to the camera descriptor (HOT_PIXEL_CORRECTION). To change the hot pixel list
/// inside the camera, please call this command first, then modify the list and write it back
/// with PCO_WriteHotPixelList(). We recommend doing a backup of the list after readout. An invalid list will break the hot pixel correction!
/// \param wListNo WORD variable which holds the number of the list (zero based).
/// \param wArraySize WORD variable which holds the number of members, which can be transferred to the list
/// \param wNumValid Pointer to a WORD variable to receive the number of valid hotpixel.
/// \param wNumMax Pointer to a WORD variable to receive the max. possible number of hotpixel.
/// \param wHotPixX WORD array which gets the x coordinates of a hotpixel list
///                 This ptr can be set to ZERO if only the valid and max number have to be read.
/// \param wHotPixY WORD array which gets the y coordinates of a hotpixel list
///                 This ptr can be set to ZERO if only the valid and max number have to be read.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_ReadHotPixelList(WORD wListNo, WORD wArraySize,WORD* wNumValid, WORD* wNumMax, WORD* wHotPixX, WORD* wHotPixY);

///
/// \brief Clears a hot pixel list in the camera.
///
/// This command is optional and depends on the hardware and firmware. Check the availability
/// according to the camera descriptor (HOT_PIXEL_CORRECTION). To change the hot pixel list
/// inside the camera, please first call PCO_ReadHotPixelList(). Then modify the list and write it back
/// with PCO_WriteHotPixelList(). We recommend doing a backup of the list after readout. An invalid
/// list will break the hot pixel correction! This command clears the list addressed completely. Use with caution!
///
/// Set the following parameter:
/// - wListNo: Number of the list to modify (0 ...).
/// - dwMagic1: Unlock code, set to 0x1000AFFE
/// - dwMagic2: Unlock code, set to 0x2000ABBA
///
/// \param wListNo WORD variable which holds the number of the list (zero based).
/// \param dwMagic1 DWORD variable which holds the unlock code 1.
/// \param dwMagic2 DWORD variable which holds the unlock code 2.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_ClearHotPixelList(WORD wListNo, DWORD dwMagic1,DWORD dwMagic2);

///
/// \overload
///
DWORD PCO_ClearHotPixelList(WORD wListNo);

///
/// \brief Requests the Bayer multipliers.
///
/// The Bayer multipliers are used by cameras with color sensor in order to compensate the color response of the sensor and the optical setup. Thus when exposed to
/// white light the R Gr Gb B pixels will ideally show the same amplitude. This option is only available with a pco.dimax
/// \param wMode
/// - 0x0001: The returned values are changed, but not yet saved.
/// - 0x0002: The returned values are saved.
/// \param wMul Pointer to an array of four WORD;
///             Red/GreenRed/GreenBlue/Blue Multiplier: Number from 0 to 3999, where 1000 corresponds to multiplier of 1.0 (leave values unchanged).
///             Element 0 is the same as in the color descriptor. See wColorPatternDESC in PCO_Description.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetBayerMultiplier(WORD* wMode,WORD* wMul);

///
/// \brief Sets the Bayer multipliers.
///
/// The Bayer multipliers are used by cameras with color sensor in order to compensate the color response of the sensor and the optical setup. Thus when exposed to
/// white light the R Gr Gb B pixels will ideally show the same amplitude. This option is only available with a pco.dimax
/// \param wMode
/// - 0x0001: Set new values immediately but do not save.
/// - 0x0002: Save values and set immediately.
/// \param wMul Pointer to an array of four WORD;
///             Red/GreenRed/GreenBlue/Blue Multiplier: Number from 0 to 3999, where 1000 corresponds to multiplier of 1.0 (leave values unchanged).
///             Element 0 is the same as in the color descriptor. See wColorPatternDESC in PCO_Description.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetBayerMultiplier(WORD wMode,WORD* wMul);

///
/// \brief Gets the color multiplier matrix to normalize the color values of a color camera to 6500k.
///
/// Only available with a dimax
/// \param szCCM Pointer to a string to hold the values.
/// \param len Pointer to a WORD that holds the string length.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetColorCorrectionMatrix(char* szCCM,WORD* len);

///
/// \brief Gets the camera internal DSNU adjustment mode.
///
/// Only available with a dimax.
///
/// \param wMode
/// - 0: no DSNU correction.
/// - 1: automatic DSNU correction.
/// - 2: manual DSNU correction.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetDSNUAdjustMode(WORD *wMode);

///
/// \brief Sets the camera internal DSNU adjustment mode.
///
/// Only available with a dimax.
///
/// \param wMode
/// - 0: no DSNU correction.
/// - 1: automatic DSNU correction.
/// - 2: manual DSNU correction.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetDSNUAdjustMode(WORD wMode);

///
/// \brief Starts the camera internal DSNU adjustment in case it is set to manual.
///
/// Only available with a dimax.
///
/// \param wMode
/// - 0: no DSNU correction.
/// - 1: automatic DSNU correction.
/// - 2: manual DSNU correction.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_InitDSNUAdjustment(WORD *wMode);

///
/// \brief Gets the correlated double image mode of the camera.
///
/// Only available with a dimax.
///
/// \param wMode Pointer to a WORD variable to receive the correlated double image mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetCDIMode(WORD* wMode);

///
/// \brief Sets the correlated double image mode of the camera.
///
/// Only available with a dimax.
///
///
/// \param wMode WORD variable to set the correlated double image mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetCDIMode(WORD wMode);

///
/// \brief Gets infos about lookup tables in the camera, if available.
///
/// Only available with a pco.edge.
///
/// \param wLUTNum WORD variable to hold the number of LUT to query.
/// \param wNumberOfLuts Pointer to WORD variable to recieve the number of LUTs which can be queried
/// \param Description Pointer to string to recieve the description, e.g. "HD/SDI 12 to 10".
/// \param wDescLen Pointer to WORD variable to recieve the string length.
/// \param wIdentifier Pointer to WORD variable to recieve the loadable LUTs. Range from 0x0001 to 0xFFFF
/// \param bInputWidth Pointer to BYTE variable to recieve the maximum input in bits.
/// \param bOutputWidth Pointer to BYTE variable to recieve the maximum output in bits.
/// \param wFormat Pointer to WORD variable to recieve the accepted data structures (see defines!)
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetLookupableInfo(WORD wLUTNum, WORD* wNumberOfLuts, char* Description,WORD wDescLen,WORD* wIdentifier, BYTE* bInputWidth,BYTE* bOutputWidth, WORD* wFormat);

///
/// \brief Loads a lookup table to the camera, if available.
///
/// Only available with a pco.edge.
///
/// \param wIdentifier WORD variable to hold the LUT to be loaded
/// \param wPacketNum WORD variable to hold the packet number to load the LUT in several steps.
/// \param wFormat WORD variable to hold the data structure in bData (see defnines)
/// \param wLength WORD variable to hold the length of the data structure
/// \param bData Pointer to BYTE to hold the actual data
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_LoadLookuptable(WORD wIdentifier,WORD wPacketNum,WORD wFormat,WORD wLength,BYTE* bData);

///
/// \brief Reads a lookup table from the camera, if available.
///
/// Only available with a pco.edge.
///
/// \param wIdentifier WORD variable to hold the LUT to be read
/// \param wPacketNum WORD variable to hold the packet number to read the LUT in several steps.
/// \param wFormat Pointer to WORD variable to receive the data structure in bData (see defnines)
/// \param wLength Pointer to WORD variable to receive the length of the data structure
/// \param bData Pointer to BYTE array to receive the data
/// \param buflen WORD variable to hold the length of the BYTE array (bData)
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_ReadLookuptable(WORD wIdentifier,WORD wPacketNum,WORD* wFormat,WORD* wLength,BYTE* bData,WORD buflen);

DWORD PCO_GetLookuptableInfoExt(WORD wIndex);

///
/// \brief Gets the cooling setpoints of the camera.
///
/// This is used when there is no min max range available.
///
/// \param wBlockID Number of the block to query (currently 0)
/// \param sSetPoints Pointer to a SHORT array to receive the possible cooling setpoint temperatures.
/// \param wValidSetPoints WORD Pointer to set the max number of setpoints to query and to get the valid number of set points inside the camera. In case more than COOLING_SETPOINTS_BLOCKSIZE set points are valid they can be queried by incrementing the wBlockID till wNumSetPoints is reached.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetCoolingSetpoints(WORD wBlockID,SHORT* sSetPoints,WORD* wValidSetPoints);

///
/// \brief Gets the busy state of the camera.
///
/// Get camera busy status: a trigger is ignored if the camera is still busy ([exposure] or [readout]). In
/// case of force trigger command, the user may request the camera busy status in order to be able to
/// start a valid force trigger command. Please do not use this function for image synchronization.
///
/// \b Note: The busy status is according to the hardware signal \<busy\> at the \<status output\> at the
/// power supply unit. Due to response and processing times, e.g., caused by the interface and/or
/// the operating system, the delay between the delivered status and the actual status may be
/// several 10 ms up to 100 ms. If timing is critical, it is strongly recommended that the hardware
/// signal (\<busy\>) be used.
///
/// \param wCameraBusyState Pointer to a WORD variable to receive the busy state.
///                         - 0x0000 = camera is [not busy], ready for a new trigger command
///                         - 0x0001 = camera is [busy], not ready for a new trigger command
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetCameraBusyStatus(WORD* wCameraBusyState);

///
/// \brief Get user values for CCD or CMOS power down threshold time (see camera manual).
///
/// \param dwPdnTime Pointer to a DWORD variable to receive the power down time.
///                  Current CCD power down threshold time as multiples of ms (0ms .. 49.7 days)
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetUserPowerDownTime(DWORD* dwPdnTime);

///
/// \brief Set user values for CCD or CMOS power down threshold time (see camera manual).
///
/// If the exposure time is greater than the selected Power Down Time, then the CCD or CMOS sensor is
/// switched (electrically) into a special power down mode to reduce dark current effects. If power
/// down mode = [user] is selected, the power down threshold time set by this function will become
/// effective. The default Power Down Time is one second.
/// \param dwPdnTime DWORD variable to set the power down time.
///                  Current CCD power down threshold time as multiples of ms (0ms .. 49.7 days)
/// \return
///
DWORD PCO_SetUserPowerDownTime(DWORD dwPdnTime);

///
/// \brief Get delay / exposure time table.
///
/// \b General \b note:
///
/// For some camera types it is possible to define a table with delay / exposure times (defined in the
/// camera description). After the exposure is started, the camera will take a series of consecutive
/// images with delay and exposure times, as defined in the table. Therefore, a flexible message
/// format has been defined. The table consists of a maximum of 16 delay / exposure time pairs. If an
/// exposure time entry is set to the value zero, then at execution time this delay / exposure pair is
/// disregarded and the sequence is started automatically with the first valid entry in the table. This
/// results in a sequence of 1 to 16 images with different delay and exposure time settings. External oautomatic image triggering is fully functional for every image in the sequence. If the user wants
/// maximum speed (at CCDs overlapping exposure and read out is taken), [auto trigger] should be
/// selected and the sequence should be controlled with the \<acq enbl\> input.
/// \b Note:
///
/// The commands set delay / exposure time and set delay / exposure time table can only be
/// used alternatively. Using set delay / exposure time has the same effect as using the table
/// command and setting all but the first delay / exposure entry to zero.
/// Despite the same parameter set, this function is different to PCO_GetDelayExposureTime() because the corresponding pointers are used as an array of 16 values each.
///
/// Timebase:
/// - 0: ns
/// - 1: \f$\mu\f$s;
/// - 2: ms
///
/// \param dwDelay Pointer to a DWORD array to receive the exposure times.
/// \param dwExposure Pointer to a DWORD array to receive the delay times.
/// \param wTimeBaseDelay Pointer to a WORD variable to receive the exp. timebase.
/// \param wTimebaseExposure Pointer to a WORD variable to receive the del. timebase.
/// \param wCount Maximum count of delay and exposure pairs, not more than 16 DWORDS.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetDelayExposureTimeTable(DWORD* dwDelay,DWORD* dwExposure,WORD* wTimeBaseDelay,WORD* wTimebaseExposure,WORD wCount);

///
/// \brief Sets the exposure and delay time table and the time bases of the camera.
///
/// Timebase:
/// - 0: ns
/// - 1: \f$\mu\f$s;
/// - 2: ms
///
/// \param dwDelay Pointer to a DWORD array to hold the exposure times.
/// \param dwExposure Pointer to a DWORD array to hold the delay times.
/// \param wTimeBaseDelay WORD variable to hold the exp. timebase.
/// \param wTimebaseExposure WORD variable to hold the del. timebase.
/// \param wCount Count of delay and exposure pairs.
/// \return Error message, 0 in case of success else less than 0.
/// \see PCO_GetDelayExposureTimeTable
DWORD PCO_SetDelayExposureTimeTable(DWORD* dwDelay,DWORD* dwExposure,WORD wTimeBaseDelay,WORD wTimebaseExposure,WORD wCount);

///
/// \brief Get mode for CCD or CMOS power down mode (see camera manual).
///
/// Mode:
/// - 0x0000 = [AUTO]
/// - 0x0001 = [USER]
///
/// \param wPowerDownMode Pointer to a WORD variable to receive the power down mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetPowerDownMode(WORD* wPowerDownMode);

///
/// \brief Set mode for CCD or CMOS power down threshold time control.
///
/// Power down functions are controllable when power down mode = [user] is selected (see camera manual).
///
/// Mode:
/// - 0x0000 = [AUTO]
/// - 0x0001 = [USER]
///
/// \param wPowerDownMode WORD variable to hold the power down mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetPowerDownMode(WORD wPowerDownMode);

///
/// \brief Get the current status of the \<exp trig\> user input (one of the \<control in\> inputs at the rear of pco.power or the camera connectors).
///
/// See camera manual for more information about hardware signals.
/// \param wExpTrgSignal Pointer to a WORD variable to receive the exposure trigger signal state.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetExpTrigSignalStatus(WORD* wExpTrgSignal);

///
/// \brief The FPS Exposure Mode is available for the pco.1200hs camera model only!
///
/// The FPS exposure mode is useful if the user wants to get the maximum exposure time for the maximum frame rate.
/// The maximum image frame rate (FPS = Frames Per Second) depends on the pixelrate, the vertical ROI and the exposure time.
/// \param wFPSExposureMode Pointer to a WORD variable to receive the FPS-exposure-mode.
///   - 0: FPS Exposure Mode off, exposure time set by PCO_SetDelay/Exposure Time command
///   - 1: FPS Exposure Mode on, exposure time set automatically to 1 / FPS max. PCO_SetDelay/Exposure Time commands are ignored.
/// \param dwFPSExposureTime Pointer to a DWORD variable to receive the FPS exposure time in nanoseconds.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetFPSExposureMode(WORD* wFPSExposureMode,DWORD* dwFPSExposureTime);

///
/// \brief The FPS Exposure Mode is available for the pco.1200hs camera model only!
///
/// The FPS exposure mode is useful if the user wants to get the maximum exposure time for the maximum frame rate.
/// The maximum image frame rate (FPS = Frames Per Second) depends on the pixelrate, the vertical ROI and the exposure time.
/// \param wFPSExposureMode WORD variable to hold the FPS-exposure-mode.
/// - 0: FPS Exposure Mode off, exposure time set by PCO_SetDelay/Exposure Time command
/// - 1: FPS Exposure Mode on, exposure time set automatically to 1 / FPS max. PCO_SetDelay/Exposure Time commands are ignored.
/// \param dwFPSExposureTime Pointer to a DWORD variable to receive the FPS exposure time in nanoseconds.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetFPSExposureMode(WORD wFPSExposureMode,DWORD* dwFPSExposureTime);

///
/// \brief Gets the modulation mode and necessary parameters.
///
/// The Modulation Mode is an optional feature which is not available for all camera models. See the descriptors of the camera.
///
/// - Current modulation mode:
///   - 0x0000 = [modulation mode off]
///   - 0x0001 = [modulation mode on]
/// - Periodical time as a multiple of the timebase unit: The periodical time, delay and exposure time must meet the following condition : tp - (te + td) \> 'Min Per Condition'
/// - Timebase for periodical time
///   - 0x0000 => timebase = [ns]
///   - 0x0001 => timebase = [\f$\mu\f$s]
///   - 0x0002 => timebase = [ms]
/// - Number of exposures: number of exposures done for one frame
/// - Monitor signal offset [ns]: controls the offset for the \<status out\> signal. The possible range is
/// limited in a very special way. See tm in the above timing diagrams. The minimum range is -tstd...0.
/// The negative limit can be enlarged by adding a delay. The maximum negative
/// monitor offset is limited to -20\f$\mu\f$s, no matter how long the delay will be set. The positive
/// limit can be enlarged by longer exposure times than the minimum exposure time. The
/// maximum positive monitor offset is limited to 20us, no matter how long the exposure will be set.
///
/// \param wModulationMode Pointer to a WORD variable to receive the modulation mode
/// \param dwPeriodicalTime Pointer to a DWORD variable to receive the periodical time
/// \param wTimebasePeriodical Pointer to a WORD variable to receive the time base of pt
/// \param dwNumberOfExposures Pointer to a DWORD variable to receive the number of exposures
/// \param lMonitorOffset Pointer to a signed DWORD variable to receive the monitor offset
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetModulationMode(WORD *wModulationMode, DWORD* dwPeriodicalTime, WORD *wTimebasePeriodical, DWORD *dwNumberOfExposures, LONG *lMonitorOffset);

///
/// \brief Set the modulation mode and necessary parameters.
///
/// The Modulation Mode is an optional feature which is not available for all camera models. See the descriptors of the camera.
///
/// - Current modulation mode:
///   - 0x0000 = [modulation mode off]
///   - 0x0001 = [modulation mode on]
/// - Periodical time as a multiple of the timebase unit: The periodical time, delay and exposure time must meet the following condition : tp - (te + td) \> 'Min Per Condition'
/// - Timebase for periodical time
///   - 0x0000 => timebase = [ns]
///   - 0x0001 => timebase = [\f$\mu\f$s]
///   - 0x0002 => timebase = [ms]
/// - Number of exposures: number of exposures done for one frame
/// - Monitor signal offset [ns]: controls the offset for the \<status out\> signal. The possible range is
/// limited in a very special way. See tm in the above timing diagrams. The minimum range is -tstd...0.
/// The negative limit can be enlarged by adding a delay. The maximum negative
/// monitor offset is limited to -20\f$\mu\f$s, no matter how long the delay will be set. The positive
/// limit can be enlarged by longer exposure times than the minimum exposure time. The
/// maximum positive monitor offset is limited to 20us, no matter how long the exposure will be set.
///
/// \param wModulationMode WORD variable to hold the modulation mode
/// \param dwPeriodicalTime DWORD variable to hold the periodical time
/// \param wTimebasePeriodical WORD variable to hold the time base of pt
/// \param dwNumberOfExposures DWORD variable to hold the number of exposures
/// \param lMonitorOffset DWORD variable to hold the monitor offset
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetModulationMode(WORD wModulationMode, DWORD dwPeriodicalTime, WORD wTimebasePeriodical, DWORD dwNumberOfExposures, LONG lMonitorOffset);

///
/// \brief Get frame rate / exposure time.
///
/// \b Note:
/// - Frame rate and exposure time are also affected by the "Set Delay/Exposure Time" command.
/// It is strongly recommend to use either the "Set Framerate" or the "Set Delay/Exposure
/// Time" command! The last issued command will determine the timing before calling the ARM command.
/// - Function is not supported by all cameras, at that moment only by the pco.dimax!
///
/// \param wFrameRateStatus Pointer to WORD variable to receive the status
/// - 0x0000: Settings consistent, all conditions met
/// - 0x0001: Framerate trimmed, framerate was limited by readout time
/// - 0x0002: Framerate trimmed, framerate was limited by exposure time
/// - 0x0004: Exposure time trimmed, exposure time cut to frame time
/// - 0x8000: The return values dwFrameRate and dwFrameRateExposure are not yet validated. The values returned are the values which were passed with the most recent call of PCO_SetFramerate() function.
/// \param dwFrameRate DWORD variable to receive the actual frame rate in mHz
/// \param dwFrameRateExposure DWORD variable to receive the actual exposure time (in ns)
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetFrameRate (WORD* wFrameRateStatus, DWORD* dwFrameRate, DWORD* dwFrameRateExposure);

///
/// \brief Sets the frame rate mode, rate and exposure time
///
/// Set frame rate and exposure time. This command is intended to set directly the frame rate and the
/// exposure time of the camera. The frame rate is limited by the readout time and the exposure time:
/// \f[
/// Framerate \leq \frac{1}{t_{readout}}
/// \f]
/// \f[
/// Framerate \leq \frac{1}{t_{expos}}
/// \f]
/// Please note that there are some overhead times, therefore the real values can differ slightly, e.g.
/// the maximum frame rate will be a little bit less than 1 / exposure time. The mode parameter of the
/// function call defines how the function works if these conditions are not met.
/// The function differs, if the camera is recording (recording state = 1) or if recording is off:
///
/// Camera is recording:
/// The frame rate / exposure time is changed immediately. The function returns the actually configured frame rate and exposure time.
///
/// Record is off:
/// The frame rate / exposure time is stored. The function does not change the input values for frame
/// rate and exposure time. A succeeding "Arm Camera" command (PCO_ArmCamera()) validates the
/// input parameters together with other settings, e.g. The status returned indicates, if the input
/// arameters are validated. The following procedure is recommended:
/// - Set frame rate and exposure time using the PCO_SetFrameRate() function.
/// - Do other settings, before or after the PCO_SetFrameRate() function.
/// - Call the PCO_ArmCamera() function in order to validate the settings.
/// - Retrieve the actually set frame rate and exposure time using PCO_GetFrameRate.
///
/// \param wFrameRateStatus Pointer to WORD variable to receive the status
/// - 0x0000: Settings consistent, all conditions met
/// - 0x0001: Framerate trimmed, framerate was limited by readout time
/// - 0x0002: Framerate trimmed, framerate was limited by exposure time
/// - 0x0004: Exposure time trimmed, exposure time cut to frame time
/// \param wFramerateMode Pointer to WORD variable to set the frame rate mode
/// - 0x0000: auto mode (camera decides which parameter will be trimmed)
/// - 0x0001: Framerate has priority, (exposure time will be trimmed)
/// - 0x0002: Exposure time has priority, (framerate will be trimmed)
/// - 0x0003: Strict, function shall return with error if values are not possible.
/// \param dwFrameRate DWORD variable to receive the actual frame rate in mHz (milli!)
/// \param dwFrameRateExposure DWORD variable to receive the actual exposure time (in ns)
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetFrameRate (WORD* wFrameRateStatus, WORD wFramerateMode, DWORD* dwFrameRate, DWORD* dwFrameRateExposure);

DWORD PCO_GetCOCExptime(DWORD* dwtime_s,DWORD* dwtime_ns);

///
/// \brief Gets the camera synchronisation mode for a dimax.
///
/// Dimax cameras can be cascaded in order to synchronize the timing of a camera chain. It is mandatory to set one of the cameras in the chain to
/// master mode. Usually this is the first camera connected to the chain. All output side connected
/// cameras should be set to slave mode. Those cameras will follow the timing of the master camera, thus all timing settings are disabled at the slave cameras.
/// \param wCameraSynchMode Pointer to a WORD variable to receive the synch mode.
/// - 0x0000 = [off]
/// - 0x0001 = [master]
/// - 0x0002 = [slave]
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetCameraSynchMode(WORD* wCameraSynchMode);

///
/// \brief Sets the camera synchronisation mode for a dimax.
///
/// Dimax cameras can be cascaded in order to synchronize the timing of a camera chain. It is mandatory to set one of the cameras in the chain to
/// master mode. Usually this is the first camera connected to the chain. All output side connected
/// cameras should be set to slave mode. Those cameras will follow the timing of the master camera,
/// thus all timing settings are disabled at the slave cameras.
/// \param wCameraSynchMode WORD variable to set the synch mode.
/// - 0x0000 = [off]
/// - 0x0001 = [master]
/// - 0x0002 = [slave]
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetCameraSynchMode(WORD wCameraSynchMode);

///
/// \brief Gets the timing of one image, including trigger delay, trigger jitter, etc.
///
/// The input structure will be filled with the following parameters:
/// - FrameTime_ns: Nanoseconds part of the time to expose and readout one image.
/// - FrameTime_s: Seconds part of the time to expose and readout one image.
/// - ExposureTime_ns: Nanoseconds part of the exposure time.
/// - ExposureTime_s: Seconds part of the exposure time.
/// - TriggerSystemDelay_ns: System internal minimum trigger delay, till a trigger is recognized and executed by the system.
/// - TriggerSystemJitter_ns: Maximum possible trigger jitter, which influences the real trigger delay. Real trigger delay=TriggerDelay_ns +/-TriggerSystemJitter
/// - TriggerDelay_ns: Total trigger delay part in ns, till a trigger is recognized and executed by the system.
/// - TriggerDelay_ns: Total trigger delay part in s, till a trigger is recognized and executed by the system.
///
/// \param imageresp Pointer to a image timing structure
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetImageTiming (SC2_Get_Image_Timing_Response *imageresp);


DWORD PCO_GetCMOSLinetiming(WORD* wParameter,WORD* wTimebase,DWORD* dwLineTime);
DWORD PCO_SetCMOSLinetiming(WORD wParameter,WORD wTimebase,DWORD dwLineTime);
DWORD PCO_GetCMOSLineExposureDelay(DWORD* dwExposureLines,DWORD* dwDelayLines);
DWORD PCO_SetCMOSLineExposureDelay(DWORD dwExposureLines,DWORD dwDelayLines);

///
/// \brief Gets the camera fast timing mode for a dimax.
///
/// To increase the possible exposure time with high frame rates it is possible to enable the 'Fast Timing' mode. This means that the maximum possible
/// exposure time can be longer than in normal mode, while getting stronger offset drops. In case,
/// especially in PIV applications, image quality is less important, but exposure time is, this mode
/// reduces the gap between exposure end and start of the next exposure from ~75\f$\mu\f$S to 3.5\f$\mu\f$S.
/// \param wFastTimingMode Pointer to a WORD variable to receive the fast timing mode.
/// - 0x0000 = [OFF]
/// - 0x0001 = [ON]
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetFastTimingMode(WORD* wFastTimingMode);

///
/// \brief Set the camera fast timing mode for a dimax.
///
/// To increase the possible exposure time with high frame rates it is possible to enable the 'Fast Timing' mode. This means that the maximum possible
/// exposure time can be longer than in normal mode, while getting stronger offset drops. In case,
/// especially in PIV applications, image quality is less important, but exposure time is, this mode
/// reduces the gap between exposure end and start of the next exposure from ~75\f$\mu\f$S to 3.5\f$\mu\f$S.
/// \param wFastTimingMode WORD variable to set the fast timing mode.
/// - 0x0000 = [OFF]
/// - 0x0001 = [ON]
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetFastTimingMode(WORD wFastTimingMode);

DWORD PCO_GetHWLEDSignal(DWORD* dwParameter);
DWORD PCO_SetHWLEDSignal(DWORD dwParameter);

///
/// \brief Gets the ram and page size of the camera.
///
/// One page is the smallest unit for RAM segmentation as well as for storing images. Segment
/// sizes can only configured as multiples of pages. The size reserved for one image is also
/// calculated as multiples of whole pages. Therefore, there may be some unused RAM memory
/// if the page size is not exactly a multiple of the image size. The number of pages needed for
/// one image depends on the image size (Xres x Yres) divided by the pixels per page (page size).
/// Every page size that has been started must be considered, so if 50.6 pages are used for an
/// image 51 pages are actually needed for this image. With this value of 'pages per image',the
/// user can calculate the number of images fitting into the segment.
/// \param dwRamSize Pointer to a DWORD variable to receive the total camera RAM.
/// \param wPageSize Pointer to a DWORD variable to receive the pagesize as a multiple of pixels.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetCameraRamSize(DWORD* dwRamSize, WORD* wPageSize);

///
/// \brief Gets the segment sizes of the camera.
///
/// \b Note:
/// - the sum of all segment sizes must not be larger than the total size of the RAM (as multiples of pages)
/// - \b size = [0] indicates that the segment will not be used
/// - using only one segment is possible by assigning the total RAM size to segment 1 and 0x0000 to all other segments.
/// - The segment number is 1 based, while the array dwRamSegSize is zero based, e.g. ram size of segment 1 is stored in dwRamSegSize[0]!
///
/// \param dwRamSegSize Pointer to a DWORD array to receive the ramsegmentsizes in pages.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetCameraRamSegmentSize(DWORD* dwRamSegSize);

///
/// \brief Set Camera RAM Segment Size. The segment size has to be big enough to hold at least two images.
///
/// \b Note:
/// - the sum of all segment sizes must not be larger than the total size of the RAM (as multiples of pages)
/// - a single segment size can have the value 0x0000, but the sum of all four segments must be bigger than the size of two images.
/// - the command will be rejected, if Recording State is [run]
/// - The segment number is 1 based, while the array dwRamSegSize is zero based, e.g. ram size of segment 1 is stored in dwRamSegSize[0]!
/// - This function will result in \b all segments being \b cleared. All previously recorded images will be \b lost!}
///
/// \param dwRamSegSize
/// Pointer to a DWORD array to receive the ramsegmentsize in pages.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetCameraRamSegmentSize(DWORD* dwRamSegSize);

///
/// \brief Clear active camera RAM segment, delete all image info and prepare segment for new images.
///
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_ClearRamSegment();

///
/// \brief Get the active camera RAM segment.
///
/// The active segment is where images are stored.
///
/// \param wActSeg Pointer to a WORD variable to receive the actual segment. (1 - 4)
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetActiveRamSegment(WORD* wActSeg);

///
/// \brief Set the active camera RAM segment.
///
/// The active segment is where images are stored.
///
/// \param wActSeg WORD variable to hold the actual segment.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetActiveRamSegment(WORD wActSeg);

///
/// \brief Get storage mode [recorder] or [FIFO buffer].
///
/// \f[
/// \begin{tabular}{| p{7cm} | p{7cm} |}
/// \hline
/// \textbf{Recorder Mode} & \textbf{FIFO Buffer mode} \\ \hline
/// \begin{itemize}
/// \item images are recorded and stored within the internal camera memory camRAM
/// \item Live View transfers the most recent image to the PC (for viewing/monitoring)
/// \item indexed or total image readout after the recording has been stopped
/// \end{itemize}
/// &
/// \begin{itemize}
/// \item all images taken are transferred to the PC in chronological order
/// \item camera memory (camRAM) is used as a huge FIFO buffer to bypass short data transmission bottlenecks
/// \item if buffer overflows, the oldest images are overwritten
/// \end{itemize} \\ \hline
/// \end{tabular}
/// \f]
/// \param wStorageMode Pointer to a WORD variable to receive the storage mode.
/// - 0: Recorder
/// - 1: FIFO
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetStorageMode(WORD* wStorageMode);

///
/// \brief Set storage mode [recorder] or [FIFO buffer].
///
/// \f[
/// \begin{tabular}{| p{7cm} | p{7cm} |}
/// \hline
/// \textbf{Recorder Mode} & \textbf{FIFO Buffer mode} \\ \hline
/// \begin{itemize}
/// \item images are recorded and stored within the internal camera memory camRAM
/// \item Live View transfers the most recent image to the PC (for viewing/monitoring)
/// \item indexed or total image readout after the recording has been stopped
/// \end{itemize}
/// &
/// \begin{itemize}
/// \item all images taken are transferred to the PC in chronological order
/// \item camera memory (camRAM) is used as a huge FIFO buffer to bypass short data transmission bottlenecks
/// \item if buffer overflows, the oldest images are overwritten
/// \item if set recorder = [stop] is sent, recording is stopped and the transfer of the current image to the PC is finished. Images not read are stored within the segment and can be read with the ReadImageFromSegment command.
/// \end{itemize} \\ \hline
/// \end{tabular}
/// \f]
/// \param wStorageMode WORD variable to hold the storage mode.
/// - 0: Recorder
/// - 1: FIFO
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetStorageMode(WORD wStorageMode);

///
/// \brief Get recorder sub mode: [sequence] or [ring buffer] (see explanation boxes below).
///
/// Recorder submode is only available if the storage mode is set to [recorder].
/// \f[
/// \begin{tabular}{| p{7cm} | p{7cm} |}
/// \hline
/// \textbf{recorder sub mode = [sequence]} & \textbf{recorder sub mode = [ring buffer]} \\ \hline
/// \begin{itemize}
/// \item recording is stopped when the allocated buffer is full
/// \end{itemize}
/// &
/// \begin{itemize}
/// \item camera records continuously into ring buffer
/// \item if the allocated buffer overflows, the oldest images are overwritten
/// \item recording is stopped by software or disabling acquire signal (<acq enbl>)
/// \end{itemize} \\ \hline
/// \end{tabular}
/// \f]
/// \param wRecSubmode Pointer to a WORD variable to receive the recorder sub mode.
/// - 0: Sequence
/// - 1: Ring buffer
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetRecorderSubmode(WORD* wRecSubmode);

///
/// \brief Set recorder sub mode: [sequence] or [ring buffer] (see explanation boxes below).
///
/// Recorder sub mode is only available if the storage mode is set to [recorder].
/// \f[
/// \begin{tabular}{| p{7cm} | p{7cm} |}
/// \hline
/// \textbf{recorder sub mode = [sequence]} & \textbf{recorder sub mode = [ring buffer]} \\ \hline
/// \begin{itemize}
/// \item recording is stopped when the allocated buffer is full
/// \end{itemize}
/// &
/// \begin{itemize}
/// \item camera records continuously into ring buffer
/// \item if the allocated buffer overflows, the oldest images are overwritten
/// \item recording is stopped by software or disabling acquire signal (<acq enbl>)
/// \end{itemize} \\ \hline
/// \end{tabular}
/// \f]
/// \param wRecSubmode WORD variable to hold the recorder sub mode.
/// - 0: Sequence
/// - 1: Ring buffer
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetRecorderSubmode(WORD wRecSubmode);

///
/// \brief Get acquire mode: [auto] or [external] (see camera manual for explanation)
///
/// Acquire modes:
/// - 0x0000 = [auto] - all images taken are stored
/// - 0x0001 = [external] - the external control input \<acq enbl\> is a static enable signal of
/// images. If this input is TRUE (level depending on the DIP switch), exposure triggers are
/// accepted and images are taken. If this signal is set FALSE, all exposure triggers are
/// ignored and the sensor readout is stopped.
/// - 0x0002 = [external] - the external control input \<acq enbl\> is a dynamic frame start
/// signal. If this input has got a rising edge TRUE (level depending on the DIP switch), a
/// frame will be started with modulation mode. This is only available with modulation mode
/// enabled (see camera description).
///
/// \param wAcquMode Pointer to a WORD variable to receive the acquire mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetAcquireMode(WORD* wAcquMode);

///
/// \brief Set acquire mode: [auto] or [external] (see camera manual for explanation).
///
/// Acquire modes:
/// - 0x0000 = [auto] - all images taken are stored
/// - 0x0001 = [external] - the external control input \<acq enbl\> is a static enable signal of
/// images. If this input is TRUE (level depending on the DIP switch), exposure triggers are
/// accepted and images are taken. If this signal is set FALSE, all exposure triggers are
/// ignored and the sensor readout is stopped.
/// - 0x0002 = [external] - the external control input \<acq enbl\> is a dynamic frame start
/// signal. If this input has got a rising edge TRUE (level depending on the DIP switch), a
/// frame will be started with modulation mode. This is only available with modulation mode
/// enabled (see camera description).
///
/// \param wAcquMode WORD variable to hold the acquire mode.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetAcquireMode(WORD wAcquMode);

///
/// \brief Get the current status of the \<acq enbl\> user input (one of the \<control in\> inputs at the rear of pco.power or the camera). See camera manual for more information.
///
/// \b Note:
/// Due to response and processing times e.g. caused by the interface and/or the operating
/// system, the delay between the delivered status and the actual status may be several 10 ms up
/// to 100 ms. If timing is critical it is strongly recommended to use other trigger modes.
/// \param wAcquEnableState Pointer to a WORD variable to receive the acquire enable signal status.
/// - 0x0000 = [FALSE]
/// - 0x0001 = [TRUE]
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetAcqEnblSignalStatus(WORD* wAcquEnableState);

///
/// \brief This command can be used for setting up the record stop event.
///
/// After a stop event the camera records the configured number of images and stops after that. The command is useful to record a
/// series of images to see what happens before and after the stop event.
///
/// A record stop event can be either a software command or an edge at the \<acq enbl\> input (at the
/// power unit). The edge detection depends on the DIP switch setting at the power unit. If the DIP switch shows \f$\rfloor\lfloor\f$
/// then a rising edge is the stop event. If the DIP switch shows \f$\bigsqcup\f$ then a falling edge is the stop event.
///
/// The software command is the command "Stop Record" described below.
///
/// Use the record stop even function only when Storage Mode = [Recorder] and Recorder Sub mode = [Ring buffer]!
///
/// \b Note:
/// - Use the record stop event function only when Storage Mode = [Recorder] and Recorder Sub mode = [Ring buffer]!
/// - Due to internal timing issues the actual number of images taken after the event may differ by +/- 1 from the configured number.
/// - The command is not available for all cameras. It is currently only available on the pco.1200hs. See the descriptor for availability.
///
/// \param wRecordStopEventMode Pointer to a WORD variable to receive the record stop event mode.
///  - 0x0000 = no record stop event is accepted
///  - 0x0001 = record stop by software command
///  - 0x0002 = record stop by edge at the \<acq enbl\> input or by software
/// \param dwRecordStopDelayImages Pointer to a DWORD variable to receive the number of images which are taken after the record stop event. If the number of images is taken, record will be stopped automatically.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetRecordStopEvent(WORD* wRecordStopEventMode,DWORD* dwRecordStopDelayImages);

///
/// \brief This command can be used for setting up the record stop event.
///
/// After a stop event the camera records the configured number of images and stops after that. The command is useful to record a
/// series of images to see what happens before and after the stop event.
///
/// A record stop event can be either a software command or an edge at the \<acq enbl\> input (at the
/// power unit). The edge detection depends on the DIP switch setting at the power unit. If the DIP switch shows \f$\rfloor\lfloor\f$
/// then a rising edge is the stop event. If the DIP switch shows \f$\bigsqcup\f$ then a falling edge is the stop event.
///
/// The software command is the command PCO_StopRecord().
///
/// Use the record stop even function only when Storage Mode = [Recorder] and Recorder Sub mode = [Ring buffer]!
///
/// \b Note:
/// - Use the record stop event function only when Storage Mode = [Recorder] and Recorder Sub mode = [Ring buffer]!
/// - Due to internal timing issues the actual number of images taken after the event may differ by +/- 1 from the configured number.
/// - The command is not available for all cameras. It is currently only available on the pco.1200hs. See the descriptor for availability.
///
/// \param wRecordStopEventMode WORD variable to hold the record stop event mode.
///  - 0x0000 = no record stop event is accepted
///  - 0x0001 = record stop by software command
///  - 0x0002 = record stop by edge at the \<acq enbl\> input or by software
/// \param dwRecordStopDelayImages DWORD variable to hold the number of images which are taken after the record stop event. If the number of images is taken, record will be stopped automatically.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetRecordStopEvent(WORD wRecordStopEventMode,DWORD dwRecordStopDelayImages);

///
/// \brief This command is useful to generate a stop event by software for the record stop event mode. See also PCO_GetRecordStopEvent() and PCO_SetRecordStopEvent().
///
/// If you want to stop immediately please use PCO_SetRecordingState(0).
///
/// \b Note:
/// - Use the record stop event function only when Storage Mode = [Recorder] and Recorder Sub mode = [Ring buffer]!
/// - Due to internal timing issues the actual number of images taken after the event may differ by +/- 1 from the configured number.
/// - The command is not available for all cameras. It is currently only available on the pco.1200hs. See the descriptor for availability.
///
/// \param wReserved0 Pointer to a WORD variable (Set to zero!).
/// \param dwReserved1 Pointer to a DWORD variable (Set to zero!).
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_StopRecord(WORD* wReserved0, DWORD *dwReserved1);

DWORD PCO_GetEventMonConfiguration(WORD* wConfig);
DWORD PCO_SetEventMonConfiguration(WORD wConfig);
DWORD PCO_GetEventList(WORD wIndex,WORD *wMaxEvents,WORD* wValidEvents,WORD* wValidEventsInTelegram,SC2_EVENT_LIST_ENTRY* list);
DWORD	PCO_GetAcquireControl(WORD* wMode);
DWORD	PCO_SetAcquireControl(WORD wMode);

///
/// \brief Set acquire mode: [auto] or [external] (see camera manual for explanation).
///
/// Acquire modes:
/// - 0x0000 = [auto] - all images taken are stored
/// - 0x0001 = [external] - the external control input \<acq enbl\> is a static enable signal of
/// images. If this input is TRUE (level depending on the DIP switch), exposure triggers are
/// accepted and images are taken. If this signal is set FALSE, all exposure triggers are
/// ignored and the sensor readout is stopped.
/// - 0x0002 = [external] - the external control input \<acq enbl\> is a dynamic frame start
/// signal. If this input has got a rising edge TRUE (level depending on the DIP switch), a
/// frame will be started with modulation mode. This is only available with modulation mode
/// enabled (see camera description).
///
/// \param wAcquMode Pointer to a WORD variable to receive the acquire mode.
/// \param dwNumberImages Pointer to a DWORD variable to receive the number of images (for mode sequence).
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetAcquireModeEx(WORD* wAcquMode, DWORD* dwNumberImages);

///
/// \brief Get acquire mode: [auto] or [external] (see camera manual for explanation)
///
/// Acquire modes:
/// - 0x0000 = [auto] - all images taken are stored
/// - 0x0001 = [external] - the external control input \<acq enbl\> is a static enable signal of
/// images. If this input is TRUE (level depending on the DIP switch), exposure triggers are
/// accepted and images are taken. If this signal is set FALSE, all exposure triggers are
/// ignored and the sensor readout is stopped.
/// - 0x0002 = [external] - the external control input \<acq enbl\> is a dynamic frame start
/// signal. If this input has got a rising edge TRUE (level depending on the DIP switch), a
/// frame will be started with modulation mode. This is only available with modulation mode
/// enabled (see camera description).
///
/// \param wAcquMode WORD variable to hold the acquire mode.
/// \param dwNumberImages DWORD variable to hold the number of images (for mode sequence).
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetAcquireModeEx(WORD wAcquMode, DWORD dwNumberImages);

///
/// \brief Get the image settings for images stored into one of the four segments. This function is not applicable with cameras without internal recorder memory.
///
/// Gets the sizes information for one segment.
///
/// X0, Y0 start at 1. X1, Y1 end with max. sensor size.
///
/// \latexonly \begin{tabular}{| l c r | } \hline x0,y0 & & \\ & ROI & \\ & & x1,y1 \\ \hline \end{tabular} \endlatexonly
///
/// \param wSegment WORD variable that holds the segment to query.
/// \param wRes_hor Pointer to a WORD variable to receive the x resolution of the image in segment
/// \param wRes_ver Pointer to a WORD variable to receive the y resolution of the image in segment
/// \param wBin_x Pointer to a WORD variable to receive the horizontal binning of the image in segment
/// \param wBin_y Pointer to a WORD variable to receive the vertical binning of the image in segment
/// \param wRoi_x0 Pointer to a WORD variable to receive the left x offset of the image in segment
/// \param wRoi_y0 Pointer to a WORD variable to receive the upper y offset of the image in segment
/// \param wRoi_x1 Pointer to a WORD variable to receive the right x offset of the image in segment
/// \param wRoi_y1 Pointer to a WORD variable to receive the lower y offset of the image in segment
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetSegmentImageSettings(WORD wSegment,WORD* wRes_hor,WORD* wRes_ver,WORD* wBin_x,WORD* wBin_y,WORD* wRoi_x0,WORD* wRoi_y0,WORD* wRoi_x1,WORD* wRoi_y1);

///
/// \brief Get the number of valid images within the segment.
///
/// This function is not applicable with cameras
///without internal recorder memory. The operation is slightly different due to the selected storage mode:
///
/// In [recorder mode], if recording is not stopped and in [FIFO buffer mode] the number of images is
/// dynamic due to read and write accesses to the camera RAM. If the \b camera \b storage \b mode is in
/// [recorder mode] and recording is stopped, the number is fixed.
///
/// In [FIFO buffer] mode the ratio of valid number of images to the maximum number of images is some sort of filling indicator.
/// \param wSegment WORD variable that holds the segment to query.
/// \param dwValid Pointer to a DWORD varibale to receive the valid image count.
/// \param dwMax Pointer to a DWORD varibale to receive the max image count which may be saved to this segment.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetNumberOfImagesInSegment(WORD wSegment,DWORD* dwValid,DWORD* dwMax);

///
/// \brief  Reads the specified images from segment.
///
/// \param wSegment WORD variable that holds the segment to query.
/// \param dwStartImage DWORD variable that holds the first image to receive.
/// \param dwLastImage DWORD variable that holds the last image to recieve.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_ReadImagesFromSegment(WORD wSegment,DWORD dwStartImage,DWORD dwLastImage);

///
/// \brief Cancels the image processing.
///
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_CancelImageTransfer();

///
/// \brief Repeats the last image.
///
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_RepeatImage();

///
/// \brief Plays the images recorded to the camera RAM.
///
/// The command is especially for HD-SDI interface
/// (output only interfaces). This interface does not request images, but it has to be supplied with a continuous data stream.
///
/// \b Note: Command is only valid, if \b storage \b mode is set to [recorder] and recording to the camera RAM segment is stopped!
///
/// The play speed is defined by the Speed parameter together with the Mode parameter:
///
/// - Fast forward: The play position is increased by [Speed], i.e. [Speed - 1] images are leaped.
/// - Fast rewind: The play position is decreased by [Speed], i.e. [Speed - 1] images are leaped.
/// - Slow forward: The current image is sent [Speed] times before the position is increased
/// - Slow rewind: The current image is sent [Speed] times before the position is decreased
///
/// The play command can also be sent to change parameters (e.g. speed) while a play is active. The
/// new parameters will be changed immediately. It is possible to change parameters like play speed
/// or play direction without changing the current position by setting Start No. to -1 or
/// 0xFFFFFFFFH in the DWORD format.
///
/// \param wSegment WORD variable with the segment to read from
/// \param wInterface WORD variable to set the interface (0x0001 for HDSDI)
/// \param wMode WORD variable to set the play mode
///  - 0: Stop play,
///  - 1: Fast forward (step 'wSpeed' images),
///  - 2: Fast rewind (step 'wSpeed' images),
///  - 3: Slow forward (show each image 'wSpeed'-times)
///  - 4: Slow rewind (show each image 'wSpeed'-times)
///  - Additional flags: 0x0100-> - 0: Repeat last image
///                               - 1: Repeat sequence
/// \param wSpeed WORD variable to set the stepping or repeat count
/// \param dwRangeLow Lowest image number to be played
/// \param dwRangeHigh Highest image number to be played
/// \param dwStartPos Set position to image number #, -1: unchanged
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_PlayImagesFromSegment(WORD wSegment,WORD wInterface,WORD wMode,WORD wSpeed,DWORD dwRangeLow,DWORD dwRangeHigh,DWORD dwStartPos);

///
/// \brief The "Get Play Position" command requests at which position the play pointer of the currently started sequence is.
///
/// When the command "Play Images from Segment" was called, the sequence is started and the
/// response message is sent immediately, whereas it may take seconds or up to minutes, until the sequence transmission is finished.
///
/// \b Note: Due to time necessary for communication and processing the command, the actual pointer may be 1 or 2 images ahead at the time, when the response is sent completely.
///
/// \param wStatus WORD variable to get the status of image play state machine.
///  - 0: no play is active, or play has already stopped
///  - 1: play is active, position is valid
/// \param dwPosition: Number of the image currently sent to the interface. It is between Range Low and Range High, as set by "Play Images from Segment". Only valid when sequence play is still active.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetPlayPosition(WORD* wStatus,DWORD *dwPosition);

DWORD PCO_SetVideoPayloadIdentifier(WORD wSegment,WORD wMode1,WORD wMode2,DWORD dwSetPos1,DWORD dwClrPos1,DWORD dwSetPos2,DWORD dwClrPos2,DWORD dwSetPos3,DWORD dwClrPos3,DWORD dwSetPos4,DWORD dwClrPos4);

///
/// \brief Gets the metadata mode
///
/// his command is optional and depends on the hardware and firmware. Check the availability
/// according to the camera descriptor (METADATA). Gets the mode for meta data. See
/// PCO_GetMetaData() (dimax only) for more information.
///
/// When wMode is set to 1, the user is responsible to add further
/// line(s) to the buffers, where the number of lines depends on x-resolution and needed wMetaDataSize.
///
/// \param wMode Pointer to a WORD variable receiving the meta data mode.
///  - 0x0000: [OFF]
///  - 0x0001: [ON]
/// \param wMetadataSize Pointer to a WORD variable receiving the meta data block size in additional pixels.
/// \param wMetadataVersion Pointer to a WORD variable receiving the meta data version information.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetMetadataMode(WORD* wMode,WORD* wMetadataSize,WORD* wMetadataVersion);

///
/// \brief Sets the meta data mode
///
/// This command is optional and depends on the hardware and firmware. Check the availability
/// according to the camera descriptor (METADATA). Sets the mode for meta data. See
/// PCO_GetMetaData() (dimax only) for more information.
/// When wMetaDataMode is set to 1, the user is responsible to add further
/// line(s) to the buffers, where the number of lines depends on x-resolution and needed wMetaDataSize.
///
/// This option is only available with pco.dimax
///
/// \param wMode WORD variable to set the meta data mode.
///  - 0x0000: [OFF]
///  - 0x0001: [ON]
/// \param wMetadataSize Pointer to a WORD variable receiving the meta data block size in additional pixels.
/// \param wMetadataVersion Pointer to a WORD variable receiving the meta data version information.
/// \return
///
DWORD PCO_SetMetadataMode(WORD wMode,WORD* wMetadataSize,WORD* wMetadataVersion);

///
/// \brief Gets the color convert settings inside the camera.
///
/// This option is only available with pco.dimax and HD/SDI interface.
///
/// \param ColSetResp Pointer to a SC2_Set_Color_Settings structure to receive the color set data.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetColorSettings(SC2_Get_Color_Settings_Response* ColSetResp);

///
/// \brief Sets the color convert settings inside the camera.
///
/// This option is only available with pco.dimax and HD/SDI interface.
///
/// \param SetColSet Pointer to a SC2_Set_Color_Settings structure to set the color set data.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetColorSettings(SC2_Set_Color_Settings* SetColSet);

///
/// \brief  Starts a white balancing calculation.
///
/// This option is only available with pco.dimax and HD/SDI interface.
///
/// \param wMode WORD variable to set the meta data mode. Set to 1.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_DoWhiteBalance(WORD wMode);

///
/// \brief Gets the white balancing status.
///
/// This option is only available with pco.dimax and HD/SDI interface.
///
/// \param wStatus Pointer to WORD variable to receive the status.
/// \param wColorTemp Pointer to WORD variable to recieve the color temperature.
/// \param sTint Pointer to SHORT variable to recieve the tint.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetWhiteBalanceStatus(WORD* wStatus,WORD* wColorTemp,SHORT* sTint);

///
/// \brief Gets the IEEE1394 interface parameters.
///
/// Gets the FireWire transfer parameters.
///
/// \param wMasterNode Pointer to WORD variable to receive the master node address.
/// \param wIsochChannel Pointer to WORD variable to recieve the used ISO channel.
/// \param wIsochPacketLen Pointer to WORD variable to receive the ISO packet length.
/// \param wIsochPacketNum Pointer to WORD variable to receive the ISO packet count.
/// \return Error message, 0 in case of success else less than 0.
/// \see PCO_SetIEEE1394InterfaceParams
DWORD PCO_GetIEEE1394InterfaceParams(WORD* wMasterNode,WORD* wIsochChannel,WORD* wIsochPacketLen,WORD* wIsochPacketNum);

///
/// \brief PCO_SetIEEE1394InterfaceParams Sets the IEEE1394 interface parameters.
///
/// The user can instantiate a structure _PCO1394_ISO_PARAMS, which is defined in SC2_SDKAddendum.h.
/// - bandwidth_bytes: set to a bandwidth size which is a fraction of 4096 / (num of cameras). e.g. 2
/// - cameras connected: bandwidth_bytes = 2048.
/// - speed_of_isotransfer: 1,2,4, whereas 1 is 100MBit/s, 2=200 and 4=400; default is 4.
/// - number_of_isochannel: Channel numbers are 32-bit encoded and the highest bit equals the lowest channel. (e.g. 0x80000000 = channel 0).
/// - number_of_isobuffers: 16...256; default is 128
/// - byte_per_isoframe: set to the same value as bandwidth_bytes.
///
/// Remarks for number_of_isochannel: Usually it is not necessary to change this parameter (Open_Grabber() does it automatically), but in
/// case the user wants to transfer images from more than one camera, the iso channel must be unique
/// for each camera.
/// Only one bit may be set at a time.
///
/// In case the user wants to establish a connection, this has to be the first command sent or the camera won't know how to respond to commands.
///
/// \param wMasterNode WORD variable to set the master node address.
/// \param wIsochChannel WORD variable to set the ISO channel.
/// \param wIsochPacketLen WORD variable to set the ISO packet length.
/// \param wIsochPacketNum WORD variable to set the ISO packet count.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetIEEE1394InterfaceParams(WORD wMasterNode,WORD wIsochChannel,WORD wIsochPacketLen,WORD wIsochPacketNum);

///
/// \brief Gets the IEEE1394 byte order.
///
/// \param wMode Pointer to WORD variable to receive the IEEE1394 byte order.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetIEEE1394ISOByteorder(WORD* wMode);

///
/// \brief Sets the IEEE1394 byte order.
///
/// \param wMode WORD variable to set the IEEE1394 byte order.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetIEEE1394ISOByteorder(WORD wMode);

///
/// \brief Gets the actual interface output format.
///
/// This is only valid with a dimax with a built in HD/SDI
/// interface. This command can be used to determine the image streaming interface, which is active.
/// If the addressed interface is set to [off], then the standard interface, e.g. GigE or USB, is used to
/// stream the data. If the addressed interface is activated, the standard interface is only for camera
/// control, thus streaming to this interface is disabled.
///
/// \param wInterface WORD variable to get the desired interface.
///   - 0: reserved
///   - 1: HD/SDI
///   - 2: DVI
/// \param wFormat Pointer to WORD variable to get the interface format
///   - 0: Output is disabled
///   - 1: HD/SDI, 1080p25, RGB
///   - 2: HD/SDI, 1080p25, arbitrary raw mode
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetInterfaceOutputFormat(WORD wInterface,WORD* wFormat);

///
/// \brief Sets the actual interface output format.
///
/// This is only valid with a dimax with a built in HD/SDI
/// interface. This command can be used to set the image streaming interface, which is active. If the
/// addressed interface is set to [off], then the standard interface, e.g. GigE or USB, is used to stream
/// the data. If the addressed interface is activated, the standard interface is only for camera control,
/// thus streaming to this interface is disabled.
///
/// \param wInterface WORD variable to set the desired interface.
///   - 0: reserved
///   - 1: HD/SDI
///   - 4: DVI
/// \param wFormat WORD variable to set the interface format
///   - 0: Output is disabled
///   - 1: HD/SDI, 1080p25, RGB
///   - 2: HD/SDI, 1080p25, arbitrary raw mode
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetInterfaceOutputFormat(WORD wInterface,WORD wFormat);

///
/// \brief Gets the image transfer mode
///
/// \param wMode Pointer to WORD variable to receive the image mode. (e.g. full, scaled, cutout etc.)
/// \param wImageWidth Pointer to WORD variable to receive the original image width
/// \param wImageHeight Pointer to WORD variable to receive the original image height
/// \param wTxWidth Pointer to WORD variable to receive the transferred image width
/// \param wTxHeight Pointer to WORD variable to receive the transferred image height
/// \param wTxLineWordCnt Meaning depends on selected mode
/// \param wParam Meaning depends on selected mode
/// \param wParamLen Pointer to WORD variable to receive wParam length.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetImageTransferMode(WORD* wMode,WORD* wImageWidth,WORD* wImageHeight,WORD* wTxWidth,WORD* wTxHeight,WORD* wTxLineWordCnt,WORD* wParam,WORD* wParamLen);

///
/// \brief Sets the image transfer mode
///
/// \param wMode WORD variable to set the image mode. (e.g. full, scaled, cutout etc.)
/// \param wImageWidth WORD variable to set the original image width
/// \param wImageHeight WORD variable to set the original image height
/// \param wTxWidth WORD variable to set the scaled/cutout image width
/// \param wTxHeight WORD variable to set the scaled/cutout image height
/// \param wTxLineWordCnt Meaning depends on selected mode
/// \param wParam Meaning depends on selected mode
/// \param wParamLen WORD variable to hold the wParam length.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_SetImageTransferMode(WORD wMode,WORD wImageWidth,WORD wImageHeight,WORD wTxWidth,WORD wTxHeight,WORD wTxLineWordCnt,WORD* wParam,WORD wParamLen);

///
/// \brief Get interface status messages.
///
/// \param wInterface WORD variable holding the interface to be queried.
/// \param dwWarnings Pointer to WORD variable to receive the warnings.
/// \param dwErrors Pointer to WORD variable to receive the errors.
/// \param dwStatus Pointer to WORD variable to receive the status.
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_GetInterfaceStatus(WORD wInterface,DWORD* dwWarnings,DWORD* dwErrors,DWORD* dwStatus);

DWORD PCO_GetFlimModulationParams(WORD* wSourceSelect,WORD* wOutputWaveform);
DWORD PCO_SetFlimModulationParams(WORD wSourceSelect,WORD wOutputWaveform);
DWORD PCO_GetFlimPhaseSequenceParams(WORD* wPhaseNumber,WORD* wPhaseSymmetry,WORD* wPhaseOrder,WORD* wTapSelect);
DWORD PCO_SetFlimPhaseSequenceParams(WORD wPhaseNumber,WORD wPhaseSymmetry,WORD wPhaseOrder,WORD wTapSelect);
DWORD PCO_GetFlimImageProcessingFlow(WORD* wAsymmetryCorrection);
DWORD PCO_SetFlimImageProcessingFlow(WORD wAsymmetryCorrection);
DWORD PCO_GetFlimMasterModulationFrequency(DWORD* dwFrequency);
DWORD PCO_SetFlimMasterModulationFrequency(DWORD dwFrequency);
DWORD PCO_GetFlimRelativePhase(DWORD* dwPhaseMilliDeg);
DWORD PCO_SetFlimRelativePhase(DWORD dwPhaseMilliDeg);

// implementation dependent on actual class 
virtual DWORD PCO_GetTransferParameter(void* buf ATTRIBUTE_UNUSED,int length ATTRIBUTE_UNUSED){ return PCO_NOERROR;}
virtual DWORD PCO_SetTransferParameter(void* buf ATTRIBUTE_UNUSED,int length ATTRIBUTE_UNUSED){ return PCO_NOERROR;}

///
/// \brief PCO_CancelImage
///
/// Cancels the image transfer.
///
/// \return Error message, 0 in case of success else less than 0.
///
DWORD PCO_CancelImage();

#endif
