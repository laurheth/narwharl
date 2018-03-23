CXX=g++
SOURCES=main.cpp aoe.cpp map.cpp screen.cpp creature.cpp player.cpp llist.cpp item.cpp mtrand/mtrand.cpp random.cpp genmap.cpp stuffread.cpp magic.cpp pathfinder.cpp monster.cpp combat.cpp newgenmap.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=narwharl
LDFLAGS=-lpanel -lmenu -lncurses
CFLAGS=-g -c

all : $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -Wall $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

.PHONEY: clean

clean:
	rm $(OBJECTS) $(EXECUTABLE)
