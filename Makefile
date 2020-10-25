ARCH=$(shell uname -m)
CORE_SRC_FILES=$(shell find src/core -type f -name "*.cpp")
CORE_OBJ_FILES=$(patsubst src/%,obj/%,$(CORE_SRC_FILES:.cpp=.o))
TMBASIC_SRC_FILES=$(shell find src/tmbasic -type f -name "*.cpp")
TMBASIC_OBJ_FILES=$(patsubst src/%,obj/%,$(TMBASIC_SRC_FILES:.cpp=.o))
TEST_SRC_FILES=$(shell find src/test -type f -name "*.cpp")
TEST_OBJ_FILES=$(patsubst src/%,obj/%,$(TEST_SRC_FILES:.cpp=.o))
INCLUDE_FILES=$(shell find src -type f -name "*.h")
DOC_FILES=$(shell find doc -type f -name "*.txt") $(shell find doc -type f -name "*.html")
EXT_HEADER_FILES=$(shell find ext -type f -name "*.h")
TVISION_SRC_FILES=$(shell find ext/tvision -type f -name "*.cpp") $(shell find ext/tvision -type f -name "*.h")

BUILDCC ?= $(CC)
TVHC ?= obj/tvision/tvhc
TINFO_LIB_FLAG ?= -ltinfo

# for debugging, override with: OPTFLAGS='-g -O0'
# to dump AST parse trees, override with: EXTRADEFS='-DDUMP_AST'
OPTFLAGS=-Os -flto
CXXFLAGS=$(WIN_INCLUDE_FLAGS) -Isrc -Iobj -Iext/tvision/include -Iext/immer -Iext/nameof -I/usr/include/libmpdec -I/usr/include/libmpdec++ -Wall -Werror -Winvalid-pch -Wno-unknown-pragmas -Wno-reorder -std=c++17 $(OPTFLAGS) $(EXTRADEFS)
LDFLAGS=-lstdc++ -lncursesw $(TINFO_LIB_FLAG) -lmpdec -lmpdec++

.PHONY: all
all: bin/tmbasic bin/test bin/LICENSE.txt

.PHONY: help
help:
	@echo ""
	@echo "Commands:"
	@echo "make"
	@echo "make clean"
	@echo "make format"
	@echo "make run"
	@echo "make test"
	@echo "make valgrind"
	@echo "make win"
	@echo ""

.PHONY: clean
clean:
	@rm -rf bin obj obj/tvision valgrind.txt doc/diagrams/license*.txt

.PHONY: run
run: bin/tmbasic
	@bin/tmbasic

.PHONY: test
test: bin/test
	@cd bin && ./test

.PHONY: valgrind
valgrind: bin/tmbasic
	@valgrind --leak-check=full --show-leak-kinds=all --undef-value-errors=no --log-file=valgrind.txt bin/tmbasic

.PHONY: format
format:
	@cd src && find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i --style="{BasedOnStyle: Chromium, IndentWidth: 4, ColumnLimit: 120, SortIncludes: false, AlignAfterOpenBracket: AlwaysBreak, AlignOperands: false, Cpp11BracedListStyle: false, PenaltyReturnTypeOnItsOwnLine: 10000}"

.PHONY: win
win:
	@BUILDCC=gcc \
		CC=x86_64-w64-mingw32-gcc \
		CXX=x86_64-w64-mingw32-g++ \
		AR=x86_64-w64-mingw32-ar \
		HOSTFLAGS="--host=x86_64-w64-mingw32" \
		WIN_INCLUDE_FLAGS="-I/usr/share/mingw-w64/include/" \
		TVHC="WINEPATH=/usr/lib/gcc/x86_64-w64-mingw32/9.3-win32 wine64 obj/tvision/tvhc.exe" \
		TINFO_LIB_FLAG="" \
		$(MAKE) bin/tmbasic bin/LICENSE.txt

# shared

