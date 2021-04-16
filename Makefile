# Run "make help" to get started.

OPTFLAGS ?= -g -O0



### Input validation ##################################################################################################

# TARGET_OS
ifneq ($(TARGET_OS),linux)
ifneq ($(TARGET_OS),win)
ifneq ($(TARGET_OS),mac)
$(error Unknown TARGET_OS '$(TARGET_OS)')
endif
endif
endif

# LINUX_DISTRO
ifeq ($(TARGET_OS),linux)
ifneq ($(LINUX_DISTRO),alpine)
ifneq ($(LINUX_DISTRO),ubuntu)
$(error Unknown LINUX_DISTRO '$(LINUX_DISTRO)')
endif
endif
endif

# ARCH
ifneq ($(ARCH),i686)
ifneq ($(ARCH),x86_64)
ifneq ($(ARCH),arm32v7)
ifneq ($(ARCH),arm64v8)
$(error Unknown ARCH '$(ARCH)')
endif
endif
endif
endif

# SHORT_ARCH
ifeq ($(ARCH),i686)
SHORT_ARCH=x86
endif
ifeq ($(ARCH),x86_64)
SHORT_ARCH=x64
endif
ifeq ($(ARCH),arm32v7)
SHORT_ARCH=arm32
endif
ifeq ($(ARCH),arm64v8)
SHORT_ARCH=arm64
endif



### Source files ######################################################################################################

ifeq ($(TARGET_OS),win)
EXE_EXTENSION=.exe
endif

# Operating system + architecture pairs
PLATFORMS=linux_arm64 linux_arm32 linux_x64 linux_x86 mac_x64 mac_arm64  win_x64 win_x86

# We build several runners for each platform, each with a different length of dummy pcode. These sizes refer to the
# length of that pcode in bytes.
BZIPPED_RUNNER_SIZE=102400
BSDIFFED_RUNNER_SIZES=524288 1048576 5242880
RUNNER_SIZES=$(BZIPPED_RUNNER_SIZE) $(BSDIFFED_RUNNER_SIZES)

# Input runner builds, which will be 0-byte files for debug builds. for full release builds, these runners will be built
# separately and provided ahead of time.
ALL_PLATFORM_RUNNER_BIN_FILENAMES=$(foreach X,$(PLATFORMS),$(foreach Y,$(RUNNER_SIZES),$X_$Y))
ALL_PLATFORM_RUNNER_COMPRESSED_FILENAMES=\
	$(foreach X,$(PLATFORMS),$(foreach Y,$(BZIPPED_RUNNER_SIZE),$X_$Y.bz2)) \
	$(foreach X,$(PLATFORMS),$(foreach Y,$(BSDIFFED_RUNNER_SIZES),$X_$Y.bsdiff))
ALL_PLATFORM_RUNNER_OBJ_FILES=$(patsubst %,obj/resources/runners/%,$(ALL_PLATFORM_RUNNER_COMPRESSED_FILENAMES:=.o))
ALL_PLATFORM_RUNNER_COMPRESSED_FILES=$(ALL_PLATFORM_RUNNER_OBJ_FILES:.o=)

# Output runner builds for the particular platform targeted by this build.
THIS_PLATFORM_RUNNER_BIN_FILES=$(patsubst %,bin/runners/%,$(RUNNER_SIZES:=$(EXE_EXTENSION)))
THIS_PLATFORM_RUNNER_BSDIFF_FILES=$(patsubst %,bin/runners/%,$(BSDIFFED_RUNNER_SIZES:=.bsdiff))
THIS_PLATFORM_RUNNER_BZIP_FILES=$(patsubst %,bin/runners/%,$(BZIPPED_RUNNER_SIZE:=.bz2))

