ifeq ($(B),win64)
CTARGET=x86_64-pc-mingw32-
SFIX=64
else
CTARGET=i686-pc-mingw32-
SFIX=
B=win32
endif

CC=$(CTARGET)gcc
CXX=$(CTARGET)g++
LD=$(CTARGET)ld
STRIP=$(CTARGET)strip
RC=$(CTARGET)windres
AR=$(CTARGET)ar
RANLIB=$(CTARGET)ranlib
DLLTOOL=$(CTARGET)dlltool
DLLWRAP=$(CTARGET)dllwrap --driver-name $(CXX) --dlltool-name $(DLLTOOL)

INCLUDES=-Igdiplus -I. -I..
UNICODE_DEFS=-DUNICODE -D_UNICODE
LDFLAGS=-shared -lshlwapi -luuid -lgdi32 -lole32 -Wl,--enable-stdcall-fixup
ifeq ($(B),win64)
LDFLAGS+=-m64
endif
RCFLAGS=--input-format rc --output-format coff -DEMBED_MANIFEST $(UNICODE_DEFS)
CSRCS=
CXXSRCS=KuContextMenu.cpp KuMenuSet.cpp KuShellExtension.cpp KuShellExtInit.cpp KuShellExtensionFactory.cpp StringConv.cpp FSLinks/FSLinks.cpp FSLinks/Misc.cpp FSLinks/Reparse_Dir_HANDLE.cpp dll.cpp globals.cpp
target=KuShellExtension$(SFIX).dll
objects=

CFLAGS=-pipe -O3 -fomit-frame-pointer -D_USRDLL \
	$(UNICODE_DEFS) \
	$(if $(findstring -D_DEBUG,$(MYCFLAGS)),,-DNDEBUG) \
	$(INCLUDES) $(MYCFLAGS)
CXXFLAGS=$(CFLAGS)

SRCS=$(CSRCS) $(CXXSRCS)
objects+=$(patsubst %.c,%.o,$(CSRCS)) $(patsubst %.cpp,%.o,$(CXXSRCS))

.PHONY: clean distclean depend pch

all: $(target)

clean:
	$(RM) *.o *.gch *.coff *.a $(objects)

distclean: clean
	$(RM) $(target)

depend:
	echo -n >Makefile.deps
ifdef CSRCS
	$(CC) -MM $(CFLAGS) $(CSRCS) >>Makefile.deps
endif
	$(CXX) -MM $(CXXFLAGS) $(CXXSRCS) >>Makefile.deps

pch: depend
	sed -i 's/stdafx\.h/stdafx.h.gch/g' Makefile.deps

$(target): $(objects)
	$(CXX) $(CXXFLAGS) -o $@ KuShellExtension.def $^ $(LDFLAGS)
	$(STRIP) $@
ifeq ($(B),win32)
	upx --lzma -9 $@
endif

%.coff: %.rc
	$(RC) $(RCFLAGS) -o $@ -i $<

%.gch: %
	$(CXX) $(CXXFLAGS) -o $@ $<

include Makefile.deps
