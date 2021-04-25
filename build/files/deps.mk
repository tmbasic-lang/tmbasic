# set by caller: $(ARCH) $(TARGET_OS) $(TARGET_PREFIX) $(NATIVE_PREFIX) $(TARGET_COMPILER_PREFIX)
# $NATIVE_PREFIX/bin should be in the $PATH

BOOST_VERSION=1.76.0
BSDIFF_VERSION=b817e9491cf7b8699c8462ef9e2657ca4ccd7667
BZIP2_VERSION=6211b6500c8bec13a17707440d3a84ca8b34eed5
CMAKE_VERSION=3.20.1
FMT_VERSION=7.1.3
GOOGLETEST_VERSION=1.10.0
ICU_VERSION=69.1
IMMER_VERSION=39f86900ab2b077051ddb9a17213dbfac05584e0
LIBCLIPBOARD_VERSION=1.1
LIBXAU_VERSION=1.0.9
LIBXCB_VERSION=1.14
MPDECIMAL_VERSION=2.5.1
NAMEOF_VERSION=a9813bd7a15c92293384f2505e44bcb4005b97ba
NCURSES_VERSION=6.2
TURBO_VERSION=506457abf46ff467ba296fd774521c95a5c2540d
TVISION_VERSION=dd5da212548ae0af7ac4a05456fae3d8b5211b5c
XCBPROTO_VERSION=1.14
XORGPROTO_VERSION=2021.3

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

# these match the directory name in the tarballs that we download
BOOST_DIR=$(PWD)/boost_$(shell echo $(BOOST_VERSION) | tr '.' '_')
BSDIFF_DIR=$(PWD)/bsdiff-$(BSDIFF_VERSION)
BZIP2_DIR=$(PWD)/bzip2-$(BZIP2_VERSION)
FMT_DIR=$(PWD)/fmt-$(FMT_VERSION)
GOOGLETEST_DIR=$(PWD)/googletest-release-$(GOOGLETEST_VERSION)
ICU_DIR=$(PWD)/icu
IMMER_DIR=$(PWD)/immer-$(IMMER_VERSION)
LIBCLIPBOARD_DIR=$(PWD)/libclipboard-$(LIBCLIPBOARD_VERSION)
LIBXAU_DIR=$(PWD)/libXau-$(LIBXAU_VERSION)
LIBXCB_DIR=$(PWD)/libxcb-$(LIBXCB_VERSION)
MPDECIMAL_DIR=$(PWD)/mpdecimal-$(MPDECIMAL_VERSION)
NAMEOF_DIR=$(PWD)/nameof-$(NAMEOF_VERSION)
NCURSES_DIR=$(PWD)/ncurses-$(NCURSES_VERSION)
TURBO_DIR=$(PWD)/turbo-$(TURBO_VERSION)
TVISION_DIR=$(PWD)/tvision-$(TVISION_VERSION)
XCBPROTO_DIR=$(PWD)/xcb-proto-$(XCBPROTO_VERSION)
XORGPROTO_DIR=$(PWD)/xorgproto-$(XORGPROTO_VERSION)

ifeq ($(TARGET_OS),mac)
CMAKE_DIR=$(PWD)/cmake-$(CMAKE_VERSION)-macos-universal
else
CMAKE_DIR=$(PWD)/cmake-$(CMAKE_VERSION)-linux-$(shell uname -m)
endif

ifeq ($(TARGET_OS),win)
CMAKE_TOOLCHAIN_FLAG=-DCMAKE_TOOLCHAIN_FILE=/tmp/toolchain-cross-mingw64-linux-$(ARCH).cmake
HOST_FLAG=--host=$(ARCH)-w64-mingw32
else
CMAKE_TOOLCHAIN_FLAG=
HOST_FLAG=
endif

.PHONY: all
all: $(NCURSES_DIR)/install $(GOOGLETEST_DIR)/install $(BSDIFF_DIR)/install $(BZIP2_DIR)/install $(ICU_DIR)/install \
	$(FMT_DIR)/install $(LIBCLIPBOARD_DIR)/install $(IMMER_DIR)/install $(BOOST_DIR)/install $(MPDECIMAL_DIR)/install \
	$(TVISION_DIR)/install $(TURBO_DIR)/install $(NAMEOF_DIR)/install



# cmake ---------------------------------------------------------------------------------------------------------------

