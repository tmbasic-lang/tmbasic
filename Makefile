# Run "make help" to get started.

# Define DISABLE_SANITIZERS to disable sanitizers in debug builds, otherwise they are enabled by default.
# Valgrind is incompatible with ASan.

OPTFLAGS ?= -g -O0
STRIP_TMBASIC ?= 0



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

# LINUX_TRIPLE
ifeq ($(TARGET_OS),linux)
ifneq ($(LINUX_TRIPLE),i586-alpine-linux-musl)
ifneq ($(LINUX_TRIPLE),x86_64-alpine-linux-musl)
ifneq ($(LINUX_TRIPLE),armv7-alpine-linux-musleabihf)
ifneq ($(LINUX_TRIPLE),aarch64-alpine-linux-musl)
ifneq ($(LINUX_TRIPLE),x86_64-linux-gnu)
ifneq ($(LINUX_TRIPLE),aarch64-linux-gnu)
$(error Unknown LINUX_TRIPLE '$(LINUX_TRIPLE)')
endif
endif
endif
endif
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

# Input runner builds, which will be 0-byte files for debug builds. for full release builds, these runners will be built
# separately and provided ahead of time.
ALL_PLATFORM_RUNNER_COMPRESSED_FILENAMES=$(foreach X,$(PLATFORMS),$X.gz)
ALL_PLATFORM_RUNNER_OBJ_FILES=$(patsubst %,obj/resources/runners/%,$(ALL_PLATFORM_RUNNER_COMPRESSED_FILENAMES:=.o))
ALL_PLATFORM_RUNNER_COMPRESSED_FILES=$(ALL_PLATFORM_RUNNER_OBJ_FILES:.o=)

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
SHARED_SRC_FILES=$(shell find src/shared -type f -name "*.cpp")
SHARED_H_FILES=$(shell find src/shared -type f -name "*.h")
SHARED_OBJ_FILES=$(patsubst src/%,obj/%,$(SHARED_SRC_FILES:.cpp=.o))
TEST_SRC_FILES=$(shell find src/test -type f -name "*.cpp")
TEST_H_FILES=$(shell find src/test -type f -name "*.h")
TEST_OBJ_FILES=$(patsubst src/%,obj/%,$(TEST_SRC_FILES:.cpp=.o))
TMBASIC_SRC_FILES=$(shell find src/tmbasic -type f -name "*.cpp")
TMBASIC_H_FILES=$(shell find src/tmbasic -type f -name "*.h")
TMBASIC_OBJ_FILES=$(patsubst src/%,obj/%,$(TMBASIC_SRC_FILES:.cpp=.o))

# tidy files
ALL_NON_TEST_CPP_FILES=$(COMPILER_SRC_FILES) $(RUNNER_SRC_FILES) $(SHARED_SRC_FILES) $(VM_SRC_FILES) $(TMBASIC_SRC_FILES) src/buildDoc.cpp
TIDY_TARGETS=$(patsubst src/%,obj/tidy/%,$(ALL_NON_TEST_CPP_FILES:.cpp=.tidy))

# ghpages files
FAVICON_IN_FILES=$(shell find doc/art/favicon -type f)
FAVICON_OUT_FILES=$(patsubst doc/art/favicon/%,bin/ghpages/%,$(FAVICON_IN_FILES))
PNG_IN_FILES=$(shell find doc/art/ -maxdepth 1 -type f -name "*.png")
PNG_OUT_FILES=$(patsubst doc/art/%,bin/ghpages/%,$(PNG_IN_FILES))

