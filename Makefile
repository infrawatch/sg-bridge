
MAJOR?=0
MINOR?=1

VERSION=$(MAJOR).$(MINOR)

BIN := bridge

SRCS = $(wildcard *.c)

OBJDIR := obj

DEPDIR := $(OBJDIR)/.deps

# object files, auto generated from source files
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRCS)))
# dependency files, auto generated from source files
DEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

# compilers (at least gcc and clang) don't create the subdirectories automatically
$(shell mkdir -p $(dir $(OBJS)) >/dev/null)
$(shell mkdir -p $(dir $(DEPS)) >/dev/null)

CC=gcc
CFLAGS+=-O2 -flto=auto -ffat-lto-objects -fexceptions -g -grecord-gcc-switches -pipe -Wall -Werror=format-security -Wp,-D_FORTIFY_SOURCE=2 -Wp,-D_GLIBCXX_ASSERTIONS -specs=/usr/lib/rpm/redhat/redhat-hardened-cc1 -fstack-protector-strong -specs=/usr/lib/rpm/redhat/redhat-annobin-cc1 -m64 -march=x86-64 -mtune=generic -fasynchronous-unwind-tables -fstack-clash-protection -fcf-protection
LDLIBS=-lqpid-proton -lpthread
LDFLAGS+=-Wl,-z,relro -Wl,--as-needed  -Wl,-z,now -specs=/usr/lib/rpm/redhat/redhat-hardened-ld -specs=/usr/lib/rpm/redhat/redhat-annobin-cc1

DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td

# compile C source files
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@

# link object files to binary
LINK.o = $(LD) $(LDFLAGS) $(CFLAGS) $(LDLIBS) -o $@

# precompile step
PRECOMPILE =
# postcompile step
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

HUB_NAMESPACE = "localhost"
BRIDGE_IMAGE_NAME = "sgbridge"

all: $(BIN)
debug: CFLAGS=-Wall -g
debug: all

.PHONY: clean
clean:
	rm -fr $(OBJDIR) $(DEPDIR)

.PHONY: clean-image
clean-image: version-check
	@echo "+ $@"
	@podman rmi ${HUB_NAMESPACE}/${BUILDER_IMAGE_NAME}:latest  || true

.PHONY: image
image: version-check
	@echo "+ $@"
	@buildah bud -t ${HUB_NAMESPACE}/${BRIDGE_IMAGE_NAME}:latest -f build/Dockerfile .
	@echo 'Done.'

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) $(LDLIBS)

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

#################################
# Utilities
#################################

.PHONY: version-check
version-check:
	@echo "+ $@"
    ifdef VERSION
		@echo "VERSION is ${VERSION}"
    else
		@echo "VERSION is not set!"
		@false;
    endif

-include $(DEPS)
