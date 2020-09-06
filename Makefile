# for debugging, override with: make OPTFLAGS='-g -O0'
OPTFLAGS=-Os
CC=clang
CFLAGS=-Isrc -Wall -Werror -static $(OPTFLAGS)
ARCH=$(shell uname -m)

SRC_CLI=$(wildcard src/cli/*.c)

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
	@rm -rf bin dist ext/tvision/bin

.PHONY: run
run: bin/$(ARCH)/tmbasic
	@bin/$(ARCH)/tmbasic

.PHONY: format
format:
	cd src ; find ./ -type f \( -iname \*.h -o -iname \*.c \) | xargs clang-format -i --style="{BasedOnStyle: Chromium, IndentWidth: 4, ColumnLimit: 120, SortIncludes: false, AlignAfterOpenBracket: AlwaysBreak, AlignOperands: false, Cpp11BracedListStyle: false, PenaltyReturnTypeOnItsOwnLine: 10000}"

bin/$(ARCH)/tmbasic: ext/tvision/bin/$(ARCH)/libtvision.a $(SRC_CLI)
	@mkdir -p bin/$(ARCH)
	$(CC) $(CFLAGS) -o $@ $(SRC_CLI)

ext/tvision/bin/$(ARCH)/libtvision.a:
	@mkdir -p ext/tvision/bin/$(ARCH)
	cd ext/tvision/bin/$(ARCH) ; CXXFLAGS="-I/usr/include/ncursesw -std=c++17 -stdlib=libc++" cmake ../../ ; make -j 2
