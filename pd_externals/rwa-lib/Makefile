# oggread~ built as a standalone Pd library ("libdir") named `rwa`, so the
# player patches in puredata/ can be opened and developed in a normal Pd instead
# of only inside the Creator. See README.md.
#
# Only oggread~ is here, for now only oggread~ has to come from us: it is a fork
# the patches depend on. The other non-vanilla object the patches need,
# freeverb~, is stock and comes from deken, check the README.md for instructions.
#
#   make                                    build oggread~
#   make install                            install into ~/Library/Pd/rwa
#   make install objectsdir=~/Documents/Pd/externals
#   make check-upstream                     diff oggread~.c against pristine pdogg
#   make PDINCLUDEDIR=/Applications/Pd-0.56-5.app/Contents/Resources/src
#
# Also reachable from the CMake build as the `pd-externals` /
# `pd-externals-install` targets.

# ------------------------------------------------------------------------------
# Universal binaries
#
# pd-lib-builder 0.7.0 assigns `arch` itself inside its Darwin section, after it
# evaluates our forDarwin block, so a makefile-level setting cannot survive: the
# documented way to build universal is the command-line override
# `make arch="x86_64 arm64"` (pd-lib-builder/tips-tricks.md). Forward whatever
# was asked for to a sub-make carrying that override, so that a plain `make`
# here does not silently produce an arm64-only library that no Intel Mac and no
# Rosetta Pd can load. `make arch=arm64` still gets you a host-only build.
# ------------------------------------------------------------------------------
ifeq ($(shell uname -s)/$(origin arch), Darwin/undefined)

goals := $(or $(MAKECMDGOALS), all)
.PHONY: $(goals)
$(goals):
	@$(MAKE) --no-print-directory arch="x86_64 arm64" $@

else

lib.name = rwa

# ------------------------------------------------------------------------------
# Where everything lives. All paths are relative to this makefile.
# ------------------------------------------------------------------------------
ROOT  = ../..
PDOGG = $(ROOT)/pd-extra/pd/externals/pdogg
STAGE = staged

# Pd headers come from the libpd submodule (0.52-2), which is the Pd that
# Creator and Player actually embed, not from whatever Pd is installed, which
# pd-lib-builder would otherwise autodetect. Building an external against newer
# headers than the Pd it gets loaded into is the direction that breaks. Same
# pin, and same reasoning, as vas_library's libdir.
PDINCLUDEDIR ?= $(ROOT)/libpd/pure-data/src

# ------------------------------------------------------------------------------
# Sources, as they live in the tree
#
# oggread~.c is our fork of pdogg 0.25's oggread~ (deferred end-of-file bang,
# `start <seconds>`); `make check-upstream` shows the delta. Ogg/Vorbis is
# compiled in from the same vendored
# submodules the app uses, so the decoder is identical to Creator's and the
# result needs no libvorbis installed on the machine.
# ------------------------------------------------------------------------------
classes.vendored = $(ROOT)/oggread~.c

oggvorbis.vendored = \
    $(ROOT)/libogg/src/bitwise.c \
    $(ROOT)/libogg/src/framing.c \
    $(ROOT)/vorbis/lib/analysis.c \
    $(ROOT)/vorbis/lib/bitrate.c \
    $(ROOT)/vorbis/lib/block.c \
    $(ROOT)/vorbis/lib/codebook.c \
    $(ROOT)/vorbis/lib/envelope.c \
    $(ROOT)/vorbis/lib/floor0.c \
    $(ROOT)/vorbis/lib/floor1.c \
    $(ROOT)/vorbis/lib/info.c \
    $(ROOT)/vorbis/lib/lookup.c \
    $(ROOT)/vorbis/lib/lpc.c \
    $(ROOT)/vorbis/lib/lsp.c \
    $(ROOT)/vorbis/lib/mapping0.c \
    $(ROOT)/vorbis/lib/mdct.c \
    $(ROOT)/vorbis/lib/misc.c \
    $(ROOT)/vorbis/lib/psy.c \
    $(ROOT)/vorbis/lib/registry.c \
    $(ROOT)/vorbis/lib/res0.c \
    $(ROOT)/vorbis/lib/sharedbook.c \
    $(ROOT)/vorbis/lib/smallft.c \
    $(ROOT)/vorbis/lib/synthesis.c \
    $(ROOT)/vorbis/lib/vorbisfile.c \
    $(ROOT)/vorbis/lib/window.c

# vorbisenc.c is left out: it is the encoder API, nothing here calls it, and it
# carries the large static setup tables. `make check-symbols` proves nothing
# went missing -- on macOS the externals link with a flat namespace, so an
# omitted source is not a link error but a dlopen failure at load time.

