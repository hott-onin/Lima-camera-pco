# Makefile for libpcocom_me4.a and libpcocam_me4.a
#    libpcocom_me4.a = (ar crus libpcocom_me4.a pcocom.o)
#       pcocom.o
#       $(CPCODIR)/Cpco_com.o $(CPCODIR)/Cpco_com_func.o $(CPCODIR)/Cpco_com_func_2.o Cpco_com_cl_me4.o 
#    libpcocam_me4.a  (ar crus libpcocam_me4.a pcocom.o pcograb.o)
#           Cpco_grab_cl_me4.o Cpco_grab_cl_me4_GL.o ../../pco_common/pco_lib/libreorderfunc.a -->  pcograb.o
# 
# pco_common/pco_lib 
# ------------ pco_camera/pco_me4/pco_classes/Makefile
#-rw-r--r-- 1 homsrego bliss 225656 Jun 13 14:44 libpcocam_me4.a 
#-rw-r--r-- 1 homsrego bliss 116812 Jun 13 14:43 libpcocom_me4.a 
#
# ------------ from pco
#-rw-r--r-- 1 homsrego bliss  26010 Mar  8 11:42 libreorderfunc.a
#-rw-r--r-- 1 homsrego bliss  33578 Jun 13 14:54 libpcofile.a
#
# ------------ pco_common/pco_lib_base/Makefile
#-rw-r--r-- 1 homsrego bliss  11068 Jun 13 14:54 libpcolog.a 
#-rw-r--r-- 1 homsrego bliss  33578 Jun 13 14:54 libpcofile.a


VERSION = $(word 3,$(shell head ./VersionNo.h -n40 | awk '/VERSION/{print $2}'))
SUBVERSION = $(word 3,$(shell head ./VersionNo.h -n40 | awk '/SUBVERSION/{print $2}'))
SUBAGE = $(word 3,$(shell head ./VersionNo.h -n40 | awk '/SUBAGE/{print $2}'))

PCOHDIR = ../include
PCOLIBDIR = ../pco_lib
CPCODIR = ../pco_classes
SDKLIBDIR = ../pco_libsdk


REORDERLIB=$(SDKLIBDIR)/libreorderfunc.a
LOGLIB=$(PCOLIBDIR)/libpcolog.a

SISOINCLUDEDIR = $(SISODIR5)/include

HEADERS = $(PCOHDIR)/VersionNo.h $(PCOHDIR)/Cpco_com_cl_me4.h $(PCOHDIR)/Cpco_com.h $(PCOHDIR)/Cpco_com_func.h $(PCOHDIR)/Cpco_com_func_2.h

CFLAGS := -O2 -Wall -DLINUX -fPIC -I$(PCOHDIR)  -I$(SISOINCLUDEDIR) 
LFLAGS += -L$(PCOLIBDIR)


COM_OBJ =  $(CPCODIR)/Cpco_com.o $(CPCODIR)/Cpco_com_func.o $(CPCODIR)/Cpco_com_func_2.o Cpco_com_cl_me4.o

COM_O = pcocom.o

GRAB_OBJ = Cpco_grab_cl_me4.o Cpco_grab_cl_me4_GL.o

GRAB_O = pcograb.o 

LIB_COM = libpcocom_me4.a

#LIB_COM_SO = libpcocom_me4.so

LIB_CAM = libpcocam_me4.a


all: $(LIB_COM) $(LIB_CAM) copy

clean:
		$(RM) *.o *~ $(PCOHDIR)*~ *.a *.so* $(CPCODIR)/*.o $(PCOLIBDIR)/$(LIB_COM) $(PCOLIBDIR)/$(LIB_CAM)

copy:
		@mv $(LIB_COM) $(PCOLIBDIR)
		@mv $(LIB_CAM) $(PCOLIBDIR)


#		rm $(PCOLIBDIR)/$(LIB_COM_SO)* ;\
#		mv $(LIB_COM_SO).$(VERSION).$(SUBVERSION).$(SUBAGE) $(PCOLIBDIR) ; \
#		cd $(PCOLIBDIR); \
#		ln -s $(LIB_COM_SO).$(VERSION).$(SUBVERSION).$(SUBAGE) $(LIB_COM_SO).$(VERSION) ; \
#		ln -s $(LIB_COM_SO).$(VERSION) $(LIB_COM_SO) ; \
#		cd ../.. \


# libpcocam_me4.a
$(LIB_CAM):	$(COM_O) $(GRAB_O) $(HEADERS) 
	  	$(AR) crus $@ $(COM_O) $(GRAB_O) 

# libpcocom_me4.a
$(LIB_COM):	$(COM_O) $(HEADERS) 
	  	$(AR) crus $@ $(COM_O) 

$(COM_O):	$(COM_OBJ) $(HEADERS)
		$(LD) -r  $(COM_OBJ) -o $@
#$(LFLAGS) -lpcolog


$(COM_OBJ):	%.o:	$(patsubst %.o,%.cpp,$(COM_OBJ)) $(HEADERS) Makefile
			$(CXX) $(CFLAGS) -o $@ -c $*.cpp

$(GRAB_O):	$(GRAB_OBJ) $(HEADERS)
		$(LD) -r  $(GRAB_OBJ) $(REORDERLIB) -o $@  


$(GRAB_OBJ):	%.o:	$(patsubst %.o,%.cpp,$(GRAB_OBJ)) $(HEADERS) Makefile
			$(CXX) $(CFLAGS) -o $@ -c $(notdir $*).cpp


#&>$(DIRM)$(notdir $*).err


$(LIB_COM_SO):		$(COM_O) $(LOGLIB)
			$(RM) $(LIB_COM_SO)* ;
			$(CXX) -shared -fPIC -Wl,-soname=$(LIB_COM_SO).$(VERSION) \
                        -o $(LIB_COM_SO).$(VERSION).$(SUBVERSION).$(SUBAGE) $^

