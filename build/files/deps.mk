# Build dependencies on Linux and macOS.
# set by caller: $(ARCH) $(TARGET_OS) $(TARGET_PREFIX) $(NATIVE_PREFIX) $(TARGET_COMPILER_PREFIX) $(DOWNLOAD_DIR)
# $NATIVE_PREFIX/bin should be in the $PATH

ABSEIL_DIR=$(PWD)/abseil
BINUTILS_DIR=$(PWD)/binutils
CLI11_DIR=$(PWD)/cli11
FMT_DIR=$(PWD)/fmt
GOOGLETEST_DIR=$(PWD)/googletest
IMMER_DIR=$(PWD)/immer
LIBZIP_DIR=$(PWD)/libzip
LIEF_DIR=$(PWD)/lief
MICROTAR_DIR=$(PWD)/microtar
MPDECIMAL_DIR=$(PWD)/mpdecimal
NAMEOF_DIR=$(PWD)/nameof
NCURSES_DIR=$(PWD)/ncurses
TURBO_DIR=$(PWD)/turbo
TVISION_DIR=$(PWD)/tvision
TZDB_DIR=$(PWD)/tzdb
UTF8PROC_DIR=$(PWD)/utf8proc
ZLIB_DIR=$(PWD)/zlib

ifneq ($(ARCH),i686)
ifneq ($(ARCH),x86_64)
ifneq ($(ARCH),arm32v7)
ifneq ($(ARCH),arm64v8)
$(error Unknown ARCH '$(ARCH)')
endif
endif
endif
endif

ifneq ($(TARGET_OS),linux)
ifneq ($(TARGET_OS),mac)
$(error Unknown TARGET_OS '$(TARGET_OS)')
endif
endif

ifeq ($(TARGET_OS),linux)
ifneq ($(LINUX_DISTRO),alpine)
ifneq ($(LINUX_DISTRO),ubuntu)
$(error Unknown LINUX_DISTRO '$(LINUX_DISTRO)')
endif
endif
endif

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
ifeq ($(LINUX_DISTRO),alpine)
HOST_FLAG=--host=$(LINUX_TRIPLE)
CMAKE_TOOLCHAIN_FLAG=-DCMAKE_TOOLCHAIN_FILE=/tmp/cmake-toolchain-linux-$(ARCH).cmake
NATIVE_CC=clang
CC=clang --target=$(LINUX_TRIPLE) --sysroot=/target-sysroot
TARGET_CC=$(CC)
CXX=clang++ --target=$(LINUX_TRIPLE) --sysroot=/target-sysroot
LD=$(LINUX_TRIPLE)-ld
AR=$(LINUX_TRIPLE)-ar
TARGET_AR=$(AR)
RANLIB=$(LINUX_TRIPLE)-ranlib
else
CC=gcc
NATIVE_CC=gcc
TARGET_CC=$(CC)
CXX=g++
LD=ld
AR=ar
TARGET_AR=$(AR)
RANLIB=ranlib
endif
endif

ifeq ($(TARGET_OS),mac)
ifeq ($(ARCH),x86_64)
MACARCH=x86_64
MACVER=10.13
MACTRIPLE=x86_64-apple-macos10.13
endif
ifeq ($(ARCH),arm64v8)
MACARCH=arm64
MACVER=11.0
MACTRIPLE=arm64-apple-macos11
endif
CMAKE_FLAGS += -DCMAKE_OSX_ARCHITECTURES="$(MACARCH)" -DCMAKE_OSX_DEPLOYMENT_TARGET="$(MACVER)"
endif

ifeq ($(TARGET_OS),mac)
CFLAGS += -arch $(MACARCH) -mmacosx-version-min=$(MACVER)
endif

ifeq ($(TARGET_OS),mac)
CMAKE_DIR=$(PWD)/cmake-macos-universal
else
CMAKE_DIR=$(PWD)/cmake-linux-$(shell uname -m)
endif

NATIVE_CC ?= $(CC)

.PHONY: all
all: \
	$(ABSEIL_DIR)/install \
	$(CLI11_DIR)/install \
	$(FMT_DIR)/install \
	$(GOOGLETEST_DIR)/install \
	$(IMMER_DIR)/install \
	$(LIBZIP_DIR)/install \
	$(LIEF_DIR)/install \
	$(MICROTAR_DIR)/install \
	$(MPDECIMAL_DIR)/install \
	$(NAMEOF_DIR)/install \
	$(NCURSES_DIR)/install \
	$(TURBO_DIR)/install \
	$(TVISION_DIR)/install \
	$(TZDB_DIR)/install \
	$(UTF8PROC_DIR)/install \
	$(ZLIB_DIR)/install



