/*
 * ButtonsDialog.cpp
 *
 *       Created on: 04.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "ButtonsDialog.h"

const int margin = 3;

const int MSIZE = 6;

static const STRING_ID TYPE[][MSIZE] = { { STRING_INVALID }, { STRING_OK,
		STRING_INVALID }, { STRING_OK, STRING_CANCEL, STRING_INVALID }, {
		STRING_YES, STRING_NO, STRING_CANCEL, STRING_INVALID }, {
		STRING_TO_ONE_FILE, STRING_TO_MANY_FILES, STRING_ADD, STRING_CLEAR,
		STRING_CANCEL, STRING_INVALID }, { STRING_OK, STRING_CANCEL,
		STRING_CLEAR, STRING_INVALID }, { STRING_OK, STRING_CANCEL,
		STRING_RESTORE_DEFAULT, STRING_INVALID } };

static const char *IMG[][MSIZE - 1] = { { }, { "ok" }, { "ok", "cancel" }, {
		"ok", "cancel", "cancel" }, { "ok", "split", "add", "clear", "cancel" },
		{ "ok", "cancel", "clear" }, { "ok", "cancel", "restore" } };

static const GtkResponseType RESPONSE[][MSIZE - 1] = { { }, { GTK_RESPONSE_OK } //not used
		, { GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL } //used in InsertProblemDialog
		, { GTK_RESPONSE_YES, GTK_RESPONSE_NO, GTK_RESPONSE_CANCEL } //used in SaveModifiedWarningDialog
		, { GTK_RESPONSE_OK, GTK_RESPONSE_OK, GTK_RESPONSE_OK, GTK_RESPONSE_OK,
				GTK_RESPONSE_OK } //not used
		, { GTK_RESPONSE_OK, GTK_RESPONSE_OK, GTK_RESPONSE_OK } //not used
		, { GTK_RESPONSE_OK, GTK_RESPONSE_OK, GTK_RESPONSE_OK } //not used
};

static void button_clicked(GtkWidget *widget, ButtonsDialog *d) {
	d->clickButton(widget);
}

void ButtonsDialog::init(BUTTONS_DIALOG_TYPE type) {
	GtkWidget *w;
	const STRING_ID *p;
	int i;

	m_type = type;

	assert(SIZE(TYPE)==BUTTONS_DIALOG_SIZE);
	assert(SIZE(IMG)==BUTTONS_DIALOG_SIZE);
	assert(SIZE(RESPONSE)==BUTTONS_DIALOG_SIZE);

	for (i = 0, p = TYPE[type]; *p != STRING_INVALID; p++, i++) {
		w = createButton((IMG[m_type][i] + std::string("24.png")).c_str(), *p);
		m_button.push_back(w);
		g_signal_connect(w, "clicked", G_CALLBACK(button_clicked),
				gpointer(this));
	}

}

ButtonsDialog::~ButtonsDialog() {
}

void ButtonsDialog::show(GtkWidget *w, bool before, VGtkWidgetPtr const &v) {
	GtkWidget *b = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_widget_set_margin_top(b, 5);
	gtk_widget_set_margin_bottom(b, 5);

	if (w != NULL && before) {
		gtk_box_pack_start(GTK_BOX(b), w, TRUE, TRUE, margin);
	}

	for (auto a : m_button) {
		gtk_box_pack_start(GTK_BOX(b), a, TRUE, TRUE, margin);
	}

	if (w != NULL && !before) {
		gtk_box_pack_start(GTK_BOX(b), w, TRUE, TRUE, margin);
	}

	GtkWidget *b1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_container_add(GTK_CONTAINER(b1), b);
	gtk_container_add(GTK_CONTAINER(getContentArea()), b1);

	BaseDialog::showExclude(v);
}

void ButtonsDialog::show(GtkWidget *w, bool before) {
	VGtkWidgetPtr v;
	show(w, before, v);
}

void ButtonsDialog::clickButton(GtkWidget *w) {
	int i = indexOf(w, m_button);
	if (click(i)) {
		//response code
		gtk_dialog_response(GTK_DIALOG(getWidget()), RESPONSE[m_type][i]);
	}
}

