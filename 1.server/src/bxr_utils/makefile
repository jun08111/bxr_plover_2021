CC = /usr/bin/gcc
LD = /usr/bin/ld

#DATETIME = `date "+%Y%m%d%H%M%S"`
DATETIME = `date "+%m%d%H%M%S"`

LIBNAME = libBxrUtils.so
LIBVER = 0.1.3

SRCHOME =`echo $(HOME)/Blue-X-ray-Plover/1.server`
APPHOME =`echo $(BXRG_HOME)`

INILIB = -L$(SRCHOME)/lib/iniparser-4.1/lib64
INIINC = -I$(SRCHOME)/lib/iniparser-4.1/include

NNGLIB = -L$(SRCHOME)/lib/nng-1.5.1/lib64
NNGINC = -I$(SRCHOME)/lib/nng-1.5.1/include

JSNINC = -I$(SRCHOME)/lib/json-c-0.15/include
JSNLIB = -L$(SRCHOME)/lib/json-c-0.15/lib64

LIBHOME = $(SRCHOME)/lib
INCHOME = $(SRCHOME)/inc

# application compiler options
#CFLAGS = -g -O2 -fPIC -m64 -DLINUX
CFLAGS = -g -fPIC -m64 -D_BXDBG

# library linker options
#LFLAGS = -shared -Wl, -soname, libBxrUtils.so
LFLAGS = -shared

#LLIBS = -ldl -lrt -lpthread -lm -lstdc++ -lssl
LLIBS = -lpthread

LIB = $(INILIB) $(NNGLIB) $(JSNLIB)
INC = -I$(INCHOME) $(INIINC) $(NNGINC) $(JSNINC)

# application linker options
LDFLAGS = $(LIB)

#.SUFFIXES: .c .cpp .cxx .o
CORULE = $(CC) -c $(@F:.o=.c) $(DEBUG) $(CFLAGS) $(INC)

.c.o:
	$(CORULE)

sources = bxr_utils.o

out=$(LIBNAME).$(LIBVER)

all: $(out)
$(out): $(sources)
	$(CC) $(LFLAGS) -o $(out) $(sources) $(LLIBS) $(LIB) 
	ln -sf $(out) $(LIBNAME)

install:
#	mv $(LIBHOME)/$(out)* $(LIBHOME)/Backup/$(out)*.$(DATETIME)
#	mv $(LIBHOME)/$(LIBNAME).* $(LIBHOME)/Backup/$(LIBNAME).*.$(DATETIME)
	ls -al $(out)
#	echo -e "##----------------------------------##\n"
#	find $(LIBHOME) -maxdepth 1 -name "$(LIBNAME).*.*.*.*" -exec mv {} $(SRCHOME)/backup/. \;
#	find $(LIBHOME) -maxdepth 1 -name "$(LIBNAME).*" -exec mv {} {}.$(DATETIME) \;
	find $(APPHOME)/lib -maxdepth 1 -name "$(LIBNAME).*" -exec mv {} $(APPHOME)/backup/. \;
	cp -arp $(LIBNAME)* $(LIBHOME)/.
	cp -arp $(LIBNAME)* $(APPHOME)/lib/.

clean :
	rm -rf *.o $(LIBNAME)*

