
CC       = gcc
CXX      = g++
# Important: -D_FILE_OFFSET_BITS=64 is required for fuse
CPPFLAGS = -I. -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
CFLAGS   = -fPIC -Wall -O0 -ggdb
CXXFLAGS = -fno-exceptions -fno-rtti -fPIC -Wall -O0 -ggdb
LD       = g++
LDFLAGS  = -fno-exceptions -fno-rtti -fPIC -Wl,-export-dynamic -O0 -ggdb
LIBS     = -lpthread -lfuse

Q = @

default: all

all: failfs.exe failfs_cmd_util.exe

failfs.exe: main.o failfs.o error.o failfs_fuse.o
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "LD  $@"

failfs_cmd_util.exe: failfs_cmd_util.o error.o
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "LD  $@"

%.o: %.cxx
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<
	@echo "CXX $@"

%.o: %.c
	$(Q)$(CC)  $(CPPFLAGS) $(CFLAGS)   -o $@ -c $<
	@echo "CC  $@"

clean:
	-rm -f *.o failfs.exe failfs_cmd_util.exe

