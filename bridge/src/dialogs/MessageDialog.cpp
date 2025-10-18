/*
 * MessageDialog.cpp
 *
 *       Created on: 04.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "MessageDialog.h"
#include "../Frame.h"

static MessageDialog *pd;

static const char *IMG[] = { "ok24.png", "cancel24.png" };

static void button_clicked(GtkWidget *widget, int i) {
	pd->clickButton(i);
}

MessageDialog::MessageDialog(MESSAGE_ICON_TYPE iconType, const char *s,
		BUTTONS_DIALOG_TYPE type, Widget *parent) :
		ButtonsDialog(MENU_INVALID, type, parent, true) {
	GtkWidget *w, *w1;

	//pd=this;not need

	//if run application using command line "bridge.exe 1.bts" and got some errors, Frame is not visible, show the window
	gframe->showIfNotVisible();

	CSize sz = getArea().getTextExtents(TextWithAttributes(s));
	int width, height;
	if (sz.cx > 800) {
		width = 800;
		height = sz.cy * sz.cx / width;
	} else {
		width = sz.cx;
		height = sz.cy;
	}

	if (height > 700) {
		height = 700;
	}

	w1 = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(w1), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(w1), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(w1), GTK_WRAP_WORD);

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w1));
//	gtk_text_buffer_set_text(buffer, s, -1);

//markup for SolveAllDealsDialog
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
	gtk_text_buffer_insert_markup(buffer, &iter, s, -1);

	w = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(w), w1);
	//+1 somehow ProblemVector::showError() not fully showed don't know why
	gtk_widget_set_size_request(w, width + 1, height);

	if (iconType == MESSAGE_ICON_NONE) {
		w1 = w;
	} else {
		w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
		gtk_container_add(GTK_CONTAINER(w1),
				image(IMG[iconType == MESSAGE_ICON_ERROR])); //no stretch
		gtk_box_pack_start(GTK_BOX(w1), w, TRUE, TRUE, 0); //stretch
	}

	gtk_box_pack_start(GTK_BOX(getContentArea()), w1, TRUE, TRUE, 0); //stretch

	show();
}

MessageDialog::MessageDialog(const VParseException &e, BUTTONS_DIALOG_TYPE type,
		Widget *parent) :
		ButtonsDialog(MENU_INVALID, type, parent) {
	GtkWidget *w, *g;
	int i, j;

	pd = this;

	//if run application using command line "bridge.exe 1.bts" and got some errors, Frame is not visible, show the window
	gframe->showIfNotVisible();

	g = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g), 3);
	gtk_grid_set_row_spacing(GTK_GRID(g), 3);

	w = gtk_label_new(
			getString(STRING_ERRORS_WAS_FOUND_DURING_FILE_PROCEEDING));
	gtk_grid_attach(GTK_GRID(g), w, 0, 0, 4, 1);

	i = 0;
	w = gtk_label_new(getString(STRING_FILE));
	gtk_grid_attach(GTK_GRID(g), w, i++, 1, 1, 1);

	w = gtk_label_new(getString(STRING_ERROR));
	gtk_grid_attach(GTK_GRID(g), w, i++, 1, 1, 1);

	w = gtk_label_new(getString(MENU_ADDONS));
	gtk_grid_attach(GTK_GRID(g), w, i++, 1, 1, 1);

	w = gtk_label_new(getString(MENU_VIEW));
	gtk_grid_attach(GTK_GRID(g), w, i++, 1, 1, 1);

	j = 2;
	for (auto &a : e) {
		i = 0;
		w = gtk_label_new(getFileInfo(a.m_parseFile, FILEINFO::NAME).c_str());
		gtk_grid_attach(GTK_GRID(g), w, i++, j, 1, 1);

		w = gtk_label_new(a.getErrorString().c_str());
		gtk_grid_attach(GTK_GRID(g), w, i++, j, 1, 1);

		w = gtk_label_new(a.getPlaceInfo().c_str());
		gtk_grid_attach(GTK_GRID(g), w, i++, j, 1, 1);

		w = createButton("add24.png", "");
		gtk_grid_attach(GTK_GRID(g), w, i++, j, 1, 1);

		g_signal_connect(w, "clicked", G_CALLBACK(button_clicked),
				gpointer(m_content.size()));

		m_content.push_back(a.m_content);
		j++;
	}

	if (e.size() > 13) {
		w = gtk_scrolled_window_new(NULL, NULL);

		gtk_container_add(GTK_CONTAINER(w), g);
		gtk_container_add(GTK_CONTAINER(getContentArea()), w);

		showAll();
		//only after showAll() can get width
		gtk_widget_get_preferred_width(g, NULL, &i);

		g = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(w));
		gtk_widget_get_preferred_width(g, NULL, &j);

		//if do (i+j) not full grid visible
		gtk_widget_set_size_request(w, i + j + 5, getArea().getSize().cy);

		hide(); //if not call hide() position of window is bad

	} else {
		gtk_container_add(GTK_CONTAINER(getContentArea()), g);
	}

	gtk_window_set_resizable(GTK_WINDOW(getWidget()), TRUE);
	show();

}

void MessageDialog::clickButton(int i) {
	//m_content could be very long so show in scrolled window
	auto s = localeToUtf8(m_content[i]);
	MessageDialog d(MESSAGE_ICON_NONE, s.c_str(), BUTTONS_DIALOG_NONE, this);
}

MessageDialog::~MessageDialog() {
}

