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
DIAGRAM_SRC_FILES=$(shell find doc/diagrams -type f -name "*.txt")
DIAGRAM_CP437_FILES=$(patsubst doc/diagrams/%,obj/doc-temp/diagrams-cp437/%,$(DIAGRAM_SRC_FILES))
LICENSE_DIAGRAM_SRC_FILES=obj/doc-temp/diagrams-license/license_tmbasic.txt obj/doc-temp/diagrams-license/license_boost.txt obj/doc-temp/diagrams-license/license_immer.txt obj/doc-temp/diagrams-license/license_libstdc++_gpl3.txt obj/doc-temp/diagrams-license/license_libstdc++_gcc1.txt obj/doc-temp/diagrams-license/license_libstdc++_gcc2.txt obj/doc-temp/diagrams-license/license_mpdecimal.txt obj/doc-temp/diagrams-license/license_nameof.txt obj/doc-temp/diagrams-license/license_ncurses.txt obj/doc-temp/diagrams-license/license_tvision.txt obj/doc-temp/diagrams-license/license_notoserif.txt obj/doc-temp/diagrams-license/license_opensans.txt obj/doc-temp/diagrams-license/license_oxygenmono.txt
LICENSE_DIAGRAM_CP437_FILES=$(patsubst obj/doc-temp/diagrams-license/%,obj/doc-temp/diagrams-cp437/%,$(LICENSE_DIAGRAM_SRC_FILES))

BUILDCC ?= $(CC)
TVHC ?= obj/tvision/tvhc
CMAKE ?= cmake
NODE ?= node
HELP_FILE_OBJ ?= obj/helpfile.o
STATIC_FLAG ?= -static
LIBMPDEC_FLAG ?= -lmpdec -lmpdec++
LIBNCURSESW_FLAG ?= -lncursesw
LIBTINFO_FLAG ?= -ltinfo

# for debugging, override with: OPTFLAGS='-g -O0'
# to dump AST parse trees, override with: EXTRADEFS='-DDUMP_AST'
OPTFLAGS=-Os -flto
CXXFLAGS=$(WIN_INCLUDE_FLAGS) $(MAC_INCLUDE_FLAGS) -Isrc -Iobj -Iext/tvision/include -Iext/immer -Iext/nameof -I/usr/include/libmpdec -I/usr/include/libmpdec++ -Wall -Werror -Winvalid-pch -Wno-unknown-pragmas -Wno-reorder -std=c++17 $(OPTFLAGS) $(EXTRADEFS)
LDFLAGS=$(MAC_LD_FLAGS) -lstdc++ $(LIBNCURSESW_FLAG) $(LIBTINFO_FLAG) $(LIBMPDEC_FLAG)

.PHONY: all
all: bin/tmbasic bin/test bin/LICENSE.txt

.PHONY: help
help:
	@echo ""
	@echo "Commands:"
	@echo
	@echo "make"
	@echo "make win"
	@echo "make mac"
	@echo
	@echo "make run"
	@echo
	@echo "make clean"
	@echo "make format"
	@echo "make test"
	@echo "make valgrind"
	@echo ""

.PHONY: clean
clean:
	@rm -rf bin obj obj/tvision valgrind.txt

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
		LIBTINFO_FLAG="" \
		$(MAKE) bin/tmbasic bin/LICENSE.txt

.PHONY: mac
mac:
	@MAC_INCLUDE_FLAGS="-I$(PWD)/mac/boost -I$(PWD)/mac/mpdecimal/libmpdec -I$(PWD)/mac/mpdecimal/libmpdec++ -I$(PWD)/mac/ncurses/include" \
		MAC_LD_FLAGS="-L$(PWD)/mac/mpdecimal/libmpdec -L$(PWD)/mac/mpdecimal/libmpdec++ -L$(PWD)/mac/ncurses/lib" \
		CMAKE="$(PWD)/mac/cmake/CMake.app/Contents/bin/cmake -D CMAKE_PREFIX_PATH=$(PWD)/mac/ncurses -D NCURSESW=$(PWD)/mac/ncurses/lib/libncursesw.a" \
		NODE="$(PWD)/mac/node/bin/node" \
		HELP_FILE_OBJ="" \
		MAC_HELP_FILE_LINK_FLAG="-Wl,-sectcreate,__DATA,__help_h32,obj/help.h32" \
		STATIC_FLAG="" \
		LIBTINFO_FLAG="" \
		LIBMPDEC_FLAG="$(PWD)/mac/mpdecimal/libmpdec/libmpdec.a $(PWD)/mac/mpdecimal/libmpdec++/libmpdec++.a " \
		LIBNCURSESW_FLAG="$(PWD)/mac/ncurses/lib/libncursesw.a" \
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

obj/doc-temp/diagrams-license/license_tmbasic.txt: LICENSE
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_boost.txt: ext/boost/LICENSE_1_0.txt
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

obj/doc-temp/diagrams-license/license_notoserif.txt: ext/notoserif/OFL.txt
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_opensans.txt: ext/opensans/LICENSE.txt
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/doc-temp/diagrams-license/license_oxygenmono.txt: ext/oxygenmono/OFL.txt
	@echo $@
	@mkdir -p $(@D)
	@cp -f $< $@

obj/insert-cp437-diagram: doc/insert-cp437-diagram.c
	@echo $@
	@mkdir -p obj
	@$(BUILDCC) -o $@ doc/insert-cp437-diagram.c

obj/helpfile.h: obj/doc-txt/help.txt obj/tvision/libtvision.a
	@echo $@
	@mkdir -p obj
	@mkdir -p bin
	@rm -f obj/help.h32
	@rm -f obj/helpfile.h
	@$(TVHC) obj/doc-txt/help.txt obj/help.h32 obj/helpfile.h

obj/help.h32: obj/helpfile.h
	@echo $@

obj/doc-txt/help.txt: $(DOC_FILES) doc/build-doc.js obj/insert-cp437-diagram $(DIAGRAM_CP437_FILES) $(LICENSE_DIAGRAM_CP437_FILES)
	@echo $@
	@mkdir -p obj
	@cd doc && $(NODE) build-doc.js

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

# tvision

obj/tvision/Makefile:
	@echo $@
	@mkdir -p $(@D)
	@cd obj/tvision && CXXFLAGS="-Wno-unused-result -Wno-attributes" $(CMAKE) ../../ext/tvision

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

bin/tmbasic: $(TMBASIC_OBJ_FILES) obj/tvision/libtvision.a obj/core.a obj/common.h.gch obj/helpfile.h obj/help.h32 $(HELP_FILE_OBJ)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(MAC_HELP_FILE_LINK_FLAG) $(STATIC_FLAG) -include obj/common.h -o $@ $(TMBASIC_OBJ_FILES) obj/core.a obj/tvision/libtvision.a $(HELP_FILE_OBJ) $(LDFLAGS)

# test

$(TEST_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch obj/helpfile.h obj/help.h32 $(INCLUDE_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/test: $(TEST_OBJ_FILES) obj/tvision/libtvision.a obj/core.a obj/common.h.gch obj/helpfile.h obj/help.h32 $(HELP_FILE_OBJ)
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(MAC_HELP_FILE_LINK_FLAG) -include obj/common.h -o $@ $(TEST_OBJ_FILES) obj/core.a obj/tvision/libtvision.a $(HELP_FILE_OBJ) $(LDFLAGS) -lgtest -lgtest_main -lpthread
