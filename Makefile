# environment variable defaults
TARGET_OS ?= linux
BUILDCC ?= $(CC)
TVHC ?= tvhc
CMAKE ?= cmake
HELP_FILE_OBJ ?= obj/helpfile.o
STATIC_FLAG ?= -static
LIBMPDEC_FLAG ?= -lmpdec -lmpdec++
LIBNCURSESW_FLAG ?= -lncursesw
LIBGTEST_FLAG ?= -lgtest -lgtest_main
STRIP ?= strip
LICENSE_PROCESS_CMD ?= >/dev/null echo
OPTFLAGS ?= -Os -flto
TEST_CMD ?= ./test

CXXFLAGS=$(WIN_INCLUDE_FLAGS) $(MAC_INCLUDE_FLAGS) -Isrc -Iobj -Iext/nameof -I/usr/include/libmpdec -I/usr/include/libmpdec++ -Wall -Werror -Winvalid-pch -Wno-unknown-pragmas -Wno-reorder -std=c++17 $(OPTFLAGS) $(EXTRADEFS)
LDFLAGS=$(MAC_LD_FLAGS) -lstdc++ $(LIBNCURSESW_FLAG) $(LIBTINFO_FLAG) $(LIBMPDEC_FLAG)
CLANG_FORMAT_FLAGS=-i --style="{BasedOnStyle: Chromium, IndentWidth: 4, ColumnLimit: 120, SortIncludes: false, AlignAfterOpenBracket: AlwaysBreak, AlignOperands: false, Cpp11BracedListStyle: false, PenaltyReturnTypeOnItsOwnLine: 10000}"

COMPILER_SRC_FILES=$(shell find src/compiler -type f -name "*.cpp")
COMPILER_H_FILES=$(shell find src/compiler -type f -name "*.h")
COMPILER_OBJ_FILES=$(patsubst src/%,obj/%,$(COMPILER_SRC_FILES:.cpp=.o))
RUNNER_SRC_FILES=$(shell find src/runner -type f -name "*.cpp")
RUNNER_H_FILES=$(shell find src/runner -type f -name "*.h")
RUNNER_OBJ_FILES=$(patsubst src/%,obj/%,$(RUNNER_SRC_FILES:.cpp=.o))
SHARED_SRC_FILES=$(shell find src/shared -type f -name "*.cpp")
SHARED_H_FILES=$(shell find src/shared -type f -name "*.h")
SHARED_OBJ_FILES=$(patsubst src/%,obj/%,$(SHARED_SRC_FILES:.cpp=.o))
TEST_SRC_FILES=$(shell find src/test -type f -name "*.cpp")
TEST_H_FILES=$(shell find src/test -type f -name "*.h")
TEST_OBJ_FILES=$(patsubst src/%,obj/%,$(TEST_SRC_FILES:.cpp=.o))
TMBASIC_SRC_FILES=$(shell find src/tmbasic -type f -name "*.cpp")
TMBASIC_H_FILES=$(shell find src/tmbasic -type f -name "*.h")
TMBASIC_OBJ_FILES=$(patsubst src/%,obj/%,$(TMBASIC_SRC_FILES:.cpp=.o))

INCLUDE_FILES=$(shell find src -type f -name "*.h")

EXT_HEADER_FILES=$(shell find ext -type f -name "*.h")

DOC_FILES=$(shell find doc -type f -name "*.txt") $(shell find doc -type f -name "*.html")
DIAGRAM_SRC_FILES=$(shell find doc/diagrams -type f -name "*.txt")
DIAGRAM_CP437_FILES=$(patsubst doc/diagrams/%,obj/doc-temp/diagrams-cp437/%,$(DIAGRAM_SRC_FILES))
LICENSE_DIAGRAM_SRC_FILES=obj/doc-temp/diagrams-license/license_tmbasic.txt obj/doc-temp/diagrams-license/license_boost.txt obj/doc-temp/diagrams-license/license_musl.txt obj/doc-temp/diagrams-license/license_immer.txt obj/doc-temp/diagrams-license/license_libstdc++_gpl3.txt obj/doc-temp/diagrams-license/license_libstdc++_gcc1.txt obj/doc-temp/diagrams-license/license_libstdc++_gcc2.txt obj/doc-temp/diagrams-license/license_mpdecimal.txt obj/doc-temp/diagrams-license/license_nameof.txt obj/doc-temp/diagrams-license/license_ncurses.txt obj/doc-temp/diagrams-license/license_tvision.txt
LICENSE_DIAGRAM_CP437_FILES=$(patsubst obj/doc-temp/diagrams-license/%,obj/doc-temp/diagrams-cp437/%,$(LICENSE_DIAGRAM_SRC_FILES))

.PHONY: all
all: bin/tmbasic$(EXE_EXTENSION) bin/test$(EXE_EXTENSION) bin/runner$(EXE_EXTENSION) bin/LICENSE.txt obj/doc-html

