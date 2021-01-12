# Run "make help" to get started.

OPTFLAGS ?= -g -O0



### Input validation ##################################################################################################

# TARGET_OS
ifneq ($(TARGET_OS),linux)
ifneq ($(TARGET_OS),windows)
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



### Source files ######################################################################################################

ifeq ($(TARGET_OS),win)
EXE_EXTENSION=.exe
endif

# runner builds, which will be 0-byte files for debug builds. for full release builds, these runners will be built
# separately and provided ahead of time
RUNNERS_OBJ_FILES=\
	obj/runner_linux_arm32.o \
	obj/runner_linux_arm64.o \
	obj/runner_linux_x64.o \
	obj/runner_linux_x86.o \
	obj/runner_mac_x64.o \
	obj/runner_win_x64.o \
	obj/runner_win_x86.o
RUNNERS_BIN_FILES=$(RUNNERS_OBJ_FILES:.o=)

ifeq ($(TARGET_OS),linux)
LINUX_RESOURCE_OBJ_FILES ?= obj/helpfile.o $(RUNNERS_OBJ_FILES)
endif

# C++ build files
COMPILER_SRC_FILES=$(shell find src/compiler -type f -name "*.cpp")
COMPILER_H_FILES=$(shell find src/compiler -type f -name "*.h")
COMPILER_OBJ_FILES=$(patsubst src/%,obj/%,$(COMPILER_SRC_FILES:.cpp=.o))
RUNNER_SRC_FILES=$(shell find src/runner -type f -name "*.cpp")
RUNNER_H_FILES=$(shell find src/runner -type f -name "*.h")
RUNNER_OBJ_FILES=$(patsubst src/%,obj/%,$(RUNNER_SRC_FILES:.cpp=.o))
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
ALL_NON_TEST_CPP_FILES=$(COMPILER_SRC_FILES) $(RUNNER_SRC_FILES) $(SHARED_SRC_FILES) $(TMBASIC_SRC_FILES)
TIDY_TARGETS=$(patsubst src/%,obj/tidy/%,$(ALL_NON_TEST_CPP_FILES:.cpp=.tidy))

# ghpages files
FAVICON_IN_FILES=$(shell find art/favicon -type f)
FAVICON_OUT_FILES=$(patsubst art/favicon/%,bin/ghpages/%,$(FAVICON_IN_FILES))

# help files
DOC_FILES=$(shell find doc -type f -name "*.txt") $(shell find doc -type f -name "*.html")
DIAGRAM_SRC_FILES=$(shell find doc/diagrams -type f -name "*.txt")
DIAGRAM_CP437_FILES=$(patsubst doc/diagrams/%,obj/doc-temp/diagrams-cp437/%,$(DIAGRAM_SRC_FILES))
LICENSE_DIAGRAM_SRC_FILES=\
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
	obj/doc-temp/diagrams-license/license_tvision.txt
LICENSE_DIAGRAM_CP437_FILES=\
	$(patsubst obj/doc-temp/diagrams-license/%,obj/doc-temp/diagrams-cp437/%,$(LICENSE_DIAGRAM_SRC_FILES))



### Commands ##########################################################################################################

# Toolchain: We use cross-compilation to build the Windows binaries on Linux.
ifeq ($(TARGET_OS),win)
BUILDCC=gcc
CC=$(ARCH)-w64-mingw32-gcc
CXX=$(ARCH)-w64-mingw32-g++
AR=$(ARCH)-w64-mingw32-ar
LD=$(ARCH)-w64-mingw32-ld
STRIP=$(ARCH)-w64-mingw32-strip
WINDRES=$(ARCH)-w64-mingw32-windres
else
BUILDCC=$(CC)
STRIP=strip
endif

# TVHC_CMD: We run tvhc to generate our help file.
ifeq ($(TARGET_OS),linux)
TVHC_CMD=tvhc
endif
ifeq ($(TARGET_OS),win)
TVHC_CMD=wine64 /usr/$(ARCH)-w64-mingw32/bin/tvhc.exe
endif
ifeq ($(TARGET_OS),mac)
TVHC_CMD=$(PWD)/mac/tvision/build/tvhc
endif

# TEST_CMD: We run our unit test executable in "make test". For the Windows target, we use Wine since we cross-compile
# from Linux. This command is executed from the "bin" directory.
ifeq ($(TARGET_OS),win)
TEST_CMD="WINEPATH=/usr/$(ARCH)-w64-mingw32/bin wine64 test.exe"
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

