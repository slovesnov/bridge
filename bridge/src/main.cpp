/*
 * main.cpp
 *
 *       Created on: 10.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "Frame.h"
#include "aslov.h"


//signature https://developer.gnome.org/gio/stable/GApplication.html#GApplication-open
static void application_open(GtkApplication *application, GFile **files, gint n_files,
		const gchar *hint, gpointer data) {
	int i;
	char* c;
	std::string s;

	//no encode string here, tested on russian filenames using windows association
	for (i = 0; i < n_files; i++) {
		if (i != 0) {
			s += G_SEARCHPATH_SEPARATOR;
		}
		c = g_file_get_path(files[i]);
		s += c;
		g_free(c);
	}

	//list is always is NULL if application is created with G_APPLICATION_NON_UNIQUE flag
	GList* list = gtk_application_get_windows(application);
	if (list) {
		g_signal_emit_by_name(list->data, OPEN_FILE_SIGNAL_NAME, s.c_str());
		gtk_window_present(GTK_WINDOW(list->data));
	}
	else {
		Config config;
		//Frame constructor & g_signal_emit_by_name call the same function which do check for empty string
		Frame(application, s.c_str());
	}
}

//signature https://developer.gnome.org/gio/stable/GApplication.html#GApplication-activate
static void activate(GtkApplication *application, gpointer data) {
	application_open(application, NULL, 0, NULL, data);
}

/* On file association Windows is always use absolute path for argv[0] and argv[1]
 * For console commands, something like this "D:\slovesno\MyProjects\eclipse\bridge_cpp\bridge\bridge.exe 1.bts"
 * need to get absolute file path and need to pass arguments to other application if option "allow one instance only" is on
 * The solution is store absolute path in config to load language, images etc.
 * Function open() has info about command line arguments, so just union files in one string and call signal
 *
 * in gtk 3.20.6 allow load config (some css operations do work only after application is created)
 * so in main() function call only static Config functions, and do fully load Config in open() function
 */
int main(int argc, char *argv[]) {
	aslovInit(argv);
	GApplicationFlags flags = GApplicationFlags(
			G_APPLICATION_HANDLES_OPEN
					| (Config::allowOnlyOneInstance(argv[0]) ?
							G_APPLICATION_FLAGS_NONE : G_APPLICATION_NON_UNIQUE));
	GtkApplication *app = gtk_application_new(Config::getUniqueApplicationName().c_str(), flags);
	g_signal_connect(app, "activate", G_CALLBACK (activate), gpointer(argv[0])); //this function is called when application has no arguments
	g_signal_connect(app, "open", G_CALLBACK (application_open), gpointer(argv[0])); //this function is called when application has arguments
	g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return 0;
}
