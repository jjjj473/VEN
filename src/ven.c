#include <gtk/gtk.h>
#include <gio/gio.h>
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
    gboolean wrap;
} VenApp;

static void update_status(VenApp *app, const gchar *msg) {
    gchar *text = g_strdup_printf("[%s]%s%s",
        app->command_mode ? "COMMAND" : "INSERT",
        msg ? " - " : "", msg ? msg : "");
    gtk_statusbar_pop(GTK_STATUSBAR(app->statusbar), app->status_ctx);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_ctx, text);
    g_free(text);
}

static void show_error(VenApp *app, const gchar *msg) {
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void open_file_dialog(VenApp *app);
static void save_file_dialog(VenApp *app);
static void duplicate_line(VenApp *app);
static void delete_line(VenApp *app);
static void to_upper(VenApp *app);
static void to_lower(VenApp *app);
static void trim_trailing(VenApp *app);
static void insert_path(VenApp *app);
static void show_stats(VenApp *app);
static void tabs_to_spaces(VenApp *app);
static void spaces_to_tabs(VenApp *app);
static void toggle_wrap(VenApp *app);
static void insert_todo(VenApp *app);
static void add_line_numbers(VenApp *app);
static void comment_lines(VenApp *app);
static void uncomment_lines(VenApp *app);
static void sort_lines(VenApp *app);
static void reverse_lines(VenApp *app);
static void indent_selection(VenApp *app);
static void unindent_selection(VenApp *app);
static void clear_buffer(VenApp *app);
static void insert_uuid(VenApp *app);
static void run_file(VenApp *app);
static void open_config(VenApp *app);
static void duplicate_word(VenApp *app);
static void join_lines(VenApp *app);
static void remove_blank_lines(VenApp *app);
static void count_words(VenApp *app);

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
    GError *err = NULL;
    if (g_file_get_contents(fname, &content, &len, &err)) {
        gtk_text_buffer_set_text(app->buffer, content, len);
        g_free(content);
        g_free(app->current_file);
        app->current_file = g_strdup(fname);
        update_status(app, g_strdup_printf("Opened %s", fname));
    } else {
        show_error(app, err ? err->message : "Failed to open file");
        update_status(app, "Failed to open file");
        if (err)
            g_error_free(err);
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
    GError *err = NULL;
    if (g_file_set_contents(fname, text, -1, &err)) {
        g_free(app->current_file);
        app->current_file = g_strdup(fname);
        update_status(app, g_strdup_printf("Saved %s", fname));
    } else {
        show_error(app, err ? err->message : "Failed to save file");
        update_status(app, "Failed to save file");
        if (err)
            g_error_free(err);
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
        ":dup - duplicate current line\n"
        ":del - delete current line\n"
        ":upper - selection to upper case\n"
        ":lower - selection to lower case\n"
        ":trim - trim trailing whitespace\n"
        ":insertpath - insert file path\n"
        ":stats - show buffer statistics\n"
        ":tabs2spaces - convert tabs to spaces\n"
        ":spaces2tabs - convert spaces to tabs\n"
        ":wrap - toggle wrapping\n"
        ":todo - insert TODO label\n"
        ":addlnum - add line numbers\n"
        ":comment - comment lines\n"
        ":uncomment - uncomment lines\n"
        ":sort - sort lines\n"
        ":reverse - reverse lines\n"
        ":indent - indent selection\n"
        ":unindent - unindent selection\n"
        ":clear - clear buffer\n"
        ":uuid - insert UUID\n"
        ":runfile - run current file\n"
        ":openconf - open configuration\n"
        ":dupword - duplicate word\n"
        ":join - join lines\n"
        ":noblank - remove blank lines\n"
        ":wordcount - word count\n"
        ":/pattern - search\n"
        ":!cmd - run shell command\n"
        ":goto N - jump to line N\n"
        ":replace A B - replace first A with B\n"
        ":date - insert current date\n"
        ":visit - open Linuxksdteam.site\n"
        ":about - about dialog\n"
        ":help - show this help";
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void show_about(VenApp *app) {
    const gchar *msg = "VEN - simple GTK3 editor\nhttps://linuxksdteam.site";
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
    gboolean ok = g_spawn_command_line_sync(cmd, &output, NULL, NULL, &err);
    if (!ok) {
        show_error(app, err ? err->message : "Failed to run command");
    } else {
        GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app->window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            "%s", output ? output : "");
        gtk_dialog_run(GTK_DIALOG(d));
        gtk_widget_destroy(d);
    }
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

static void goto_line(VenApp *app, int line) {
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line(app->buffer, &iter, line > 0 ? line - 1 : 0);
    gtk_text_buffer_place_cursor(app->buffer, &iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(app->textview), &iter,
        0.0, TRUE, 0.5, 0.5);
    update_status(app, g_strdup_printf("Go to line %d", line));
}

static void replace_text(VenApp *app, const gchar *search, const gchar *replace) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    gchar *pos = g_strstr_len(text, -1, search);
    if (pos) {
        GString *str = g_string_new_len(text, pos - text);
        g_string_append(str, replace);
        g_string_append(str, pos + strlen(search));
        gtk_text_buffer_set_text(app->buffer, str->str, -1);
        update_status(app, "Replaced text");
        g_string_free(str, TRUE);
    } else {
        update_status(app, "Pattern not found");
    }
    g_free(text);
}

static void insert_date(VenApp *app) {
    GDateTime *dt = g_date_time_new_now_local();
    gchar *s = g_date_time_format(dt, "%Y-%m-%d %H:%M:%S");
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(app->buffer, &iter,
        gtk_text_buffer_get_insert(app->buffer));
    gtk_text_buffer_insert(app->buffer, &iter, s, -1);
    g_free(s);
    g_date_time_unref(dt);
}

static void open_website(VenApp *app) {
    (void)app;
    g_app_info_launch_default_for_uri("https://linuxksdteam.site", NULL, NULL);
}

static void duplicate_line(VenApp *app) {
    GtkTextIter iter, start, end;
    gtk_text_buffer_get_iter_at_mark(app->buffer, &iter,
        gtk_text_buffer_get_insert(app->buffer));
    start = iter;
    gtk_text_iter_set_line_offset(&start, 0);
    end = start;
    if (!gtk_text_iter_ends_line(&end))
        gtk_text_iter_forward_to_line_end(&end);
    gchar *line = gtk_text_buffer_get_text(app->buffer, &start, &end, FALSE);
    gtk_text_buffer_insert(app->buffer, &end, "\n", 1);
    gtk_text_buffer_insert(app->buffer, &end, line, -1);
    g_free(line);
}

static void delete_line(VenApp *app) {
    GtkTextIter iter, start, end;
    gtk_text_buffer_get_iter_at_mark(app->buffer, &iter,
        gtk_text_buffer_get_insert(app->buffer));
    start = iter;
    gtk_text_iter_set_line_offset(&start, 0);
    end = start;
    if (!gtk_text_iter_ends_line(&end))
        gtk_text_iter_forward_to_line_end(&end);
    gtk_text_iter_forward_char(&end);
    gtk_text_buffer_delete(app->buffer, &start, &end);
}

static void to_upper(VenApp *app) {
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(app->buffer, &start, &end))
        gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    gchar *up = g_utf8_strup(text, -1);
    gtk_text_buffer_delete(app->buffer, &start, &end);
    gtk_text_buffer_insert(app->buffer, &start, up, -1);
    g_free(text);
    g_free(up);
}

static void to_lower(VenApp *app) {
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(app->buffer, &start, &end))
        gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    gchar *low = g_utf8_strdown(text, -1);
    gtk_text_buffer_delete(app->buffer, &start, &end);
    gtk_text_buffer_insert(app->buffer, &start, low, -1);
    g_free(text);
    g_free(low);
}

