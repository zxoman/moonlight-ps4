TITLE := Moonlight PS4
VERSION := 01.00
TITLE_ID := MLHT00001
CONTENT_ID := IV0000-MLHT00001_00-MOONLIGHT0000000

LIBS := -lc -lkernel -lSceVideoOut -lScePad -lSceUserService -lSceSystemService -lSceNet -lSceSysmodule

TOOLCHAIN := $(OO_PS4_TOOLCHAIN)
INTDIR := build
SRCDIR := src

CFILES := \
	$(SRCDIR)/core/config.c \
	$(SRCDIR)/core/gs.c \
	$(SRCDIR)/core/http.c \
	$(SRCDIR)/core/log.c \
	$(SRCDIR)/core/rtsp.c \
	$(SRCDIR)/core/rtp.c \
	$(SRCDIR)/core/stream.c \
	$(SRCDIR)/core/util.c \
	$(SRCDIR)/core/xml.c \
	$(SRCDIR)/core/app.c \
	$(SRCDIR)/platform/orbis/ui.c \
	$(SRCDIR)/platform/orbis/input.c \
	$(SRCDIR)/platform/orbis/main.c

OBJS := $(patsubst $(SRCDIR)/%.c,$(INTDIR)/%.o,$(CFILES))

CFLAGS := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c \
	-DML_PLATFORM_ORBIS -Iinclude -Isrc/core -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include
LDFLAGS := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crt1.o

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
 CC := clang
 LD := ld.lld
 CDIR := linux
endif
ifeq ($(UNAME_S),Darwin)
 CC := /usr/local/opt/llvm/bin/clang
 LD := /usr/local/opt/llvm/bin/ld.lld
 CDIR := macos
endif

ASSETS := assets/moonlight.ini
LIBMODULES := sce_module/libc.prx sce_module/libSceFios2.prx

.PHONY: all clean pkg eboot

all: $(CONTENT_ID).pkg

$(INTDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

eboot.bin: $(OBJS)
	mkdir -p $(INTDIR)
	$(LD) $(OBJS) -o $(INTDIR)/moonlight.elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself -in=$(INTDIR)/moonlight.elf -out=$(INTDIR)/moonlight.oelf --eboot "eboot.bin" --paid 0x3800000000000011

sce_module/libc.prx sce_module/libSceFios2.prx:
	mkdir -p sce_module
	@if [ -f "$(TOOLCHAIN)/src/modules/libc.prx" ]; then \
		cp "$(TOOLCHAIN)/src/modules/libc.prx" sce_module/; \
		cp "$(TOOLCHAIN)/src/modules/libSceFios2.prx" sce_module/; \
	elif [ -f "$(TOOLCHAIN)/samples/hello_world/sce_module/libc.prx" ]; then \
		cp "$(TOOLCHAIN)/samples/hello_world/sce_module/"*.prx sce_module/; \
	else \
		echo "warning: libc.prx not found in toolchain; pkg may still build"; \
		mkdir -p sce_module; \
	fi

sce_sys/about/right.sprx:
	mkdir -p sce_sys/about
	@if [ -f "$(TOOLCHAIN)/samples/hello_world/sce_sys/about/right.sprx" ]; then \
		cp "$(TOOLCHAIN)/samples/hello_world/sce_sys/about/right.sprx" $@; \
	elif [ -f "$(TOOLCHAIN)/samples/_common/sys/about/right.sprx" ]; then \
		cp "$(TOOLCHAIN)/samples/_common/sys/about/right.sprx" $@; \
	else \
		echo "warning: right.sprx missing"; \
	fi

sce_sys/param.sfo: Makefile
	mkdir -p sce_sys
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_new $@
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ APP_TYPE --type Integer --maxsize 4 --value 1
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ APP_VER --type Utf8 --maxsize 8 --value '$(VERSION)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ ATTRIBUTE --type Integer --maxsize 4 --value 0
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ CATEGORY --type Utf8 --maxsize 4 --value 'gd'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ CONTENT_ID --type Utf8 --maxsize 48 --value '$(CONTENT_ID)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ SYSTEM_VER --type Integer --maxsize 4 --value 0
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value '$(TITLE)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ TITLE_ID --type Utf8 --maxsize 12 --value '$(TITLE_ID)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ VERSION --type Utf8 --maxsize 8 --value '$(VERSION)'

pkg.gp4: eboot.bin sce_sys/about/right.sprx sce_sys/param.sfo sce_sys/icon0.png $(LIBMODULES) $(ASSETS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-gp4 -out $@ --content-id=$(CONTENT_ID) --files "eboot.bin sce_sys/about/right.sprx sce_sys/param.sfo sce_sys/icon0.png $(LIBMODULES) $(ASSETS)"

$(CONTENT_ID).pkg: pkg.gp4
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core pkg_build $< .

pkg: $(CONTENT_ID).pkg

clean:
	rm -rf $(INTDIR) eboot.bin pkg.gp4 sce_sys/param.sfo $(CONTENT_ID).pkg
