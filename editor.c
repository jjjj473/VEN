#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <signal.h>

typedef struct {
    GtkWidget *window;
    GtkSourceBuffer *buffer;
    GtkWidget *view;
    GtkTextTag *bold_tag;
    GtkTextTag *italic_tag;
    GtkWidget *status;
    guint status_ctx;
    int font_size;
    GtkCssProvider *font_provider;
} App;

static gboolean on_right_click(GtkWidget *widget, GdkEventButton *event, App *app);
static void update_status(GtkTextBuffer *buffer, App *app);
static void on_preferences(GtkWidget *w, App *app);
static void update_font(App *app);
static void on_task_manager(GtkWidget *w, App *app);
static void refresh_process_list(GtkListStore *store);
static void on_kill_process(GtkButton *button, gpointer user_data);
static void show_task_manager(App *app);

static void load_css(void) {
    GdkScreen *screen = gdk_screen_get_default();
    if (!screen)
        return;
    GtkCssProvider *provider = gtk_css_provider_new();
    const gchar *css =
        "* {font-family: Tahoma, sans-serif;}\n"
        "window {background-color: #c0c0c0;}\n"
        "menubar, menu, toolbar, statusbar {"
        "  background-color: #d4d0c8;"
        "  border: 2px outset #ffffff;"
        "}\n"
        "button {"
        "  background-image: none;"
        "  background-color: #e0e0e0;"
        "  border: 2px outset #ffffff;"
        "  padding: 2px 6px;"
        "}\n"
        "textview {"
        "  background-color: white;"
        "  border: 2px inset #808080;"
        "}\n"
        "entry, spinbutton, treeview {"
        "  background-color: white;"
        "}\n"
        "scrolledwindow {"
        "  border: 2px inset #808080;"
        "}\n"
        "menuitem:hover {"
        "  background-color: #0a246a;"
        "  color: white;"
        "}\n"
        "statusbar {padding: 2px;}";
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(screen,
            GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void update_font(App *app) {
    if (!app->font_provider)
        return;
    char css[64];
    g_snprintf(css, sizeof(css),
               "#editor_view { font-size: %dpt; }",
               app->font_size);
    gtk_css_provider_load_from_data(app->font_provider, css, -1, NULL);
}

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

static void update_status(GtkTextBuffer *buffer, App *app) {
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buffer, &iter,
            gtk_text_buffer_get_insert(buffer));
    int line = gtk_text_iter_get_line(&iter) + 1;
    int col = gtk_text_iter_get_line_offset(&iter) + 1;
    char msg[64];
    g_snprintf(msg, sizeof(msg), "Line %d, Col %d", line, col);
    gtk_statusbar_pop(GTK_STATUSBAR(app->status), app->status_ctx);
    gtk_statusbar_push(GTK_STATUSBAR(app->status), app->status_ctx, msg);
}

static void on_preferences(GtkWidget *w, App *app) {
    GtkWidget *dlg = gtk_dialog_new_with_buttons("Preferences",
            GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Apply", GTK_RESPONSE_ACCEPT,
            NULL);
    GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    GtkWidget *spin = gtk_spin_button_new_with_range(8, 40, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), app->font_size);
    gtk_box_pack_start(GTK_BOX(box), spin, FALSE, FALSE, 4);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        app->font_size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        update_font(app);
    }
    gtk_widget_destroy(dlg);
}

static void on_task_manager(GtkWidget *w, App *app) {
    show_task_manager(app);
}

typedef struct {
    GtkTreeSelection *selection;
    GtkListStore *store;
} KillData;

static void refresh_process_list(GtkListStore *store) {
    gtk_list_store_clear(store);
    gchar *out = NULL;
    GError *err = NULL;
    if (g_spawn_command_line_sync("ps -eo pid,comm,%cpu,%mem --no-headers", &out, NULL, NULL, &err)) {
        char **lines = g_strsplit(out, "\n", -1);
        for (int i = 0; lines[i]; i++) {
            char pid[16], comm[256], cpu[16], mem[16];
            if (sscanf(lines[i], "%15s %255s %15s %15s", pid, comm, cpu, mem) == 4) {
                GtkTreeIter it;
                gtk_list_store_append(store, &it);
                gtk_list_store_set(store, &it,
                                   0, pid,
                                   1, comm,
                                   2, cpu,
                                   3, mem,
                                   -1);
            }
        }
        g_strfreev(lines);
        g_free(out);
    } else {
        g_warning("Failed to run ps: %s", err->message);
        g_clear_error(&err);
    }
}