static void trim_trailing(VenApp *app) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    gchar **lines = g_strsplit(text, "\n", -1);
    GString *out = g_string_new(NULL);
    for (int i = 0; lines[i]; i++) {
        gchar *tmp = g_strchomp(lines[i]);
        g_string_append(out, tmp);
        if (lines[i + 1])
            g_string_append_c(out, '\n');
    }
    gtk_text_buffer_set_text(app->buffer, out->str, -1);
    g_string_free(out, TRUE);
    g_strfreev(lines);
    g_free(text);
}

static void insert_path(VenApp *app) {
    if (!app->current_file)
        return;
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(app->buffer, &iter,
        gtk_text_buffer_get_insert(app->buffer));
    gtk_text_buffer_insert(app->buffer, &iter, app->current_file, -1);
}

static void show_stats(VenApp *app) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    int chars = g_utf8_strlen(text, -1);
    int lines = 1;
    for (const gchar *p = text; *p; p++)
        if (*p == '\n')
            lines++;
    gchar **words = g_strsplit_set(text, " \t\n", -1);
    int words_count = 0;
    for (; words[words_count]; words_count++)
        ;
    gchar *msg = g_strdup_printf("Lines: %d\nWords: %d\nChars: %d", lines,
        words_count ? words_count - 1 : 0, chars);
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
    g_free(msg);
    g_strfreev(words);
    g_free(text);
}