# C++ build files
COMPILER_SRC_FILES=$(shell find src/compiler -type f -name "*.cpp")
COMPILER_H_FILES=$(shell find src/compiler -type f -name "*.h")
COMPILER_OBJ_FILES=$(patsubst src/%,obj/%,$(COMPILER_SRC_FILES:.cpp=.o))
RUNNER_SRC_FILES=$(shell find src/runner -type f -name "*.cpp")
RUNNER_H_FILES=$(shell find src/runner -type f -name "*.h")
RUNNER_OBJ_FILES=$(patsubst src/%,obj/%,$(RUNNER_SRC_FILES:.cpp=.o))
VM_SRC_FILES=$(shell find src/vm -type f -name "*.cpp")
VM_H_FILES=$(shell find src/vm -type f -name "*.h")
VM_OBJ_FILES=$(patsubst src/%,obj/%,$(VM_SRC_FILES:.cpp=.o))
UTIL_SRC_FILES=$(shell find src/util -type f -name "*.cpp")
UTIL_H_FILES=$(shell find src/util -type f -name "*.h")
UTIL_OBJ_FILES=$(patsubst src/%,obj/%,$(UTIL_SRC_FILES:.cpp=.o))
TEST_SRC_FILES=$(shell find src/test -type f -name "*.cpp")
TEST_H_FILES=$(shell find src/test -type f -name "*.h")
TEST_OBJ_FILES=$(patsubst src/%,obj/%,$(TEST_SRC_FILES:.cpp=.o))
TMBASIC_SRC_FILES=$(shell find src/tmbasic -type f -name "*.cpp")
TMBASIC_H_FILES=$(shell find src/tmbasic -type f -name "*.h")
TMBASIC_OBJ_FILES=$(patsubst src/%,obj/%,$(TMBASIC_SRC_FILES:.cpp=.o))

# tidy files
ALL_NON_TEST_CPP_FILES=$(COMPILER_SRC_FILES) $(RUNNER_SRC_FILES) $(UTIL_SRC_FILES) $(VM_SRC_FILES) $(TMBASIC_SRC_FILES) src/buildDoc.cpp
TIDY_TARGETS=$(patsubst src/%,obj/tidy/%,$(ALL_NON_TEST_CPP_FILES:.cpp=.tidy))

# ghpages files
FAVICON_IN_FILES=$(shell find art/favicon -type f)
FAVICON_OUT_FILES=$(patsubst art/favicon/%,bin/ghpages/%,$(FAVICON_IN_FILES))
PNG_IN_FILES=$(shell find art -maxdepth 1 -type f -name "*.png")
PNG_OUT_FILES=$(patsubst art/%,bin/ghpages/%,$(PNG_IN_FILES))

# help files
TOPIC_UTF8_FILES=$(shell find doc/topics -type f -name "*.txt")
TOPIC_COPY_FILES=$(patsubst doc/topics/%,obj/doc-temp/topics-copy/%,$(TOPIC_UTF8_FILES))
PROCEDURES_UTF8_FILES=$(shell find doc/procedures -type f -name "*.txt")
PROCEDURES_COPY_FILES=$(patsubst doc/procedures/%,obj/doc-temp/procedures-copy/%,$(PROCEDURES_UTF8_FILES))
DOC_FILES=$(shell find doc -type f -name "*.txt") $(shell find doc -type f -name "*.html")
DIAGRAM_SRC_FILES=$(shell find doc/diagrams -type f -name "*.txt")
DIAGRAM_COPY_FILES=$(patsubst doc/diagrams/%,obj/doc-temp/diagrams-copy/%,$(DIAGRAM_SRC_FILES))
LICENSE_FILES=\
	LICENSE \
	ext/boost/LICENSE_1_0.txt \
	ext/musl/COPYRIGHT \
	ext/immer/LICENSE \
	ext/gcc/GPL-3 \
	ext/gcc/copyright1 \
	ext/gcc/copyright2 \
	ext/mpdecimal/LICENSE.txt \
	ext/nameof/LICENSE.txt \
	ext/ncurses/COPYING \
	ext/tvision/COPYRIGHT \
	ext/bsdiff/LICENSE \
	ext/bzip2/COPYING \
	ext/icu/LICENSE \
	ext/fmt/LICENSE.rst \
	ext/libclipboard/LICENSE \
	ext/libXau/COPYING \
	ext/libxcb/COPYING \
	ext/scintilla/License.txt \
	ext/turbo/COPYRIGHT
