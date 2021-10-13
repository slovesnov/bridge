/*
 * CustomSkinDialog.cpp
 *
 *       Created on: 16.03.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "CustomSkinDialog.h"
#include "../Frame.h"

static CustomSkinDialog*pd;

static void button_clicked(GtkWidget *widget, int i) {
	pd->clickButton(i);
}

CustomSkinDialog::CustomSkinDialog() :
		ButtonsDialog(MENU_CUSTOM_SKIN, BUTTONS_DIALOG_OK_CANCEL) {
	int i;
	GtkWidget*w;

	pd = this;

	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_widget_set_margin_top(w, 10);
	gtk_widget_set_margin_bottom(w, 10);
	gtk_widget_set_margin_start(w, 10);
	gtk_widget_set_margin_end(w, 10);

	for (i = 0; i < SIZEI(CUSTOMSKINDIALOG_BUTTONS); i++) {
		m_button[i] = createButton(NULL, CUSTOMSKINDIALOG_BUTTONS[i]);
		gtk_container_add(GTK_CONTAINER(w), m_button[i]);
		g_signal_connect(m_button[i], "clicked", G_CALLBACK(button_clicked),
				GP(i));
	}

	gtk_container_add(GTK_CONTAINER(getContentArea()), w);

	show();
}

CustomSkinDialog::~CustomSkinDialog() {
}

bool CustomSkinDialog::click(int index) {
	if (index == 0) {
		gconfig->setSkin(CONFIG_CUSTOM_SKIN, REWRITE_CSS_MAIN_FILE);
		gframe->updateSkin();
	}
	return true;
}

void CustomSkinDialog::clickButton(int i) {
	assert(i == 0 || i == 1);
	gchar *c;
	if (i == 0) {
		FileChooserResult r = fileChooserOpen(FILE_TYPE_IMAGE);
		if (!r.ok()) {
			return;
		}
		//background-image:url('file:///D:/fun1.png')
		c = g_filename_to_uri(r.file().c_str(), NULL, NULL);
		gconfig->m_customSkinBackgroundImagePath = c;
		g_free(c);
	}
	else {
		if (!selectColor(getString(STRING_SELECT_COLOR),
				&gconfig->m_customSkinBackgroundColor)) {
			return;
		}
	}

	gconfig->setSkin(CONFIG_CUSTOM_SKIN,
			REWRITE_CSS_OPTION(
					REWRITE_CSS_MAIN_FILE
							| (i == 0 ?
									REWRITE_CSS_CUSTOM_FILE_BACKGROUND_IMAGE :
									REWRITE_CSS_CUSTOM_FILE_BACKGROUND_COLOR)));
	gframe->updateSkin();
	gtk_dialog_response(GTK_DIALOG(getWidget()), GTK_RESPONSE_OK);
}

