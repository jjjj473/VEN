editor: editor.c
	gcc editor.c -o editor $(shell pkg-config --cflags --libs gtk+-3.0 gtksourceview-3.0)