.PHONY: help
help:
	@echo ""
	@echo "COMMANDS"
	@echo "--------"
	@echo "make                      Build TMBASIC"
	@echo "make run                  Run TMBASIC"
	@echo "make test                 Run tests"
	@echo "make valgrind             Run TMBASIC with valgrind (Linux)"
	@echo "make clean                Delete build outputs"
	@echo "make format               Reformat all code"
	@echo "make lint                 Check code with cpplint"
	@echo "make ghpages              Build tmbasic-gh-pages"
	@echo "make ghpages-test         Host the website on port 5000"
	@echo ""
	@echo "MAKE FLAGS"
	@echo "----------"
	@echo "OPTFLAGS='-g -O0'         Compile with debugging info"
	@echo "EXTRADEFS='-DDUMP_PARSE'  Dump parse tree"
	@echo ""

.PHONY: clean
clean:
	@rm -rf bin obj valgrind.txt

.PHONY: run
run:
	@bin/tmbasic

.PHONY: test
test: bin/test$(EXE_EXTENSION)
	@cd bin && $(TEST_CMD)

.PHONY: valgrind
valgrind: bin/tmbasic
	@valgrind --leak-check=full --show-leak-kinds=all --undef-value-errors=no --log-file=valgrind.txt bin/tmbasic

.PHONY: format
format:
	@cd src && find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format $(CLANG_FORMAT_FLAGS)
	@clang-format $(CLANG_FORMAT_FLAGS) build/scripts/buildDoc.cpp

.PHONY: lint
lint:
	@cpplint --quiet --recursive --linelength 120 --filter=-whitespace/indent,-readability/todo,-build/include_what_you_use,-legal/copyright,-readability/fn_size,-build/c++11 --repository=src src build/scripts/buildDoc.cpp

