LIBS = -lcppcms -lbooster -lmupdfcpp
SKIN = views/myskin.cpp

_RES = Library.cpp Settings.cpp JSONService.cpp
RES = $(patsubst %,apps/%,$(_RES))

_TEMPLATES = master.tmpl library.tmpl upnext.tmpl collection.tmpl read.tmpl import.tmpl help.tmpl login.tmpl page_not_found.tmpl settings.tmpl user.tmpl account.tmpl general.tmpl account_management.tmpl media_management.tmpl meintenance.tmpl
TEMPLATES = $(patsubst %,views/templates/%,$(_TEMPLATES))

all: website

website: main.cpp $(SKIN) views/content.h apps/apps.h
	$(CXX) $(CXXFLAGS) -std=c++11 -Wall main.cpp $(SKIN) $(RES) -o main.o ${LIBS}

$(SKIN): ${TEMPLATES}
	cppcms_tmpl_cc ${TEMPLATES} -o $(SKIN)

testing: testing.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 -Wall testing.cpp -o testing.o -lmupdfcpp

clean:
	rm -fr main.o testing.o *.exe $(SKIN) cppcms_rundir