LICENSE_DIAGRAM_TXT_TIMESTAMP_FILE=obj/doc-temp/diagrams-license/timestamp
LICENSE_DIAGRAM_TXT_FILES=\
	obj/doc-temp/diagrams-license/license_tmbasic.txt \
	obj/doc-temp/diagrams-license/license_boost.txt \
	obj/doc-temp/diagrams-license/license_musl.txt \
	obj/doc-temp/diagrams-license/license_immer.txt \
	obj/doc-temp/diagrams-license/license_libstdc++_gpl3.txt \
	obj/doc-temp/diagrams-license/license_libstdc++_gcc1.txt \
	obj/doc-temp/diagrams-license/license_libstdc++_gcc2.txt \
	obj/doc-temp/diagrams-license/license_mpdecimal.txt \
	obj/doc-temp/diagrams-license/license_nameof.txt \
	obj/doc-temp/diagrams-license/license_ncurses.txt \
	obj/doc-temp/diagrams-license/license_tvision.txt \
	obj/doc-temp/diagrams-license/license_bsdiff.txt \
	obj/doc-temp/diagrams-license/license_bzip2.txt \
	obj/doc-temp/diagrams-license/license_icu.txt \
	obj/doc-temp/diagrams-license/license_fmt.txt \
	obj/doc-temp/diagrams-license/license_libclipboard.txt \
	obj/doc-temp/diagrams-license/license_libxau.txt \
	obj/doc-temp/diagrams-license/license_libxcb.txt \
	obj/doc-temp/diagrams-license/license_scintilla.txt \
	obj/doc-temp/diagrams-license/license_turbo.txt
LICENSE_DIAGRAM_COPY_FILES=\
	$(patsubst obj/doc-temp/diagrams-license/%,obj/doc-temp/diagrams-copy/%,$(LICENSE_DIAGRAM_TXT_FILES))

# icon resource
ifeq ($(TARGET_OS),win)
ICON_RES_OBJ_FILE=obj/tmbasic/AppWin.res.o
else
ICON_RES_OBJ_FILE=
endif



### Commands ##########################################################################################################

BSDIFF=bsdiff
BZIP2=bzip2
TVHC=tvhc
BUILDCXX=$(CXX)
STRIP=strip

# Toolchain: We use cross-compilation to build the Windows binaries on Linux.
ifeq ($(TARGET_OS),win)
BUILDCXX=g++
CC=$(ARCH)-w64-mingw32-gcc
CXX=$(ARCH)-w64-mingw32-g++
AR=$(ARCH)-w64-mingw32-ar
LD=$(ARCH)-w64-mingw32-ld
STRIP=$(ARCH)-w64-mingw32-strip
WINDRES=$(ARCH)-w64-mingw32-windres
endif

# TEST_CMD: We run our unit test executable in "make test". For the Windows target, we use Wine since we cross-compile
# from Linux. This command is executed from the "bin" directory.
ifeq ($(TARGET_OS),win)
TEST_CMD=WINEPATH=/usr/$(ARCH)-w64-mingw32/bin wine64 test.exe
else
TEST_CMD=./test
endif

# LICENSE_PROCESS_CMD: This command is applied to the license text file to produce the release-ready file for this
# platform. On Windows we use this to get CRLF line endings.
ifeq ($(TARGET_OS),win)
LICENSE_PROCESS_CMD=unix2dos
else
LICENSE_PROCESS_CMD=>/dev/null echo
endif



### Compiler flags ####################################################################################################

# Architecture and header search paths.
ifeq ($(TARGET_OS),mac)
ifeq ($(SHORT_ARCH),x64)
CXXFLAGS += -arch x86_64 -mmacosx-version-min=10.13
else
CXXFLAGS += -arch arm64 -mmacosx-version-min=11.0
endif
CXXFLAGS += -isystem $(PREFIX)/include -isystem $(PREFIX)/include/turbo
endif
ifeq ($(TARGET_OS),win)
CXXFLAGS += -isystem /usr/$(ARCH)-w64-mingw32/include/turbo
endif
ifeq ($(TARGET_OS),linux)
CXXFLAGS += -isystem /usr/include/turbo
endif

CXXFLAGS += \
	-Isrc \
	-Iobj \
	-isystem ext/nameof \
	-isystem /usr/include/libmpdec \
	-isystem /usr/include/libmpdec++ \
	-Wall \
	-Werror \
	-Winvalid-pch \
	-Wno-unknown-pragmas \
	-Wno-reorder \
	-std=c++17 \
	$(OPTFLAGS) \
	$(EXTRADEFS)