static void tabs_to_spaces(VenApp *app) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    gchar **parts = g_strsplit(text, "\t", -1);
    gchar *joined = g_strjoinv("    ", parts);
    gtk_text_buffer_set_text(app->buffer, joined, -1);
    g_free(joined);
    g_strfreev(parts);
    g_free(text);
}

static void spaces_to_tabs(VenApp *app) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    GRegex *rx = g_regex_new(" {4}", 0, 0, NULL);
    gchar *repl = g_regex_replace(rx, text, -1, 0, "\t", 0, NULL);
    gtk_text_buffer_set_text(app->buffer, repl, -1);
    g_free(repl);
    g_regex_unref(rx);
    g_free(text);
}

static void toggle_wrap(VenApp *app) {
    if (app->wrap) {
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->textview), GTK_WRAP_NONE);
        app->wrap = FALSE;
    } else {
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->textview), GTK_WRAP_WORD);
        app->wrap = TRUE;
    }
}

static void insert_todo(VenApp *app) {
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(app->buffer, &iter,
        gtk_text_buffer_get_insert(app->buffer));
    gtk_text_iter_set_line_offset(&iter, 0);
    gtk_text_buffer_insert(app->buffer, &iter, "TODO: ", -1);
}

static void add_line_numbers(VenApp *app) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    gchar **lines = g_strsplit(text, "\n", -1);
    GString *out = g_string_new(NULL);
    for (int i = 0; lines[i]; i++) {
        g_string_append_printf(out, "%d: %s", i + 1, lines[i]);
        if (lines[i + 1])
            g_string_append_c(out, '\n');
    }
    gtk_text_buffer_set_text(app->buffer, out->str, -1);
    g_string_free(out, TRUE);
    g_strfreev(lines);
    g_free(text);
}

static void comment_lines(VenApp *app) {
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(app->buffer, &start, &end))
        gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(text, "\n", -1);
    GString *out = g_string_new(NULL);
    for (int i = 0; lines[i]; i++) {
        g_string_append(out, "# ");
        g_string_append(out, lines[i]);
        if (lines[i + 1])
            g_string_append_c(out, '\n');
    }
    gtk_text_buffer_delete(app->buffer, &start, &end);
    gtk_text_buffer_insert(app->buffer, &start, out->str, -1);
    g_string_free(out, TRUE);
    g_strfreev(lines);
    g_free(text);
}

