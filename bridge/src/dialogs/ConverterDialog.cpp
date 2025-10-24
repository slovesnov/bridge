/*
 * ConverterDialog.cpp
 *
 *       Created on: 05.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "ConverterDialog.h"
#include "../DrawingArea.h"
#include "../Menu.h"

ConverterDialog::ConverterDialog() :
		ButtonsDialog(MENU_CONVERT, BUTTONS_DIALOG_CONVERTER) {
	GtkWidget *scrolled;
	GtkWidget *w, *w1, *w2;
	VString sv;
	std::string s;

	m_checkSplitEveryFile = gtk_check_button_new_with_label(
			getString(MENU_SPLIT_EVERY_FILE));
	if (gconfig->m_splitEveryFile) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkSplitEveryFile),
		TRUE);
	}

	m_checkSaveWithPictures = gtk_check_button_new_with_label(
			getString(MENU_SAVE_HTML_FILE_WITH_IMAGES));
	if (gconfig->m_htmlStoreWithImages) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkSaveWithPictures),
		TRUE);
	}

	m_checkPreview = gtk_check_button_new_with_label(
			getString(MENU_PREVIEW_HTML_FILE));
	if (gconfig->m_htmlPreview) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkPreview), TRUE);
	}

	m_firstSplitNumber.create(0, 10000, gconfig->m_firstSplitNumber, this);

	m_view = gtk_text_view_new();
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled), m_view);
	CPoint sz = getArea().getSize();
	gtk_widget_set_size_request(scrolled, 800, 3 * sz.y / 5);

	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_view), FALSE);
	gtk_text_view_set_justification(GTK_TEXT_VIEW(m_view), GTK_JUSTIFY_FILL);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_view), GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_view), FALSE);

	w2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); //whole box

	w1 = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(w1), 20);
	gtk_grid_attach(GTK_GRID(w1), m_checkSplitEveryFile, 0, 0, 1, 1);

	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_container_add(GTK_CONTAINER(w),
			gtk_label_new(getString(STRING_FIRST_SPLIT_FILE_NUMBER)));
	gtk_container_add(GTK_CONTAINER(w), m_firstSplitNumber.m_widget);
	gtk_grid_attach(GTK_GRID(w1), w, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(w1), m_checkSaveWithPictures, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(w1), m_checkPreview, 1, 1, 1, 1);

	gtk_container_add(GTK_CONTAINER(w2), w1);

	s = getString(STRING_CONVERTER_HELP);
	s += " *.bts, *.pts, *.html, *.pbn, *.txt, *.dat.";
	w = gtk_label_new(s.c_str());
	gtk_label_set_line_wrap(GTK_LABEL(w), TRUE);
	//without call this funciton, label not multiline which is too long
	gtk_label_set_max_width_chars(GTK_LABEL(w), 40);

	gtk_container_add(GTK_CONTAINER(w2), w);

	gtk_container_add(GTK_CONTAINER(w2), scrolled);

	gtk_container_add(GTK_CONTAINER(getContentArea()), w2);

	setDragDrop(getWidget());
	setDragDrop(scrolled);
	setDragDrop(m_view);

	enableButtons();

	show();
}

ConverterDialog::~ConverterDialog() {
	saveParams();
	//if user change state of checkbox and close dialog
	getMenu().setItemAttributes(MENU_SPLIT_EVERY_FILE);
	getMenu().setItemAttributes(MENU_SAVE_HTML_FILE_WITH_IMAGES);
	getMenu().setItemAttributes(MENU_PREVIEW_HTML_FILE);
}

void ConverterDialog::saveParams() {
	gconfig->m_splitEveryFile = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(m_checkSplitEveryFile));
	gconfig->m_htmlStoreWithImages = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(m_checkSaveWithPictures));
	gconfig->m_htmlPreview = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(m_checkPreview));
	gconfig->m_firstSplitNumber = atoi(
			gtk_entry_get_text(GTK_ENTRY(m_firstSplitNumber.m_widget)));
}

bool ConverterDialog::save(bool many) {
	saveParams();
	FileChooserResult r =
			fileChooser(MENU_SAVE, FILE_TYPE_ANY,
					many ? CHOOSER_OPTION_CONVERTER_SAVE_MANY_FILES : CHOOSER_OPTION_CONVERTER_SAVE_ONE_FILE,
					"");

	if (r.ok()) {
		if (many) {
			ProblemVector::addSave(m_files, r.file(),
					gconfig->m_splitEveryFile);
		} else {
			ProblemVector p;
			p.set(m_files, true);
			p.save(r.file(), false);
			if (getFileType(r.file()) == FILE_TYPE_HTML
					&& gtk_toggle_button_get_active(
							GTK_TOGGLE_BUTTON(m_checkPreview))) {
				openURL(r.file());
			}
		}
		if (many) {
			gtk_dialog_response(GTK_DIALOG(getWidget()), GTK_RESPONSE_OK); //emit to close dialog
		}
	}

	return r.ok();

}

void ConverterDialog::update() {
	std::string s;
	VStringI it;
	for (it = m_files.begin(); it != m_files.end(); it++) {
		s += *it;
		s += "\n";
	}
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view));
	gtk_text_buffer_set_text(buffer, s.c_str(), -1);

	enableButtons();
}

void ConverterDialog::openUris(char **uris) {
	const VString &v = getValidFilesList(uris);
	m_files.insert(m_files.end(), v.begin(), v.end());
	if (m_files.size() == 0) {
		return;
	}
	update();
}

bool ConverterDialog::click(int index) {
	if (index < 2) {
		save(index == 1);
		return true;
	} else if (index == 2) {
		FileChooserResult r = fileChooser(MENU_OPEN, FILE_TYPE_ANY,
				CHOOSER_OPTION_ADD_ALL_SUPPORTED, "");
		if (r.ok()) {
			//add file
			m_files.insert(m_files.end(), r.begin(), r.end());
			update();
		}
		return false;
	} else if (index == 3) {
		m_files.clear();
		update();
		return false;
	} else {
		return true;
	}
}

void ConverterDialog::enableButtons() {
	bool e = m_firstSplitNumber.m_valid && m_files.size() != 0;

	gtk_widget_set_sensitive(m_button[0], e);
	gtk_widget_set_sensitive(m_button[1], e);
}

