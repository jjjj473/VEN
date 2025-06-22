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
    gchar *text = g_strdup_printf("[%s]%s%s",
        app->command_mode ? "COMMAND" : "INSERT",
        msg ? " - " : "", msg ? msg : "");
    gtk_statusbar_pop(GTK_STATUSBAR(app->statusbar), app->status_ctx);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_ctx, text);
    g_free(text);
}

static void open_file_dialog(VenApp *app);
static void save_file_dialog(VenApp *app);

static void new_file(VenApp *app) {
    gtk_text_buffer_set_text(app->buffer, "", -1);
    g_clear_pointer(&app->current_file, g_free);
    update_status(app, "New file");
}

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
    GtkWidget *d = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(app->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        gchar *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
        open_file(app, fname);
        g_free(fname);
    }
    gtk_widget_destroy(d);
}

static void save_file_dialog(VenApp *app) {
    GtkWidget *d = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(app->window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
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
        ":new - new file\n"
        ":cut - cut selection\n"
        ":copy - copy selection\n"
        ":paste - paste clipboard\n"
        ":/pattern - search\n"
        ":!cmd - run shell command\n"
        ":about - about dialog\n"
        ":help - show this help";
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void show_about(VenApp *app) {
    const gchar *msg = "VEN - simple GTK3 editor";
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void search_text(VenApp *app, const gchar *pattern) {
    GtkTextIter start, match_start, match_end;
    gtk_text_buffer_get_start_iter(app->buffer, &start);
    gboolean found = gtk_text_iter_forward_search(&start, pattern, 0,
        &match_start, &match_end, NULL);
    if (found) {
        gtk_text_buffer_select_range(app->buffer, &match_start, &match_end);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(app->textview), &match_start,
            0.0, TRUE, 0.5, 0.5);
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

static void run_command_dialog(VenApp *app) {
    GtkWidget *d = gtk_dialog_new_with_buttons("Run Command", GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Run", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), entry, TRUE, TRUE, 0);
    gtk_widget_show(entry);
    gchar *cmd = NULL;
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT)
        cmd = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    gtk_widget_destroy(d);
    if (cmd && *cmd)
        run_shell_command(app, cmd);
    g_free(cmd);
}

static void cut_selection(VenApp *app) {
    GtkClipboard *cb = gtk_widget_get_clipboard(app->textview, GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_cut_clipboard(app->buffer, cb, TRUE);
}

static void copy_selection(VenApp *app) {
    GtkClipboard *cb = gtk_widget_get_clipboard(app->textview, GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard(app->buffer, cb);
}

static void paste_clipboard(VenApp *app) {
    GtkClipboard *cb = gtk_widget_get_clipboard(app->textview, GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_paste_clipboard(app->buffer, cb, NULL, TRUE);
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
    } else if (g_strcmp0(cmd, "new") == 0 || g_strcmp0(cmd, ":new") == 0) {
        new_file(app);
    } else if (g_strcmp0(cmd, "cut") == 0 || g_strcmp0(cmd, ":cut") == 0) {
        cut_selection(app);
    } else if (g_strcmp0(cmd, "copy") == 0 || g_strcmp0(cmd, ":copy") == 0) {
        copy_selection(app);
    } else if (g_strcmp0(cmd, "paste") == 0 || g_strcmp0(cmd, ":paste") == 0) {
        paste_clipboard(app);
    } else if (g_str_has_prefix(cmd, ":!")) {
        run_shell_command(app, cmd + 2);
    } else if (g_str_has_prefix(cmd, "!")) {
        run_shell_command(app, cmd + 1);
    } else if (g_str_has_prefix(cmd, "/")) {
        search_text(app, cmd + 1);
    } else if (g_strcmp0(cmd, "about") == 0 || g_strcmp0(cmd, ":about") == 0) {
        show_about(app);
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

static void on_menu_activate(GtkWidget *widget, gpointer data) {
    const gchar *action = data;
    VenApp *app = g_object_get_data(G_OBJECT(widget), "app");
    process_command(app, action);
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

    GtkWidget *menubar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    GtkWidget *filemenu = gtk_menu_new();
    GtkWidget *file = gtk_menu_item_new_with_mnemonic("_File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);

    GtkWidget *newi = gtk_menu_item_new_with_label("New");
    GtkWidget *openi = gtk_menu_item_new_with_label("Open");
    GtkWidget *savei = gtk_menu_item_new_with_label("Save");
    GtkWidget *saveasi = gtk_menu_item_new_with_label("Save As");
    GtkWidget *quiti = gtk_menu_item_new_with_label("Quit");
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), newi);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), openi);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), savei);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveasi);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quiti);

    GtkWidget *editmenu = gtk_menu_new();
    GtkWidget *edit = gtk_menu_item_new_with_mnemonic("_Edit");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit);

    GtkWidget *cui = gtk_menu_item_new_with_label("Cut");
    GtkWidget *copyi = gtk_menu_item_new_with_label("Copy");
    GtkWidget *pastei = gtk_menu_item_new_with_label("Paste");
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), cui);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), copyi);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), pastei);

    GtkWidget *toolsmenu = gtk_menu_new();
    GtkWidget *tools = gtk_menu_item_new_with_mnemonic("_Tools");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools), toolsmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), tools);

    GtkWidget *runi = gtk_menu_item_new_with_label("Run Command");
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), runi);

    GtkWidget *helpmenu = gtk_menu_new();
    GtkWidget *help = gtk_menu_item_new_with_mnemonic("_Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);

    GtkWidget *helpi = gtk_menu_item_new_with_label("Help");
    GtkWidget *abouti = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), helpi);
    gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), abouti);

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

    g_object_set_data(G_OBJECT(newi), "app", &app);
    g_object_set_data(G_OBJECT(openi), "app", &app);
    g_object_set_data(G_OBJECT(savei), "app", &app);
    g_object_set_data(G_OBJECT(saveasi), "app", &app);
    g_object_set_data(G_OBJECT(quiti), "app", &app);
    g_object_set_data(G_OBJECT(cui), "app", &app);
    g_object_set_data(G_OBJECT(copyi), "app", &app);
    g_object_set_data(G_OBJECT(pastei), "app", &app);
    g_object_set_data(G_OBJECT(runi), "app", &app);
    g_object_set_data(G_OBJECT(helpi), "app", &app);
    g_object_set_data(G_OBJECT(abouti), "app", &app);

    g_signal_connect(newi, "activate", G_CALLBACK(on_menu_activate), "new");
    g_signal_connect(openi, "activate", G_CALLBACK(on_menu_activate), "o");
    g_signal_connect(savei, "activate", G_CALLBACK(on_menu_activate), "w");
    g_signal_connect(saveasi, "activate", G_CALLBACK(on_menu_activate), ":w");
    g_signal_connect(quiti, "activate", G_CALLBACK(on_menu_activate), "q");
    g_signal_connect(cui, "activate", G_CALLBACK(on_menu_activate), "cut");
    g_signal_connect(copyi, "activate", G_CALLBACK(on_menu_activate), "copy");
    g_signal_connect(pastei, "activate", G_CALLBACK(on_menu_activate), "paste");
    g_signal_connect(runi, "activate", G_CALLBACK(run_command_dialog), &app);
    g_signal_connect(helpi, "activate", G_CALLBACK(on_menu_activate), "help");
    g_signal_connect(abouti, "activate", G_CALLBACK(on_menu_activate), "about");

    gtk_widget_show_all(app.window);

    gtk_main();
    g_free(app.current_file);
    return 0;
}