ifneq ($(TARGET_OS),mac)
CXXFLAGS += -Wno-psabi
endif



### Linker flags ######################################################################################################

# LDFLAGS: Linker flags used for all binaries.
LDFLAGS=-lstdc++

# TMBASIC_LDFLAGS: Additional linker flags used only for the "tmbasic" binary.
TMBASIC_LDFLAGS=

# STATIC_FLAG: We statically link on Linux and Windows. On macOS, static linking isn't an option. In our dev build, we
# also won't statically link because the dev environment is glibc-based and it doesn't like being statically linked.
ifeq ($(TARGET_OS),linux)
ifeq ($(LINUX_DISTRO),alpine)
STATIC_FLAG=-static
endif
endif
ifeq ($(TARGET_OS),win)
STATIC_FLAG=-static
endif

# On macOS we need to link against AppKit to access the clipboard. Add our deps prefix to the search path.
ifeq ($(TARGET_OS),mac)
LDFLAGS += -framework AppKit -L$(PREFIX)/lib
endif

# Linker flag to include turbo and friends.
ifeq ($(TARGET_OS),linux)
LDFLAGS += -lscintilla -lscilexers -lsciplatform -lturbo-ui -lclipboard -lxcb -lXau -lfmt
ifeq ($(LINUX_DISTRO),ubuntu)
LDFLAGS += -lpthread
endif
else
LDFLAGS += -lscintilla -lscilexers -lsciplatform -lturbo-ui -lclipboard -lfmt
endif

# Linker flag to include tvision.
LDFLAGS += -ltvision

# Linker flag to include libncursesw. On macOS, tvision uses libncurses instead because libncursesw isn't distributed
# on that platform.
ifeq ($(TARGET_OS),mac)
LDFLAGS += -lncurses
else
LDFLAGS += -lncursesw
endif

# Linker flag to include libtinfo. We don't normally need libtinfo because we build ncurses without it, but for
# development builds we use an Ubuntu package that does need it.
ifeq ($(TARGET_OS),linux)
ifeq ($(LINUX_DISTRO),ubuntu)
LDFLAGS += -ltinfo
endif
endif

# Linker flag to include libmpdec and libmpdec++ (mpdecimal).
ifeq ($(TARGET_OS),win)
LDFLAGS += /usr/$(ARCH)-w64-mingw32/lib/libmpdec.a /usr/$(ARCH)-w64-mingw32/lib/libmpdec++.a
else
LDFLAGS += -lmpdec -lmpdec++
endif

# Linker flag to include ICU.
ifeq ($(TARGET_OS),win)
ifeq ($(ARCH),i686)
LDFLAGS += -lsicuin -lsicuuc /usr/$(ARCH)-w64-mingw32/bin/libicudt.a
else
LDFLAGS += -lsicuin -lsicuuc /usr/$(ARCH)-w64-mingw32/bin/icudt.a
endif
else
LDFLAGS += -licui18n -licuuc -licudata
endif
ifeq ($(TARGET_OS),linux)
LDFLAGS += -ldl
endif

# Linker flag to include bzip2 in tmbasic/test only.
TMBASIC_LDFLAGS += -lbz2_static

# Linker flag to include libbspatch in tmbasic/test only.
TMBASIC_LDFLAGS += -lbspatch

# Linker flag to include libgtest (googletest).
LIBGTEST_FLAG += -lgtest -lgtest_main



### Phony targets #####################################################################################################

.PHONY: all
all: bin/tmbasic$(EXE_EXTENSION) bin/test$(EXE_EXTENSION) bin/LICENSE.txt runners

.PHONY: versions
ifeq ($(TARGET_OS),linux)
ifeq ($(LINUX_DISTRO),alpine)
versions:
	@apk info --license libstdc++ | tr -d '\n' | awk '{print $$1}'
	@apk info --license musl-dev | tr -d '\n' | awk '{print $$1}'
else
versions:
	@echo
endif
endif

ifeq ($(TARGET_OS),win)
versions:
	@echo
endif

ifeq ($(TARGET_OS),mac)
versions:
	@echo
endif

