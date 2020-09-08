# GNU Makefile for zerotape
#

lemondir:=libraries/lemon
lemonsrc:=$(lemondir)/lemon.c
lemonobjs:=$(lemonsrc:.c=.o)
lemondeps:=$(lemonsrc:.c=.d)

ztdir:=libraries/zerotape
ztsrc:=$(wildcard $(ztdir)/*.c)
ztobjs:=$(ztsrc:.c=.o)
ztdeps:=$(ztsrc:.c=.d)
ztlib:=libzerotape.a

testdir:=apps/zerotape-tests
testsrc:=$(wildcard $(testdir)/*.c)
testobjs:=$(testsrc:.c=.o)
testdeps:=$(testsrc:.c=.d)

demodir:=apps/zerotape-demo
demosrc:=$(wildcard $(demodir)/*.c)
demoobjs:=$(demosrc:.c=.o)
demodeps:=$(demosrc:.c=.d)

#

CC:=clang
LD:=clang

# Tools (generic)
#
CCFLAGS=-std=c89 $(WARNINGS) -MMD -MP $(INCLUDES) $(CROSSFLAGS)
ARFLAGS=rc
LDFLAGS=$(CROSSFLAGS) $(LIBS)

ifeq ($(MODE),release)
  CCFLAGS+=-Oz -DNDEBUG
else
  CCFLAGS+=-g
  LDFLAGS+=-g
endif

# Tools (config)
#
WARNINGS:=-Wall -Wextra -Wno-unused-parameter
INCLUDES:=-Iinclude -Ilibraries/zerotape -Ilibraries/fortify

# Combined tool and flags
#
CC_:=$(CC) $(CCFLAGS)
AR_:=$(AR) $(ARFLAGS)
LD_:=$(LD) $(LDFLAGS)

%.o: %.c Makefile
	$(CC_) -c $(CCFLAGS) $< -o $@
%.c: %.y lemon $(lemondir)/lempar.c
	./lemon -d$(ztdir) -T$(lemondir)/lempar.c $<

.PHONY: all
all: ztdemo zttest

lemon: $(lemonobjs)
	$(LD_) $^ -o $@

$(ztdir)/zt-gram.o: $(ztdir)/zt-gram.c

$(ztlib): $(ztdir)/zt-gram.o $(ztobjs)
	$(AR_) $@ $^

zttest: $(ztlib) $(testobjs)
	$(LD_) $(testobjs) $(ztlib) -o $@

ztdemo: $(ztlib) $(demoobjs)
	$(LD_) $(demoobjs) $(ztlib) -o $@

clean:
	-rm -f $(lemonobjs) $(lemondeps) lemon
	-rm -f $(ztdir)/zt-gram.[ch]
	-rm -f $(ztobjs) $(ztdeps) libzerotape.a
	-rm -f $(testobjs) $(testdeps) $(zttest)
	-rm -f $(demoobjs) $(demodeps) $(ztdemo)
	@echo Cleaned

# Dependencies

-include	$(lemondeps) $(ztdeps) $(testdeps) $(demodeps)