static void on_kill_process(GtkButton *button, gpointer user_data) {
    KillData *data = user_data;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(data->selection, NULL, &iter)) {
        gchar *pid_str;
        gtk_tree_model_get(GTK_TREE_MODEL(data->store), &iter, 0, &pid_str, -1);
        int pid = atoi(pid_str);
        g_free(pid_str);
        if (pid > 0)
            kill(pid, SIGTERM);
        refresh_process_list(data->store);
    }
}

static void show_task_manager(App *app) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Task Manager",
            GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
            "_Close", GTK_RESPONSE_CLOSE,
            NULL);
    GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkListStore *store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), 0, "PID",
            gtk_cell_renderer_text_new(), "text", 0, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), 1, "Command",
            gtk_cell_renderer_text_new(), "text", 1, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), 2, "CPU%",
            gtk_cell_renderer_text_new(), "text", 2, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), 3, "MEM%",
            gtk_cell_renderer_text_new(), "text", 3, NULL);
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *kill_btn = gtk_button_new_with_label("Kill Process");
    gtk_box_pack_end(GTK_BOX(btn_box), kill_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(btn_box), refresh, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btn_box, FALSE, FALSE, 4);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    KillData kd = { sel, store };
    g_signal_connect(kill_btn, "clicked", G_CALLBACK(on_kill_process), &kd);
    g_signal_connect_swapped(refresh, "clicked", G_CALLBACK(refresh_process_list), store);

    refresh_process_list(store);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_object_unref(store);
}

static gboolean on_right_click(GtkWidget *widget, GdkEventButton *event, App *app) {
    if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_SECONDARY) {
        GtkWidget *menu = gtk_menu_new();

        GtkWidget *mi_open = gtk_menu_item_new_with_label("Open");
        GtkWidget *mi_save = gtk_menu_item_new_with_label("Save");
        GtkWidget *mi_undo = gtk_menu_item_new_with_label("Undo");
        GtkWidget *mi_redo = gtk_menu_item_new_with_label("Redo");
        GtkWidget *mi_find = gtk_menu_item_new_with_label("Find/Replace");
        GtkWidget *mi_upper = gtk_menu_item_new_with_label("Uppercase");
        GtkWidget *mi_lower = gtk_menu_item_new_with_label("Lowercase");
        GtkWidget *mi_bold = gtk_menu_item_new_with_label("Bold");
        GtkWidget *mi_italic = gtk_menu_item_new_with_label("Italic");
        GtkWidget *mi_task = gtk_menu_item_new_with_label("Task Manager");

        g_signal_connect(mi_open, "activate", G_CALLBACK(on_open), app);
        g_signal_connect(mi_save, "activate", G_CALLBACK(on_save), app);
        g_signal_connect(mi_undo, "activate", G_CALLBACK(on_undo), app);
        g_signal_connect(mi_redo, "activate", G_CALLBACK(on_redo), app);
        g_signal_connect(mi_find, "activate", G_CALLBACK(on_find_replace), app);
        g_signal_connect(mi_upper, "activate", G_CALLBACK(on_upper), app);
        g_signal_connect(mi_lower, "activate", G_CALLBACK(on_lower), app);
        g_signal_connect(mi_bold, "activate", G_CALLBACK(on_bold), app);
        g_signal_connect(mi_italic, "activate", G_CALLBACK(on_italic), app);
        g_signal_connect(mi_task, "activate", G_CALLBACK(on_task_manager), app);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_open);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_save);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_undo);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_redo);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_find);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_upper);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_lower);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_bold);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_italic);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi_task);

        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*)event);
        return TRUE;
    }
    return FALSE;
}