static void uncomment_lines(VenApp *app) {
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(app->buffer, &start, &end))
        gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(text, "\n", -1);
    GString *out = g_string_new(NULL);
    for (int i = 0; lines[i]; i++) {
        gchar *l = lines[i];
        if (g_str_has_prefix(l, "# "))
            l += 2;
        else if (g_str_has_prefix(l, "#"))
            l += 1;
        g_string_append(out, l);
        if (lines[i + 1])
            g_string_append_c(out, '\n');
    }
    gtk_text_buffer_delete(app->buffer, &start, &end);
    gtk_text_buffer_insert(app->buffer, &start, out->str, -1);
    g_string_free(out, TRUE);
    g_strfreev(lines);
    g_free(text);
}

static gint cmp_str(gconstpointer a, gconstpointer b) {
    return g_strcmp0(*(char * const *)a, *(char * const *)b);
}

static void sort_lines(VenApp *app) {
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(app->buffer, &start, &end))
        gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(text, "\n", -1);
    gsize len = g_strv_length(lines);
    g_qsort_with_data(lines, len, sizeof(char *), (GCompareDataFunc)cmp_str, NULL);
    gchar *joined = g_strjoinv("\n", lines);
    gtk_text_buffer_delete(app->buffer, &start, &end);
    gtk_text_buffer_insert(app->buffer, &start, joined, -1);
    g_free(joined);
    g_strfreev(lines);
    g_free(text);
}

static void reverse_lines(VenApp *app) {
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(app->buffer, &start, &end))
        gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(text, "\n", -1);
    gsize len = g_strv_length(lines);
    GString *out = g_string_new(NULL);
    for (gssize i = len - 1; i >= 0; i--) {
        g_string_append(out, lines[i]);
        if (i > 0)
            g_string_append_c(out, '\n');
    }
    gtk_text_buffer_delete(app->buffer, &start, &end);
    gtk_text_buffer_insert(app->buffer, &start, out->str, -1);
    g_string_free(out, TRUE);
    g_strfreev(lines);
    g_free(text);
}

static void indent_selection(VenApp *app) {
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(app->buffer, &start, &end))
        gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(text, "\n", -1);
    GString *out = g_string_new(NULL);
    for (int i = 0; lines[i]; i++) {
        g_string_append(out, "    ");
        g_string_append(out, lines[i]);
        if (lines[i + 1])
            g_string_append_c(out, '\n');
    }
    gtk_text_buffer_delete(app->buffer, &start, &end);
    gtk_text_buffer_insert(app->buffer, &start, out->str, -1);
    g_string_free(out, TRUE);
    g_strfreev(lines);
    g_free(text);
}

static void unindent_selection(VenApp *app) {
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(app->buffer, &start, &end))
        gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(text, "\n", -1);
    GString *out = g_string_new(NULL);
    for (int i = 0; lines[i]; i++) {
        gchar *l = lines[i];
        if (g_str_has_prefix(l, "    "))
            l += 4;
        g_string_append(out, l);
        if (lines[i + 1])
            g_string_append_c(out, '\n');
    }
    gtk_text_buffer_delete(app->buffer, &start, &end);
    gtk_text_buffer_insert(app->buffer, &start, out->str, -1);
    g_string_free(out, TRUE);
    g_strfreev(lines);
    g_free(text);
}

static void clear_buffer(VenApp *app) {
    gtk_text_buffer_set_text(app->buffer, "", -1);
}

static void insert_uuid(VenApp *app) {
    gchar *uuid = g_uuid_string_random();
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(app->buffer, &iter,
        gtk_text_buffer_get_insert(app->buffer));
    gtk_text_buffer_insert(app->buffer, &iter, uuid, -1);
    g_free(uuid);
}

static void run_file(VenApp *app) {
    if (!app->current_file)
        return;
    gchar *cmd = g_strdup_printf("sh %s", app->current_file);
    run_shell_command(app, cmd);
    g_free(cmd);
}

static void open_config(VenApp *app) {
    gchar *path = g_build_filename(g_get_home_dir(), ".venrc", NULL);
    open_file(app, path);
    g_free(path);
}

