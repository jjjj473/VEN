#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *textview;
    GtkTextBuffer *buffer;
    GtkWidget *command_entry;
    GtkWidget *statusbar;
    guint status_ctx;
    gchar *current_file;
    gboolean command_mode;
} VenApp;

static void update_status(VenApp *app, const gchar *msg) {
    gchar *text = g_strdup_printf("[%s]%s%s", app->command_mode ? "COMMAND" : "INSERT", msg ? " - " : "", msg ? msg : "");
    gtk_statusbar_pop(GTK_STATUSBAR(app->statusbar), app->status_ctx);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_ctx, text);
    g_free(text);
}

static void open_file_dialog(VenApp *app);
static void save_file_dialog(VenApp *app);

static void open_file(VenApp *app, const gchar *fname) {
    if (!fname || !*fname) {
        open_file_dialog(app);
        return;
    }
    gchar *content = NULL;
    gsize len;
    if (g_file_get_contents(fname, &content, &len, NULL)) {
        gtk_text_buffer_set_text(app->buffer, content, len);
        g_free(content);
        g_free(app->current_file);
        app->current_file = g_strdup(fname);
        update_status(app, g_strdup_printf("Opened %s", fname));
    } else {
        update_status(app, "Failed to open file");
    }
}

static void save_file(VenApp *app, const gchar *fname) {
    if (!fname || !*fname) {
        save_file_dialog(app);
        return;
    }
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    if (g_file_set_contents(fname, text, -1, NULL)) {
        g_free(app->current_file);
        app->current_file = g_strdup(fname);
        update_status(app, g_strdup_printf("Saved %s", fname));
    } else {
        update_status(app, "Failed to save file");
    }
    g_free(text);
}

static void open_file_dialog(VenApp *app) {
    GtkWidget *d = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        gchar *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
        open_file(app, fname);
        g_free(fname);
    }
    gtk_widget_destroy(d);
}

static void save_file_dialog(VenApp *app) {
    GtkWidget *d = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        gchar *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
        save_file(app, fname);
        g_free(fname);
    }
    gtk_widget_destroy(d);
}

static void show_help(VenApp *app) {
    const gchar *msg =
        "VEN Commands:\n"
        ":w [file] - save file\n"
        ":q - quit\n"
        ":o [file] - open file\n"
        ":wq - save and quit\n"
        ":/pattern - search\n"
        ":!cmd - run shell command\n"
        ":help - show this help";
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void search_text(VenApp *app, const gchar *pattern) {
    GtkTextIter start, match_start, match_end;
    gtk_text_buffer_get_start_iter(app->buffer, &start);
    gboolean found = gtk_text_iter_forward_search(&start, pattern, 0, &match_start, &match_end, NULL);
    if (found) {
        gtk_text_buffer_select_range(app->buffer, &match_start, &match_end);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(app->textview), &match_start, 0.0, TRUE, 0.5, 0.5);
        update_status(app, g_strdup_printf("Search: %s", pattern));
    } else {
        update_status(app, "Pattern not found");
    }
}

static void run_shell_command(VenApp *app, const gchar *cmd) {
    gchar *output = NULL;
    GError *err = NULL;
    g_spawn_command_line_sync(cmd, &output, NULL, NULL, &err);
    if (!output)
        output = g_strdup(err ? err->message : "");
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", output);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
    g_free(output);
    if (err)
        g_error_free(err);
}

static void process_command(VenApp *app, const gchar *cmd) {
    if (g_strcmp0(cmd, "q") == 0 || g_strcmp0(cmd, ":q") == 0) {
        gtk_window_close(GTK_WINDOW(app->window));
    } else if (g_str_has_prefix(cmd, "w ") || g_strcmp0(cmd, "w") == 0 || g_str_has_prefix(cmd, ":w")) {
        const gchar *arg = cmd[0] == ':' ? cmd + 2 : cmd + 1;
        if (*arg == ' ')
            arg++;
        if (*arg)
            save_file(app, arg);
        else
            save_file(app, app->current_file);
    } else if (g_str_has_prefix(cmd, "o ") || g_strcmp0(cmd, "o") == 0 || g_str_has_prefix(cmd, ":o")) {
        const gchar *arg = cmd[0] == ':' ? cmd + 2 : cmd + 1;
        if (*arg == ' ')
            arg++;
        if (*arg)
            open_file(app, arg);
        else
            open_file(app, NULL);
    } else if (g_strcmp0(cmd, "wq") == 0 || g_strcmp0(cmd, ":wq") == 0) {
        if (app->current_file)
            save_file(app, app->current_file);
        else
            save_file(app, NULL);
        gtk_window_close(GTK_WINDOW(app->window));
    } else if (g_str_has_prefix(cmd, ":!")) {
        run_shell_command(app, cmd + 2);
    } else if (g_str_has_prefix(cmd, "!")) {
        run_shell_command(app, cmd + 1);
    } else if (g_str_has_prefix(cmd, "/")) {
        search_text(app, cmd + 1);
    } else if (g_strcmp0(cmd, "help") == 0 || g_strcmp0(cmd, ":help") == 0) {
        show_help(app);
    }
    gtk_entry_set_text(GTK_ENTRY(app->command_entry), "");
    app->command_mode = FALSE;
    gtk_widget_grab_focus(app->textview);
    update_status(app, NULL);
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    VenApp *app = user_data;
    if (!app->command_mode) {
        if (event->keyval == GDK_KEY_Escape) {
            app->command_mode = TRUE;
            gtk_widget_grab_focus(app->command_entry);
            update_status(app, NULL);
            return TRUE;
        }
    } else {
        if (event->keyval == GDK_KEY_Escape) {
            app->command_mode = FALSE;
            gtk_widget_grab_focus(app->textview);
            gtk_entry_set_text(GTK_ENTRY(app->command_entry), "");
            update_status(app, NULL);
            return TRUE;
        }
    }
    return FALSE;
}

static void on_command_activate(GtkEntry *entry, gpointer user_data) {
    VenApp *app = user_data;
    const gchar *text = gtk_entry_get_text(entry);
    process_command(app, text);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    VenApp app = {0};
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "VEN");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 800, 600);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app.window), vbox);

    app.textview = gtk_text_view_new();
    app.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.textview));
    GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroller), app.textview);
    gtk_box_pack_start(GTK_BOX(vbox), scroller, TRUE, TRUE, 0);

    app.command_entry = gtk_entry_new();
    gtk_box_pack_end(GTK_BOX(vbox), app.command_entry, FALSE, FALSE, 0);
    g_signal_connect(app.command_entry, "activate", G_CALLBACK(on_command_activate), &app);

    app.statusbar = gtk_statusbar_new();
    app.status_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(app.statusbar), "mode");
    gtk_box_pack_end(GTK_BOX(vbox), app.statusbar, FALSE, FALSE, 0);

    update_status(&app, NULL);

    g_signal_connect(app.window, "key-press-event", G_CALLBACK(on_key_press), &app);

    gtk_widget_show_all(app.window);

    gtk_main();
    g_free(app.current_file);
    return 0;
}
