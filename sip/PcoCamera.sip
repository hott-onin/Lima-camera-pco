namespace Pco
{
  class Camera
  {
%TypeHeaderCode
#include "lima/HwInterface.h"
#include <PcoCamera.h>
%End
  public:

        Camera(const char*);
        ~Camera();


        void 	startAcq();
        void	reset(int);

        const char *talk(const char *cmd);
        //char *talk(char *cmd);       

        int getNbAcquiredFrames();

        void getMaxWidthHeight(unsigned int& width,unsigned int& height /Out/);


//--------------------------------------------------------

        void getBytesPerPixel(unsigned int& pixbytes /Out/); // bytesPerPixel

		void getAcqTimeoutRetry(int &val /Out/);   // acqTimeoutRetry
		void setAcqTimeoutRetry(int val);

        void getAdc(int & adc /Out/);    // adc
        void setAdc(int val);

        void getAdcMax(int & adc /Out/);    // adcMax

		void getClTransferParam(std::string &o_sn /Out/) ;    // clXferPar
		
		void getLastError(std::string &o_sn /Out/) ;   // last error
		
		void getTraceAcq(std::string &o_sn /Out/) ;   // traceAcq
		

		void getLastImgRecorded(unsigned long & img /Out/);   // lastImgRecorded
		
		void getLastImgAcquired(unsigned long & img /Out/);   // lastImgAcquired
		
		void getMaxNbImages(unsigned long & nr /Out/);     // maxNbImages
		

		void getCocRunTime(double & coc /Out/);    // CocRunTime
		
		void getFrameRate(double & frameRate /Out/);  // frameRate

		void getCamInfo(std::string &o_sn /Out/) ;    // camInfo
		
		void getCamType(std::string &o_sn /Out/) ;   // camType
		
		void getVersion(std::string &o_sn /Out/) ;   // version


		void getPcoLogsEnabled(int & enabled /Out/);      // pcoLogsEnabled

		void getPixelRate(int & val /Out/);   // pixelRate
		void setPixelRate(int val);

		void getPixelRateValidValues(std::string &o_sn /Out/) ;  // pixelRateValidValues
		
		void getPixelRateInfo(std::string &o_sn /Out/) ;       // pixelRateInfo

		void getRollingShutter(int & val /Out/);    // rollingShutter
		void setRollingShutter(int val);

		void getRollingShutterInfo(std::string &o_sn /Out/) ;       // rollingShutterInfo

		void getLastFixedRoi(std::string &o_sn /Out/) ;   // lastFixedRoi

		void getCDIMode(int & val /Out/);     // cdiMode
		void setCDIMode(int val);

		void getTemperatureInfo(std::string &o_sn /Out/);
		void getCoolingTemperature(int & val /Out/);
		void setCoolingTemperature(int val);
		
		void getSdkRelease(std::string &o_sn /Out/) ;
		void getCameraName(std::string &o_sn /Out/);
		void getCameraNameBase(std::string &o_sn /Out/);
		void getCameraNameEx(std::string &o_sn /Out/) ;


  };
};


