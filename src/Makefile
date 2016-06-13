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

PCODIR     = ../sdkDev/pco_camera/pco_common/pco_include
PCOLIBDIR  = ../sdkDev/pco_camera/pco_common/pco_lib
CPCODIR    = ../sdkDev/pco_camera/pco_common/pco_classes
CPCODIRME4 = ../sdkDev/pco_camera/pco_me4/pco_classes
SISOLIBDIR = $(SISODIR5)/lib64
SISOINCLUDEDIR = $(SISODIR5)/include

DISPLIB = $(PCOLIBDIR)/libpcodisp.a
LOGLIB  = $(PCOLIBDIR)/libpcolog.a
FILELIB = $(PCOLIBDIR)/libpcofile.a

CAMLIB  = $(PCOLIBDIR)/libpcocam_me4.a

LIBSPCO = $(CAMLIB) $(FILELIB) 

LIBADD  = -lfglib5 -lclsersis -lhaprt -lpthread 
LFLAGS += -L$(SISOLIBDIR)

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


SRCS = $(pco-objs:.o=.cpp) 

INCLUDES  = -I../include \
            -I../../../hardware/include \
            -I../../../common/include \
            -I$(PCODIR) -I$(CPCODIR) -I$(CPCODIRME4) -I$(SISOINCLUDEDIR) \
            -I../sdkDev/PCO_me4/siso_include \
            -I../../../third-party/Processlib/core/include \
            -I../sdkPco/include \

CXXFLAGS += $(INCLUDES) -fPIC -Wall -pthread -g


all:	libs Pco.o pco_test

libs:
	$(MAKE) -C ../sdkDev/pco_camera/pco_common/pco_lib_base
	$(MAKE) -C ../sdkDev/ -C pco_camera/pco_me4/pco_classes


Pco.o:	$(pco-objs)
	$(LD) -o $@ -r $+

clean:
	rm -f *.o *.P
	$(MAKE) -C ../sdkDev/pco_camera/pco_common/pco_lib_base clean
	$(MAKE) -C ../sdkDev/ -C pco_camera/pco_me4/pco_classes clean

%.o : %.cpp
	$(COMPILE.cpp) -MD $(CXXFLAGS) -o $@ $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d
	

pco_%:		pco_%.cpp $(CAMLIB) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $@.cpp $(LIBSPCO) $(LOGLIB)  $(LFLAGS) $(LIBADD) -o $@


-include $(SRCS:.cpp=.P)
