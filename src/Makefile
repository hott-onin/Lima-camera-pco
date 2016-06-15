############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2011
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
.PHONY: src test clean all

#PCODIR     = ../sdkDev/pco_camera/pco_common/pco_include
#PCOLIBDIR  = ../sdkDev/pco_camera/pco_common/pco_lib
#CPCODIR    = ../sdkDev/pco_camera/pco_common/pco_classes
#CPCODIRME4 = ../sdkDev/pco_camera/pco_me4/pco_classes

PCOINCDIR     = ../sdkDevW/include
PCOLIBDIR  = ../sdkDevW/pco_lib
PCO_LIBSDK_DIR  = ../sdkDevW/pco_libsdk
PCO_LIBBASE_DIR  = ../sdkDevW/pco_lib_base
PCO_CLASSME4_DIR  = ../sdkDevW/pco_me4_classes
PCO_CLASS_DIR  = ../sdkDevW/pco_classes
CPCOINCDIR    = ../sdkDevW/include
CPCODIRME4 = ../pco_me4_classes

SISOLIBDIR = $(SISODIR5)/lib64
SISOINCLUDEDIR = $(SISODIR5)/include

DISPLIB = $(PCOLIBDIR)/libpcodisp.a
LOGLIB  = $(PCOLIBDIR)/libpcolog.a
FILELIB = $(PCOLIBDIR)/libpcofile.a

CAMLIB  = $(PCOLIBDIR)/libpcocam_me4.a

LIBSPCO = $(CAMLIB) $(FILELIB) 

LIBS  = -lfglib5 -lclsersis -lhaprt -lpthread 

LIBADD  = -lfglib5 -lclsersis -lhaprt -lpthread 
#LIBADD  += -lclsersis -lhaprt -lpthread 

LIBADD  += -lpcocam_me4 -lpcofile -lpcodisp

LFLAGS += -L$(SISOLIBDIR) -L$(PCOLIBDIR)

LIBSDK  = $(PCO_LIBSDK_DIR)/libreorderfunc.a $(PCO_LIBSDK_DIR)/libpcocnv.a

pco-objs :=  \
            PcoHwEventCtrlObj.o  \
            PcoDetInfoCtrlObj.o  \
            PcoInterface.o  \
            PcoRoiCtrlObj.o  \
            PcoSyncCtrlObj.o  \
            PcoCameraUtils.o  \
            PcoCamera.o  \
            PcoBufferCtrlObj.o  \
            PcoCameraSdk.o  \
            $(PCO_LIBBASE_DIR)/Cpco_log.o  \
            $(PCO_LIBBASE_DIR)/file12.o  \
            $(PCO_CLASSME4_DIR)/Cpco_grab_cl_me4.o  \
            $(PCO_CLASSME4_DIR)/Cpco_com_cl_me4.o  \
            $(PCO_CLASS_DIR)/Cpco_com_func.o  \
            $(PCO_CLASS_DIR)/Cpco_com.o  \
            $(PCO_CLASS_DIR)/Cpco_com_func_2.o  \
            
            #$(PCO_CLASSME4_DIR)/Cpco_grab_cl_me4_GL.o  \

SRCS = $(pco-objs:.o=.cpp) 

INCLUDES  = -I../include \
            -I../../../hardware/include \
            -I../../../common/include \
            -I$(PCOINCDIR) -I$(CPCOINCDIR) -I$(CPCODIRME4) -I$(SISOINCLUDEDIR) \
            -I../sdkDev/PCO_me4/siso_include \
            -I../../../third-party/Processlib/core/include \
            -I../sdkPco/include \

CXXFLAGS += $(INCLUDES) -fPIC -Wall -pthread -g



src:	all

all:	Pco.o pco_test


Pco.o:	$(pco-objs)
	$(LD) -d -o $@ -r $+ $(LIBSDK)
	#$(LD) -d -o $@ -r $+ $(LIBSPCO)
	#$(LD) -d -o $@ -r $+ $(LFLAGS) $(LIBADD)

clean:
	rm -f *.o *.P $(pco-objs)
	#$(MAKE) -C ../sdkDev/pco_camera/pco_common/pco_lib_base clean
	#$(MAKE) -C ../sdkDev/ -C pco_camera/pco_me4/pco_classes clean

%.o : %.cpp
	$(COMPILE.cpp) -MD $(CXXFLAGS) -o $@ $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d
	

pco_%:		pco_%.cpp $(CAMLIB) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $@.cpp $(LIBSPCO) $(LOGLIB)  $(LFLAGS) $(LIBADD) -o $@


-include $(SRCS:.cpp=.P)