static void duplicate_word(VenApp *app) {
    GtkTextIter iter, start, end;
    gtk_text_buffer_get_iter_at_mark(app->buffer, &iter,
        gtk_text_buffer_get_insert(app->buffer));
    start = iter;
    gtk_text_iter_backward_word_start(&start);
    end = iter;
    gtk_text_iter_forward_word_end(&end);
    gchar *word = gtk_text_buffer_get_text(app->buffer, &start, &end, FALSE);
    gtk_text_buffer_insert(app->buffer, &end, word, -1);
    g_free(word);
}

static void join_lines(VenApp *app) {
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(app->buffer, &start, &end)) {
        gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    }
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, FALSE);
    gchar *joined = g_strdelimit(text, "\n", ' ');
    gtk_text_buffer_delete(app->buffer, &start, &end);
    gtk_text_buffer_insert(app->buffer, &start, joined, -1);
    g_free(text);
    /* joined points to same memory as text */
}

static void remove_blank_lines(VenApp *app) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    gchar **lines = g_strsplit(text, "\n", -1);
    GString *out = g_string_new(NULL);
    for (int i = 0; lines[i]; i++) {
        if (*lines[i] != '\0') {
            g_string_append(out, lines[i]);
            if (lines[i + 1])
                g_string_append_c(out, '\n');
        }
    }
    gtk_text_buffer_set_text(app->buffer, out->str, -1);
    g_string_free(out, TRUE);
    g_strfreev(lines);
    g_free(text);
}

static void count_words(VenApp *app) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app->buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(app->buffer, &start, &end, TRUE);
    gchar **words = g_strsplit_set(text, " \n\t", -1);
    int cnt = 0;
    for (int i = 0; words[i]; i++)
        if (*words[i])
            cnt++;
    gchar *msg = g_strdup_printf("Word count: %d", cnt);
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
    g_free(msg);
    g_strfreev(words);
    g_free(text);
}

static void goto_line_dialog(GtkWidget *widget, gpointer data) {
    VenApp *app = data;
    GtkWidget *d = gtk_dialog_new_with_buttons("Go To Line", GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Go", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry), GTK_INPUT_PURPOSE_NUMBER);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), entry, TRUE, TRUE, 0);
    gtk_widget_show(entry);
    int line = -1;
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT)
        line = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));
    gtk_widget_destroy(d);
    if (line > 0)
        goto_line(app, line);
}