# help files
TOPIC_SRC_FILES=$(shell find doc/help/topics -type f -name "*.txt")
PROCEDURES_SRC_FILES=$(shell find doc/help/procedures -type f -name "*.txt")
DOC_FILES=$(shell find doc -type f -name "*.txt") $(shell find doc -type f -name "*.html")
DIAGRAM_SRC_FILES=$(shell find doc/help/diagrams -type f -name "*.txt")
LICENSE_FILES=\
	LICENSE \
	doc/licenses/boost/LICENSE_1_0.txt \
	doc/licenses/musl/COPYRIGHT \
	doc/licenses/immer/LICENSE \
	doc/licenses/gcc/GPL-3 \
	doc/licenses/gcc/copyright1 \
	doc/licenses/gcc/copyright2 \
	doc/licenses/mpdecimal/LICENSE.txt \
	doc/licenses/nameof/LICENSE.txt \
	doc/licenses/ncurses/COPYING \
	doc/licenses/tvision/COPYRIGHT \
	doc/licenses/fmt/LICENSE.rst \
	doc/licenses/scintilla/License.txt \
	doc/licenses/turbo/COPYRIGHT \
	doc/licenses/libzip/LICENSE \
	doc/licenses/microtar/LICENSE \
	doc/licenses/zlib/LICENSE.txt \
	doc/licenses/cli11/LICENSE \
	doc/licenses/libunistring/COPYING.LIB \
	doc/licenses/libunistring/COPYING \
	doc/licenses/abseil/LICENSE
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
	obj/doc-temp/diagrams-license/license_fmt.txt \
	obj/doc-temp/diagrams-license/license_scintilla.txt \
	obj/doc-temp/diagrams-license/license_turbo.txt \
	obj/doc-temp/diagrams-license/license_libzip.txt \
	obj/doc-temp/diagrams-license/license_microtar.txt \
	obj/doc-temp/diagrams-license/license_zlib.txt \
	obj/doc-temp/diagrams-license/license_cli11.txt \
	obj/doc-temp/diagrams-license/license_libunistring_lgpl.txt \
	obj/doc-temp/diagrams-license/license_libunistring_gpl.txt \
	obj/doc-temp/diagrams-license/license_abseil.txt

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

# Toolchain: We use cross-compilation to build Windows and Linux binaries.
ifeq ($(TARGET_OS),win)
BUILDCXX=g++
CC=$(ARCH)-w64-mingw32-gcc
CXX=$(ARCH)-w64-mingw32-g++
AR=$(ARCH)-w64-mingw32-ar
LD=$(ARCH)-w64-mingw32-ld
STRIP=$(ARCH)-w64-mingw32-strip
WINDRES=$(ARCH)-w64-mingw32-windres
endif

ifeq ($(TARGET_OS),linux)
ifeq ($(LINUX_DISTRO),alpine)
BUILDCXX=g++
CC=clang --target=$(LINUX_TRIPLE) --sysroot=/target-sysroot
CXX=clang++ --target=$(LINUX_TRIPLE) --sysroot=/target-sysroot
LD=$(LINUX_TRIPLE)-ld
AR=$(LINUX_TRIPLE)-ar
RANLIB=$(LINUX_TRIPLE)-ranlib
STRIP=$(LINUX_TRIPLE)-strip
else
# ubuntu dev container
CC=ccache gcc
CXX=ccache g++
CXXFLAGS += -fdiagnostics-color=always
ifndef DISABLE_SANITIZERS
	CXXFLAGS += -fdiagnostics-color=always -fsanitize=undefined -fsanitize=address -fsanitize=shift -fsanitize=shift-exponent -fsanitize=shift-base -fsanitize=integer-divide-by-zero -fsanitize=unreachable -fsanitize=null -fsanitize=return -fsanitize=signed-integer-overflow -fsanitize=bounds -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fsanitize=bool -fsanitize=enum -fsanitize=vptr -fsanitize=pointer-overflow -fsanitize=builtin -fno-sanitize-recover
	LDFLAGS += -static-libasan
endif
endif
endif

# TEST_CMD: We run our unit test executable in "make test". For the Windows target, we use Wine since we cross-compile
# from Linux. This command is executed from the "bin" directory.
ifeq ($(TARGET_OS),win)
TEST_CMD=mkdir -p /tmp/tmbasic-wine && HOME=/tmp/tmbasic-wine WINEPATH=/usr/$(ARCH)-w64-mingw32/bin wine test.exe
else
TEST_CMD=./test
endif



### Compiler flags ####################################################################################################

# Architecture and header search paths.
ifeq ($(TARGET_OS),linux)
CXXFLAGS += -isystem $(PREFIX)/include
endif

ifeq ($(TARGET_OS),mac)
ifeq ($(SHORT_ARCH),x64)
CXXFLAGS += -arch x86_64 -mmacosx-version-min=10.13
else
CXXFLAGS += -arch arm64 -mmacosx-version-min=11.0
endif
CXXFLAGS += -isystem $(PREFIX)/include
endif

ifeq ($(TARGET_OS),win)
CXXFLAGS += \
	-isystem /usr/$(ARCH)-w64-mingw32/include/libmpdec \
	-isystem /usr/$(ARCH)-w64-mingw32/include/libmpdec++ \
	-Wno-array-bounds
endif

