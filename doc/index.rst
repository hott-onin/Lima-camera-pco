.. _camera-pco:

PCO camera
----------

.. image:: pco-dimax-255x255.jpg
.. image:: pco-edge.jpg
.. image:: pco-2000-255x141.jpg


.. =======================================================================================
.. =======================================================================================

Intoduction
```````````

.. =======================================================================================
.. =======================================================================================

- **PCO camera systems** 

 - PCO develops specialized fast and sensitive video camera systems, mainly for scientific applications; 
   which covers digital camera systems with high dynamic range, high resolution, high speed and low noise. 
   `PCO home page <http://www.pco.de/>`_
 
- **Product overview and technical data of the PCO cameras supported in LIMA**
 
 - **PCO.dimax:** 
   High speed 12 bit CMOS camera with fast image rates of 1469 frames per second (fps) at full resolution of 1920 x 1080 pixel. 
   `(tech data pcodimax) <http://www.pco.de/categories/high-speed-cameras/pcodimax-hd/>`_
 
 - **PCO.edge:**
   Extremely low noise sCMOS camera with fast frame rates (100 fps), wide dynamic range (1:27000), high quantum efficiency, 
   high resolution (2560 x 2160) and large field of view.
   `(tech data pcoedge) <http://www.pco.de/categories/scmos-cameras/pcoedge-42/>`_
 
 - **PCO.2000:**
   High resolution (2048 x 2048 pixel) and low noise 14bit CCD cooled camera system with internal image memory (camRAM), 
   allows fast image recording with 160 MB/s. The available exposure times range from 500 ns to 49 days.
   `(tech data pco2000) <http://www.pco.de/categories/sensitive-cameras/pco2000/>`_
 
 - **PCO.4000:**
   High resolution (4008 x 2672 pixel) and low noise 14bit CCD cooled camera system with internal image memory (camRAM),
   allows fast image recording with 128 MB/s. The available exposure times range from 5 us to 49 days.
   `(tech data) <http://www.pco.de/categories/sensitive-cameras/pco4000/>`_

- **Interface buses**

 - **Cameralink:** used by **PCO.dimax** and **PCO.edge**
 - **Cameralink HS:** used by **PCO.edge**
 - **USB3.0:** used by **PCO.edge**
 - **GigE:** used by **PCO.2000** and **PCO.4000**
 

- **Type of applications**

 - Mainly used in scientific applications.

- **OS supported**

 - **Win7 Professional** (english) 64 bits SP1.




.. =======================================================================================
.. =======================================================================================

Prerequisites
`````````````

.. =======================================================================================
.. =======================================================================================

- **Required software packages** 

 - **download links**

  - `PCO and Silicon Software download (login/pw required) <ftp://pcoag.biz/>`_

  - `VC++ download <http://www.microsoft.com/visualstudio/en-us/products/2008-editions/express>`_

  - `GSL download <http://sourceforge.net/projects/gnuwin32/files/gsl/1.8/gsl-1.8.exe/download>`_

  - `python download <http://www.python.org/download/releases/2.6.6/>`_

  - `numpy download <http://sourceforge.net/projects/numpy/files/NumPy/1.5.1/>`_

  - `PyQt download <http://www.riverbankcomputing.co.uk/software/pyqt/download>`_

  - `PyTango download <http://www.tango-controls.org/download>`_

  - `GIT download <http://code.google.com/p/msysgit/downloads/list>`_

 
 - **md5 checksum and size of packges used (maybe not updated)**

.. code-block:: sh

 Silicon Software Runtime 5.4.4
     f8317c5145bac803f142c51b7c54ba27  RuntimeSetup_with_Applets_v5.4.4_Win64.exe

.. code-block:: sh

 pco-sdk 1.20
     eb73ab0495a66c068c408344e20c8ad9  read_me.txt
     69a8f5667b71a8cf206d782e20f526ab  SW_PCOSDKWIN_120.exe

.. code-block:: sh

 CAMWARE v403_1 
    a9f8b2e465b7702ff727ba349ef327e8     SW_CAMWAREWIN64_403_1.exe
 
.. code-block:: sh

 VC++ compiler 
    Microsoft Visual Studio 2008
    Version 9.0.30729.1 SP
    Microsoft .NET Framework
    Version 3.5 SP1

    Installed Edition: Professional
    Microsoft Visual C++ 2008   91605-270-4441125-60040
    Microsoft Visual C++ 2008

.. code-block:: sh

 Python 
     8d10ff41492919ae93a989aba4963d14  numpy-MKL-1.8.1.win-amd64-py2.7.exe
     5a38820953d38db6210a90e58f85548d  PyTango-8.0.4.win-amd64-py2.7.exe
     b73f8753c76924bc7b75afaa6d304645  python-2.7.6.amd64.msi

.. code-block:: sh

 pco edge CLHS / for firmware upgrade to 1.19
     9790828ce5265bab8b89585d8b8e83a9  pco.programmer_edgeHS.exe
     b9266e03a04ac9a9ff835311f0e27d94  pco_clhs_info.exe
     7e2f767684fb4ffaf5a5fac1af0c7679  sc2_clhs.dll
     2ed778785489846fd141f968dca3735b  README.txt
     6bdb7a27b0d7738762c878a33983dada  /FW_pco.edge_CLHS_020_V01_19.ehs

.. code-block:: sh

 UTILS
    38ba677d295b4b6c17368bb86b661103  FileZilla_3.22.1_win64-setup_bundled.exe
    0377ccd0a3283617d161f24d080fb105  Git-1.9.0-preview20140217.exe
    3cbd2488210b6e7b3e7fa1baf05022d4  MobaXterm_Setup_7.1.msi
 
- **Enviroment variables** 

 - **system variables** 
 
.. code-block:: sh

 ===> add manually the python path (it is not set by the installation program)
      PATH -> C:\Python26;

 ===> used for some utility batch files
      PATH -> C:\blissadm\bat;

..

 - **user variables** 

.. code-block:: sh

    TANGO_HOST -> <host>:20000






Module configuration
````````````````````
Configuration file **Lima/config.inc**

.. code-block:: sh

 ===> set these values to 1
      COMPILE_CORE=1
      COMPILE_PCO=1

See :ref:`Compilation`


Post installation actions
`````````````````````````
- **enable/disable PCO logs** 

.. code-block:: sh

 ===> rename file extensions (C:\ProgramData\pco): 
      .txt (disabled) / .log (enabled) ----+  
                                   camware.log   <---- created by hand
                                  PCO_CDlg.log
                                  PCO_Conv.log
                                   SC2_Cam.log




- **Command prompt console (Visual Studio)** 

.. code-block:: sh

  > All Programs
    > Microsoft Visual C++ 2008 Express Edition
      > Visual Studio Tools
        > Visual Studio 2008 Command Prompt
        

- **TODO**

- After installing PCO modules :ref:`installation`

- And probably Tango server :ref:`tango_installation`



Configuration
``````````````

- **TODO**


.. _pco-esrf-pc:


PCO EDGE notes
``````````````

.. toctree::
        :maxdepth: 2

        pco_edge