.PHONY: ghpages
ghpages: obj/help.txt bin/ghpages/index.html
	@mkdir -p bin/ghpages
	@cp obj/doc-html/* bin/ghpages/

.PHONY: ghpages-test
ghpages-test:
	@cd bin/ghpages && python3 -m http.server 5000

# ghpages

bin/ghpages/index.html: README.md doc/html/page-template-1.html doc/html/page-template-2.html doc/html/page-template-3.html
	@echo $@
	@mkdir -p $(@D)
	@cat doc/html/page-template-1.html > $@
	@echo "TMBASIC" >> $@
	@cat doc/html/page-template-2.html >> $@
	@pandoc --from=markdown --to=html $< >> $@
	@cat doc/html/page-template-3.html >> $@

# precompiled header

obj/common.h.gch: src/common.h $(EXT_HEADER_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -x c++-header -o $@ src/common.h

# help

obj/doc-html: obj/help.h32
	@echo $@
	@rm -rf obj/doc-html
	@cp -rf obj/doc-html bin/

bin/LICENSE.txt: LICENSE ext/boost/LICENSE_1_0.txt ext/immer/LICENSE ext/gcc/GPL-3 ext/gcc/copyright ext/mpdecimal/LICENSE.txt ext/nameof/LICENSE.txt ext/ncurses/COPYING ext/tvision/COPYRIGHT
	@echo $@
	@mkdir -p bin
	@rm -f $@
	@echo tmbasic license >> $@
	@echo =============== >> $@
	@echo >> $@
	@cat LICENSE >> $@
	@echo >> $@
	@echo boost license >> $@
	@echo ============= >> $@
	@echo >> $@
	@cat ext/boost/LICENSE_1_0.txt >> $@
	@echo >> $@
	@echo immer license >> $@
	@echo ============= >> $@
	@echo >> $@
	@cat ext/immer/LICENSE >> $@
	@echo >> $@
	@echo libstdc++ license >> $@
	@echo ================= >> $@
	@echo >> $@
	@cat ext/gcc/GPL-3 >> $@
	@cat ext/gcc/copyright >> $@
	@echo >> $@
	@echo mpdecimal license >> $@
	@echo ================= >> $@
	@echo >> $@
	@cat ext/mpdecimal/LICENSE.txt >> $@
	@echo >> $@
	@echo musl license >> $@
	@echo ================= >> $@
	@echo >> $@
	@cat ext/musl/COPYRIGHT >> $@
	@echo >> $@
	@echo nameof license >> $@
	@echo ============== >> $@
	@echo >> $@
	@cat ext/nameof/LICENSE.txt >> $@
	@echo >> $@
	@echo ncurses license >> $@
	@echo =============== >> $@
	@echo >> $@
	@cat ext/ncurses/COPYING >> $@
	@echo >> $@
	@echo tvision license >> $@
	@echo =============== >> $@
	@echo >> $@
	@cat ext/tvision/COPYRIGHT >> $@
	@echo >> $@
	@echo >> $@
	@echo == End == >> $@
	@$(LICENSE_PROCESS_CMD) $@

obj/doc-temp/diagrams-license/license_tmbasic.txt: LICENSE
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_boost.txt: ext/boost/LICENSE_1_0.txt
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_musl.txt: ext/musl/COPYRIGHT
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_immer.txt: ext/immer/LICENSE
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_libstdc++_gpl3.txt: ext/gcc/GPL-3
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_libstdc++_gcc1.txt: ext/gcc/copyright1
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_libstdc++_gcc2.txt: ext/gcc/copyright2
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_mpdecimal.txt: ext/mpdecimal/LICENSE.txt
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_nameof.txt: ext/nameof/LICENSE.txt
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_ncurses.txt: ext/ncurses/COPYING
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_tvision.txt: ext/tvision/COPYRIGHT
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/buildDoc: build/scripts/buildDoc.cpp
	@echo $@
	@mkdir -p obj
	@$(BUILDCC) $(BUILDDOC_WIN_INCLUDE_FLAGS) $(BUILDDOC_MAC_INCLUDE_FLAGS) -Iext/nameof -Wall -Werror -std=c++17 -o $@ $< -lstdc++

obj/helpfile.h: obj/help.txt
	@echo $@
	@mkdir -p obj
	@mkdir -p bin
	@rm -f obj/help.h32
	@rm -f obj/helpfile.h
	@$(TVHC) obj/help.txt obj/help.h32 obj/helpfile.h

obj/help.h32: obj/helpfile.h
	@echo $@

obj/help.txt: $(DOC_FILES) obj/buildDoc $(DIAGRAM_CP437_FILES) $(LICENSE_DIAGRAM_CP437_FILES) doc/html/page-template-1.html doc/html/page-template-2.html doc/html/page-template-3.html
	@echo $@
	@mkdir -p obj
	@cd doc && ../obj/buildDoc

obj/helpfile.o: obj/help.h32
	@echo $@
	@mkdir -p obj
	@cd obj && $(LD) -r -b binary -o ../obj/helpfile.o help.h32

$(DIAGRAM_CP437_FILES): obj/doc-temp/diagrams-cp437/%: doc/diagrams/%
	@echo $@
	@mkdir -p $(@D)
	@iconv -f utf8 -t cp437 $< > $@

$(LICENSE_DIAGRAM_CP437_FILES): obj/doc-temp/diagrams-cp437/%: obj/doc-temp/diagrams-license/%
	@echo $@
	@mkdir -p $(@D)
	@iconv -f utf8 -t cp437 $< > $@

# compiler

$(COMPILER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES) $(COMPILER_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

obj/compiler.a: $(COMPILER_OBJ_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(AR) rcs $@ $(COMPILER_OBJ_FILES)

# shared

$(SHARED_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

obj/shared.a: $(SHARED_OBJ_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(AR) rcs $@ $(SHARED_OBJ_FILES)

# tmbasic

$(TMBASIC_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch obj/helpfile.h obj/help.h32 $(COMPILER_H_FILES) $(SHARED_H_FILES) $(TMBASIC_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/tmbasic$(EXE_EXTENSION): $(TMBASIC_OBJ_FILES) obj/shared.a obj/compiler.a obj/common.h.gch obj/helpfile.h obj/help.h32 $(HELP_FILE_OBJ)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(MAC_HELP_FILE_LINK_FLAG) $(STATIC_FLAG) -include obj/common.h -o $@ $(TMBASIC_OBJ_FILES) obj/shared.a obj/compiler.a -ltvision $(HELP_FILE_OBJ) $(LDFLAGS)
	@$(STRIP) bin/tmbasic$(EXE_EXTENSION)

# test

$(TEST_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch obj/helpfile.h obj/help.h32 $(COMPILER_H_FILES) $(SHARED_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/test$(EXE_EXTENSION): $(TEST_OBJ_FILES) obj/shared.a obj/compiler.a obj/common.h.gch obj/helpfile.h obj/help.h32 $(HELP_FILE_OBJ)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(MAC_HELP_FILE_LINK_FLAG) -include obj/common.h -o $@ $(TEST_OBJ_FILES) obj/shared.a obj/compiler.a -ltvision $(HELP_FILE_OBJ) $(LDFLAGS) $(LIBGTEST_FLAG) -lpthread

# runner

$(RUNNER_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(SHARED_H_FILES) $(RUNNER_H_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/runner$(EXE_EXTENSION): $(RUNNER_OBJ_FILES) obj/shared.a obj/common.h.gch
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(STATIC_FLAG) -include obj/common.h -o $@ $(RUNNER_OBJ_FILES) obj/shared.a -ltvision $(LDFLAGS)
	@$(STRIP) bin/runner$(EXE_EXTENSION)
