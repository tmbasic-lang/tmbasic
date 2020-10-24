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

# for debugging, override with: OPTFLAGS='-g -O0'
# to dump AST parse trees, override with: EXTRADEFS='-DDUMP_AST'
EXTRADEFS=
OPTFLAGS=-Os -flto
CXXFLAGS=-Isrc -Iobj -Iext/tvision/include -Iext/immer -Iext/nameof -Iext/mpdecimal/libmpdec -Iext/mpdecimal/libmpdec++ -Wall -Werror -Winvalid-pch -Wno-unknown-pragmas -Wno-reorder -std=c++17 $(OPTFLAGS) $(EXTRADEFS)
LDFLAGS=-lstdc++ -lncursesw -ltinfo

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
	@echo ""

.PHONY: clean
clean:
	@rm -rf bin obj obj/tvision valgrind.txt \
		ext/mpdecimal/Makefile \
		ext/mpdecimal/config.log \
		ext/mpdecimal/config.status \
		ext/mpdecimal/config.h \
		ext/mpdecimal/libmpdec++/Makefile \
		ext/mpdecimal/libmpdec/Makefile \
		ext/mpdecimal/libmpdec/mpdecimal.h \
		ext/mpdecimal/tests++/Makefile \
		ext/mpdecimal/tests/Makefile \
		ext/mpdecimal/libmpdec++/.objs/*.o \
		ext/mpdecimal/libmpdec++/*.o \
		ext/mpdecimal/libmpdec++/*.a \
		ext/mpdecimal/libmpdec++/*.so* \
		ext/mpdecimal/libmpdec/.objs/*.o \
		ext/mpdecimal/libmpdec/*.o \
		ext/mpdecimal/libmpdec/*.a \
		ext/mpdecimal/libmpdec/*.so* \
		doc/diagrams/license*.txt

.PHONY: run
run: bin/tmbasic
	@cd bin && ./tmbasic

.PHONY: test
test: bin/test
	@cd bin && ./test

.PHONY: valgrind
valgrind: bin/tmbasic
	@valgrind --leak-check=full --show-leak-kinds=all --undef-value-errors=no --log-file=valgrind.txt bin/tmbasic

.PHONY: format
format:
	@cd src && find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i --style="{BasedOnStyle: Chromium, IndentWidth: 4, ColumnLimit: 120, SortIncludes: false, AlignAfterOpenBracket: AlwaysBreak, AlignOperands: false, Cpp11BracedListStyle: false, PenaltyReturnTypeOnItsOwnLine: 10000}"

# shared

obj/common.h.gch: src/common.h $(EXT_HEADER_FILES) ext/mpdecimal/libmpdec/mpdecimal.h
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
	@$(CC) -o $@ doc/insert-cp437-diagram.c

doc/diagrams/license.txt: bin/LICENSE.txt
	@echo $@
	@mkdir -p obj
	@cp -f $< $@

obj/helpfile.h: obj/doc-txt/help.txt obj/tvision/libtvision.a
	@echo $@
	@mkdir -p obj
	@mkdir -p bin
	@rm -f bin/help.h32
	@rm -f obj/helpfile.h
	@obj/tvision/tvhc obj/doc-txt/help.txt /code/bin/help.h32 obj/helpfile.h

bin/help.h32: obj/helpfile.h
	@echo $@

obj/doc-txt/help.txt: $(DOC_FILES) doc/build-doc.js obj/insert-cp437-diagram doc/diagrams/license_tmbasic.txt doc/diagrams/license_boost.txt doc/diagrams/license_immer.txt doc/diagrams/license_libstdc++_gpl3.txt doc/diagrams/license_libstdc++_gcc1.txt doc/diagrams/license_libstdc++_gcc2.txt doc/diagrams/license_mpdecimal.txt doc/diagrams/license_nameof.txt doc/diagrams/license_ncurses.txt doc/diagrams/license_tvision.txt doc/diagrams/license_notoserif.txt doc/diagrams/license_opensans.txt doc/diagrams/license_oxygenmono.txt
	@echo $@
	@cd doc && node build-doc.js

# tvision

obj/tvision/Makefile:
	@echo $@
	@mkdir -p $(@D)
	@cd obj/tvision && CXXFLAGS="-I/code/ext/ncurses/ncurses/include -Wno-unused-result" cmake ../../ext/tvision

obj/tvision/libtvision.a: obj/tvision/Makefile $(TVISION_SRC_FILES)
	@echo $@
	@rm -f obj/tvision/*.a obj/tvision/tv* obj/tvision/hello
	@cd obj/tvision && $(MAKE)

# mpdecimal

ext/mpdecimal/Makefile:
	@echo $@
	@cd ext/mpdecimal && ./configure

ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a ext/mpdecimal/libmpdec/mpdecimal.h: ext/mpdecimal/Makefile
	@echo $@
	@cd ext/mpdecimal && $(MAKE)

# core

$(CORE_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(INCLUDE_FILES) ext/mpdecimal/libmpdec/mpdecimal.h
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

obj/core.a: $(CORE_OBJ_FILES)
	@echo $@
	@mkdir -p $(@D)
	@$(AR) rcs $@ $(CORE_OBJ_FILES)

# tmbasic

$(TMBASIC_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch obj/helpfile.h bin/help.h32 $(INCLUDE_FILES) ext/mpdecimal/libmpdec/mpdecimal.h
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/tmbasic: $(TMBASIC_OBJ_FILES) obj/tvision/libtvision.a obj/core.a ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a obj/common.h.gch obj/helpfile.h bin/help.h32
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -static -include obj/common.h -o $@ $(TMBASIC_OBJ_FILES) obj/core.a obj/tvision/libtvision.a ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a $(LDFLAGS)

# test

$(TEST_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch obj/helpfile.h bin/help.h32 $(INCLUDE_FILES) ext/mpdecimal/libmpdec/mpdecimal.h
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/test: $(TEST_OBJ_FILES) obj/tvision/libtvision.a obj/core.a ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a obj/common.h.gch obj/helpfile.h bin/help.h32
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -include obj/common.h -o $@ $(TEST_OBJ_FILES) obj/core.a obj/tvision/libtvision.a ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a $(LDFLAGS) -lgtest -lgtest_main -lpthread
