ARCH=$(shell uname -m)
SRC_FILES=$(wildcard src/**/*.cpp) $(wildcard src/*.cpp)
INCLUDE_FILES=$(wildcard src/**/*.h) $(wildcard src/*.h)
DOC_FILES=$(wildcard doc/topics/*.txt) $(wildcard doc/procedures/*.txt) $(wildcard doc/html/*)
EXT_HEADER_FILES=$(wildcard ext/**/*.h)
TVISION_SRC_FILES=$(wildcard ext/tvision/source/**/*.cpp)

# for debugging, override with: make OPTFLAGS='-g -O0'
OPTFLAGS=-Os -flto
CXXFLAGS=-Isrc -Iobj/$(ARCH) -Iext/tvision/include -Iext/immer -Wall -Werror -Winvalid-pch -Wno-unknown-pragmas -Wno-reorder -static -std=c++17 $(OPTFLAGS)
LDFLAGS=-lstdc++ -lncursesw -ltinfo

.PHONY: all
all: bin/$(ARCH)/tmbasic

.PHONY: help
help:
	@echo ""
	@echo "Commands:"
	@echo "make"
	@echo "make clean"
	@echo "make format"
	@echo "make run"
	@echo "make valgrind"
	@echo ""

.PHONY: clean-doc
clean-doc:
	@rm -rf doc/output-html doc/output-txt doc/temp

.PHONY: clean
clean: clean-doc
	@rm -rf bin/$(ARCH) obj/$(ARCH) ext/tvision/bin/$(ARCH)

.PHONY: run
run: bin/$(ARCH)/tmbasic
	@cd bin/$(ARCH) && ./tmbasic

.PHONY: valgrind
valgrind: bin/$(ARCH)/tmbasic
	@valgrind --leak-check=full --show-leak-kinds=all --undef-value-errors=no --log-file=valgrind.txt bin/$(ARCH)/tmbasic

.PHONY: format
format:
	cd src && find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i --style="{BasedOnStyle: Chromium, IndentWidth: 4, ColumnLimit: 120, SortIncludes: false, AlignAfterOpenBracket: AlwaysBreak, AlignOperands: false, Cpp11BracedListStyle: false, PenaltyReturnTypeOnItsOwnLine: 10000}"

obj/$(ARCH)/common.h.gch: src/common.h $(EXT_HEADER_FILES)
	@mkdir -p obj/$(ARCH)
	$(CXX) $(CXXFLAGS) -x c++-header -o $@ src/common.h

obj/$(ARCH)/insert-cp437-diagram: doc/insert-cp437-diagram.c
	$(CC) -o obj/$(ARCH)/insert-cp437-diagram doc/insert-cp437-diagram.c

obj/$(ARCH)/helpfile.h obj/$(ARCH)/help.h32: doc/output-txt/help.txt ext/tvision/bin/$(ARCH)/libtvision.a
	@mkdir -p obj/$(ARCH)
	@rm -f obj/$(ARCH)/help.h32
	@rm -f obj/$(ARCH)/helpfile.h
	ext/tvision/bin/$(ARCH)/tvhc doc/output-txt/help.txt /code/obj/$(ARCH)/help.h32 obj/$(ARCH)/helpfile.h

bin/$(ARCH)/help.h32: obj/$(ARCH)/helpfile.h obj/$(ARCH)/help.h32
	@mkdir -p bin/$(ARCH)
	@cp -f obj/$(ARCH)/help.h32 bin/$(ARCH)/help.h32

doc/output-txt/help.txt: $(DOC_FILES) doc/build-doc.js obj/$(ARCH)/insert-cp437-diagram
	cd doc && node build-doc.js $(ARCH)

bin/$(ARCH)/tmbasic: ext/tvision/bin/$(ARCH)/libtvision.a obj/$(ARCH)/common.h.gch obj/$(ARCH)/helpfile.h bin/$(ARCH)/help.h32 $(SRC_FILES) $(INCLUDE_FILES)
	@mkdir -p bin/$(ARCH)
	$(CXX) $(CXXFLAGS) -include obj/$(ARCH)/common.h -o $@ $(SRC_FILES) ext/tvision/bin/$(ARCH)/libtvision.a $(LDFLAGS)

ext/tvision/bin/$(ARCH)/Makefile:
	@mkdir -p ext/tvision/bin/$(ARCH)
	cd ext/tvision/bin/$(ARCH) && CXXFLAGS="-I/code/ext/ncurses/$(ARCH)/ncurses/include -Wno-unused-result" cmake ../../

ext/tvision/bin/$(ARCH)/libtvision.a: ext/tvision/bin/$(ARCH)/Makefile $(TVISION_SRC_FILES)
	cd ext/tvision/bin/$(ARCH) && make -j 4
