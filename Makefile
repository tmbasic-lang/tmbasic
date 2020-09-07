ARCH=$(shell uname -m)
SRC_APP=$(wildcard src/app/*.cpp)

# for debugging, override with: make OPTFLAGS='-g -O0'
OPTFLAGS=-Os
CXXFLAGS=-Isrc -Iext/tvision/include -Wall -Werror -Wno-unknown-pragmas -Wno-reorder -static -std=c++17 $(OPTFLAGS)
LDFLAGS=-Lext/tvision/bin/$(ARCH) -lstdc++ -lgpm -lncursesw -ltinfo

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
	cd src && \
		find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i --style="{BasedOnStyle: Chromium, IndentWidth: 4, ColumnLimit: 120, SortIncludes: false, AlignAfterOpenBracket: AlwaysBreak, AlignOperands: false, Cpp11BracedListStyle: false, PenaltyReturnTypeOnItsOwnLine: 10000}"

bin/$(ARCH)/tmbasic: ext/tvision/bin/$(ARCH)/libtvision.a $(SRC_APP)
	@mkdir -p bin/$(ARCH)
	$(CXX) $(CXXFLAGS) -o $@ \
		$(SRC_APP) \
		ext/tvision/bin/$(ARCH)/libtvision.a \
		$(LDFLAGS)

ext/tvision/bin/$(ARCH)/Makefile:
	@mkdir -p ext/tvision/bin/$(ARCH)
	cd ext/tvision/bin/$(ARCH) && \
		CXXFLAGS="-I/code/ext/ncurses/$(ARCH)/ncurses/include" \
		cmake ../../

ext/tvision/bin/$(ARCH)/libtvision.a: ext/tvision/bin/$(ARCH)/Makefile
	cd ext/tvision/bin/$(ARCH) && \
		make -j 4
