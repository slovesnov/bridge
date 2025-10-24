/*
 * EditDescriptionDialog.cpp
 *
 *       Created on: 02.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "EditDescriptionDialog.h"
#include "../DrawingArea.h"
#include "../ProblemSelector.h"

EditDescriptionDialog::EditDescriptionDialog() :
		ButtonsDialog(MENU_EDIT_DESCRIPTION, BUTTONS_DIALOG_OK_CANCEL_CLEAR,
				NULL, true) {
	GtkWidget *scrolled;

	m_comment = gtk_text_view_new();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_comment));
	gtk_text_buffer_set_text(buffer, getProblemSelector().m_comment.c_str(),
			-1);

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	CPoint sz = getArea().getSize();
	gtk_widget_set_size_request(scrolled, 3 * sz.x / 4, 3 * sz.y / 4);

	gtk_container_add(GTK_CONTAINER(scrolled), m_comment);

	gtk_text_view_set_justification(GTK_TEXT_VIEW(m_comment), GTK_JUSTIFY_FILL);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_comment), GTK_WRAP_WORD);

	gtk_box_pack_start(GTK_BOX(getContentArea()), scrolled, TRUE, TRUE, 0); //stretch scrolled

	show();
}

EditDescriptionDialog::~EditDescriptionDialog() {
}

bool EditDescriptionDialog::click(int index) {
	if (index == 2) {
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(
				GTK_TEXT_VIEW(m_comment));
		gtk_text_buffer_set_text(buffer, "", -1);
		return false;
	}

	if (index == 0) {
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(
				GTK_TEXT_VIEW(m_comment));
		GtkTextIter start;
		GtkTextIter end;
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		getProblemSelector().setComment(
				gtk_text_buffer_get_text(buffer, &start, &end, true));
		updateModified();
	}
	return true;
}

