CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Werror
LIBS = -lcppcms -lbooster -lmupdfcpp -lsqlite3
SKIN = src/views/myskin.cpp
HEADERS = src/views/content.h

RES = $(SKIN)

_TEMPLATES = master.tmpl library.tmpl upnext.tmpl collection.tmpl import.tmpl help.tmpl login.tmpl page_not_found.tmpl forbidden.tmpl settings.tmpl user.tmpl account.tmpl general.tmpl account_management.tmpl media_management.tmpl meintenance.tmpl
TEMPLATES = $(patsubst %,src/views/templates/%,$(_TEMPLATES))

all: website

website: src/main.cpp $(SKIN) $(HEADERS)
	$(CXX) $(CXXFLAGS) $< $(RES) -o build/main.o ${LIBS}

$(SKIN): ${TEMPLATES}
	cppcms_tmpl_cc ${TEMPLATES} -o $(SKIN)

run: website build/config.json
	./build/main.o -c build/config.json

debug_build: src/main.cpp $(RES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -g -O0 $< $(RES) -o build/main.o ${LIBS}

debug: debug_build build/config.josn
	valgrind --leak-check=yes ./build/main.o -c build/config.json

init_build: src/initialize.cpp
	$(CXX) $(CXXFLAGS) $< -o build/initialize.o -lsqlite3

init: init_build
	./build/initialize.o

clean:
	rm -fr build/main.o build/initialize.o *.exe $(SKIN) ./build/testing/tmp/pages/* cppcms_rundir