# ------------------------------------------------------------------------------
# Source staging
#
# pd-lib-builder writes each object file next to its source (`%.o:: %.c`), and
# scans the sources while this makefile is being read. Pointing it straight at
# the vendored trees would therefore drop .pd_darwin.o files inside the libogg/,
# vorbis/ and pd-extra/ submodules and show up as dirt in their git status. So
# symlink every source into $(STAGE)/ instead, and do it at parse time with
# $(shell ...) so the files already exist when that dependency scan runs.
#
# basename, not $${f##*/}: a '#' inside a make function argument starts a
# comment and swallows the rest of the call.
# ------------------------------------------------------------------------------
stage := $(shell mkdir -p $(STAGE) && \
    for f in $(classes.vendored) $(oggvorbis.vendored); do \
        ln -sf "../$$f" "$(STAGE)/$$(basename "$$f")"; \
    done)

class.sources = $(addprefix $(STAGE)/, $(notdir $(classes.vendored)))

common.sources = $(addprefix $(STAGE)/, $(notdir $(oggvorbis.vendored)))

# ------------------------------------------------------------------------------
# Flags
#
# The vendored sources reach their own private headers through -I, not through
# their file's own directory: the staged symlinks are what the compiler sees,
# so a quoted #include resolves relative to $(STAGE)/ and has to fall back to
# the search path. (pd-lib-builder supplies -DPD itself; nothing here needs the
# app's -DPUREDATA.)
# ------------------------------------------------------------------------------
# oggread~.c is a 2002-era source; -Wall -Wextra on it is all pre-existing
# noise. Silence the unused-* family only, as the CMake build does, so a real
# warning still stands out.
suppress-wunused = yes

cflags = \
    -I$(ROOT)/libogg/include \
    -I$(ROOT)/vorbis/include \
    -I$(ROOT)/vorbis/lib

define forDarwin
  # Universal, like the app (CMAKE_OSX_ARCHITECTURES) and like vas_library's
  # externals: pd-lib-builder defaults to the host architecture alone, which on
  # this machine would silently produce an arm64-only library that no Intel Mac
  # and no Rosetta Pd can load.
  arch = x86_64 arm64
  # 11.0 is the oldest macOS with arm64, so one flag covers both slices.
  # pd-lib-builder picks a -mmacosx-version-min= out of cflags if it finds one.
  cflags += -mmacosx-version-min=11.0
endef

define forLinux
  ldlibs += -lm
endef

# ------------------------------------------------------------------------------
# Shipped alongside the binaries in the libdir
# ------------------------------------------------------------------------------
datafiles = \
    rwa-meta.pd \
    README.md \
    oggread~-help.pd \
    alligator-mono.ogg \
    alligator-mono.aif \
    belbbirds-stereo.ogg \
    belbbirds-stereo.wav

PDLIBBUILDER_DIR = $(ROOT)/pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder


# ------------------------------------------------------------------------------
# Extra targets (must come after the include: pd-lib-builder's first target is
# the default one)
# ------------------------------------------------------------------------------

# oggread~.c is a fork. pdogg has been unmaintained since 0.25.1 (2011) and has
# no upstream git, so "the published version" is the copy in the pd-l2ork
# submodule, verified byte-identical to pd-l2ork master. This target makes the
# fork delta visible instead of remembered; -b/-B hide the retabbing.
check-upstream:
	@echo "--- $(PDOGG)/oggread~.c (pdogg 0.25.x, pristine)"
	@echo "+++ $(ROOT)/oggread~.c (RWA fork)"
	@diff -u -b -B '$(PDOGG)/oggread~.c' '$(ROOT)/oggread~.c' \
	    && echo "==== identical apart from whitespace ===="; true

# On macOS an external links with a flat namespace, so a source file left out
# of the build is not a link error, but it fails at dlopen time. Pd's own symbols
# are meant to stay undefined and get resolved out of the host; a codec symbol
# left undefined means a missing source file here.
check-symbols: all
	@status=0; for f in *.$(extension); do \
	    missing=`nm -u "$$f" | grep -E '_(ov|vorbis|ogg|oggpack)_' || true`; \
	    if [ -n "$$missing" ]; then \
	        echo "$$f: unresolved codec symbols -- a source file is missing:"; \
	        echo "$$missing"; status=1; \
	    else \
	        echo "$$f: ok (only Pd/libc symbols undefined)"; \
	    fi; \
	done; exit $$status

.PHONY: check-upstream check-symbols

endif
