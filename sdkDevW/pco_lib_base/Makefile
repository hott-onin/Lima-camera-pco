#Makefile for pco_lib_base libraries

VERSION = 1
SUBVERSION = 01
SUBAGE = 07

PCO_INC_DIR = ../include
LIBDIR = /usr/lib/
INCLUDEDIR = /usr/include
INTHDIR = $(PCO_INC_DIR)
EXTHDIR = $(PCO_INC_DIR)

PCOLIBDIR = ../pco_lib

HEADERS = ./

CFLAGS := -O2 -Wall -DLINUX -fPIC -I$(INCLUDEDIR) -I$(INTHDIR) -I$(EXTHDIR) 

LOGOBJ = Cpco_log.o
FILEOBJ = file12.o

REORDER_SRC = pixelfly_decode.cpp 
REORDER_OBJ = $(REORDER_SRC:.cpp=.o)


LOGLIB = $(PCOLIBDIR)/libpcolog.a
#REORDER_LIB = $(PCOLIBDIR)/libreorderfunc.a
FILELIB = $(PCOLIBDIR)/libpcofile.a

#LIBALL = $(LOGLIB) $(REORDER_LIB) $(FILELIB) 
LIBALL = $(LOGLIB)  $(FILELIB) 


#TARGETS = libpcolog.a libpcofile.a libreorderfunc.a 
TARGETS = libpcolog.a libpcofile.a 

all: $(TARGETS)


clean:
		$(RM) *.o *.a $(LIBALL)

rmerr:
	@if [ ! -e $(DIRM)*.err ]; then rmdir $(DIRM); fi




%.o:            %.cpp
	  	$(CXX) $(CFLAGS) -c $< -o $@



libpcolog.a:	$(LOGOBJ) $(HEADERS)
	  	$(AR) crus $@ $(LOGOBJ) 
	  	cp $@ $(PCOLIBDIR)

libpcofile.a:	$(FILEOBJ) $(HEADERS)
	  	$(AR) crus $@ $(FILEOBJ)
	  	cp $@ $(PCOLIBDIR)

#libreorderfunc.a:	$(REORDER_OBJ) $(HEADERS)
#	  	$(AR) crus $@ $(REORDER_OBJ)
#	  	cp $@ $(PCOLIBDIR)