$(CMAKE_DIR)/install:
ifeq ($(TARGET_OS),mac)
	curl -L https://github.com/Kitware/CMake/releases/download/v$(CMAKE_VERSION)/cmake-$(CMAKE_VERSION)-macos-universal.tar.gz | tar zx
	ls -l $(CMAKE_DIR)/CMake.app/Contents/bin/*
	cp -rf $(CMAKE_DIR)/CMake.app/Contents/bin/* "$(NATIVE_PREFIX)/bin/"
	cp -rf $(CMAKE_DIR)/CMake.app/Contents/doc/* "$(NATIVE_PREFIX)/doc/"
	cp -rf $(CMAKE_DIR)/CMake.app/Contents/man/* "$(NATIVE_PREFIX)/man/"
	cp -rf $(CMAKE_DIR)/CMake.app/Contents/share/* "$(NATIVE_PREFIX)/share/"
else
ifneq ($(LINUX_DISTRO),alpine)
	curl -L https://github.com/Kitware/CMake/releases/download/v$(CMAKE_VERSION)/cmake-$(CMAKE_VERSION)-linux-$(shell uname -m).tar.gz \
		| tar zx --strip-components=1 -C $(NATIVE_PREFIX)
endif
endif
	touch $@



# ncurses -------------------------------------------------------------------------------------------------------------

$(NCURSES_DIR)/download:
	curl -L https://invisible-mirror.net/archives/ncurses/ncurses-$(NCURSES_VERSION).tar.gz | tar zx
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
$(NCURSES_DIR)/install: $(NCURSES_DIR)/download
	cd $(NCURSES_DIR) && \
		mkdir -p build && \
		cd build && \
		../configure && \
		$(MAKE) -C include && \
		$(MAKE) -C progs tic && \
		$(MAKE) -C progs infocmp
	cd $(NCURSES_DIR) && \
		./configure \
			--with-fallbacks=ansi,cons25,cons25-debian,dumb,hurd,linux,rxvt,screen,screen-256color,screen-256color-bce,screen-bce,screen-s,screen-w,screen.xterm-256color,tmux,tmux-256color,vt100,vt102,vt220,vt52,xterm,xterm-256color,xterm-color,xterm-mono,xterm-r5,xterm-r6,xterm-vt220,xterm-xfree86 \
			--disable-database \
			--without-ada \
			--without-tests \
			--disable-termcap \
			--disable-rpath-hack \
			--disable-stripping \
			--with-pkg-config-libdir=/usr/lib/pkgconfig \
			--without-cxx-binding \
			--with-terminfo-dirs="/usr/share/terminfo" \
			--enable-pc-files \
			--with-static \
			--enable-widec \
			--without-debug \
			--with-infocmp-path=$(NCURSES_DIR)/build/progs/infocmp \
			--with-tic-path=$(NCURSES_DIR)/build/progs/tic && \
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
	curl -L https://github.com/google/googletest/archive/release-$(GOOGLETEST_VERSION).tar.gz | tar zx
	touch $@

$(GOOGLETEST_DIR)/install: $(GOOGLETEST_DIR)/download $(CMAKE_DIR)/install
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



# bzip2 ---------------------------------------------------------------------------------------------------------------

$(BZIP2_DIR)/download:
	curl -L https://gitlab.com/federicomenaquintero/bzip2/-/archive/$(BZIP2_VERSION)/bzip2-$(BZIP2_VERSION).tar.gz | tar zx
	touch $@

$(BZIP2_DIR)/install: $(BZIP2_DIR)/download $(CMAKE_DIR)/install
	cd $(BZIP2_DIR) && \
		mkdir -p build-native && \
		cd build-native && \
		cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(NATIVE_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(NATIVE_PREFIX) \
			-DENABLE_STATIC_LIB=ON \
			-DCMAKE_BUILD_TYPE=Release && \
		$(MAKE) && \
		$(MAKE) install
ifeq ($(TARGET_OS),win)
	cd $(BZIP2_DIR) && \
		mkdir -p build-target && \
		cd build-target && \
		cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			-DENABLE_STATIC_LIB=ON \
			-DCMAKE_BUILD_TYPE=Release \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# bsdiff --------------------------------------------------------------------------------------------------------------

$(BSDIFF_DIR)/download:
	curl -L https://github.com/mendsley/bsdiff/archive/$(BSDIFF_VERSION).tar.gz | tar zx
	touch $@

$(BSDIFF_DIR)/install: $(BSDIFF_DIR)/download $(BZIP2_DIR)/install
	cd $(BSDIFF_DIR) && \
		gcc $(CFLAGS) -o bsdiff -isystem "$(NATIVE_PREFIX/include)" bsdiff.c -DBSDIFF_EXECUTABLE -L"$(NATIVE_PREFIX)/lib" -lbz2_static && \
		gcc $(CFLAGS) -o bspatch -isystem "$(NATIVE_PREFIX/include)" bspatch.c -DBSPATCH_EXECUTABLE -L"$(NATIVE_PREFIX)/lib" -lbz2_static && \
		mv bsdiff bspatch $(NATIVE_PREFIX)/bin/
	cd $(BSDIFF_DIR) && \
		$(TARGET_CC) $(CFLAGS) -isystem "$(TARGET_PREFIX/include)" -c bsdiff.c && \
		$(TARGET_AR) rcs libbsdiff.a bsdiff.o && \
		mv libbsdiff.a $(TARGET_PREFIX)/lib/ && \
		$(TARGET_CC) $(CFLAGS) -isystem "$(TARGET_PREFIX/include)" -c bspatch.c && \
		$(TARGET_AR) rcs libbspatch.a bspatch.o && \
		mv libbspatch.a $(TARGET_PREFIX)/lib/ && \
		mkdir -p $(TARGET_PREFIX)/include/bsdiff && \
		cp *.h $(TARGET_PREFIX)/include/bsdiff/
	touch $@



# icu -----------------------------------------------------------------------------------------------------------------

$(ICU_DIR)/download:
	curl -L https://github.com/unicode-org/icu/releases/download/release-$(shell echo $(ICU_VERSION) | tr '.' '-')/icu4c-$(shell echo $(ICU_VERSION) | tr '.' '_')-src.tgz  | tar zx
	curl -L -o $(ICU_DIR)/source/config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
	curl -L -o $(ICU_DIR)/source/config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
	touch $@

ifeq ($(TARGET_OS),mac)
ICU_PROFILE=MacOSX
ICU_CONFIGURE_FLAGS=--host=$(MACTRIPLE)
else
ICU_PROFILE=Linux/gcc
ICU_CONFIGURE_FLAGS=
endif

$(ICU_DIR)/install: $(ICU_DIR)/download
	cd $(ICU_DIR)/source && \
		mkdir -p build-native && \
		cd build-native && \
		CFLAGS="$(CFLAGS)" \
			CXXFLAGS="$(CFLAGS) -DU_USING_ICU_NAMESPACE=0 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1 -DU_HIDE_OBSOLETE_UTF_OLD_H=1 -std=c++17" \
			../runConfigureICU "$(ICU_PROFILE)" --enable-static --disable-shared --disable-tests --disable-samples \
			--with-data-packaging=static --prefix="$(NATIVE_PREFIX)" $(ICU_CONFIGURE_FLAGS)
ifeq ($(TARGET_OS),mac)
	cd $(ICU_DIR)/source && \
		cp -f config/mh-darwin config/mh-unknown
endif
	cd $(ICU_DIR)/source/build-native && \
		$(MAKE) && \
		$(MAKE) install
ifeq ($(TARGET_OS),win)
	cd $(ICU_DIR)/source && \
		mkdir -p build-win && \
		cd build-win && \
		CXXFLAGS="-DU_USING_ICU_NAMESPACE=0 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1 -DU_HIDE_OBSOLETE_UTF_OLD_H=1 -std=c++17" \
			LDFLAGS="-L$(ICU_DIR)/source/lib" \
			../runConfigureICU "MinGW" --enable-static --enable-shared --disable-tests --disable-samples \
			--with-data-packaging=static \
			--host=$(ARCH)-w64-mingw32 --with-cross-build=$(ICU_DIR)/source/build-linux --prefix="$(TARGET_PREFIX)" && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# fmt -----------------------------------------------------------------------------------------------------------------

$(FMT_DIR)/download:
	curl -L -o fmt.zip https://github.com/fmtlib/fmt/releases/download/$(FMT_VERSION)/fmt-$(FMT_VERSION).zip
	unzip -q fmt.zip
	touch $@

$(FMT_DIR)/install: $(FMT_DIR)/download $(CMAKE_DIR)/install
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
	curl -L https://github.com/jtanx/libclipboard/archive/refs/tags/v$(LIBCLIPBOARD_VERSION).tar.gz | tar zx
	touch $@

$(LIBCLIPBOARD_DIR)/install: $(LIBCLIPBOARD_DIR)/download $(CMAKE_DIR)/install $(LIBXAU_DIR)/install $(LIBXCB_DIR)/install
	cd $(LIBCLIPBOARD_DIR) && \
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



# immer ---------------------------------------------------------------------------------------------------------------

$(IMMER_DIR)/download:
	curl -L https://github.com/arximboldi/immer/archive/$(IMMER_VERSION).tar.gz | tar zx
	touch $@

$(IMMER_DIR)/install: $(IMMER_DIR)/download $(CMAKE_DIR)/install
	cd $(IMMER_DIR) && \
		rm -rf BUILD && \
		mkdir -p build && \
		cd build && \
		cmake .. \
			-DCMAKE_PREFIX_PATH=$(TARGET_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(TARGET_PREFIX) \
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		$(MAKE) install
	touch $@



# boost ---------------------------------------------------------------------------------------------------------------

$(BOOST_DIR)/download:
	curl -L https://dl.bintray.com/boostorg/release/$(BOOST_VERSION)/source/boost_$(shell echo $(BOOST_VERSION) | tr '.' '_').tar.gz | tar zx
	touch $@

$(BOOST_DIR)/install: $(BOOST_DIR)/download
	cd $(BOOST_DIR) && cp -rf boost $(NATIVE_PREFIX)/include/
ifeq ($(TARGET_OS),win)
	ln -s $(NATIVE_PREFIX)/include/boost $(TARGET_PREFIX)/include/boost
endif
	touch $@



# mpdecimal -----------------------------------------------------------------------------------------------------------

$(MPDECIMAL_DIR)/download:
	curl -L https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-$(MPDECIMAL_VERSION).tar.gz | tar zx
	curl -L -o $(MPDECIMAL_DIR)/config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
	curl -L -o $(MPDECIMAL_DIR)/config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
	touch $@

$(MPDECIMAL_DIR)/install: $(MPDECIMAL_DIR)/download
ifeq ($(TARGET_OS),mac)
	cd $(MPDECIMAL_DIR) && \
		CC="clang -arch $(MACARCH) -mmacosx-version-min=$(MACVER)" \
			CXX="clang++ -arch $(MACARCH) -mmacosx-version-min=$(MACVER)" \
			./configure --host=$(MACTRIPLE) "--prefix=$(TARGET_PREFIX)" && \
		$(MAKE) && \
		$(MAKE) install
else
	cd $(MPDECIMAL_DIR) && \
		./configure $(HOST_FLAG) --prefix=$(TARGET_PREFIX) && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# tvision -------------------------------------------------------------------------------------------------------------

$(TVISION_DIR)/download:
	curl -L https://github.com/magiblot/tvision/archive/$(TVISION_VERSION).tar.gz | tar zx
	touch $@

ifeq ($(TARGET_OS),mac)
TVISION_CXXFLAGS=-DTVISION_STL=1 -D__cpp_lib_string_view=1
else
TVISION_CXXFLAGS=
endif

$(TVISION_DIR)/install: $(TVISION_DIR)/download $(NCURSES_DIR)/install $(CMAKE_DIR)/install
	cd $(TVISION_DIR) && \
		mkdir -p build-native && \
		cd build-native/ && \
		CXXFLAGS="$(TVISION_CXXFLAGS)" cmake .. \
			$(CMAKE_FLAGS) \
			-DCMAKE_PREFIX_PATH=$(NATIVE_PREFIX) \
			-DCMAKE_INSTALL_PREFIX=$(NATIVE_PREFIX) \
			-DTV_BUILD_USING_GPM=OFF \
			-DCMAKE_BUILD_TYPE=Release && \
		$(MAKE) && \
		$(MAKE) install
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
	curl -L https://github.com/magiblot/turbo/archive/$(TURBO_VERSION).tar.gz | tar zx
	touch $@

ifeq ($(TARGET_OS),mac)
TURBO_CMAKE_FLAGS=-DCMAKE_EXE_LINKER_FLAGS="-framework ServiceManagement -framework Foundation -framework Cocoa"
endif

$(TURBO_DIR)/install: $(TURBO_DIR)/download $(TVISION_DIR)/install $(FMT_DIR)/install $(LIBCLIPBOARD_DIR)/install \
		$(CMAKE_DIR)/install $(NCURSES_DIR)/install
	cd $(TURBO_DIR) && \
		mv scintilla/lexers/LexBasic.cxx . && \
		rm -f scintilla/lexers/* && \
		mv -f LexBasic.cxx scintilla/lexers/LexBasic.cxx && \
		cat scintilla/src/Catalogue.cxx | sed 's:LINK_LEXER(lm.*::g; s:return 1;:LINK_LEXER(lmFreeBasic); return 1;:g' > Catalogue.cxx && \
		mv -f Catalogue.cxx scintilla/src/Catalogue.cxx && \
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
			$(CMAKE_TOOLCHAIN_FLAG) && \
		$(MAKE) && \
		cp -f *.a $(TARGET_PREFIX)/lib/ && \
		cp $(shell find $(TURBO_DIR) -name '*.h') $(TARGET_PREFIX)/include/
	touch $@



# xorgproto -----------------------------------------------------------------------------------------------------------

$(XORGPROTO_DIR)/download:
ifeq ($(TARGET_OS),linux)
	curl -L https://xorg.freedesktop.org/archive/individual/proto/xorgproto-${XORGPROTO_VERSION}.tar.gz | tar zx
else
	mkdir -p $(XORGPROTO_DIR)
endif
	touch $@

$(XORGPROTO_DIR)/install: $(XORGPROTO_DIR)/download
ifeq ($(TARGET_OS),linux)
	cd $(XORGPROTO_DIR) && \
		./configure && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# libXau --------------------------------------------------------------------------------------------------------------

$(LIBXAU_DIR)/download:
ifeq ($(TARGET_OS),linux)
	curl -L https://xorg.freedesktop.org/archive/individual/lib/libXau-${LIBXAU_VERSION}.tar.gz | tar zx
else
	mkdir -p $(LIBXAU_DIR)
endif
	touch $@

$(LIBXAU_DIR)/install: $(LIBXAU_DIR)/download $(XORGPROTO_DIR)/install
ifeq ($(TARGET_OS),linux)
	cd $(LIBXAU_DIR) && \
		./configure --enable-static && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# xcb-proto -----------------------------------------------------------------------------------------------------------

$(XCBPROTO_DIR)/download:
ifeq ($(TARGET_OS),linux)
	curl -L https://xorg.freedesktop.org/archive/individual/xcb/xcb-proto-${XCBPROTO_VERSION}.tar.gz | tar zx
else
	mkdir -p $(XCBPROTO_DIR)
endif
	touch $@

$(XCBPROTO_DIR)/install: $(LIBXAU_DIR)/install $(XCBPROTO_DIR)/download
ifeq ($(TARGET_OS),linux)
	cd $(XCBPROTO_DIR) && \
		./configure && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# libxcb --------------------------------------------------------------------------------------------------------------

$(LIBXCB_DIR)/download:
ifeq ($(TARGET_OS),linux)
	curl -L https://xorg.freedesktop.org/archive/individual/lib/libxcb-${LIBXCB_VERSION}.tar.gz | tar zx
else
	mkdir -p $(LIBXCB_DIR)
endif
	touch $@

$(LIBXCB_DIR)/install: $(XCBPROTO_DIR)/install $(LIBXCB_DIR)/download
ifeq ($(TARGET_OS),linux)
	cd $(LIBXCB_DIR) && \
		./configure --enable-static && \
		$(MAKE) && \
		$(MAKE) install
endif
	touch $@



# nameof --------------------------------------------------------------------------------------------------------------

$(NAMEOF_DIR)/download:
	curl -L https://github.com/Neargye/nameof/archive/$(NAMEOF_VERSION).tar.gz | tar zx
	touch $@

$(NAMEOF_DIR)/install: $(NAMEOF_DIR)/download
	cd $(NAMEOF_DIR)/include && cp -f nameof.hpp $(NATIVE_PREFIX)/include/
ifeq ($(TARGET_OS),win)
	ln -s $(NATIVE_PREFIX)/include/nameof.hpp $(TARGET_PREFIX)/include/nameof.hpp
endif
	touch $@
