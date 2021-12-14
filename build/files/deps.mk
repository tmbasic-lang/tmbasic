# set by caller: $(ARCH) $(TARGET_OS) $(TARGET_PREFIX) $(NATIVE_PREFIX) $(TARGET_COMPILER_PREFIX) $(DOWNLOAD_DIR)
# $NATIVE_PREFIX/bin should be in the $PATH

BINUTILS_DIR=$(PWD)/binutils
BOOST_DIR=$(PWD)/boost
FMT_DIR=$(PWD)/fmt
GOOGLETEST_DIR=$(PWD)/googletest
ICU_DIR=$(PWD)/icu
IMMER_DIR=$(PWD)/immer
LIBCLIPBOARD_DIR=$(PWD)/libclipboard
LIBXAU_DIR=$(PWD)/libXau
LIBXCB_DIR=$(PWD)/libxcb
LIBZIP_DIR=$(PWD)/libzip
MICROTAR_DIR=$(PWD)/microtar
MPDECIMAL_DIR=$(PWD)/mpdecimal
NAMEOF_DIR=$(PWD)/nameof
NCURSES_DIR=$(PWD)/ncurses
TURBO_DIR=$(PWD)/turbo
TVISION_DIR=$(PWD)/tvision
XCBPROTO_DIR=$(PWD)/xcbproto
XORGPROTO_DIR=$(PWD)/xorgproto
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
ifneq ($(TARGET_OS),win)
ifneq ($(TARGET_OS),mac)
$(error Unknown TARGET_OS '$(TARGET_OS)')
endif
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
CC=clang --target=$(LINUX_TRIPLE) --sysroot=/target-sysroot
TARGET_CC=$(CC)
CXX=clang++ --target=$(LINUX_TRIPLE) --sysroot=/target-sysroot
LD=$(LINUX_TRIPLE)-ld
AR=$(LINUX_TRIPLE)-ar
TARGET_AR=$(AR)
RANLIB=$(LINUX_TRIPLE)-ranlib
else
CC=gcc
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

ifeq ($(TARGET_OS),win)
EXE_EXTENSION=.exe
endif

ifeq ($(TARGET_OS),mac)
CMAKE_DIR=$(PWD)/cmake-$(CMAKE_VERSION)-macos-universal
else
CMAKE_DIR=$(PWD)/cmake-$(CMAKE_VERSION)-linux-$(shell uname -m)
endif

ifeq ($(TARGET_OS),win)
CMAKE_TOOLCHAIN_FLAG=-DCMAKE_TOOLCHAIN_FILE=/tmp/cmake-toolchain-win-$(ARCH).cmake
HOST_FLAG=--host=$(ARCH)-w64-mingw32
endif

.PHONY: all
all: \
	$(BOOST_DIR)/install \
	$(FMT_DIR)/install \
	$(GOOGLETEST_DIR)/install \
	$(ICU_DIR)/install \
	$(IMMER_DIR)/install \
	$(LIBCLIPBOARD_DIR)/install \
	$(LIBZIP_DIR)/install \
	$(MICROTAR_DIR)/install \
	$(MPDECIMAL_DIR)/install \
	$(NAMEOF_DIR)/install \
	$(NCURSES_DIR)/install \
	$(TURBO_DIR)/install \
	$(TVISION_DIR)/install \
	$(ZLIB_DIR)/install



# binutils ------------------------------------------------------------------------------------------------------------