static void replace_dialog(GtkWidget *widget, gpointer data) {
    VenApp *app = data;
    GtkWidget *d = gtk_dialog_new_with_buttons("Replace", GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Replace", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *search = gtk_entry_new();
    GtkWidget *repl = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Search for:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), search, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Replace with:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), repl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), box, TRUE, TRUE, 0);
    gtk_widget_show_all(box);
    gchar *s = NULL; gchar *r = NULL;
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        s = g_strdup(gtk_entry_get_text(GTK_ENTRY(search)));
        r = g_strdup(gtk_entry_get_text(GTK_ENTRY(repl)));
    }
    gtk_widget_destroy(d);
    if (s && r)
        replace_text(app, s, r);
    g_free(s);
    g_free(r);
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
    } else if (g_strcmp0(cmd, "dup") == 0 || g_strcmp0(cmd, ":dup") == 0) {
        duplicate_line(app);
    } else if (g_strcmp0(cmd, "del") == 0 || g_strcmp0(cmd, ":del") == 0) {
        delete_line(app);
    } else if (g_strcmp0(cmd, "upper") == 0 || g_strcmp0(cmd, ":upper") == 0) {
        to_upper(app);
    } else if (g_strcmp0(cmd, "lower") == 0 || g_strcmp0(cmd, ":lower") == 0) {
        to_lower(app);
    } else if (g_strcmp0(cmd, "trim") == 0 || g_strcmp0(cmd, ":trim") == 0) {
        trim_trailing(app);
    } else if (g_strcmp0(cmd, "insertpath") == 0 || g_strcmp0(cmd, ":insertpath") == 0) {
        insert_path(app);
    } else if (g_strcmp0(cmd, "stats") == 0 || g_strcmp0(cmd, ":stats") == 0) {
        show_stats(app);
    } else if (g_strcmp0(cmd, "tabs2spaces") == 0 || g_strcmp0(cmd, ":tabs2spaces") == 0) {
        tabs_to_spaces(app);
    } else if (g_strcmp0(cmd, "spaces2tabs") == 0 || g_strcmp0(cmd, ":spaces2tabs") == 0) {
        spaces_to_tabs(app);
    } else if (g_strcmp0(cmd, "wrap") == 0 || g_strcmp0(cmd, ":wrap") == 0) {
        toggle_wrap(app);
    } else if (g_strcmp0(cmd, "todo") == 0 || g_strcmp0(cmd, ":todo") == 0) {
        insert_todo(app);
    } else if (g_strcmp0(cmd, "addlnum") == 0 || g_strcmp0(cmd, ":addlnum") == 0) {
        add_line_numbers(app);
    } else if (g_strcmp0(cmd, "comment") == 0 || g_strcmp0(cmd, ":comment") == 0) {
        comment_lines(app);
    } else if (g_strcmp0(cmd, "uncomment") == 0 || g_strcmp0(cmd, ":uncomment") == 0) {
        uncomment_lines(app);
    } else if (g_strcmp0(cmd, "sort") == 0 || g_strcmp0(cmd, ":sort") == 0) {
        sort_lines(app);
    } else if (g_strcmp0(cmd, "reverse") == 0 || g_strcmp0(cmd, ":reverse") == 0) {
        reverse_lines(app);
    } else if (g_strcmp0(cmd, "indent") == 0 || g_strcmp0(cmd, ":indent") == 0) {
        indent_selection(app);
    } else if (g_strcmp0(cmd, "unindent") == 0 || g_strcmp0(cmd, ":unindent") == 0) {
        unindent_selection(app);
    } else if (g_strcmp0(cmd, "clear") == 0 || g_strcmp0(cmd, ":clear") == 0) {
        clear_buffer(app);
    } else if (g_strcmp0(cmd, "uuid") == 0 || g_strcmp0(cmd, ":uuid") == 0) {
        insert_uuid(app);
    } else if (g_strcmp0(cmd, "runfile") == 0 || g_strcmp0(cmd, ":runfile") == 0) {
        run_file(app);
    } else if (g_strcmp0(cmd, "openconf") == 0 || g_strcmp0(cmd, ":openconf") == 0) {
        open_config(app);
    } else if (g_strcmp0(cmd, "dupword") == 0 || g_strcmp0(cmd, ":dupword") == 0) {
        duplicate_word(app);
    } else if (g_strcmp0(cmd, "join") == 0 || g_strcmp0(cmd, ":join") == 0) {
        join_lines(app);
    } else if (g_strcmp0(cmd, "noblank") == 0 || g_strcmp0(cmd, ":noblank") == 0) {
        remove_blank_lines(app);
    } else if (g_strcmp0(cmd, "wordcount") == 0 || g_strcmp0(cmd, ":wordcount") == 0) {
        count_words(app);
    } else if (g_str_has_prefix(cmd, "goto ") || g_str_has_prefix(cmd, ":goto")) {
        const gchar *arg = strstr(cmd, " ");
        if (arg)
            goto_line(app, atoi(arg + 1));
    } else if (g_str_has_prefix(cmd, "replace ") || g_str_has_prefix(cmd, ":replace")) {
        const gchar *arg = strstr(cmd, " ");
        if (arg) {
            gchar **parts = g_strsplit(arg + 1, " ", 2);
            if (parts[0] && parts[1])
                replace_text(app, parts[0], parts[1]);
            g_strfreev(parts);
        }
    } else if (g_strcmp0(cmd, "date") == 0 || g_strcmp0(cmd, ":date") == 0) {
        insert_date(app);
    } else if (g_strcmp0(cmd, "visit") == 0 || g_strcmp0(cmd, ":visit") == 0) {
        open_website(app);
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
    GtkWidget *gotoi = gtk_menu_item_new_with_label("Go To Line");
    GtkWidget *replacei = gtk_menu_item_new_with_label("Replace");
    GtkWidget *datei = gtk_menu_item_new_with_label("Insert Date");
    GtkWidget *visiti = gtk_menu_item_new_with_label("Visit Website");
    GtkWidget *dupi = gtk_menu_item_new_with_label("Duplicate Line");
    GtkWidget *deli = gtk_menu_item_new_with_label("Delete Line");
    GtkWidget *upperi = gtk_menu_item_new_with_label("Uppercase");
    GtkWidget *loweri = gtk_menu_item_new_with_label("Lowercase");
    GtkWidget *trimi = gtk_menu_item_new_with_label("Trim Whitespace");
    GtkWidget *pathi = gtk_menu_item_new_with_label("Insert Path");
    GtkWidget *statsi = gtk_menu_item_new_with_label("Statistics");
    GtkWidget *t2si = gtk_menu_item_new_with_label("Tabs->Spaces");
    GtkWidget *s2ti = gtk_menu_item_new_with_label("Spaces->Tabs");
    GtkWidget *wrapi = gtk_menu_item_new_with_label("Toggle Wrap");
    GtkWidget *commenti = gtk_menu_item_new_with_label("Comment Lines");
    GtkWidget *uncommenti = gtk_menu_item_new_with_label("Uncomment Lines");
    GtkWidget *sorti = gtk_menu_item_new_with_label("Sort Lines");
    GtkWidget *reversei = gtk_menu_item_new_with_label("Reverse Lines");
    GtkWidget *indenti = gtk_menu_item_new_with_label("Indent");
    GtkWidget *unindenti = gtk_menu_item_new_with_label("Unindent");
    GtkWidget *cleari = gtk_menu_item_new_with_label("Clear Buffer");
    GtkWidget *uuidi = gtk_menu_item_new_with_label("Insert UUID");
    GtkWidget *runfilei = gtk_menu_item_new_with_label("Run File");
    GtkWidget *confopeni = gtk_menu_item_new_with_label("Open Config");
    GtkWidget *dupwordi = gtk_menu_item_new_with_label("Duplicate Word");
    GtkWidget *joini = gtk_menu_item_new_with_label("Join Lines");
    GtkWidget *noblanki = gtk_menu_item_new_with_label("Remove Blank Lines");
    GtkWidget *wordcounti = gtk_menu_item_new_with_label("Word Count");
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), runi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), gotoi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), replacei);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), datei);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), visiti);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), dupi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), deli);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), upperi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), loweri);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), trimi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), pathi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), statsi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), t2si);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), s2ti);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), wrapi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), commenti);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), uncommenti);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), sorti);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), reversei);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), indenti);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), unindenti);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), cleari);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), uuidi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), runfilei);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), confopeni);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), dupwordi);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), joini);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), noblanki);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), wordcounti);

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
    app.wrap = TRUE;
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app.textview), GTK_WRAP_WORD);
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
    g_object_set_data(G_OBJECT(gotoi), "app", &app);
    g_object_set_data(G_OBJECT(replacei), "app", &app);
    g_object_set_data(G_OBJECT(datei), "app", &app);
    g_object_set_data(G_OBJECT(visiti), "app", &app);
    g_object_set_data(G_OBJECT(dupi), "app", &app);
    g_object_set_data(G_OBJECT(deli), "app", &app);
    g_object_set_data(G_OBJECT(upperi), "app", &app);
    g_object_set_data(G_OBJECT(loweri), "app", &app);
    g_object_set_data(G_OBJECT(trimi), "app", &app);
    g_object_set_data(G_OBJECT(pathi), "app", &app);
    g_object_set_data(G_OBJECT(statsi), "app", &app);
    g_object_set_data(G_OBJECT(t2si), "app", &app);
    g_object_set_data(G_OBJECT(s2ti), "app", &app);
    g_object_set_data(G_OBJECT(wrapi), "app", &app);
    g_object_set_data(G_OBJECT(commenti), "app", &app);
    g_object_set_data(G_OBJECT(uncommenti), "app", &app);
    g_object_set_data(G_OBJECT(sorti), "app", &app);
    g_object_set_data(G_OBJECT(reversei), "app", &app);
    g_object_set_data(G_OBJECT(indenti), "app", &app);
    g_object_set_data(G_OBJECT(unindenti), "app", &app);
    g_object_set_data(G_OBJECT(cleari), "app", &app);
    g_object_set_data(G_OBJECT(uuidi), "app", &app);
    g_object_set_data(G_OBJECT(runfilei), "app", &app);
    g_object_set_data(G_OBJECT(confopeni), "app", &app);
    g_object_set_data(G_OBJECT(dupwordi), "app", &app);
    g_object_set_data(G_OBJECT(joini), "app", &app);
    g_object_set_data(G_OBJECT(noblanki), "app", &app);
    g_object_set_data(G_OBJECT(wordcounti), "app", &app);
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
    g_signal_connect(gotoi, "activate", G_CALLBACK(goto_line_dialog), &app);
    g_signal_connect(replacei, "activate", G_CALLBACK(replace_dialog), &app);
    g_signal_connect(datei, "activate", G_CALLBACK(on_menu_activate), "date");
    g_signal_connect(visiti, "activate", G_CALLBACK(on_menu_activate), "visit");
    g_signal_connect(dupi, "activate", G_CALLBACK(on_menu_activate), "dup");
    g_signal_connect(deli, "activate", G_CALLBACK(on_menu_activate), "del");
    g_signal_connect(upperi, "activate", G_CALLBACK(on_menu_activate), "upper");
    g_signal_connect(loweri, "activate", G_CALLBACK(on_menu_activate), "lower");
    g_signal_connect(trimi, "activate", G_CALLBACK(on_menu_activate), "trim");
    g_signal_connect(pathi, "activate", G_CALLBACK(on_menu_activate), "insertpath");
    g_signal_connect(statsi, "activate", G_CALLBACK(on_menu_activate), "stats");
    g_signal_connect(t2si, "activate", G_CALLBACK(on_menu_activate), "tabs2spaces");
    g_signal_connect(s2ti, "activate", G_CALLBACK(on_menu_activate), "spaces2tabs");
    g_signal_connect(wrapi, "activate", G_CALLBACK(on_menu_activate), "wrap");
    g_signal_connect(commenti, "activate", G_CALLBACK(on_menu_activate), "comment");
    g_signal_connect(uncommenti, "activate", G_CALLBACK(on_menu_activate), "uncomment");
    g_signal_connect(sorti, "activate", G_CALLBACK(on_menu_activate), "sort");
    g_signal_connect(reversei, "activate", G_CALLBACK(on_menu_activate), "reverse");
    g_signal_connect(indenti, "activate", G_CALLBACK(on_menu_activate), "indent");
    g_signal_connect(unindenti, "activate", G_CALLBACK(on_menu_activate), "unindent");
    g_signal_connect(cleari, "activate", G_CALLBACK(on_menu_activate), "clear");
    g_signal_connect(uuidi, "activate", G_CALLBACK(on_menu_activate), "uuid");
    g_signal_connect(runfilei, "activate", G_CALLBACK(on_menu_activate), "runfile");
    g_signal_connect(confopeni, "activate", G_CALLBACK(on_menu_activate), "openconf");
    g_signal_connect(dupwordi, "activate", G_CALLBACK(on_menu_activate), "dupword");
    g_signal_connect(joini, "activate", G_CALLBACK(on_menu_activate), "join");
    g_signal_connect(noblanki, "activate", G_CALLBACK(on_menu_activate), "noblank");
    g_signal_connect(wordcounti, "activate", G_CALLBACK(on_menu_activate), "wordcount");
    g_signal_connect(helpi, "activate", G_CALLBACK(on_menu_activate), "help");
    g_signal_connect(abouti, "activate", G_CALLBACK(on_menu_activate), "about");

    gtk_widget_show_all(app.window);

    gtk_main();
    g_free(app.current_file);
    return 0;
}
