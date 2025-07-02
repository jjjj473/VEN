CFLAGS=-Wall -Wextra -fPIC -I./src

all: tuxpad plugins/wordcount.so

src/editor_features.o: src/editor_features.c src/editor_features.h
	$(CC) $(CFLAGS) -c $< -o $@

src/plugin_manager.o: src/plugin_manager.c src/plugin_manager.h
	$(CC) $(CFLAGS) -c $< -o $@

src/main.o: src/main.c src/editor_features.h src/plugin_manager.h
	$(CC) $(CFLAGS) -c $< -o $@

tuxpad: src/main.o src/editor_features.o src/plugin_manager.o
	$(CC) $(CFLAGS) $^ -ldl -o $@

plugins/wordcount.so: plugins/wordcount.c
	$(CC) $(CFLAGS) -shared $< -o $@

clean:
	rm -f src/*.o tuxpad plugins/*.so