.PHONY: help
# only show the help text in the dev container
ifeq ($(LINUX_DISTRO),ubuntu)
help: versions
	@echo "Target: $(TARGET_OS) $(ARCH)"
	@echo ""
	@echo "COMMANDS"
	@echo "--------"
	@echo "make               Build TMBASIC (debug)"
	@echo "make release       Build TMBASIC (release)"
	@echo "make run           Run TMBASIC"
	@echo "make test          Run tests"
	@echo "make clean         Delete build outputs"
	@echo "make valgrind      Run TMBASIC with valgrind"
	@echo "make format        Reformat code"
	@echo "make lint          Check code with cpplint"
	@echo "make tidy          Check code with clang-tidy"
	@echo "make tidy-commit   Check 'git status' files with clang-tidy"
	@echo "make ghpages       Build tmbasic-gh-pages"
	@echo "make ghpages-test  Host tmbasic-gh-pages on port 5000"
	@echo ""
else
help: versions
endif

.PHONY: release
release:
	@OPTFLAGS="-Os" EXTRADEFS="-DNDEBUG" $(MAKE)

.PHONY: clean
clean:
	@rm -rf bin obj valgrind.txt

.PHONY: run
run:
	@bin/tmbasic

.PHONY: test
test: bin/test$(EXE_EXTENSION)
	@cd bin && $(TEST_CMD)

.PHONY: valgrind
valgrind: bin/tmbasic
	@valgrind --leak-check=full --show-leak-kinds=all --undef-value-errors=no --log-file=valgrind.txt bin/tmbasic

.PHONY: format
format:
	@find src/ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i

.PHONY: lint
lint:
	@cpplint --quiet --recursive --repository=src src

.PHONY: tidy
tidy: $(TIDY_TARGETS)

.PHONY: tidy-commit
tidy-commit: $(shell 2>/dev/null git status | grep "\.cpp" | sed 's/modified://' | sed 's:src:obj/tidy:g; s/cpp$$/tidy/g')

