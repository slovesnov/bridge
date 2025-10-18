/*
 * BaseDialog.cpp
 *
 *       Created on: 21.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "../Frame.h"
#include "BaseDialog.h"

static void destroyDialog(BaseDialog *bd, gpointer) {
	delete bd;
}

void BaseDialog::init(const char *title, bool modal, Widget *parent,
		bool resizable) {
	m_modal = modal;
	gtk_window_set_modal(GTK_WINDOW(getWidget()), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(getWidget()),
			GTK_WINDOW(parent==NULL ? gframe->getWidget():parent->getWidget()));
	gtk_window_set_title(GTK_WINDOW(getWidget()), title);
	gtk_window_set_resizable(GTK_WINDOW(getWidget()), resizable);
}

GtkWidget* BaseDialog::createButton(const char *img, STRING_ID id) {
	return createButton(img, id == STRING_INVALID ? "" : getString(id));
}

GtkWidget* BaseDialog::createButton(const char *img, MENU_ID id) {
	return createButton(img, getString(id));
}

GtkWidget* BaseDialog::createTextButton(STRING_ID id) {
	return createButton(nullptr, id);
}

GtkWidget* BaseDialog::createTextButton(MENU_ID id) {
	return createButton(nullptr, id);
}

GtkWidget* BaseDialog::createTextButton(const char *str) {
	return createButton(nullptr, str);
}

BaseDialog::~BaseDialog() {
	gtk_widget_destroy(getWidget());
}

void BaseDialog::show() {
	VGtkWidgetPtr v;
	showExclude(v);
}

void BaseDialog::showExclude(VGtkWidgetPtr const &v) {
	showAllExclude(v);
	if (m_modal) {
		m_returnCode = gtk_dialog_run(GTK_DIALOG(getWidget()));
	} else {
		g_signal_connect_swapped(getWidget(), "response",
				G_CALLBACK (destroyDialog), this);
	}
}

GtkWidget* BaseDialog::createButton(const char *img, const char *str) {
	GtkWidget *button = gtk_button_new_with_label(str);
	if (img != NULL) {
		gtk_button_set_image(GTK_BUTTON(button), image(img));
	}

	addClass(button, "sbutton");
	return button;
}