CXXFLAGS += \
	-Isrc \
	-Iobj \
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
LDFLAGS += -lstdc++

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

# In our Linux containers we need to provide a search path
ifeq ($(TARGET_OS),linux)
ifeq ($(LINUX_DISTRO),ubuntu)
LDFLAGS += -L/usr/local/$(LINUX_TRIPLE)/lib
endif
ifeq ($(LINUX_DISTRO),alpine)
LDFLAGS += -L/target-sysroot/usr/local/lib
endif
endif

# On macOS we need to link against AppKit to access the clipboard. Add our deps prefix to the search path.
ifeq ($(TARGET_OS),mac)
LDFLAGS += -framework AppKit -L$(PREFIX)/lib
endif

# Linker flag to include turbo and friends.
ifeq ($(TARGET_OS),linux)
LDFLAGS += -lturbo-core -lfmt
ifeq ($(LINUX_DISTRO),ubuntu)
LDFLAGS += -lpthread
endif
else
LDFLAGS += -lturbo-core -lfmt
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

# Linker flag to include libmpdec and libmpdec++ (mpdecimal).
LDFLAGS += -lmpdec -lmpdec++

# Linker flag to include libunistring
LDFLAGS += -lunistring

# Linker flag to include libasbl_time
LDFLAGS += -labsl_time -labsl_time_zone -labsl_int128 -labsl_raw_logging_internal

# Linker flag to include microtar
LDFLAGS += -lmicrotar

# Linker flag to include libzip, zlib in tmbasic/test only (not runners).
TMBASIC_LDFLAGS += -lzip -lz

# Linker flag to include libgtest (googletest).
LIBGTEST_FLAG += -lgtest -lgtest_main

# Linker flag to include Windows-specific libraries
ifeq ($(TARGET_OS),win)
LDFLAGS += -lshlwapi
endif



### Phony targets #####################################################################################################

.PHONY: all
all: bin/tmbasic$(EXE_EXTENSION) bin/test$(EXE_EXTENSION) runner

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
ifeq ($(LINUX_DISTRO),ubuntu)
	@echo "make valgrind      Run TMBASIC with valgrind"
	@echo "make callgrind     Run tests with callgrind"
	@echo "make format        Reformat code"
	@echo "make lint          Check code with cpplint"
	@echo "make tidy          Check code with clang-tidy"
	@echo "make ghpages       Build website"
endif
	@echo ""

.PHONY: release
release:
	OPTFLAGS="-O3" EXTRADEFS="-DNDEBUG" STRIP_TMBASIC=1 DISABLE_SANITIZERS=1 $(MAKE) bin/tmbasic$(EXE_EXTENSION) bin/test$(EXE_EXTENSION)

.PHONY: clean
clean:
	@rm -rf bin obj valgrind.txt

.PHONY: run
run:
	@cd bin && ./tmbasic || (printf "\r\nCrash detected! Resetting terminal in 5 seconds...\r\n" && sleep 5 && reset && echo "Eating input. Press Ctrl+D." && cat >/dev/null)

.PHONY: test
test: bin/test$(EXE_EXTENSION)
	cd bin && $(TEST_CMD) --gtest_shuffle

.PHONY: valgrind
valgrind: bin/tmbasic
	valgrind --log-file=valgrind.txt bin/tmbasic || cat valgrind.txt
	cat valgrind.txt

.PHONE: callgrind
callgrind:
	OPTFLAGS="-O3 -g" EXTRADEFS="-DNDEBUG" $(MAKE) bin/test$(EXE_EXTENSION)
	rm -f /code/callgrind.out /code/callgrind.txt
	cd bin && valgrind --tool=callgrind --dump-instr=yes --trace-jump=yes --callgrind-out-file=/code/callgrind.out ./test
	callgrind_annotate --include=/code/src --auto=yes /code/callgrind.out > /code/callgrind.txt

.PHONY: format
format:
	find src/ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i

.PHONY: lint
lint:
	cpplint --quiet --recursive --repository=src src

.PHONY: tidy
tidy: $(TIDY_TARGETS)

