CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Werror
LIBS = -lcppcms -lbooster -lmupdfcpp -lsqlite3
SKIN = views/myskin.cpp
HEADERS = views/content.h

RES = $(SKIN)

_TEMPLATES = master.tmpl library.tmpl upnext.tmpl collection.tmpl import.tmpl help.tmpl login.tmpl page_not_found.tmpl forbidden.tmpl settings.tmpl user.tmpl account.tmpl general.tmpl account_management.tmpl media_management.tmpl meintenance.tmpl
TEMPLATES = $(patsubst %,views/templates/%,$(_TEMPLATES))

all: website

website: main.cpp $(SKIN) $(HEADERS)
	$(CXX) $(CXXFLAGS) $< $(RES) -o main.o ${LIBS}

$(SKIN): ${TEMPLATES}
	cppcms_tmpl_cc ${TEMPLATES} -o $(SKIN)

run: website config.json
	./main.o -c config.json

debug_build: main.cpp $(RES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -g -O0 $< $(RES) -o website.o ${LIBS}

debug: debug_build config.josn
	valgrind --leak-check=yes ./website.o -c config.json

init_build: initialize.cpp
	$(CXX) $(CXXFLAGS) $< -o initialize.o -lsqlite3

init: init_build
	./initialize.o

clean:
	rm -fr main.o initialize.o *.exe $(SKIN) ./testing/tmp/pages/* cppcms_rundir