$(BINUTILS_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/binutils-*.tar.gz
	mv -f binutils-*/ $(BINUTILS_DIR)/
	touch $@

$(BINUTILS_DIR)/install: $(BINUTILS_DIR)/download
ifeq ($(TARGET_OS),linux)
	cd $(BINUTILS_DIR) && \
	    ./configure --target=$(LINUX_TRIPLE) && \
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

ifeq ($(TARGET_OS),win)
$(NCURSES_DIR)/install: $(NCURSES_DIR)/download
	cd $(NCURSES_DIR) && \
		./configure \
			--host=$(ARCH)-w64-mingw32 \
			--without-ada \
			--with-static \
			--with-normal \
			--without-debug \
			--disable-relink \
			--disable-rpath \
			--with-ticlib \
			--without-termlib \
			--enable-widec \
			--enable-ext-colors \
			--enable-ext-mouse \
			--enable-sp-funcs \
			--with-wrap-prefix=ncwrap_ \
			--enable-sigwinch \
			--enable-term-driver \
			--enable-colorfgbg \
			--enable-tcap-names \
			--disable-termcap \
			--disable-mixed-case \
			--with-pkg-config \
			--enable-pc-files \
			--enable-echo \
			--with-build-cflags=-D_XOPEN_SOURCE_EXTENDED \
			--without-progs \
			--without-tests \
			--prefix=$(TARGET_PREFIX) \
			--without-cxx-binding \
			--disable-home-terminfo \
			--enable-interop && \
		$(MAKE) && \
		$(MAKE) install
	touch $@
endif

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



# icu -----------------------------------------------------------------------------------------------------------------

$(ICU_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/icu-*.tar.gz
	cp -f $(DOWNLOAD_DIR)/config.guess $(ICU_DIR)/source/config.guess
	cp -f $(DOWNLOAD_DIR)/config.sub $(ICU_DIR)/source/config.sub
	touch $@

ifeq ($(TARGET_OS),mac)
ICU_PROFILE=MacOSX
ICU_CONFIGURE_FLAGS=--host=$(MACTRIPLE)
else
ICU_PROFILE=Linux/gcc
ICU_CONFIGURE_FLAGS=
endif

ICU_BUILD_NATIVE=0
ifeq ($(TARGET_OS),linux)
ifeq ($(LINUX_DISTRO),ubuntu)
ICU_BUILD_NATIVE=1
endif
endif
ifneq ($(TARGET_OS),linux)
ICU_BUILD_NATIVE=1
endif

$(ICU_DIR)/install-native: $(ICU_DIR)/download
ifeq ($(ICU_BUILD_NATIVE),1)
	cd $(ICU_DIR)/source && \
		mkdir -p build-native && \
		cd build-native && \
		CFLAGS="$(CFLAGS)" \
			CXXFLAGS="$(CFLAGS) -DU_USING_ICU_NAMESPACE=0 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1 -DU_HIDE_OBSOLETE_UTF_OLD_H=1 -std=c++17" \
			../runConfigureICU "$(ICU_PROFILE)" --enable-static --disable-shared --disable-tests --disable-samples \
			--with-data-packaging=static --prefix="$(NATIVE_PREFIX)" $(ICU_CONFIGURE_FLAGS)
endif
ifeq ($(TARGET_OS),mac)
	cd $(ICU_DIR)/source && \
		cp -f config/mh-darwin config/mh-unknown
endif
ifeq ($(ICU_BUILD_NATIVE),1)
	cd $(ICU_DIR)/source/build-native && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@

$(ICU_DIR)/install: $(ICU_DIR)/install-native $(BINUTILS_DIR)/install
ifeq ($(TARGET_OS),win)
	cd $(ICU_DIR)/source && \
		mkdir -p build-win && \
		cd build-win && \
		CXXFLAGS="-DU_USING_ICU_NAMESPACE=0 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1 -DU_HIDE_OBSOLETE_UTF_OLD_H=1 -std=c++17" \
			LDFLAGS="-L$(ICU_DIR)/source/lib" \
			../runConfigureICU "MinGW" --enable-static --enable-shared --disable-tests --disable-samples \
			--with-data-packaging=static \
			--host=$(ARCH)-w64-mingw32 --with-cross-build=$(ICU_DIR)/source/build-native --prefix="$(TARGET_PREFIX)" && \
		$(MAKE) && \
		$(MAKE) install
endif
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



# libclipboard --------------------------------------------------------------------------------------------------------

$(LIBCLIPBOARD_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/libclipboard-*.tar.gz
	mv -f libclipboard-*/ $(LIBCLIPBOARD_DIR)/
	touch $@

$(LIBCLIPBOARD_DIR)/install: $(LIBCLIPBOARD_DIR)/download $(CMAKE_DIR)/install $(LIBXAU_DIR)/install \
		$(LIBXCB_DIR)/install $(BINUTILS_DIR)/install
ifneq ($(TARGET_OS),mac)
	cd $(LIBCLIPBOARD_DIR) && \
		dos2unix CMakeLists.txt && \
		patch CMakeLists.txt /tmp/libclipboard-CMakeLists.txt.diff
endif
	cd $(LIBCLIPBOARD_DIR) && \
		mkdir -p build && \
		cd build && \
		PKG_CONFIG_PATH="/usr/local/$(LINUX_TRIPLE)/lib/pkgconfig/:/usr/local/$(LINUX_TRIPLE)/share/pkgconfig/" \
			cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_BUILD_TYPE=Release \
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

$(IMMER_DIR)/install: $(IMMER_DIR)/download $(CMAKE_DIR)/install $(BINUTILS_DIR)/install
	cd $(IMMER_DIR) && \
		rm -rf BUILD && \
		mkdir -p build && \
		cd build && \
		cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# boost ---------------------------------------------------------------------------------------------------------------

$(BOOST_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/boost-*.tar.gz
	mv -f boost_*/ $(BOOST_DIR)/
	touch $@

$(BOOST_DIR)/install: $(BOOST_DIR)/download
	cd $(BOOST_DIR) && cp -rf boost $(NATIVE_PREFIX)/include/
ifeq ($(TARGET_OS),win)
	ln -s $(NATIVE_PREFIX)/include/boost $(TARGET_PREFIX)/include/boost
endif
ifeq ($(TARGET_OS),linux)
	ln -s $(NATIVE_PREFIX)/include/boost $(TARGET_PREFIX)/include/boost
endif
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
ifeq ($(TARGET_OS),win)
	cd $(MPDECIMAL_DIR) && \
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
ifeq ($(TARGET_OS),win)
	cd $(TVISION_DIR) && \
		mkdir -p build-win && \
		cd build-win && \
		cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
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

$(TURBO_DIR)/install: $(TURBO_DIR)/download $(TVISION_DIR)/install $(FMT_DIR)/install $(LIBCLIPBOARD_DIR)/install \
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
			-DTURBO_USE_SYSTEM_DEPS=ON \
			-DTURBO_BUILD_APP=OFF \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# xorgproto -----------------------------------------------------------------------------------------------------------

$(XORGPROTO_DIR)/download:
ifeq ($(TARGET_OS),linux)
	tar zxf $(DOWNLOAD_DIR)/xorgproto-*.tar.gz
	mv -f xorgproto-*/ $(XORGPROTO_DIR)/
else
	mkdir -p $(XORGPROTO_DIR)
endif
	touch $@

$(XORGPROTO_DIR)/install: $(XORGPROTO_DIR)/download $(BINUTILS_DIR)/install
ifeq ($(TARGET_OS),linux)
	cd $(XORGPROTO_DIR) && \
		CC="$(CC)" CXX="$(CXX)" LD="$(LD)" AR="$(AR)" \
			./configure $(HOST_FLAG) --prefix=$(TARGET_PREFIX) && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# libXau --------------------------------------------------------------------------------------------------------------

$(LIBXAU_DIR)/download:
ifeq ($(TARGET_OS),linux)
	tar zxf $(DOWNLOAD_DIR)/libXau-*.tar.gz
	mv -f libXau-*/ $(LIBXAU_DIR)/
else
	mkdir -p $(LIBXAU_DIR)
endif
	touch $@

$(LIBXAU_DIR)/install: $(LIBXAU_DIR)/download $(XORGPROTO_DIR)/install $(BINUTILS_DIR)/install
ifeq ($(TARGET_OS),linux)
	cd $(LIBXAU_DIR) && \
		CC="$(CC)" CXX="$(CXX)" LD="$(LD)" AR="$(AR)" PKG_CONFIG_PATH="/usr/local/$(LINUX_TRIPLE)/share/pkgconfig/" \
			./configure $(HOST_FLAG) --prefix=$(TARGET_PREFIX) --enable-static && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# xcb-proto -----------------------------------------------------------------------------------------------------------

$(XCBPROTO_DIR)/download:
ifeq ($(TARGET_OS),linux)
	tar zxf $(DOWNLOAD_DIR)/xcbproto-*.tar.gz
	mv -f xcbproto-*/ $(XCBPROTO_DIR)/
else
	mkdir -p $(XCBPROTO_DIR)
endif
	touch $@

$(XCBPROTO_DIR)/install: $(LIBXAU_DIR)/install $(XCBPROTO_DIR)/download $(BINUTILS_DIR)/install
ifeq ($(TARGET_OS),linux)
	cd $(XCBPROTO_DIR) && \
		./autogen.sh && \
		CC="$(CC)" CXX="$(CXX)" LD="$(LD)" AR="$(AR)" \
			./configure $(HOST_FLAG) --prefix=$(TARGET_PREFIX) && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# libxcb --------------------------------------------------------------------------------------------------------------

$(LIBXCB_DIR)/download:
ifeq ($(TARGET_OS),linux)
	tar zxf $(DOWNLOAD_DIR)/libxcb-*.tar.gz
	mv -f libxcb-*/ $(LIBXCB_DIR)/
else
	mkdir -p $(LIBXCB_DIR)
endif
	touch $@

$(LIBXCB_DIR)/install: $(XCBPROTO_DIR)/install $(LIBXCB_DIR)/download $(BINUTILS_DIR)/install
ifeq ($(TARGET_OS),linux)
	cd $(LIBXCB_DIR) && \
		CC="$(CC)" CXX="$(CXX)" LD="$(LD)" AR="$(AR)" PKG_CONFIG_PATH="/usr/local/$(LINUX_TRIPLE)/lib/pkgconfig/:/usr/local/$(LINUX_TRIPLE)/share/pkgconfig/" \
			./configure $(HOST_FLAG) --prefix=$(TARGET_PREFIX) --enable-static --disable-shared && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# nameof --------------------------------------------------------------------------------------------------------------

$(NAMEOF_DIR)/download:
	tar zxf $(DOWNLOAD_DIR)/nameof-*.tar.gz
	mv -f nameof-*/ $(NAMEOF_DIR)/
	touch $@

$(NAMEOF_DIR)/install: $(NAMEOF_DIR)/download
	cd $(NAMEOF_DIR)/include && cp -f nameof.hpp $(NATIVE_PREFIX)/include/
ifeq ($(TARGET_OS),win)
	ln -s $(NATIVE_PREFIX)/include/nameof.hpp $(TARGET_PREFIX)/include/nameof.hpp
endif
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

ifeq ($(TARGET_OS),win)
ZLIB_CONFIGURE_ENV=AR=$(ARCH)-w64-mingw32-ar CC=$(ARCH)-w64-mingw32-gcc RANLIB=$(ARCH)-w64-mingw32-ranlib
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

$(MICROTAR_DIR)/install: $(MICROTAR_DIR)/download $(BINUTILS_DIR)/install
	cd $(MICROTAR_DIR)/src && \
		$(TARGET_CC) $(CFLAGS) -isystem "$(TARGET_PREFIX)/include" -c microtar.c && \
		$(TARGET_AR) rcs libmicrotar.a microtar.o && \
		mv libmicrotar.a $(TARGET_PREFIX)/lib/ && \
		cp microtar.h $(TARGET_PREFIX)/include/
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
			-DBUILD_EXAMPLES=OFF \
			-DBUILD_DOC=OFF \
			$(CMAKE_FLAGS) \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@