.PHONY: ghpages
ghpages: obj/resources/help/help.txt bin/ghpages/index.html
	@mkdir -p bin/ghpages
	@cp obj/doc-html/* bin/ghpages/

.PHONY: ghpages-test
ghpages-test:
	@cd bin/ghpages && python3 -m http.server 5000

.PHONY: runners
runners: $(patsubst %,bin/runners/%,$(BZIPPED_RUNNER_SIZE:=.bz2)) \
		$(patsubst %,bin/runners/%,$(BSDIFFED_RUNNER_SIZES:=.bsdiff))



### Build targets #####################################################################################################



# tidy ----------------------------------------------------------------------------------------------------------------

$(TIDY_TARGETS): obj/tidy/%.tidy: src/%.cpp
	@printf "%16s  %s\n" "clang-tidy" "$<"
	@mkdir -p $(@D)
	@clang-tidy $< --quiet --fix --extra-arg=-Wno-unknown-warning-option -- $(CXXFLAGS) -DCLANG_TIDY | tee $@
	@touch $@



# ghpages -------------------------------------------------------------------------------------------------------------

bin/ghpages/index.html: README.md \
		doc/html/page-template-1.html \
		doc/html/page-template-2.html \
		doc/html/page-template-3.html \
		$(FAVICON_OUT_FILES) \
		$(PNG_OUT_FILES)
	@printf "%16s  %s\n" "pandoc" "$@"
	@mkdir -p $(@D)
	@cat doc/html/page-template-1.html > $@
	@echo -n "TMBASIC" >> $@
	@cat doc/html/page-template-2.html >> $@
	@pandoc --from=markdown --to=html $< >> $@
	@cat doc/html/page-template-3.html >> $@

$(FAVICON_OUT_FILES): bin/ghpages/%: art/favicon/%
	@printf "%16s  %s\n" "cp" "$@"
	@mkdir -p $(@D)
	@cp -f $< $@

$(PNG_OUT_FILES): bin/ghpages/%: art/%
	@printf "%16s  %s\n" "cp" "$@"
	@mkdir -p $(@D)
	@cp -f $< $@



# precompiled header --------------------------------------------------------------------------------------------------

obj/common.h.gch: src/common.h
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -x c++-header src/common.h



# help ----------------------------------------------------------------------------------------------------------------

bin/LICENSE.txt: $(LICENSE_FILES)
	@printf "%16s  %s\n" "cat" "$@"
	@mkdir -p bin
	@rm -f $@
	@echo === tmbasic license === >> $@
	@cat LICENSE >> $@
	@echo >> $@
	@echo === boost license === >> $@
	@cat ext/boost/LICENSE_1_0.txt >> $@
	@echo >> $@
	@echo === fmt license === >> $@
	@cat ext/fmt/LICENSE.rst >> $@
	@echo >> $@
	@echo === icu license === >> $@
	@cat ext/icu/LICENSE >> $@
	@echo >> $@
	@echo === immer license === >> $@
	@cat ext/immer/LICENSE >> $@
	@echo >> $@
	@echo === libclipboard license === >> $@
	@cat ext/libclipboard/LICENSE >> $@
	@echo >> $@
	@echo === libstdc++ license === >> $@
	@cat ext/gcc/GPL-3 >> $@
	@cat ext/gcc/copyright >> $@
	@echo >> $@
	@echo === libXau license === >> $@
	@cat ext/libXau/COPYING >> $@
	@echo >> $@
	@echo === libxcb license === >> $@
	@cat ext/libxcb/COPYING >> $@
	@echo >> $@
	@echo === mpdecimal license === >> $@
	@cat ext/mpdecimal/LICENSE.txt >> $@
	@echo >> $@
	@echo === musl license === >> $@
	@cat ext/musl/COPYRIGHT >> $@
	@echo >> $@
	@echo === ncurses license === >> $@
	@cat ext/ncurses/COPYING >> $@
	@echo >> $@
	@echo === turbo license === >> $@
	@cat ext/turbo/COPYRIGHT >> $@
	@echo >> $@
	@echo === tvision license === >> $@
	@cat ext/tvision/COPYRIGHT >> $@
	@echo >> $@
	@echo === scintilla license === >> $@
	@cat ext/scintilla/License.txt >> $@
	@echo >> $@
	@$(LICENSE_PROCESS_CMD) $@

$(LICENSE_DIAGRAM_TXT_TIMESTAMP_FILE): $(LICENSE_FILES)
	@printf "%16s  %s\n" "copyLicenses.sh" "$@"
	@mkdir -p $(@D)
	@build/scripts/copyLicenses.sh
	@touch $@

$(LICENSE_DIAGRAM_TXT_FILES): $(LICENSE_DIAGRAM_TXT_TIMESTAMP_FILE)

obj/buildDoc: src/buildDoc.cpp
	@printf "%16s  %s\n" "$(BUILDCXX)" "$@"
	@mkdir -p $(@D)
	@$(BUILDCXX) \
		-o $@ $< \
		$(CXXFLAGS) \
		-Iext/nameof \
		-Wall \
		-Werror \
		-std=c++17 \
		-lstdc++

obj/resources/help/helpfile.h: obj/resources/help/help.txt
	@printf "%16s  %s\n" "tvhc" "$@"
	@mkdir -p obj/resources/help
	@mkdir -p bin
	@rm -f obj/resources/help/help.h32
	@rm -f obj/resources/help/helpfile.h
	@$(TVHC) obj/resources/help/help.txt obj/resources/help/help.h32 obj/resources/help/helpfile.h >/dev/null

obj/resources/help/help.h32: obj/resources/help/helpfile.h
	@# noop

obj/resources/help/help.txt: $(DOC_FILES) $(TOPIC_COPY_FILES) $(PROCEDURES_COPY_FILES) \
		obj/buildDoc \
		$(DIAGRAM_COPY_FILES) \
		$(LICENSE_DIAGRAM_COPY_FILES) \
		doc/html/page-template-1.html \
		doc/html/page-template-2.html \
		doc/html/page-template-3.html
	@printf "%16s  %s\n" "buildDoc" "$@"
	@mkdir -p $(@D)
	@cd doc && ../obj/buildDoc

$(DIAGRAM_COPY_FILES): obj/doc-temp/diagrams-copy/%: doc/diagrams/%
	@printf "%16s  %s\n" "cp" "$@"
	@mkdir -p $(@D)
	@cp -f $< $@

$(LICENSE_DIAGRAM_COPY_FILES): obj/doc-temp/diagrams-copy/%: obj/doc-temp/diagrams-license/%
	@printf "%16s  %s\n" "cp" "$@"
	@mkdir -p $(@D)
	@cp -f $< $@

$(TOPIC_COPY_FILES): obj/doc-temp/topics-copy/%: doc/topics/%
	@printf "%16s  %s\n" "cp" "$@"
	@mkdir -p $(@D)
	@cp -f $< $@

$(PROCEDURES_COPY_FILES): obj/doc-temp/procedures-copy/%: doc/procedures/%
	@printf "%16s  %s\n" "cp" "$@"
	@mkdir -p $(@D)
	@cp -f $< $@



# compiler ------------------------------------------------------------------------------------------------------------

$(COMPILER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(UTIL_H_FILES) $(VM_H_FILES) $(COMPILER_H_FILES)
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/compiler.a: $(COMPILER_OBJ_FILES)
	@printf "%16s  %s\n" "$(AR)" "$@"
	@mkdir -p $(@D)
	@$(AR) rcs $@ $(COMPILER_OBJ_FILES)



# util ----------------------------------------------------------------------------------------------------------------

$(UTIL_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(UTIL_H_FILES)
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/util.a: $(UTIL_OBJ_FILES)
	@printf "%16s  %s\n" "$(AR)" "$@"
	@mkdir -p $(@D)
	@$(AR) rcs $@ $(UTIL_OBJ_FILES)



# vm ------------------------------------------------------------------------------------------------------------------

$(VM_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(VM_H_FILES)
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/vm.a: $(VM_OBJ_FILES)
	@printf "%16s  %s\n" "$(AR)" "$@"
	@mkdir -p $(@D)
	@$(AR) rcs $@ $(VM_OBJ_FILES)



# resources -----------------------------------------------------------------------------------------------------------

obj/resources/help/helpfile.o: obj/resources/help/help.h32
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@xxd -i $< | sed s/obj_resources_help_help_h32/kResourceHelp/g > obj/resources/help/kResourceHelp.cpp
	@$(CXX) -o $@ $(CXXFLAGS) -c obj/resources/help/kResourceHelp.cpp

$(ALL_PLATFORM_RUNNER_COMPRESSED_FILES): %: $(THIS_PLATFORM_RUNNER_BSDIFF_FILES) $(THIS_PLATFORM_RUNNER_BZIP_FILES)
	@printf "%16s  %s\n" "runnerFile.sh" "$@"
	@mkdir -p $(@D)
	@TARGET_OS=$(TARGET_OS) SHORT_ARCH=$(SHORT_ARCH) RUNNER_FILE=$@ build/scripts/runnerFile.sh

$(ALL_PLATFORM_RUNNER_OBJ_FILES): obj/resources/runners/%.o: obj/resources/runners/% \
		$(ALL_PLATFORM_RUNNER_COMPRESSED_FILES)
	@printf "%16s  %s\n" "runnerRes.sh" "$@"
	@mkdir -p $(@D)
	@OBJ_FILE=$@ CXX="$(CXX) $(CXXFLAGS)" build/scripts/runnerRes.sh



# tmbasic -------------------------------------------------------------------------------------------------------------

$(TMBASIC_OBJ_FILES): obj/%.o: src/%.cpp \
		obj/common.h.gch \
		obj/resources/help/helpfile.h \
		obj/resources/help/help.h32 \
		$(COMPILER_H_FILES) \
		$(UTIL_H_FILES) \
		$(VM_H_FILES) \
		$(TMBASIC_H_FILES)
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

bin/tmbasic$(EXE_EXTENSION): $(TMBASIC_OBJ_FILES) \
		obj/util.a \
		obj/vm.a \
		obj/compiler.a \
		obj/common.h.gch \
		obj/resources/help/helpfile.h \
		obj/resources/help/help.h32 \
		obj/resources/help/helpfile.o \
		$(ALL_PLATFORM_RUNNER_OBJ_FILES) \
		$(ICON_RES_OBJ_FILE)
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) \
		-o $@ $(TMBASIC_OBJ_FILES) \
		$(CXXFLAGS) \
		$(STATIC_FLAG) \
		-include obj/common.h \
		obj/util.a \
		obj/vm.a \
		obj/compiler.a \
		obj/resources/help/helpfile.o \
		$(ALL_PLATFORM_RUNNER_OBJ_FILES) \
		$(ICON_RES_OBJ_FILE) \
		$(TMBASIC_LDFLAGS) \
		$(LDFLAGS)
	@$(STRIP) bin/tmbasic$(EXE_EXTENSION)

ifeq ($(TARGET_OS),win)
obj/tmbasic/AppWin.res.o: src/tmbasic/AppWin.rc art/favicon/favicon.ico
	@printf "%16s  %s\n" "$(WINDRES)" "$@"
	@mkdir -p $(@D)
	@$(WINDRES) $< -o $@
endif



# test ----------------------------------------------------------------------------------------------------------------

$(TEST_OBJ_FILES): obj/%.o: src/%.cpp \
		obj/common.h.gch \
		obj/resources/help/helpfile.h \
		obj/resources/help/help.h32 \
		$(COMPILER_H_FILES) \
		$(UTIL_H_FILES) \
		$(VM_H_FILES)
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

bin/test$(EXE_EXTENSION): $(TEST_OBJ_FILES) \
		obj/util.a \
		obj/vm.a \
		obj/compiler.a \
		obj/common.h.gch \
		obj/resources/help/helpfile.h \
		obj/resources/help/help.h32 \
		obj/resources/help/helpfile.o \
		$(ALL_PLATFORM_RUNNER_OBJ_FILES)
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) \
		-o $@ \
		$(CXXFLAGS) \
		-include obj/common.h \
		$(TEST_OBJ_FILES) \
		obj/util.a \
		obj/vm.a \
		obj/compiler.a \
		obj/resources/help/helpfile.o \
		$(ALL_PLATFORM_RUNNER_OBJ_FILES) \
		$(TMBASIC_LDFLAGS) \
		$(LDFLAGS) \
		$(LIBGTEST_FLAG) \
		-lpthread



# runner (this platform) ----------------------------------------------------------------------------------------------
# We build several versions that are identical except for the length of the dummy pcode they have embedded.
# We ship the 100KB runner and a set of binary patches to convert the 100KB runner to the other sizes.

$(RUNNER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(UTIL_H_FILES) $(VM_H_FILES) $(RUNNER_H_FILES)
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

$(patsubst %,obj/resources/pcode/%,$(RUNNER_SIZES:=.o)): %:
	@printf "%16s  %s\n" "pcodeRes.sh" "$@"
	@mkdir -p $(@D)
	@OBJ_FILE=$@ CXX="$(CXX) $(CXXFLAGS)" build/scripts/pcodeRes.sh

$(THIS_PLATFORM_RUNNER_BIN_FILES): bin/runners/%$(EXE_EXTENSION): \
		obj/resources/pcode/%.o $(RUNNER_OBJ_FILES) obj/util.a obj/vm.a
	@printf "%16s  %s\n" "$(CXX)" "$@"
	@mkdir -p $(@D)
	@$(CXX) \
		-o $@ \
		$(CXXFLAGS) \
		$(STATIC_FLAG) \
		-include obj/common.h \
		$(RUNNER_OBJ_FILES) \
		obj/resources/pcode/$(patsubst bin/runners/%$(EXE_EXTENSION),%,$@).o \
		obj/util.a \
		obj/vm.a \
		$(LDFLAGS)
	@$(STRIP) $@

$(THIS_PLATFORM_RUNNER_BSDIFF_FILES): bin/runners/%.bsdiff: bin/runners/%$(EXE_EXTENSION) \
		bin/runners/$(BZIPPED_RUNNER_SIZE)$(EXE_EXTENSION)
	@printf "%16s  %s\n" "bsdiff" "$@"
	@mkdir -p $(@D)
	@$(BSDIFF) bin/runners/$(BZIPPED_RUNNER_SIZE)$(EXE_EXTENSION) $< $@

$(THIS_PLATFORM_RUNNER_BZIP_FILES): bin/runners/%.bz2: bin/runners/%$(EXE_EXTENSION)
	@printf "%16s  %s\n" "bzip2" "$@"
	@mkdir -p $(@D)
	@rm -f $@
	@cat $< | $(BZIP2) --keep --best > $@
	@[ -e "$@" ] && touch $@