# macOS header search paths.
ifeq ($(TARGET_OS),mac)
CXXFLAGS += \
	-I$(PWD)/mac/boost \
	-I$(PWD)/mac/mpdecimal/libmpdec \
	-I$(PWD)/mac/mpdecimal/libmpdec++ \
	-I$(PWD)/mac/ncurses/include \
	-I$(PWD)/mac/googletest/googletest/include \
	-I$(PWD)/mac/tvision/include \
	-I$(PWD)/mac/immer
endif

# Windows header search paths.
ifeq ($(TARGET_OS),win)
CXXFLAGS += -I/usr/share/mingw-w64/include/
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



### Linker flags ######################################################################################################

# LDFLAGS: Linker flags used for all binaries.
LDFLAGS=-lstdc++

# TMBASIC_LDFLAGS: Additional linker flags used only for the "tmbasic" binary.
TMBASIC_LDFLAGS=

# STATIC_FLAG: We statically link on Linux and Windows. On macOS, static linking isn't an option.
ifeq ($(TARGET_OS),mac)
STATIC_FLAG=
else
STATIC_FLAG=-static
endif

# On macOS we need to add some search paths.
ifeq ($(TARGET_OS),mac)
LDFLAGS += -L$(PWD)/mac/mpdecimal/libmpdec -L$(PWD)/mac/mpdecimal/libmpdec++ -L$(PWD)/mac/tvision/build
endif

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
ifeq ($(TARGET_OS),linux)
LDFLAGS += -lmpdec -lmpdec++
endif
ifeq ($(TARGET_OS),win)
LDFLAGS += /usr/$(ARCH)-w64-mingw32/lib/libmpdec.a /usr/$(ARCH)-w64-mingw32/lib/libmpdec++.a
endif
ifeq ($(TARGET_OS),mac)
LDFLAGS += $(PWD)/mac/mpdecimal/libmpdec/libmpdec.a $(PWD)/mac/mpdecimal/libmpdec++/libmpdec++.a
endif

# Linker flag to include libgtest (googletest).
ifeq ($(TARGET_OS),mac)
LDFLAGS += $(PWD)/mac/googletest/build/lib/libgtest.a $(PWD)/mac/googletest/build/lib/libgtest_main.a
else
LDFLAGS += -lgtest -lgtest_main
endif

# macOS-specific resource objects to be linked into the "tmbasic" binary.
ifeq ($(TARGET_OS),mac)
TMBASIC_LDFLAGS += \
	-Wl,-sectcreate,__DATA,__help_h32,obj/help.h32 \
	-Wl,-sectcreate,__DATA,__rla32,obj/runner_linux_arm32 \
	-Wl,-sectcreate,__DATA,__rla64,obj/runner_linux_arm64 \
	-Wl,-sectcreate,__DATA,__rlx64,obj/runner_linux_x64 \
	-Wl,-sectcreate,__DATA,__rlx86,obj/runner_linux_x86 \
	-Wl,-sectcreate,__DATA,__rmx64,obj/runner_mac_x64 \
	-Wl,-sectcreate,__DATA,__rwx64,obj/runner_win_x64 \
	-Wl,-sectcreate,__DATA,__rwx86,obj/runner_win_x86
endif



### Phony targets #####################################################################################################

.PHONY: all
all: bin/tmbasic$(EXE_EXTENSION) bin/test$(EXE_EXTENSION) bin/LICENSE.txt

.PHONY: help
help:
	@echo ""
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
	@echo "make ghpages       Build tmbasic-gh-pages"
	@echo "make ghpages-test  Host tmbasic-gh-pages on port 5000"
	@echo ""

.PHONY: release
release:
	@OPTFLAGS="-Os -flto" EXTRADEFS="-DNDEBUG" $(MAKE)

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
	@clang-format -i build/files/buildDoc.cpp

.PHONY: lint
lint:
	@cpplint --quiet --recursive --repository=src src build/files/buildDoc.cpp

.PHONY: tidy
tidy: $(TIDY_TARGETS)

