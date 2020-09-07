ARCH=$(shell uname -m)
SRC_APP=$(wildcard src/app/*.cpp)

# for debugging, override with: make OPTFLAGS='-g -O0'
OPTFLAGS=-Os
CXX=clang++
CXXFLAGS=-Isrc -Iext/tvision/include -Wall -Werror -Wno-unknown-pragmas -Wno-reorder-ctor -static -std=c++17 $(OPTFLAGS)
LDFLAGS=-Lext/tvision/bin/$(ARCH) -lgpm -lstdc++

.PHONY: all
all: bin/$(ARCH)/tmbasic

.PHONY: help
help:
	@echo ""
	@echo "Commands:"
	@echo "make all"
	@echo "make clean"
	@echo "make format"
	@echo "make run"
	@echo ""

.PHONY: clean
clean:
	@rm -rf bin/$(ARCH) ext/tvision/bin/$(ARCH) ext/llvm

.PHONY: run
run: bin/$(ARCH)/tmbasic
	@bin/$(ARCH)/tmbasic

.PHONY: format
format:
	cd src && \
		find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i --style="{BasedOnStyle: Chromium, IndentWidth: 4, ColumnLimit: 120, SortIncludes: false, AlignAfterOpenBracket: AlwaysBreak, AlignOperands: false, Cpp11BracedListStyle: false, PenaltyReturnTypeOnItsOwnLine: 10000}"

bin/$(ARCH)/tmbasic: ext/tvision/bin/$(ARCH)/libtvision.a ext/llvm/$(ARCH)/llvm-toolchain/build/lib/libc++.a ext/ncurses/$(ARCH)/ncurses/lib/libncursesw.a $(SRC_APP)
	@mkdir -p bin/$(ARCH)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_APP) ext/tvision/bin/$(ARCH)/libtvision.a ext/llvm/$(ARCH)/llvm-toolchain/build/lib/libc++.a ext/ncurses/$(ARCH)/ncurses/lib/libncursesw.a $(LDFLAGS)

#
# libc++
#

ext/llvm/$(ARCH)/llvm-toolchain/libcxx/CMakeLists.txt:
	@mkdir -p ext/llvm/$(ARCH)
	cd ext/llvm/$(ARCH) && \
		apt-get source llvm-toolchain-10 && \
		mv llvm-toolchain*/ llvm-toolchain

ext/llvm/$(ARCH)/llvm-toolchain/build/Makefile: ext/llvm/$(ARCH)/llvm-toolchain/libcxx/CMakeLists.txt
	cd ext/llvm/$(ARCH)/llvm-toolchain && \
		mkdir -p build && \
		cd build && \
		cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLIBCXX_CXX_ABI=libstdc++ -DLIBCXX_ENABLE_THREADS=OFF -DLIBCXX_ENABLE_SHARED=OFF ../libcxx

ext/llvm/$(ARCH)/llvm-toolchain/build/lib/libc++.a: ext/llvm/$(ARCH)/llvm-toolchain/build/Makefile
	cd ext/llvm/$(ARCH)/llvm-toolchain/build && \
		make -j 2

#
# tvision
#

ext/tvision/bin/$(ARCH)/Makefile: ext/llvm/$(ARCH)/llvm-toolchain/build/lib/libc++.a ext/ncurses/$(ARCH)/ncurses/lib/libncursesw.a
	@mkdir -p ext/tvision/bin/$(ARCH)
	cd ext/tvision/bin/$(ARCH) && \
		CXXFLAGS="-I/code/ext/ncurses/$(ARCH)/ncurses/include" LDFLAGS="/code/ext/llvm/$(ARCH)/llvm-toolchain/build/lib/libc++.a" cmake -DNCURSESW=/code/ext/ncurses/$(ARCH)/ncurses/lib/libncursesw.a ../../

ext/tvision/bin/$(ARCH)/libtvision.a: ext/tvision/bin/$(ARCH)/Makefile
	cd ext/tvision/bin/$(ARCH) && \
		make -j 2

#
# ncurses
#

ext/ncurses/$(ARCH)/ncurses/configure:
	@mkdir -p ext/ncurses/$(ARCH)
	cd ext/ncurses/$(ARCH) && \
		apt-get source ncurses && \
		mv ncurses*/ ncurses

ext/ncurses/$(ARCH)/ncurses/Makefile: ext/ncurses/$(ARCH)/ncurses/configure
	cd ext/ncurses/$(ARCH)/ncurses && \
		CC=clang CXX=clang++ ./configure --disable-db-install --disable-database --with-fallbacks=xterm-256color,xterm,vt100,linux --enable-ext-colors --with-build-cc=clang --without-tests --with-gpm --without-dlsym --enable-widec

ext/ncurses/$(ARCH)/ncurses/lib/libncursesw.a: ext/ncurses/$(ARCH)/ncurses/Makefile
	cd ext/ncurses/$(ARCH)/ncurses && \
		make -j 2
