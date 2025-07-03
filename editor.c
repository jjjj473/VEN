#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

typedef struct {
    GtkWidget *window;
    GtkSourceBuffer *buffer;
    GtkWidget *view;
    GtkTextTag *bold_tag;
    GtkTextTag *italic_tag;
} App;

static void on_open(GtkWidget *w, App *app) {
    GtkWidget *dlg = gtk_file_chooser_dialog_new("Open File",
            GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_OPEN,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Open", GTK_RESPONSE_ACCEPT,
            NULL);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        gchar *contents = NULL; gsize len = 0;
        if (g_file_get_contents(fn, &contents, &len, NULL)) {
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(app->buffer), contents, len);
            g_free(contents);
        }
        g_free(fn);
    }
    gtk_widget_destroy(dlg);
}

static void on_save(GtkWidget *w, App *app) {
    GtkWidget *dlg = gtk_file_chooser_dialog_new("Save File",
            GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_SAVE,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Save", GTK_RESPONSE_ACCEPT,
            NULL);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(app->buffer), &start, &end);
        gchar *text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(app->buffer), &start, &end, FALSE);
        g_file_set_contents(fn, text, -1, NULL);
        g_free(text);
        g_free(fn);
    }
    gtk_widget_destroy(dlg);
}

static void on_undo(GtkWidget *w, App *app) {
    if (gtk_source_buffer_can_undo(app->buffer))
        gtk_source_buffer_undo(app->buffer);
}

static void on_redo(GtkWidget *w, App *app) {
    if (gtk_source_buffer_can_redo(app->buffer))
        gtk_source_buffer_redo(app->buffer);
}

static void on_upper(GtkWidget *w, App *app) {
    GtkTextIter start, end;
    if (gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(app->buffer), &start, &end)) {
        gchar *txt = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(app->buffer), &start, &end, FALSE);
        gchar *up = g_utf8_strup(txt, -1);
        gtk_text_buffer_delete(GTK_TEXT_BUFFER(app->buffer), &start, &end);
        gtk_text_buffer_insert(GTK_TEXT_BUFFER(app->buffer), &start, up, -1);
        g_free(txt); g_free(up);
    }
}

static void on_lower(GtkWidget *w, App *app) {
    GtkTextIter start, end;
    if (gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(app->buffer), &start, &end)) {
        gchar *txt = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(app->buffer), &start, &end, FALSE);
        gchar *down = g_utf8_strdown(txt, -1);
        gtk_text_buffer_delete(GTK_TEXT_BUFFER(app->buffer), &start, &end);
        gtk_text_buffer_insert(GTK_TEXT_BUFFER(app->buffer), &start, down, -1);
        g_free(txt); g_free(down);
    }
}

static void apply_tag(GtkWidget *w, App *app, GtkTextTag *tag) {
    GtkTextIter start, end;
    if (gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(app->buffer), &start, &end)) {
        gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(app->buffer), tag, &start, &end);
    }
}

static void on_bold(GtkWidget *w, App *app) { apply_tag(w, app, app->bold_tag); }
static void on_italic(GtkWidget *w, App *app) { apply_tag(w, app, app->italic_tag); }

static void on_find_replace(GtkWidget *w, App *app) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Find and Replace",
            GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Replace", GTK_RESPONSE_ACCEPT,
            NULL);
    GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *find = gtk_entry_new();
    GtkWidget *repl = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(find), "Find");
    gtk_entry_set_placeholder_text(GTK_ENTRY(repl), "Replace with");
    gtk_box_pack_start(GTK_BOX(box), find, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(box), repl, FALSE, FALSE, 4);
    gtk_widget_show_all(dialog);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const gchar *f = gtk_entry_get_text(GTK_ENTRY(find));
        const gchar *r = gtk_entry_get_text(GTK_ENTRY(repl));
        GtkTextIter start, match_start, match_end;
        gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(app->buffer), &start);
        if (gtk_text_iter_forward_search(&start, f, GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL)) {
            gtk_text_buffer_delete(GTK_TEXT_BUFFER(app->buffer), &match_start, &match_end);
            gtk_text_buffer_insert(GTK_TEXT_BUFFER(app->buffer), &match_start, r, -1);
        }
    }
    gtk_widget_destroy(dialog);
}

static void activate(GtkApplication *app_g, gpointer user_data) {
    App *app = user_data;
    app->window = gtk_application_window_new(app_g);
    gtk_window_set_title(GTK_WINDOW(app->window), "VEN Editor");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 600, 400);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(app->window), grid);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_grid_attach(GTK_GRID(grid), toolbar, 0, 0, 1, 1);

    app->view = gtk_source_view_new();
    app->buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->view)));
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(app->view), TRUE);

    app->bold_tag = gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(app->buffer), "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    app->italic_tag = gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(app->buffer), "italic", "style", PANGO_STYLE_ITALIC, NULL);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), app->view);
    gtk_grid_attach(GTK_GRID(grid), scrolled, 0, 1, 1, 1);

    GtkWidget *btn_open = gtk_button_new_with_label("Open");
    GtkWidget *btn_save = gtk_button_new_with_label("Save");
    GtkWidget *btn_undo = gtk_button_new_with_label("Undo");
    GtkWidget *btn_redo = gtk_button_new_with_label("Redo");
    GtkWidget *btn_find = gtk_button_new_with_label("Find/Replace");
    GtkWidget *btn_up = gtk_button_new_with_label("Uppercase");
    GtkWidget *btn_low = gtk_button_new_with_label("Lowercase");
    GtkWidget *btn_bold = gtk_button_new_with_label("Bold");
    GtkWidget *btn_italic = gtk_button_new_with_label("Italic");

    gtk_box_pack_start(GTK_BOX(toolbar), btn_open, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_save, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_undo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_redo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_find, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_up, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_low, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_bold, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_italic, FALSE, FALSE, 0);

    g_signal_connect(btn_open, "clicked", G_CALLBACK(on_open), app);
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save), app);
    g_signal_connect(btn_undo, "clicked", G_CALLBACK(on_undo), app);
    g_signal_connect(btn_redo, "clicked", G_CALLBACK(on_redo), app);
    g_signal_connect(btn_find, "clicked", G_CALLBACK(on_find_replace), app);
    g_signal_connect(btn_up, "clicked", G_CALLBACK(on_upper), app);
    g_signal_connect(btn_low, "clicked", G_CALLBACK(on_lower), app);
    g_signal_connect(btn_bold, "clicked", G_CALLBACK(on_bold), app);
    g_signal_connect(btn_italic, "clicked", G_CALLBACK(on_italic), app);

    gtk_widget_show_all(app->window);
}

int main(int argc, char **argv) {
    GtkApplication *app_g;
    int status;
    App app = {0};
    app_g = gtk_application_new("com.example.veneditor", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app_g, "activate", G_CALLBACK(activate), &app);
    status = g_application_run(G_APPLICATION(app_g), argc, argv);
    g_object_unref(app_g);
    return status;
}