obj/common.h.gch: src/common.h $(EXT_HEADER_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -x c++-header -o $@ src/common.h

# help

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

doc/diagrams/license_tmbasic.txt: LICENSE
	@echo $@
	@cp -f $< $@

doc/diagrams/license_boost.txt: ext/boost/LICENSE_1_0.txt
	@echo $@
	@cp -f $< $@

doc/diagrams/license_immer.txt: ext/immer/LICENSE
	@echo $@
	@cp -f $< $@

doc/diagrams/license_libstdc++_gpl3.txt: ext/gcc/GPL-3
	@echo $@
	@cp -f $< $@

doc/diagrams/license_libstdc++_gcc1.txt: ext/gcc/copyright1
	@echo $@
	@cp -f $< $@

doc/diagrams/license_libstdc++_gcc2.txt: ext/gcc/copyright2
	@echo $@
	@cp -f $< $@

doc/diagrams/license_mpdecimal.txt: ext/mpdecimal/LICENSE.txt
	@echo $@
	@cp -f $< $@

doc/diagrams/license_nameof.txt: ext/nameof/LICENSE.txt
	@echo $@
	@cp -f $< $@

doc/diagrams/license_ncurses.txt: ext/ncurses/COPYING
	@echo $@
	@cp -f $< $@

doc/diagrams/license_tvision.txt: ext/tvision/COPYRIGHT
	@echo $@
	@cp -f $< $@

doc/diagrams/license_notoserif.txt: ext/notoserif/OFL.txt
	@echo $@
	@cp -f $< $@

doc/diagrams/license_opensans.txt: ext/opensans/LICENSE.txt
	@echo $@
	@cp -f $< $@

doc/diagrams/license_oxygenmono.txt: ext/oxygenmono/OFL.txt
	@echo $@
	@cp -f $< $@

obj/insert-cp437-diagram: doc/insert-cp437-diagram.c
	@echo $@
	@mkdir -p obj
	@$(BUILDCC) -o $@ doc/insert-cp437-diagram.c

doc/diagrams/license.txt: bin/LICENSE.txt
	@echo $@
	@mkdir -p obj
	@cp -f $< $@

obj/helpfile.h: obj/doc-txt/help.txt obj/tvision/libtvision.a
	@echo $@
	@mkdir -p obj
	@mkdir -p bin
	@rm -f obj/help.h32
	@rm -f obj/helpfile.h
	@$(TVHC) obj/doc-txt/help.txt obj/help.h32 obj/helpfile.h

obj/help.h32: obj/helpfile.h
	@echo $@

obj/doc-txt/help.txt: $(DOC_FILES) doc/build-doc.js obj/insert-cp437-diagram doc/diagrams/license_tmbasic.txt doc/diagrams/license_boost.txt doc/diagrams/license_immer.txt doc/diagrams/license_libstdc++_gpl3.txt doc/diagrams/license_libstdc++_gcc1.txt doc/diagrams/license_libstdc++_gcc2.txt doc/diagrams/license_mpdecimal.txt doc/diagrams/license_nameof.txt doc/diagrams/license_ncurses.txt doc/diagrams/license_tvision.txt doc/diagrams/license_notoserif.txt doc/diagrams/license_opensans.txt doc/diagrams/license_oxygenmono.txt
	@echo $@
	@mkdir -p obj
	@cd doc && node build-doc.js

obj/helpfile.o: obj/help.h32
	@echo $@
	@mkdir -p obj
	@cd obj && $(LD) -r -b binary -o ../obj/helpfile.o help.h32

# tvision

obj/tvision/Makefile:
	@echo $@
	@mkdir -p $(@D)
	@cd obj/tvision && CXXFLAGS="-I/code/ext/ncurses/ncurses/include -Wno-unused-result -Wno-attributes" cmake ../../ext/tvision

obj/tvision/libtvision.a: obj/tvision/Makefile $(TVISION_SRC_FILES)
	@echo $@
	@rm -f obj/tvision/*.a obj/tvision/tv* obj/tvision/hello
	@cd obj/tvision && $(MAKE)

# core

$(CORE_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(INCLUDE_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

obj/core.a: $(CORE_OBJ_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(AR) rcs $@ $(CORE_OBJ_FILES)

# tmbasic

$(TMBASIC_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch obj/helpfile.h obj/help.h32 $(INCLUDE_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/tmbasic: $(TMBASIC_OBJ_FILES) obj/tvision/libtvision.a obj/core.a obj/common.h.gch obj/helpfile.h obj/help.h32 obj/helpfile.o
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -static -include obj/common.h -o $@ $(TMBASIC_OBJ_FILES) obj/core.a obj/tvision/libtvision.a obj/helpfile.o $(LDFLAGS)

# test

$(TEST_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch obj/helpfile.h obj/help.h32 $(INCLUDE_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/test: $(TEST_OBJ_FILES) obj/tvision/libtvision.a obj/core.a obj/common.h.gch obj/helpfile.h obj/help.h32 obj/helpfile.o
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -include obj/common.h -o $@ $(TEST_OBJ_FILES) obj/core.a obj/tvision/libtvision.a obj/helpfile.o $(LDFLAGS) -lgtest -lgtest_main -lpthread
