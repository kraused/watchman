
CXX      = g++
CPPFLAGS = -I. -D_GNU_SOURCE
CXXFLAGS = -fno-exceptions -fno-rtti -fPIC -Wall -O0 -ggdb
LD       = g++
LDFLAGS  = -fno-exceptions -fno-rtti -fPIC -Wl,-export-dynamic -O0 -ggdb
LIBS     = -ldl

OBJ   = main.o watchman.o plugin.o libc_alloc.o child.o program.o \
        buffer.o file.o named_file.o initfini.o error.o
TESTS = tests/test1.so tests/test2.so tests/test3.so tests/test4.so

Q = @

default: all

all: watchman.exe tests/failfs/failfs.exe $(TESTS)

watchman.exe: $(OBJ)
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "LD  $@"

tests/failfs/failfs.exe:
	make -C tests/failfs

%.o: %.cxx
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<
	@echo "CXX $@"

%.so: %.o
	$(Q)$(CXX) $(LDFLAGS) -shared -o $@ $<
	@echo "CC  $@"

clean:
	make -C tests/failfs clean
	-rm -f tests/*.o tests/*.so *.o watchman.exe