# binutils ------------------------------------------------------------------------------------------------------------

$(BINUTILS_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/binutils-*.tar.gz
	mv -f binutils-*/ $(BINUTILS_DIR)/
	touch $@

$(BINUTILS_DIR)/install: $(BINUTILS_DIR)/download
ifeq ($(TARGET_OS),linux)
	cd $(BINUTILS_DIR) && \
	    ./configure --target=$(LINUX_TRIPLE) --enable-gprofng=no && \
	    $(MAKE) && \
	    $(MAKE) install
endif
	touch $@



# cmake ---------------------------------------------------------------------------------------------------------------

$(CMAKE_DIR)/install:
ifeq ($(TARGET_OS),mac)
	tar zxf $(DOWNLOAD_DIR)/cmake-mac-*.tar.gz
	mv -f cmake-*/ $(CMAKE_DIR)/
	cp -rf $(CMAKE_DIR)/CMake.app/Contents/bin/* "$(NATIVE_PREFIX)/bin/"
	cp -rf $(CMAKE_DIR)/CMake.app/Contents/doc/* "$(NATIVE_PREFIX)/doc/"
	cp -rf $(CMAKE_DIR)/CMake.app/Contents/man/* "$(NATIVE_PREFIX)/man/"
	cp -rf $(CMAKE_DIR)/CMake.app/Contents/share/* "$(NATIVE_PREFIX)/share/"
else
ifneq ($(LINUX_DISTRO),alpine)
	tar zx --strip-components=1 -f $(DOWNLOAD_DIR)/cmake-linux-glibc-$(shell uname -m)-*.tar.gz -C $(NATIVE_PREFIX)
endif
	mkdir -p $(@D)
endif
	touch $@



# ncurses -------------------------------------------------------------------------------------------------------------

$(NCURSES_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/ncurses-*.tar.gz
	mv -f ncurses-*/ $(NCURSES_DIR)/
	touch $@

ifeq ($(TARGET_OS),linux)
$(NCURSES_DIR)/install: $(NCURSES_DIR)/download $(BINUTILS_DIR)/install
	cd $(NCURSES_DIR) && \
		mkdir -p build && \
		cd build && \
		../configure \
			--without-ada \
			--without-tests \
			--disable-termcap \
			--disable-rpath-hack \
			--disable-stripping \
			--without-cxx-binding \
			--disable-pc-files \
			--with-static \
			--enable-widec \
			--without-debug && \
		$(MAKE) && \
		$(MAKE) install
	cd $(NCURSES_DIR) && \
		CC="$(CC)" \
			LD="$(LD)" \
			./configure \
			--host=$(LINUX_TRIPLE) \
			--prefix=/usr/local/$(LINUX_TRIPLE) \
			--with-fallbacks=ansi,cons25,cons25-debian,dumb,hurd,linux,rxvt,screen,screen-256color,screen-256color-bce,screen-bce,screen-s,screen-w,screen.xterm-256color,tmux,tmux-256color,vt100,vt102,vt220,vt52,xterm,xterm-256color,xterm-color,xterm-mono,xterm-r5,xterm-r6,xterm-vt220,xterm-xfree86 \
			--disable-database \
			--without-ada \
			--without-tests \
			--disable-termcap \
			--disable-rpath-hack \
			--disable-stripping \
			--without-cxx-binding \
			--with-terminfo-dirs="/usr/share/terminfo" \
			--disable-pc-files \
			--with-static \
			--enable-widec \
			--without-debug && \
		$(MAKE) && \
		$(MAKE) install
	touch $@
endif

ifeq ($(TARGET_OS),mac)
$(NCURSES_DIR)/install:
	mkdir -p $(@D)
	touch $@
endif



# googletest ----------------------------------------------------------------------------------------------------------

$(GOOGLETEST_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/googletest-*.tar.gz
	mv -f googletest-*/ $(GOOGLETEST_DIR)/
	touch $@

$(GOOGLETEST_DIR)/install: $(GOOGLETEST_DIR)/download $(CMAKE_DIR)/install $(BINUTILS_DIR)/install
	cd $(GOOGLETEST_DIR) && \
		mkdir -p build && \
		cd build && \
		cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			-DCMAKE_BUILD_TYPE=Release \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# fmt -----------------------------------------------------------------------------------------------------------------

$(FMT_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/fmt-*.tar.gz
	mv -f fmt-*/ $(FMT_DIR)/
	touch $@

$(FMT_DIR)/install: $(FMT_DIR)/download $(CMAKE_DIR)/install $(BINUTILS_DIR)/install
	cd $(FMT_DIR) && \
		mkdir -p build && \
		cd build && \
		cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_BUILD_TYPE=Release -DFMT_TEST=OFF -DFMT_FUZZ=OFF -DFMT_CUDA_TEST=OFF -DFMT_DOC=OFF \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# immer ---------------------------------------------------------------------------------------------------------------

$(IMMER_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/immer-*.tar.gz
	mv -f immer-*/ $(IMMER_DIR)/
	touch $@

$(IMMER_DIR)/install: $(IMMER_DIR)/download
	cp -rf $(IMMER_DIR)/immer $(TARGET_PREFIX)/include/
	touch $@



# mpdecimal -----------------------------------------------------------------------------------------------------------

$(MPDECIMAL_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/mpdecimal-*.tar.gz
	mv -f mpdecimal-*/ $(MPDECIMAL_DIR)/
	cp -f $(DOWNLOAD_DIR)/config.guess $(MPDECIMAL_DIR)/config.guess
	cp -f $(DOWNLOAD_DIR)/config.sub $(MPDECIMAL_DIR)/config.sub
	touch $@

$(MPDECIMAL_DIR)/install: $(MPDECIMAL_DIR)/download $(BINUTILS_DIR)/install
ifeq ($(TARGET_OS),mac)
	cd $(MPDECIMAL_DIR) && \
		CC="clang -arch $(MACARCH) -mmacosx-version-min=$(MACVER)" \
			CXX="clang++ -arch $(MACARCH) -mmacosx-version-min=$(MACVER)" \
			./configure --host=$(MACTRIPLE) "--prefix=$(TARGET_PREFIX)" --disable-shared
endif
ifeq ($(TARGET_OS),linux)
	cd $(MPDECIMAL_DIR) && \
		CC="$(CC)" CXX="$(CXX)" LD="$(LD)" AR="$(AR)" \
			./configure $(HOST_FLAG) --prefix=$(TARGET_PREFIX) --disable-shared
endif
	cd $(MPDECIMAL_DIR) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# tvision -------------------------------------------------------------------------------------------------------------

$(TVISION_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/tvision-*.tar.gz
	mv -f tvision-*/ $(TVISION_DIR)/
	touch $@

ifeq ($(TARGET_OS),mac)
TVISION_CXXFLAGS=-DTVISION_STL=1 -D__cpp_lib_string_view=1
endif

$(TVISION_DIR)/install-native: $(TVISION_DIR)/download $(NCURSES_DIR)/install $(CMAKE_DIR)/install
	cd $(TVISION_DIR) && \
		mkdir -p build-native && \
		cd build-native && \
		CXXFLAGS="$(TVISION_CXXFLAGS)" cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(NATIVE_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(NATIVE_PREFIX) \
			-DTV_BUILD_USING_GPM=OFF \
			-DCMAKE_BUILD_TYPE=Release && \
		$(MAKE) && \
		$(MAKE) install
	touch $@

$(TVISION_DIR)/install: $(TVISION_DIR)/install-native $(BINUTILS_DIR)/install
ifeq ($(TARGET_OS),linux)
	cd $(TVISION_DIR) && \
		mkdir -p build-target && \
		cd build-target && \
		CXXFLAGS="$(TVISION_CXXFLAGS)" cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			-DTV_BUILD_USING_GPM=OFF \
			-DCMAKE_BUILD_TYPE=Release \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# turbo ---------------------------------------------------------------------------------------------------------------

$(TURBO_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/turbo-*.tar.gz
	mv -f turbo-*/ $(TURBO_DIR)/
	touch $@

ifeq ($(TARGET_OS),mac)
TURBO_CMAKE_FLAGS=-DCMAKE_EXE_LINKER_FLAGS="-framework ServiceManagement -framework Foundation -framework Cocoa"
endif

$(TURBO_DIR)/install: $(TURBO_DIR)/download $(TVISION_DIR)/install \
		$(CMAKE_DIR)/install $(NCURSES_DIR)/install $(BINUTILS_DIR)/install
	cd $(TURBO_DIR) && \
		mkdir -p build && \
		cd build && \
		cmake .. \
			$(CMAKE_FLAGS) \
			$(TURBO_CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			-DCMAKE_BUILD_TYPE=Release \
			-DTURBO_USE_SYSTEM_TVISION=ON \
			-DTURBO_BUILD_APP=OFF \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# nameof --------------------------------------------------------------------------------------------------------------

$(NAMEOF_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/nameof-*.tar.gz
	mv -f nameof-*/ $(NAMEOF_DIR)/
	touch $@

$(NAMEOF_DIR)/install: $(NAMEOF_DIR)/download
	cd $(NAMEOF_DIR)/include && cp -f nameof.hpp $(NATIVE_PREFIX)/include/
ifeq ($(TARGET_OS),linux)
	ln -s $(NATIVE_PREFIX)/include/nameof.hpp $(TARGET_PREFIX)/include/nameof.hpp
endif
	touch $@



# zlib ----------------------------------------------------------------------------------------------------------------

$(ZLIB_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/zlib-*.tar.gz
	mv -f zlib-*/ $(ZLIB_DIR)/
	touch $@

ifeq ($(TARGET_OS),mac)
ZLIB_CONFIGURE_FLAGS=--archs="-arch $(MACARCH)"
endif

ifeq ($(TARGET_OS),linux)
ZLIB_CONFIGURE_ENV=AR="$(AR)" CC="$(CC)" RANLIB="$(RANLIB)"
endif

$(ZLIB_DIR)/install: $(ZLIB_DIR)/download $(BINUTILS_DIR)/install
	cd $(ZLIB_DIR) && \
		$(ZLIB_CONFIGURE_ENV) ./configure --static --prefix=$(TARGET_PREFIX) $(ZLIB_CONFIGURE_FLAGS) && \
		$(MAKE) CFLAGS="$(CFLAGS)" && \
		$(MAKE) install
	touch $@



# microtar ------------------------------------------------------------------------------------------------------------

$(MICROTAR_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/microtar-*.tar.gz
	mv -f microtar-*/ $(MICROTAR_DIR)/
	touch $@

$(MICROTAR_DIR)/install: $(MICROTAR_DIR)/install-native-mtar $(MICROTAR_DIR)/install-target-library

$(MICROTAR_DIR)/install-target-library: $(MICROTAR_DIR)/download $(BINUTILS_DIR)/install
	cd $(MICROTAR_DIR)/src && \
		$(TARGET_CC) $(CFLAGS) -isystem "$(TARGET_PREFIX)/include" -c microtar.c && \
		$(TARGET_AR) rcs libmicrotar.a microtar.o && \
		mv libmicrotar.a $(TARGET_PREFIX)/lib/ && \
		cp microtar.h $(TARGET_PREFIX)/include/
	touch $@

$(MICROTAR_DIR)/install-native-mtar: $(MICROTAR_DIR)/download $(BINUTILS_DIR)/install
	cp $(MICROTAR_DIR)/src/microtar.h $(NATIVE_PREFIX)/include/
	$(NATIVE_CC) -o $(NATIVE_PREFIX)/bin/mtar -isystem "$(NATIVE_PREFIX)/include" $(MICROTAR_DIR)/src/microtar.c $(PWD)/../mtar.c && \
	touch $@



# libzip --------------------------------------------------------------------------------------------------------------

$(LIBZIP_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/libzip-*.tar.gz
	mv -f libzip-*/ $(LIBZIP_DIR)/
	touch $@

$(LIBZIP_DIR)/install: $(LIBZIP_DIR)/download $(CMAKE_DIR)/install $(ZLIB_DIR)/install $(BINUTILS_DIR)/install
	cd $(LIBZIP_DIR) && \
		mkdir -p build && \
		cd build && \
		cmake .. \
			-DBUILD_SHARED_LIBS=OFF \
			-DENABLE_COMMONCRYPTO=OFF \
			-DENABLE_GNUTLS=OFF \
			-DENABLE_MBEDTLS=OFF \
			-DENABLE_OPENSSL=OFF \
			-DENABLE_WINDOWS_CRYPTO=OFF \
			-DENABLE_BZIP2=OFF \
			-DENABLE_LZMA=OFF \
			-DENABLE_ZSTD=OFF \
			-DBUILD_TOOLS=OFF \
			-DBUILD_REGRESS=OFF \
			-DBUILD_OSSFUZZ=OFF \
			-DBUILD_EXAMPLES=OFF \
			-DBUILD_DOC=OFF \
			$(CMAKE_FLAGS) \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			$(CMAKE_TOOLCHAIN_FLAG) \
			-DCMAKE_C_FLAGS="-Wno-incompatible-pointer-types" && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# cli11 ---------------------------------------------------------------------------------------------------------------

$(CLI11_DIR)/download:
	mkdir -p $(CLI11_DIR)
	mv -f $(DOWNLOAD_DIR)/cli11-*.hpp $(CLI11_DIR)/CLI11.hpp
	touch $@

$(CLI11_DIR)/install: $(CLI11_DIR)/download
	cp -f $(CLI11_DIR)/CLI11.hpp $(TARGET_PREFIX)/include/



# abseil --------------------------------------------------------------------------------------------------------------

$(ABSEIL_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/abseil-*.tar.gz
	mv -f abseil-*/ $(ABSEIL_DIR)/
	touch $@

$(ABSEIL_DIR)/install: $(ABSEIL_DIR)/download $(CMAKE_DIR)/install $(BINUTILS_DIR)/install
	cd $(ABSEIL_DIR) && \
		mkdir -p build && \
		cd build && \
		cmake .. \
			-DCMAKE_CXX_STANDARD=17 \
			-DABSL_PROPAGATE_CXX_STD=ON \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			-DCMAKE_BUILD_TYPE=Release \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# tzdb ----------------------------------------------------------------------------------------------------------------

$(TZDB_DIR)/download:
	mkdir -p $(TZDB_DIR)/
	tar zxf $(DOWNLOAD_DIR)/tzdata*.tar.gz -C $(TZDB_DIR)/
	tar zxf $(DOWNLOAD_DIR)/tzcode*.tar.gz -C $(TZDB_DIR)/
	touch $@

$(TZDB_DIR)/install: $(TZDB_DIR)/download $(BINUTILS_DIR)/install $(MICROTAR_DIR)/install
	cd $(TZDB_DIR) && \
		mkdir -p build && \
		$(MAKE) TOPDIR=build install
	cd $(TZDB_DIR)/build/usr/share/zoneinfo && \
		rm -f *.tab tzdata.zi && \
		rm -rf build
	mkdir -p $(TARGET_PREFIX)/share
	cd $(TZDB_DIR)/build/usr/share/zoneinfo && \
		find . -type f | xargs $(NATIVE_PREFIX)/bin/mtar $(TARGET_PREFIX)/share/tzdb.tar
	ls -l $(TARGET_PREFIX)/share/tzdb.tar
	touch $@



# utf8proc ------------------------------------------------------------------------------------------------------------

$(UTF8PROC_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/utf8proc-*.tar.gz
	mv -f utf8proc-*/ $(UTF8PROC_DIR)/
	touch $@

$(UTF8PROC_DIR)/install: $(UTF8PROC_DIR)/download $(BINUTILS_DIR)/install $(CMAKE_DIR)/install
	cd $(UTF8PROC_DIR) && \
		mkdir -p build && \
		cd build && \
		cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			-DBUILD_SHARED_LIBS=OFF \
			-DCMAKE_BUILD_TYPE=Release \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# lief ----------------------------------------------------------------------------------------------------------------

$(LIEF_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/lief-*.tar.gz
	mv -f LIEF-*/ $(LIEF_DIR)/
	if [ -f $(DOWNLOAD_DIR)/lief-ChainedPointerAnalysis.diff ]; then \
		patch -p1 -d $(LIEF_DIR)/include/LIEF/MachO < $(DOWNLOAD_DIR)/lief-ChainedPointerAnalysis.diff; \
	fi
	touch $@

$(LIEF_DIR)/install: $(LIEF_DIR)/download $(CMAKE_DIR)/install $(BINUTILS_DIR)/install
	cd $(LIEF_DIR) && \
		mkdir -p build && \
		cd build && \
		cmake \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			-DBUILD_SHARED_LIBS=OFF \
			-DLIEF_C_API=OFF \
			-DLIEF_EXAMPLES=OFF \
			-DLIEF_USE_CCACHE=OFF \
			-DLIEF_LOGGING=OFF \
			-DLIEF_LOGGING_DEBUG=OFF \
			-DLIEF_ENABLE_JSON=OFF \
			-DLIEF_DEX=OFF \
			-DLIEF_ART=OFF \
			-DCMAKE_BUILD_TYPE=Release \
			$(CMAKE_TOOLCHAIN_FLAG) \
			.. && \
		$(MAKE) && \
		$(MAKE) install
	touch $@
