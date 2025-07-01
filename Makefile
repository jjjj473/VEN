CFLAGS=-Wall -Wextra -I./src
CXXFLAGS=$(CFLAGS)

all: tuxpad

src/editor_features.o: src/editor_features.cpp src/editor_features.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/main.o: src/main.c src/editor_features.h
	$(CC) $(CFLAGS) -c $< -o $@

TUXPAD_OBJS=src/main.o src/editor_features.o

tuxpad: $(TUXPAD_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

gui:
	python3 scripts/editor_gui.py

clean:
	rm -f src/*.o tuxpad