.PHONY: ghpages
ghpages: obj/help.txt bin/ghpages/index.html
	@mkdir -p bin/ghpages
	@cp obj/doc-html/* bin/ghpages/

.PHONY: ghpages-test
ghpages-test:
	@cd bin/ghpages && python3 -m http.server 5000



### Build targets #####################################################################################################

# tidy

$(TIDY_TARGETS): obj/tidy/%.tidy: src/%.cpp
	@echo $<
	@mkdir -p $(@D)
	@clang-tidy $< --quiet --fix $(CXXFLAGS) -DCLANG_TIDY | tee $@
	@touch $@

# ghpages

bin/ghpages/index.html: README.md \
		doc/html/page-template-1.html \
		doc/html/page-template-2.html \
		doc/html/page-template-3.html \
		$(FAVICON_OUT_FILES) \
		bin/ghpages/screenshot.png
	@echo $@
	@mkdir -p $(@D)
	@cat doc/html/page-template-1.html > $@
	@echo -n "TMBASIC" >> $@
	@cat doc/html/page-template-2.html >> $@
	@pandoc --from=markdown --to=html $< >> $@
	@cat doc/html/page-template-3.html >> $@

$(FAVICON_OUT_FILES): bin/ghpages/%: art/favicon/%
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

bin/ghpages/screenshot.png: art/screenshot.png
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

# precompiled header

obj/common.h.gch: src/common.h
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -x c++-header src/common.h

# help

bin/LICENSE.txt: LICENSE \
		ext/boost/LICENSE_1_0.txt \
		ext/immer/LICENSE \
		ext/gcc/GPL-3 \
		ext/gcc/copyright \
		ext/mpdecimal/LICENSE.txt \
		ext/nameof/LICENSE.txt \
		ext/ncurses/COPYING \
		ext/tvision/COPYRIGHT
	@echo $@
	@mkdir -p bin
	@rm -f $@
	@echo === tmbasic license === >> $@
	@cat LICENSE >> $@
	@echo >> $@
	@echo === boost license === >> $@
	@cat ext/boost/LICENSE_1_0.txt >> $@
	@echo >> $@
	@echo === immer license === >> $@
	@cat ext/immer/LICENSE >> $@
	@echo >> $@
	@echo === libstdc++ license === >> $@
	@cat ext/gcc/GPL-3 >> $@
	@cat ext/gcc/copyright >> $@
	@echo >> $@
	@echo === mpdecimal license === >> $@
	@cat ext/mpdecimal/LICENSE.txt >> $@
	@echo >> $@
	@echo === musl license === >> $@
	@cat ext/musl/COPYRIGHT >> $@
	@echo >> $@
	@echo === nameof license === >> $@
	@cat ext/nameof/LICENSE.txt >> $@
	@echo >> $@
	@echo === ncurses license === >> $@
	@cat ext/ncurses/COPYING >> $@
	@echo >> $@
	@echo === tvision license === >> $@
	@cat ext/tvision/COPYRIGHT >> $@
	@echo >> $@
	@$(LICENSE_PROCESS_CMD) $@

obj/doc-temp/diagrams-license/license_tmbasic.txt: LICENSE
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_boost.txt: ext/boost/LICENSE_1_0.txt
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_musl.txt: ext/musl/COPYRIGHT
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_immer.txt: ext/immer/LICENSE
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_libstdc++_gpl3.txt: ext/gcc/GPL-3
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_libstdc++_gcc1.txt: ext/gcc/copyright1
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_libstdc++_gcc2.txt: ext/gcc/copyright2
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_mpdecimal.txt: ext/mpdecimal/LICENSE.txt
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_nameof.txt: ext/nameof/LICENSE.txt
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_ncurses.txt: ext/ncurses/COPYING
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_tvision.txt: ext/tvision/COPYRIGHT
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/buildDoc: build/files/buildDoc.cpp
	@echo $@
	@$(BUILDCC) \
		-o $@ $< \
		-I$(PWD)/mac/boost \
		-Iext/nameof \
		-Wall \
		-Werror \
		-std=c++17 \
		-lstdc++

obj/helpfile.h: obj/help.txt
	@echo $@
	@mkdir -p obj
	@mkdir -p bin
	@rm -f obj/help.h32
	@rm -f obj/helpfile.h
	@$(TVHC_CMD) obj/help.txt obj/help.h32 obj/helpfile.h

obj/help.h32: obj/helpfile.h
	@echo $@

obj/help.txt: $(DOC_FILES) \
		obj/buildDoc \
		$(DIAGRAM_CP437_FILES) \
		$(LICENSE_DIAGRAM_CP437_FILES) \
		doc/html/page-template-1.html \
		doc/html/page-template-2.html \
		doc/html/page-template-3.html
	@echo $@
	@mkdir -p obj
	@cd doc && ../obj/buildDoc

$(DIAGRAM_CP437_FILES): obj/doc-temp/diagrams-cp437/%: doc/diagrams/%
	@echo $@
	@mkdir -p $(@D)
	@iconv -f utf8 -t cp437 $< > $@

$(LICENSE_DIAGRAM_CP437_FILES): obj/doc-temp/diagrams-cp437/%: obj/doc-temp/diagrams-license/%
	@echo $@
	@mkdir -p $(@D)
	@iconv -f utf8 -t cp437 $< > $@

# compiler

$(COMPILER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES) $(COMPILER_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/compiler.a: $(COMPILER_OBJ_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(AR) rcs $@ $(COMPILER_OBJ_FILES)

# shared

$(SHARED_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/shared.a: $(SHARED_OBJ_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(AR) rcs $@ $(SHARED_OBJ_FILES)

# resources

obj/helpfile.o: obj/help.h32
	@echo $@
	@mkdir -p $(@D)
	@cd obj && $(LD) -o ../obj/helpfile.o -r -b binary help.h32

$(RUNNERS_OBJ_FILES): obj/%.o: obj/%
	@echo $@
	@mkdir -p $(@D)
	@$(LD) -o $@ -r -b binary $<

# tmbasic

ifeq ($(TARGET_OS),win)
obj/Resources-win32.o: src/tmbasic/Resources-win32.rc obj/helpfile.o $(RUNNERS_BIN_FILES)
	@echo $@
	@$(WINDRES) -o $@ -i $< 
else
obj/Resources-win32.o: src/tmbasic/Resources-win32.rc obj/helpfile.o $(RUNNERS_BIN_FILES)
	@echo $@
	@echo -n > obj/Resources-win32.cpp && $(CXX) -c obj/Resources-win32.cpp -o $@
endif

$(TMBASIC_OBJ_FILES): obj/%.o: src/%.cpp \
		obj/common.h.gch \
		obj/helpfile.h \
		obj/help.h32 \
		$(COMPILER_H_FILES) \
		$(SHARED_H_FILES) \
		$(TMBASIC_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

bin/tmbasic$(EXE_EXTENSION): $(TMBASIC_OBJ_FILES) \
		obj/shared.a \
		obj/compiler.a \
		obj/common.h.gch \
		obj/helpfile.h \
		obj/help.h32 \
		$(LINUX_RESOURCE_OBJ_FILES) \
		obj/Resources-win32.o \
		$(RUNNERS_BIN_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) \
		-o $@ $(TMBASIC_OBJ_FILES) \
		$(CXXFLAGS) \
		$(TMBASIC_LDFLAGS) \
		$(STATIC_FLAG) \
		-include obj/common.h \
		obj/shared.a \
		obj/compiler.a \
		-ltvision \
		$(LINUX_RESOURCE_OBJ_FILES) \
		obj/Resources-win32.o \
		$(LDFLAGS)
	@$(STRIP) bin/tmbasic$(EXE_EXTENSION)

# test

$(TEST_OBJ_FILES): obj/%.o: src/%.cpp \
		obj/common.h.gch \
		obj/helpfile.h \
		obj/help.h32 \
		$(COMPILER_H_FILES) \
		$(SHARED_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

bin/test$(EXE_EXTENSION): $(TEST_OBJ_FILES) \
		obj/shared.a \
		obj/compiler.a \
		obj/common.h.gch \
		obj/helpfile.h \
		obj/help.h32 \
		$(LINUX_RESOURCE_OBJ_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) \
		-o $@ \
		$(CXXFLAGS) \
		$(TMBASIC_LDFLAGS) \
		-include obj/common.h \
		$(TEST_OBJ_FILES) \
		obj/shared.a \
		obj/compiler.a \
		-ltvision \
		$(LINUX_RESOURCE_OBJ_FILES) \
		$(LDFLAGS) \
		$(LIBGTEST_FLAG) \
		-lpthread

# runner (native platform)

$(RUNNER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES) $(RUNNER_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

bin/runner$(EXE_EXTENSION): $(RUNNER_OBJ_FILES) obj/shared.a obj/common.h.gch
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) \
		-o $@ \
		$(CXXFLAGS) \
		$(STATIC_FLAG) \
		-include obj/common.h \
		$(RUNNER_OBJ_FILES) \
		obj/shared.a \
		-ltvision \
		$(LDFLAGS)
	@$(STRIP) bin/runner$(EXE_EXTENSION)

# runners for full publish builds

$(RUNNERS_BIN_FILES): obj/%:
	@echo $@
	@mkdir -p $(@D)
	@touch $@
