SRCS=$(wildcard src/*.c)
SOBJ=$(SRCS:.c=.$(LIB_EXTENSION))
INSTALL?=install
ifdef URL_COVERAGE
COVFLAGS=--coverage
endif


.PHONY: all install clean

all: $(SOBJ)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNINGS) $(COVFLAGS) $(CPPFLAGS) -o $@ -c $<

%.$(LIB_EXTENSION): %.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(PLATFORM_LDFLAGS) $(COVFLAGS)

install: $(SOBJ)
	$(INSTALL) -d $(INST_LIBDIR)
	$(INSTALL) $(SOBJ) $(INST_LIBDIR)
	$(INSTALL) url.lua $(INST_LUADIR)
	rm -f ./src/*.o
	rm -f ./src/*.so
