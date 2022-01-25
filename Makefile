CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Werror
LIBS = -Iinclude -Llib -lcppcms -lbooster -lmupdfcpp -lsqlite3

_HEADERS = content.h rpc.h database.h services.h
HEADERS = $(patsubst %,include/%,$(_HEADERS))

SKIN = src/views/myskin.cpp
_SRC = services.cpp database.cpp ReadingRPC.cpp DataRPC.cpp
SRC = $(SKIN) $(patsubst %,src/%,$(_SRC))

_TEMPLATES = master.tmpl library.tmpl upnext.tmpl collection.tmpl import.tmpl help.tmpl login.tmpl page_not_found.tmpl forbidden.tmpl settings.tmpl user.tmpl account.tmpl general.tmpl account_management.tmpl media_management.tmpl meintenance.tmpl
TEMPLATES = $(patsubst %,src/views/templates/%,$(_TEMPLATES))

all: website

website: src/main.cpp $(SKIN) $(HEADERS)
	$(CXX) $(CXXFLAGS) $< $(SRC) -o bin/exec ${LIBS}

$(SKIN): ${TEMPLATES}
	cppcms_tmpl_cc ${TEMPLATES} -o $(SKIN)

run: website bin/config.json
	./bin/exec -c bin/config.json

debug_build: src/main.cpp $(RES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -g -O0 $< $(RES) -o bin/debug_exec ${LIBS}

debug: debug_build config.josn
	valgrind --leak-check=yes ./bin/debug_exec -c ./bin/config.json

init_build: src/initialize.cpp
	$(CXX) $(CXXFLAGS) $< -o bin/init -lsqlite3

init: init_build
	./bin/init

clean:
	rm -fr ./bin/exec ./bin/init $(SKIN) ./bin/testing/tmp/pages/* cppcms_rundir
