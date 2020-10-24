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
all: bin/tmbasic bin/test

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
		ext/mpdecimal/libmpdec/*.so*

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
	@mkdir -p $(@D)
	@echo $(CXX) -o $@
	@$(CXX) $(CXXFLAGS) -x c++-header -o $@ src/common.h

# help

obj/insert-cp437-diagram: doc/insert-cp437-diagram.c
	@mkdir -p obj
	@echo $(CC) -o $@
	@$(CC) -o $@ doc/insert-cp437-diagram.c

obj/helpfile.h: obj/doc-txt/help.txt obj/tvision/libtvision.a
	@mkdir -p obj
	@mkdir -p bin
	@rm -f bin/help.h32
	@rm -f obj/helpfile.h
	@echo tvhc bin/help.h32 obj/helpfile.h
	@obj/tvision/tvhc obj/doc-txt/help.txt /code/bin/help.h32 obj/helpfile.h

bin/help.h32: obj/helpfile.h

obj/doc-txt/help.txt: $(DOC_FILES) doc/build-doc.js obj/insert-cp437-diagram
	@echo node doc/build-doc.js
	@cd doc && node build-doc.js

# tvision

obj/tvision/Makefile:
	@mkdir -p $(@D)
	@echo cmake ext/tvision
	@cd obj/tvision && CXXFLAGS="-I/code/ext/ncurses/ncurses/include -Wno-unused-result" cmake ../../ext/tvision

obj/tvision/libtvision.a: obj/tvision/Makefile $(TVISION_SRC_FILES)
	@rm -f obj/tvision/*.a obj/tvision/tv* obj/tvision/hello
	@cd obj/tvision && $(MAKE)

# mpdecimal

ext/mpdecimal/Makefile:
	@echo ext/mpdecimal/configure
	@cd ext/mpdecimal && ./configure

ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a ext/mpdecimal/libmpdec/mpdecimal.h: ext/mpdecimal/Makefile
	@cd ext/mpdecimal && $(MAKE)

# core

$(CORE_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch $(INCLUDE_FILES) ext/mpdecimal/libmpdec/mpdecimal.h
	@mkdir -p $(@D)
	@echo $(CXX) -o $@
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

obj/core.a: $(CORE_OBJ_FILES)
	@mkdir -p $(@D)
	@echo $(AR) $@
	@$(AR) rcs $@ $(CORE_OBJ_FILES)

# tmbasic

$(TMBASIC_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch obj/helpfile.h bin/help.h32 $(INCLUDE_FILES) ext/mpdecimal/libmpdec/mpdecimal.h
	@mkdir -p $(@D)
	@echo $(CXX) -o $@
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/tmbasic: $(TMBASIC_OBJ_FILES) obj/tvision/libtvision.a obj/core.a ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a obj/common.h.gch obj/helpfile.h bin/help.h32
	@mkdir -p $(@D)
	@echo $(CXX) -o $@
	@$(CXX) $(CXXFLAGS) -static -include obj/common.h -o $@ $(TMBASIC_OBJ_FILES) obj/core.a obj/tvision/libtvision.a ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a $(LDFLAGS)

# test

$(TEST_OBJ_FILES): obj/%.o: src/%.cpp obj/common.h.gch obj/helpfile.h bin/help.h32 $(INCLUDE_FILES) ext/mpdecimal/libmpdec/mpdecimal.h
	@mkdir -p $(@D)
	@echo $(CXX) -o $@
	@$(CXX) $(CXXFLAGS) -c -include obj/common.h -o $@ $<

bin/test: $(TEST_OBJ_FILES) obj/tvision/libtvision.a obj/core.a ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a obj/common.h.gch obj/helpfile.h bin/help.h32
	@mkdir -p $(@D)
	@echo $(CXX) -o $@
	@$(CXX) $(CXXFLAGS) -include obj/common.h -o $@ $(TEST_OBJ_FILES) obj/core.a obj/tvision/libtvision.a ext/mpdecimal/libmpdec/libmpdec.a ext/mpdecimal/libmpdec++/libmpdec++.a $(LDFLAGS) -lgtest -lgtest_main -lpthread