static void activate(GtkApplication *app_g, gpointer user_data) {
    App *app = user_data;
    load_css();
    app->window = gtk_application_window_new(app_g);
    gtk_window_set_title(GTK_WINDOW(app->window), "VEN Editor");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 800, 600);
    app->font_size = 12;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);

    GtkAccelGroup *accel = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(app->window), accel);

    GtkWidget *menubar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *edit_menu = gtk_menu_new();
    GtkWidget *format_menu = gtk_menu_new();
    GtkWidget *view_menu = gtk_menu_new();

    GtkWidget *mi_file = gtk_menu_item_new_with_label("File");
    GtkWidget *mi_edit = gtk_menu_item_new_with_label("Edit");
    GtkWidget *mi_format = gtk_menu_item_new_with_label("Format");
    GtkWidget *mi_view = gtk_menu_item_new_with_label("View");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi_file), file_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi_edit), edit_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi_format), format_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi_view), view_menu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), mi_file);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), mi_edit);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), mi_format);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), mi_view);

    GtkWidget *mi_open = gtk_menu_item_new_with_label("Open");
    GtkWidget *mi_save = gtk_menu_item_new_with_label("Save");
    GtkWidget *mi_quit = gtk_menu_item_new_with_label("Quit");
    gtk_widget_add_accelerator(mi_open, "activate", accel, GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(mi_save, "activate", accel, GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(mi_quit, "activate", accel, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(mi_open, "activate", G_CALLBACK(on_open), app);
    g_signal_connect(mi_save, "activate", G_CALLBACK(on_save), app);
    g_signal_connect_swapped(mi_quit, "activate", G_CALLBACK(gtk_window_close), app->window);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), mi_open);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), mi_save);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), mi_quit);

    GtkWidget *mi_undo = gtk_menu_item_new_with_label("Undo");
    GtkWidget *mi_redo = gtk_menu_item_new_with_label("Redo");
    GtkWidget *mi_find = gtk_menu_item_new_with_label("Find/Replace");
    gtk_widget_add_accelerator(mi_undo, "activate", accel, GDK_KEY_z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(mi_redo, "activate", accel, GDK_KEY_z, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(mi_find, "activate", accel, GDK_KEY_f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(mi_undo, "activate", G_CALLBACK(on_undo), app);
    g_signal_connect(mi_redo, "activate", G_CALLBACK(on_redo), app);
    g_signal_connect(mi_find, "activate", G_CALLBACK(on_find_replace), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), mi_undo);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), mi_redo);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), mi_find);

    GtkWidget *mi_upper = gtk_menu_item_new_with_label("Uppercase");
    GtkWidget *mi_lower = gtk_menu_item_new_with_label("Lowercase");
    GtkWidget *mi_bold = gtk_menu_item_new_with_label("Bold");
    GtkWidget *mi_italic = gtk_menu_item_new_with_label("Italic");
    gtk_widget_add_accelerator(mi_upper, "activate", accel, GDK_KEY_u, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(mi_lower, "activate", accel, GDK_KEY_l, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(mi_bold, "activate", accel, GDK_KEY_b, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(mi_italic, "activate", accel, GDK_KEY_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(mi_upper, "activate", G_CALLBACK(on_upper), app);
    g_signal_connect(mi_lower, "activate", G_CALLBACK(on_lower), app);
    g_signal_connect(mi_bold, "activate", G_CALLBACK(on_bold), app);
    g_signal_connect(mi_italic, "activate", G_CALLBACK(on_italic), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(format_menu), mi_upper);
    gtk_menu_shell_append(GTK_MENU_SHELL(format_menu), mi_lower);
    gtk_menu_shell_append(GTK_MENU_SHELL(format_menu), mi_bold);
    gtk_menu_shell_append(GTK_MENU_SHELL(format_menu), mi_italic);

    GtkWidget *mi_prefs = gtk_menu_item_new_with_label("Preferences");
    g_signal_connect(mi_prefs, "activate", G_CALLBACK(on_preferences), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), mi_prefs);

    GtkWidget *mi_task = gtk_menu_item_new_with_label("Task Manager");
    g_signal_connect(mi_task, "activate", G_CALLBACK(on_task_manager), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), mi_task);

    app->view = gtk_source_view_new();
    app->buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->view)));
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(app->view), TRUE);
    gtk_widget_set_name(app->view, "editor_view");
    app->font_provider = gtk_css_provider_new();
    gtk_style_context_add_provider(gtk_widget_get_style_context(app->view),
            GTK_STYLE_PROVIDER(app->font_provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    update_font(app);

    app->bold_tag = gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(app->buffer), "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    app->italic_tag = gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(app->buffer), "italic", "style", PANGO_STYLE_ITALIC, NULL);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_widget_set_name(toolbar, "toolbar");
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(scrolled), app->view);

    app->status = gtk_statusbar_new();
    app->status_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(app->status), "pos");
    gtk_box_pack_start(GTK_BOX(vbox), app->status, FALSE, FALSE, 0);
    g_signal_connect(app->buffer, "notify::cursor-position", G_CALLBACK(update_status), app);
    update_status(GTK_TEXT_BUFFER(app->buffer), app);

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

    g_signal_connect(app->view, "button-press-event", G_CALLBACK(on_right_click), app);

    gtk_widget_show_all(app->window);
}

int main(int argc, char **argv) {
    GtkApplication *app_g;
    int status;
    App app = {0};
    const char *display = g_getenv("DISPLAY");
    if (!display || !*display) {
        g_printerr("DISPLAY not set. Unable to launch GUI.\n");
        return 1;
    }

    app_g = gtk_application_new("com.example.veneditor", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app_g, "activate", G_CALLBACK(activate), &app);
    status = g_application_run(G_APPLICATION(app_g), argc, argv);
    g_object_unref(app_g);
    if (app.font_provider)
        g_object_unref(app.font_provider);
    return status;
}

