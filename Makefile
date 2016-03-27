
VERSION    = 1
PATCHLEVEL = 0

CXX      = g++
CPPFLAGS = -I. -D_GNU_SOURCE
CXXFLAGS = -fno-exceptions -fno-rtti -fPIC -Wall -O0 -ggdb
LD       = g++
LDFLAGS  = -fno-exceptions -fno-rtti -fPIC -Wl,-export-dynamic -O0 -ggdb
LIBS     = -ldl

OBJ   = main.o watchman.o plugin.o libc_alloc.o child.o program.o \
        buffer.o file.o named_file.o named_clingy_file.o initfini.o \
        error.o
TESTS = tests/test1.so tests/test2.so tests/test3.so \
        tests/test4.so tests/test5.so tests/test6.so

Q = @

default: all

all: watchman.exe tests/failfs/failfs.exe $(TESTS)

watchman.exe: $(OBJ)
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "LD  $@"

tests/failfs/failfs.exe:
	make -C tests/failfs

%.o: watchman/%.cxx
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<
	@echo "CXX $@"

tests/%.o: tests/%.cxx
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<
	@echo "CXX $@"

%.so: %.o
	$(Q)$(CXX) $(LDFLAGS) -shared -o $@ $<
	@echo "CXX $@"

tar:
	python2 tar.py watchman $(VERSION).$(PATCHLEVEL)

# Header to install
HEADER = watchman.hxx config.hxx compiler.hxx error.hxx alloc.hxx buffer.hxx \
         child.hxx program.hxx plugin.hxx \
         file.hxx named_file.hxx named_clingy_file.hxx

install:
	install -m0755 -d $(PREFIX)/usr/sbin/
	install -m0755 -d $(PREFIX)/usr/include/watchman/
	install -m0755 watchman.exe		$(PREFIX)/usr/sbin/watchman.exe
	for H in $(HEADER); do								\
		install -m0644 watchman/$${H}	$(PREFIX)/usr/include/watchman/$${H} ;	\
	done

clean:
	make -C tests/failfs clean
	-rm -f tests/*.o tests/*.so *.o watchman.exe

