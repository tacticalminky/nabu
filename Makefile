CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic
LIBS = -Iinclude -Llib -lcppcms -lbooster -lmupdfcpp -lsqlite3

SKIN = src/views/myskin.cpp
_SRC = Website.cpp services.cpp database.cpp ReadingRPC.cpp DataRPC.cpp
SRC = $(SKIN) $(patsubst %,src/%,$(_SRC))

_TEMPLATES = master.tmpl library.tmpl upnext.tmpl collection.tmpl import.tmpl help.tmpl login.tmpl page_not_found.tmpl forbidden.tmpl settings.tmpl user.tmpl account.tmpl general.tmpl account_management.tmpl media_management.tmpl meintenance.tmpl
TEMPLATES = $(patsubst %,src/views/templates/%,$(_TEMPLATES))

exec: bin/exec

bin/exec: src/main.cpp $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ ${LIBS}

$(SKIN): ${TEMPLATES}
	cppcms_tmpl_cc $^ -o $@

run: bin/exec bin/config.json
	$< -c bin/config.json

bin/init: src/initialize.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -lsqlite3

init: bin/init
	$<

bin/debug: src/main.cpp $(SRC)
	$(CXX) $(CXXFLAGS) -g -O0 $^ -o $@ ${LIBS}

debug: bin/debug config.josn
	valgrind --leak-check=yes $< -c ./bin/config.json

clean:
	rm -f bin/exec bin/init bin/debug $(SKIN) ./bin/testing/tmp/pages/* cppcms_rundir

.DEFAULT_GOAL := exec
.PHONEY := run debug init clean
