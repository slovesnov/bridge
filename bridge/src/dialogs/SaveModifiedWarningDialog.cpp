/*
 * SaveModifiedWarningDialog.cpp
 *
 *       Created on: 06.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "SaveModifiedWarningDialog.h"
#include "../Menu.h"
#include "../ProblemSelector.h"

SaveModifiedWarningDialog::SaveModifiedWarningDialog() :
		ButtonsDialog(MENU_INVALID, BUTTONS_DIALOG_YES_NO_CANCEL) {
	GtkWidget *w;

	w = gtk_label_new(getString(STRING_DOCUMENT_MODIFIED_WARNING_MESSAGE));
	gtk_label_set_line_wrap(GTK_LABEL(w), TRUE);
	gtk_container_add(GTK_CONTAINER(getContentArea()), w);

	m_checkPreview = gtk_check_button_new_with_label(
			getString(STRING_NEVER_SHOW_THIS_MESSAGE_AGAIN));
	if (!gconfig->m_documentModifiedWarning) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkPreview), TRUE);
	}

	show(m_checkPreview);

}

SaveModifiedWarningDialog::~SaveModifiedWarningDialog() {
}

bool SaveModifiedWarningDialog::click(int index) {
	if (index == 0) {
		getProblemSelector().save();
	}
	gconfig->m_documentModifiedWarning = !gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(m_checkPreview));
	getMenu().setItemAttributes(MENU_SHOW_MODIFIED_WARNING);
	return true;
}
