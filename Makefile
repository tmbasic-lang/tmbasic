ARCH=$(shell uname -m)
SRC_APP=$(wildcard src/**/*.cpp) $(wildcard src/*.cpp)
INCLUDE_APP=$(wildcard src/**/*.h) $(wildcard src/*.h)

# for debugging, override with: make OPTFLAGS='-g -O0'
OPTFLAGS=-Os -flto
CXXFLAGS=-Isrc -Iext/tvision/include -Iext/immer -Wall -Werror -Winvalid-pch -Wno-unknown-pragmas -Wno-reorder -static -std=c++17 $(OPTFLAGS)
LDFLAGS=-Lext/tvision/bin/$(ARCH) -lstdc++ -lncursesw -ltinfo

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
	@rm -rf bin/$(ARCH) obj/$(ARCH) ext/tvision/bin/$(ARCH)

.PHONY: run
run: bin/$(ARCH)/tmbasic
	@bin/$(ARCH)/tmbasic

.PHONY: format
format:
	cd src && \
		find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i --style="{BasedOnStyle: Chromium, IndentWidth: 4, ColumnLimit: 120, SortIncludes: false, AlignAfterOpenBracket: AlwaysBreak, AlignOperands: false, Cpp11BracedListStyle: false, PenaltyReturnTypeOnItsOwnLine: 10000}"

obj/$(ARCH)/common.h.gch: src/common.h
	@mkdir -p obj/$(ARCH)
	$(CXX) $(CXXFLAGS) -x c++-header -o $@ src/common.h

bin/$(ARCH)/tmbasic: ext/tvision/bin/$(ARCH)/libtvision.a obj/$(ARCH)/common.h.gch $(SRC_APP) $(INCLUDE_APP)
	@mkdir -p bin/$(ARCH)
	$(CXX) $(CXXFLAGS) -include obj/$(ARCH)/common.h -o $@ \
		$(SRC_APP) \
		ext/tvision/bin/$(ARCH)/libtvision.a \
		$(LDFLAGS)

ext/tvision/bin/$(ARCH)/Makefile:
	@mkdir -p ext/tvision/bin/$(ARCH)
	cd ext/tvision/bin/$(ARCH) && \
		CXXFLAGS="-I/code/ext/ncurses/$(ARCH)/ncurses/include -Wno-unused-result" \
		cmake ../../

ext/tvision/bin/$(ARCH)/libtvision.a: ext/tvision/bin/$(ARCH)/Makefile
	cd ext/tvision/bin/$(ARCH) && \
		make -j 4
