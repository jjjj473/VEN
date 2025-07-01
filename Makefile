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

# Create a Python virtual environment with plugin dependencies
VENV=.venv

$(VENV)/bin/activate:
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install --upgrade pip
	$(VENV)/bin/pip install pygments pyspellchecker black matplotlib

venv: $(VENV)/bin/activate

# Build the project and run Tuxpad using the virtual environment
FILE?=README.md
run: tuxpad venv
	PATH=$(VENV)/bin:$$PATH ./tuxpad $(FILE)

clean:
	rm -f src/*.o tuxpad
