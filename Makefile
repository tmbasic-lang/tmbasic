ARCH=$(shell uname -m)
SRC_APP=$(wildcard src/app/*.cpp)

# for debugging, override with: make OPTFLAGS='-g -O0'
OPTFLAGS=-Os
CXX=clang++
CXXFLAGS=-Isrc -Iext/tvision/include -Wall -Werror -Wno-unknown-pragmas -Wno-reorder-ctor -static -std=c++17 -stdlib=libc++ $(OPTFLAGS)
LDFLAGS=-Lext/tvision/bin/$(ARCH) -lgpm -lncursesw -ltermcap -lc++abi -pthread

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
	@rm -rf bin/$(ARCH) ext/tvision/bin/$(ARCH)

.PHONY: run
run: bin/$(ARCH)/tmbasic
	@bin/$(ARCH)/tmbasic

.PHONY: format
format:
	cd src ; find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i --style="{BasedOnStyle: Chromium, IndentWidth: 4, ColumnLimit: 120, SortIncludes: false, AlignAfterOpenBracket: AlwaysBreak, AlignOperands: false, Cpp11BracedListStyle: false, PenaltyReturnTypeOnItsOwnLine: 10000}"

bin/$(ARCH)/tmbasic: ext/tvision/bin/$(ARCH)/libtvision.a $(SRC_APP)
	@mkdir -p bin/$(ARCH)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_APP) ext/tvision/bin/$(ARCH)/libtvision.a $(LDFLAGS)

ext/tvision/bin/$(ARCH)/libtvision.a:
	@mkdir -p ext/tvision/bin/$(ARCH)
	cd ext/tvision/bin/$(ARCH) ; CXXFLAGS="-I/usr/include/ncursesw -stdlib=libc++" cmake ../../ ; make -j 2
