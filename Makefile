# Define DISABLE_SANITIZERS to disable sanitizers in debug builds, otherwise they are enabled by default.
# Valgrind is incompatible with ASan.

OPTFLAGS ?= -g -O0
STRIP_TMBASIC ?= 0

# Sanitizers are extremely slow in WSL on Arm64. Disable sanitizers for now.
ifeq ($(TARGET_OS),linux)
ifeq ($(LINUX_TRIPLE),aarch64-linux-gnu)
ifeq ($(shell uname -r | grep -c WSL),1)
DISABLE_SANITIZERS=1
endif
endif
endif

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



### Commands ##########################################################################################################

STRIP=strip

# Toolchain: We use cross-compilation to build Windows and Linux binaries.
ifeq ($(TARGET_OS),win)
CC=$(ARCH)-w64-mingw32-gcc
CXX=$(ARCH)-w64-mingw32-g++
AR=$(ARCH)-w64-mingw32-ar
LD=$(ARCH)-w64-mingw32-ld
STRIP=$(ARCH)-w64-mingw32-strip
WINDRES=$(ARCH)-w64-mingw32-windres
endif

ifeq ($(TARGET_OS),linux)
ifeq ($(LINUX_DISTRO),alpine)
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
LDFLAGS += -lstdc++ -labsl_time -labsl_time_zone -labsl_int128 -labsl_raw_logging_internal -lmicrotar -lmpdec -lmpdec++ -lutf8proc

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
all: bin/tmbasic$(EXE_EXTENSION) bin/test$(EXE_EXTENSION) bin/runtime_$(TARGET_OS)_$(SHORT_ARCH).dat

.PHONY: help
help:
	@echo "Target: $(TARGET_OS) $(ARCH)"
	@echo "--------"
	@echo "make               Build TMBASIC"
ifeq ($(LINUX_DISTRO),ubuntu)
	@echo "make tidy          Check code with clang-tidy"
endif
	@echo ""

.PHONY: tidy
tidy: $(TIDY_TARGETS)



### Build targets #####################################################################################################



# tidy ----------------------------------------------------------------------------------------------------------------

$(TIDY_TARGETS): obj/tidy/%.tidy: src/%.cpp
	@echo "$<"
	@mkdir -p $(@D)
	@clang-tidy $< --quiet --fix --extra-arg=-Wno-unknown-warning-option -- $(CXXFLAGS) -DCLANG_TIDY | tee $@
	@touch $@



# precompiled header --------------------------------------------------------------------------------------------------

obj/common.h.gch: src/common.h
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -x c++-header src/common.h



# compiler ------------------------------------------------------------------------------------------------------------

$(COMPILER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES) $(VM_H_FILES) $(COMPILER_H_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/compiler.a: $(COMPILER_OBJ_FILES)
	@mkdir -p $(@D)
	$(AR) rcs $@ $(COMPILER_OBJ_FILES)



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

obj/resources/tzdb.o: $(PREFIX)/share/tzdb.tar
	@mkdir -p $(@D)
	cd "$(PREFIX)/share" && xxd -i tzdb.tar | sed s/tzdb_tar/kResourceTzdb/g > $(PWD)/obj/resources/kResourceTzdb.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c obj/resources/kResourceTzdb.cpp



# tmbasic -------------------------------------------------------------------------------------------------------------

$(TMBASIC_OBJ_FILES): obj/%.o: src/%.cpp \
		obj/common.h.gch \
		$(COMPILER_H_FILES) \
		$(SHARED_H_FILES) \
		$(TMBASIC_H_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

# icon resource
ifeq ($(TARGET_OS),win)
ICON_RES_OBJ_FILE=obj/tmbasic/AppWin.res.o
else
ICON_RES_OBJ_FILE=
endif

bin/tmbasic$(EXE_EXTENSION): $(TMBASIC_OBJ_FILES) \
		obj/shared.a \
		obj/compiler.a \
		obj/common.h.gch \
		$(ICON_RES_OBJ_FILE) \
		bin/runtime_$(TARGET_OS)_$(SHORT_ARCH).dat
	@mkdir -p $(@D)
	$(CXX) -o $@ $(TMBASIC_OBJ_FILES) $(CXXFLAGS) $(STATIC_FLAG) -include obj/common.h obj/compiler.a obj/shared.a obj/resources/tzdb.o $(ICON_RES_OBJ_FILE) $(TMBASIC_LDFLAGS) $(LDFLAGS)
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
		obj/resources/tzdb.o
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) $(STATIC_FLAG) -include obj/common.h $(TEST_OBJ_FILES) obj/compiler.a obj/vm.a obj/shared.a obj/resources/tzdb.o $(TMBASIC_LDFLAGS) $(LDFLAGS) $(LIBGTEST_FLAG) -lpthread



# runner --------------------------------------------------------------------------------------------------------------

$(RUNNER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES) $(VM_H_FILES) $(RUNNER_H_FILES)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) -c -include obj/common.h $<

obj/resources/pcode/pcode.o: %:
	@mkdir -p $(@D)
	head -c 1048576 /dev/zero | tr '\0' 'T' > obj/resources/pcode/pcode.dat
	xxd -i obj/resources/pcode/pcode.dat | sed s/obj_resources_pcode_pcode_dat/kResourcePcode/g > obj/resources/pcode/pcode.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c obj/resources/pcode/pcode.cpp

bin/runtime_$(TARGET_OS)_$(SHORT_ARCH).dat: obj/resources/pcode/pcode.o $(RUNNER_OBJ_FILES) obj/shared.a obj/vm.a obj/resources/tzdb.o
	@mkdir -p $(@D)
	$(CXX) -o $@ $(CXXFLAGS) $(STATIC_FLAG) -include obj/common.h $(RUNNER_OBJ_FILES) obj/resources/pcode/pcode.o obj/vm.a obj/shared.a obj/resources/tzdb.o $(LDFLAGS)
	$(STRIP) $@
	chmod -x $@
