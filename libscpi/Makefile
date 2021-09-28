VERSION = 2.1.0
LIBNAME = scpi

CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar
OBJCOPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump
SIZE=arm-none-eabi-size
RANLIB=arm-none-eabi-ranlib

CFLAGS += -mfloat-abi=soft
CFLAGS += -Wall -g -std=c99 -Os -fno-strict-aliasing
CFLAGS += -mlittle-endian -mcpu=cortex-m3 -mthumb

CFLAGS+=-I. -DUSE_FULL_ERROR_LIST -DUSE_DEVICE_DEPENDENT_ERROR_INFORMATION -DUSE_MEMORY_ALLOCATION_FREE=0
CFLAGS += -Wextra -Wmissing-prototypes -Wimplicit -Iinc

LDFLAGS += -lm -Wl,--as-needed
#TESTCFLAGS += $(CFLAGS) `pkg-config --cflags cunit`
#TESTLDFLAGS += $(LDFLAGS) `pkg-config --libs cunit`
TESTCFLAGS += $(CFLAGS)
TESTLDFLAGS += $(LDFLAGS) -lcunit

OBJDIR=obj
OBJDIR_STATIC=$(OBJDIR)/static
DISTDIR=dist
TESTDIR=test

PREFIX := $(DESTDIR)/usr/local
LIBDIR := $(PREFIX)/lib
INCDIR := $(PREFIX)/include

STATICLIBFLAGS = rcs

STATICLIB = lib$(LIBNAME).a

SRCS = $(addprefix src/, \
	error.c fifo.c ieee488.c \
	minimal.c parser.c units.c utils.c \
	lexer.c expression.c \
	)

OBJS_STATIC = $(addprefix $(OBJDIR_STATIC)/, $(notdir $(SRCS:.c=.o)))

HDRS = $(addprefix inc/scpi/, \
	scpi.h constants.h error.h \
	ieee488.h minimal.h parser.h types.h units.h \
	expression.h \
	) \
	$(addprefix src/, \
	lexer_private.h utils_private.h fifo_private.h \
	parser_private.h \
	) \


TESTS = $(addprefix $(TESTDIR)/, \
	test_fifo.c test_scpi_utils.c test_lexer_parser.c test_parser.c\
	)

TESTS_OBJS = $(TESTS:.c=.o)
TESTS_BINS = $(TESTS_OBJS:.o=.test)

.PHONY: all clean static test install

all: static

static: $(DISTDIR)/$(STATICLIB)

clean:
	$(RM) -r $(OBJDIR) $(DISTDIR) $(TESTS_BINS) $(TESTS_OBJS)

test: $(TESTS_BINS)
	$(TESTS_BINS:.test=.test &&) true

install: $(DISTDIR)/$(STATICLIB)
	test -d $(PREFIX) || mkdir $(PREFIX)
	test -d $(LIBDIR) || mkdir $(LIBDIR)
	test -d $(INCDIR) || mkdir $(INCDIR)
	test -d $(INCDIR)/scpi || mkdir $(INCDIR)/scpi
	install -m 0644 $(DISTDIR)/$(STATICLIB) $(LIBDIR)
	install -m 0644 inc/scpi/*.h $(INCDIR)/scpi

$(OBJDIR_STATIC):
	mkdir -p $@

$(DISTDIR):
	mkdir -p $@

$(OBJDIR_STATIC)/%.o: src/%.c $(HDRS) | $(OBJDIR_STATIC)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(DISTDIR)/$(STATICLIB): $(OBJS_STATIC) | $(DISTDIR)
	$(AR) $(STATICLIBFLAGS) $(DISTDIR)/$(STATICLIB) $(OBJS_STATIC)

$(TESTDIR)/%.o: $(TESTDIR)/%.c
	$(CC) -c $(TESTCFLAGS) $(CPPFLAGS) -o $@ $<

$(TESTDIR)/%.test: $(TESTDIR)/%.o $(DISTDIR)/$(STATICLIB)
	$(CC) $< -o $@ $(DISTDIR)/$(STATICLIB) $(TESTLDFLAGS)



