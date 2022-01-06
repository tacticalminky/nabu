LIBS = -lcppcms -lbooster -lmupdfcpp
SKIN = views/myskin.cpp
HEADERS = views/content.h

RES = $(SKIN)

_TEMPLATES = master.tmpl library.tmpl upnext.tmpl collection.tmpl read.tmpl import.tmpl help.tmpl login.tmpl page_not_found.tmpl settings.tmpl user.tmpl account.tmpl general.tmpl account_management.tmpl media_management.tmpl meintenance.tmpl
TEMPLATES = $(patsubst %,views/templates/%,$(_TEMPLATES))

all: website

website: main.cpp $(RES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -std=c++11 -Wall main.cpp $(RES) -o main.o ${LIBS}

$(SKIN): ${TEMPLATES}
	cppcms_tmpl_cc ${TEMPLATES} -o $(SKIN)

run: website config.json
	./main.o -c config.json

debug_build: main.cpp $(RES) $(HEADERS) config.json
	$(CXX) $(CXXFLAGS) -std=c++11 -g -O0 -Wall main.cpp $(RES) -o main.o ${LIBS}

debug: debug_build
	valgrind --leak-check=yes ./main.o -c config.json

clean:
	rm -fr main.o *.exe $(SKIN) ./testing/tmp/pages/* cppcms_rundir