.PHONY: ghpages
ghpages: obj/resources/help/help.txt bin/ghpages/index.html
	@mkdir -p bin/ghpages
	cp obj/doc-html/* bin/ghpages/

.PHONY: runner
runner: bin/runner.gz



### Build targets #####################################################################################################



# tidy ----------------------------------------------------------------------------------------------------------------

$(TIDY_TARGETS): obj/tidy/%.tidy: src/%.cpp
	@echo "$<"
	@mkdir -p $(@D)
	@clang-tidy $< --quiet --fix --extra-arg=-Wno-unknown-warning-option -- $(CXXFLAGS) -DCLANG_TIDY | tee $@
	@touch $@



# ghpages -------------------------------------------------------------------------------------------------------------

bin/ghpages/index.html: README.md \
		doc/help/html/page-template-1.html \
		doc/help/html/page-template-2.html \
		doc/help/html/page-template-3.html \
		$(FAVICON_OUT_FILES) \
		$(PNG_OUT_FILES)
	@mkdir -p $(@D)
	cat doc/help/html/page-template-1.html > $@
	echo -n "TMBASIC Programming Language" >> $@
	cat doc/help/html/page-template-2.html >> $@
	pandoc --from=markdown --to=html $< >> $@
	cat doc/help/html/page-template-3.html >> $@
	cat $@ | sed 's!https://tmbasic.com/!/!g' > $@-temp
	mv -f $@-temp $@

$(FAVICON_OUT_FILES): bin/ghpages/%: doc/art/favicon/%
	@mkdir -p $(@D)
	cp -f $< $@

$(PNG_OUT_FILES): bin/ghpages/%: doc/art/%
	@mkdir -p $(@D)
	cp -f $< $@



# precompiled header --------------------------------------------------------------------------------------------------

obj/common.h.gch: src/common.h
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -x c++-header src/common.h



# help ----------------------------------------------------------------------------------------------------------------

obj/resources/LICENSE.o: obj/resources/LICENSE.txt
	xxd -i $< | sed s/obj_resources_LICENSE_txt/kLicense/g > obj/resources/LICENSE.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c obj/resources/LICENSE.cpp

obj/resources/LICENSE.txt: $(LICENSE_FILES)
	@mkdir -p $(@D)
	@rm -f $@
	@echo === tmbasic license === >> $@
	@cat LICENSE >> $@
	@echo >> $@
	@echo === abseil license === >> $@
	@cat doc/licenses/abseil/LICENSE >> $@
	@echo >> $@
	@echo === boost license === >> $@
	@cat doc/licenses/boost/LICENSE_1_0.txt >> $@
	@echo >> $@
	@echo === fmt license === >> $@
	@cat doc/licenses/fmt/LICENSE.rst >> $@
	@echo >> $@
	@echo === immer license === >> $@
	@cat doc/licenses/immer/LICENSE >> $@
	@echo >> $@
	@echo === libstdc++ license === >> $@
	@cat doc/licenses/gcc/GPL-3 >> $@
	@cat doc/licenses/gcc/copyright >> $@
	@echo >> $@
	@echo === libunistring license === >> $@
	@cat doc/licenses/libunistring/COPYING.LIB >> $@
	@cat doc/licenses/libunistring/COPYING >> $@
	@echo >> $@
	@echo === microtar license === >> $@
	@cat doc/licenses/microtar/LICENSE >> $@
	@echo >> $@
	@echo === mpdecimal license === >> $@
	@cat doc/licenses/mpdecimal/LICENSE.txt >> $@
	@echo >> $@
	@echo === musl license === >> $@
	@cat doc/licenses/musl/COPYRIGHT >> $@
	@echo >> $@
	@echo === nameof license === >> $@
	@cat doc/licenses/nameof/LICENSE.txt >> $@
	@echo >> $@
	@echo === ncurses license === >> $@
	@cat doc/licenses/ncurses/COPYING >> $@
	@echo >> $@
	@echo === turbo license === >> $@
	@cat doc/licenses/turbo/COPYRIGHT >> $@
	@echo >> $@
	@echo === tvision license === >> $@
	@cat doc/licenses/tvision/COPYRIGHT >> $@
	@echo >> $@
	@echo === scintilla license === >> $@
	@cat doc/licenses/scintilla/License.txt >> $@
	@echo >> $@

$(LICENSE_DIAGRAM_TXT_TIMESTAMP_FILE): $(LICENSE_FILES)
	@mkdir -p $(@D)
	build/scripts/copyLicenses.sh
	@touch $@

$(LICENSE_DIAGRAM_TXT_FILES): $(LICENSE_DIAGRAM_TXT_TIMESTAMP_FILE)

obj/buildDoc: src/buildDoc.cpp
	@mkdir -p $(@D)
	$(BUILDCXX) -o $@ $< $(CXXFLAGS) -Wall -Werror -std=c++17 -lstdc++

obj/resources/help/helpfile.h: obj/resources/help/help.txt
	@mkdir -p obj/resources/help
	@mkdir -p bin
	@rm -f obj/resources/help/help.h32
	@rm -f obj/resources/help/helpfile.h
	$(TVHC) obj/resources/help/help.txt obj/resources/help/help.h32 obj/resources/help/helpfile.h >/dev/null

obj/resources/help/help.h32: obj/resources/help/helpfile.h
	@# noop

obj/resources/help/help.txt: $(DOC_FILES) $(TOPIC_SRC_FILES) $(PROCEDURES_SRC_FILES) \
		obj/buildDoc \
		$(DIAGRAM_SRC_FILES) \
		$(LICENSE_DIAGRAM_TXT_FILES) \
		doc/help/html/page-template-1.html \
		doc/help/html/page-template-2.html \
		doc/help/html/page-template-3.html
	@mkdir -p $(@D)
	cd doc && ../obj/buildDoc



# compiler ------------------------------------------------------------------------------------------------------------

$(COMPILER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES) $(VM_H_FILES) $(COMPILER_H_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/compiler.a: $(COMPILER_OBJ_FILES) obj/resources/LICENSE.o
	@mkdir -p $(@D)
	$(AR) rcs $@ $(COMPILER_OBJ_FILES) obj/resources/LICENSE.o



# shared ----------------------------------------------------------------------------------------------------------------

$(SHARED_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/shared.a: $(SHARED_OBJ_FILES)
	@mkdir -p $(@D)
	$(AR) rcs $@ $(SHARED_OBJ_FILES)



# vm ------------------------------------------------------------------------------------------------------------------

$(VM_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(VM_H_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/vm.a: $(VM_OBJ_FILES)
	@mkdir -p $(@D)
	$(AR) rcs $@ $(VM_OBJ_FILES)



# resources -----------------------------------------------------------------------------------------------------------

obj/resources/help/helpfile.o: obj/resources/help/help.h32
	@mkdir -p $(@D)
	xxd -i $< | sed s/obj_resources_help_help_h32/kResourceHelp/g > obj/resources/help/kResourceHelp.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c obj/resources/help/kResourceHelp.cpp

obj/resources/tzdb.o: $(PREFIX)/share/tzdb.tar
	@mkdir -p $(@D)
	cd "$(PREFIX)/share" && xxd -i tzdb.tar | sed s/tzdb_tar/kResourceTzdb/g > $(PWD)/obj/resources/kResourceTzdb.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c obj/resources/kResourceTzdb.cpp

$(ALL_PLATFORM_RUNNER_COMPRESSED_FILES): %: bin/runner.gz
	@mkdir -p $(@D)
	TARGET_OS=$(TARGET_OS) SHORT_ARCH=$(SHORT_ARCH) RUNNER_FILE=$@ build/scripts/runnerFile.sh

# runnerRes.sh takes a ton of memory, so run these one at a time instead of in parallel
obj/resources/runners/all: $(ALL_PLATFORM_RUNNER_COMPRESSED_FILES)
	@mkdir -p $(@D)
	OBJ_FILE=obj/resources/runners/linux_arm64.gz.o CXX="$(CXX) $(CXXFLAGS)" build/scripts/runnerRes.sh
	OBJ_FILE=obj/resources/runners/linux_arm32.gz.o CXX="$(CXX) $(CXXFLAGS)" build/scripts/runnerRes.sh
	OBJ_FILE=obj/resources/runners/linux_x64.gz.o CXX="$(CXX) $(CXXFLAGS)" build/scripts/runnerRes.sh
	OBJ_FILE=obj/resources/runners/linux_x86.gz.o CXX="$(CXX) $(CXXFLAGS)" build/scripts/runnerRes.sh
	OBJ_FILE=obj/resources/runners/mac_x64.gz.o CXX="$(CXX) $(CXXFLAGS)" build/scripts/runnerRes.sh
	OBJ_FILE=obj/resources/runners/mac_arm64.gz.o CXX="$(CXX) $(CXXFLAGS)" build/scripts/runnerRes.sh
	OBJ_FILE=obj/resources/runners/win_x64.gz.o CXX="$(CXX) $(CXXFLAGS)" build/scripts/runnerRes.sh
	OBJ_FILE=obj/resources/runners/win_x86.gz.o CXX="$(CXX) $(CXXFLAGS)" build/scripts/runnerRes.sh
	@touch $@

$(ALL_PLATFORM_RUNNER_OBJ_FILES): obj/resources/runners/%.o: obj/resources/runners/all
	@mkdir -p $(@D)



# tmbasic -------------------------------------------------------------------------------------------------------------

$(TMBASIC_OBJ_FILES): obj/%.o: src/%.cpp \
		obj/common.h.gch \
		obj/resources/help/helpfile.h \
		obj/resources/help/help.h32 \
		$(COMPILER_H_FILES) \
		$(SHARED_H_FILES) \
		$(TMBASIC_H_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

bin/tmbasic$(EXE_EXTENSION): $(TMBASIC_OBJ_FILES) \
		obj/shared.a \
		obj/compiler.a \
		obj/common.h.gch \
		obj/resources/help/helpfile.h \
		obj/resources/help/help.h32 \
		obj/resources/help/helpfile.o \
		$(ALL_PLATFORM_RUNNER_OBJ_FILES) \
		$(ICON_RES_OBJ_FILE)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(TMBASIC_OBJ_FILES) $(CXXFLAGS) $(STATIC_FLAG) -include obj/common.h obj/compiler.a obj/shared.a obj/resources/help/helpfile.o obj/resources/tzdb.o $(ALL_PLATFORM_RUNNER_OBJ_FILES) $(ICON_RES_OBJ_FILE) $(TMBASIC_LDFLAGS) $(LDFLAGS)
ifeq ($(STRIP_TMBASIC),1)
	$(STRIP) bin/tmbasic$(EXE_EXTENSION)
endif
ifeq ($(TARGET_OS),mac)
	fileicon set bin/tmbasic doc/art/favicon/favicon.icns
endif

ifeq ($(TARGET_OS),win)
obj/tmbasic/AppWin.res.o: src/tmbasic/AppWin.rc doc/art/favicon/favicon.ico
	@mkdir -p $(@D)
	$(WINDRES) $< -o $@
endif



# test ----------------------------------------------------------------------------------------------------------------

$(TEST_OBJ_FILES): obj/%.o: src/%.cpp \
		obj/common.h.gch \
		obj/resources/help/helpfile.h \
		obj/resources/help/help.h32 \
		$(COMPILER_H_FILES) \
		$(SHARED_H_FILES) \
		$(VM_H_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

bin/test$(EXE_EXTENSION): $(TEST_OBJ_FILES) \
		obj/shared.a \
		obj/vm.a \
		obj/compiler.a \
		obj/common.h.gch \
		obj/resources/help/helpfile.h \
		obj/resources/help/help.h32 \
		obj/resources/help/helpfile.o \
		obj/resources/tzdb.o \
		$(ALL_PLATFORM_RUNNER_OBJ_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) $(STATIC_FLAG) -include obj/common.h $(TEST_OBJ_FILES) obj/compiler.a obj/vm.a obj/shared.a obj/resources/help/helpfile.o obj/resources/tzdb.o $(ALL_PLATFORM_RUNNER_OBJ_FILES) $(TMBASIC_LDFLAGS) $(LDFLAGS) $(LIBGTEST_FLAG) -lpthread



# runner (this platform) ----------------------------------------------------------------------------------------------

$(RUNNER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES) $(VM_H_FILES) $(RUNNER_H_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/resources/pcode/pcode.o: %:
	@mkdir -p $(@D)
	head -c 1048576 /dev/zero | tr '\0' 'T' > obj/resources/pcode/pcode.dat
	xxd -i obj/resources/pcode/pcode.dat | sed s/obj_resources_pcode_pcode_dat/kResourcePcode/g > obj/resources/pcode/pcode.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c obj/resources/pcode/pcode.cpp

bin/runner$(EXE_EXTENSION): obj/resources/pcode/pcode.o $(RUNNER_OBJ_FILES) obj/shared.a obj/vm.a obj/resources/tzdb.o
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) $(STATIC_FLAG) -include obj/common.h $(RUNNER_OBJ_FILES) obj/resources/pcode/pcode.o obj/vm.a obj/shared.a obj/resources/tzdb.o $(LDFLAGS)
	$(STRIP) $@

bin/runner.gz: bin/runner$(EXE_EXTENSION)
	@mkdir -p $(@D)
	@rm -f $@
	cat $< | gzip -k -1 > $@
	@[ -e "$@" ] && touch $@
