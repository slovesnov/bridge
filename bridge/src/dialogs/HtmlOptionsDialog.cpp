/*
 * HtmlOptionsDialog.cpp
 *
 *       Created on: 03.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "HtmlOptionsDialog.h"
#include "../Menu.h"

const STRING_ID SID[] = { STRING_STORE_BEST_MOVE,
		STRING_STORE_NUMBER_OF_TRICKS_IN_OPTIMAL_GAME, STRING_INVALID,
		STRING_NEVER_SHOW_THIS_MESSAGE_AGAIN };

HtmlOptionsDialog::HtmlOptionsDialog() :
		ButtonsDialog(MENU_INVALID, BUTTONS_DIALOG_OK) {
	unsigned i;
	GtkWidget *box, *w;
	bool b;
	const char *p;

	assert(SIZE(m_check)==SIZE(SID));

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	for (i = 0; i < SIZE(m_check); i++) {
		if (SID[i] == STRING_INVALID) {
			p = getString(MENU_PREVIEW_HTML_FILE);
		} else {
			p = getString(SID[i]);
		}
		w = m_check[i] = gtk_check_button_new_with_label(p);

		switch (SID[i]) {
		case STRING_STORE_BEST_MOVE:
			b = gconfig->m_htmlStoreBestMove;
			break;

		case STRING_STORE_NUMBER_OF_TRICKS_IN_OPTIMAL_GAME:
			b = gconfig->m_htmlStoreNumberOfTricks;
			break;

		case STRING_INVALID:
			b = gconfig->m_htmlPreview;
			break;

		case STRING_NEVER_SHOW_THIS_MESSAGE_AGAIN:
			b = !gconfig->m_htmlShowDialog;
			break;

		default:
			b = false; //for gcc
			assert(0);
		}

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), b);
		gtk_container_add(GTK_CONTAINER(box), w);

	}

	gtk_container_add(GTK_CONTAINER(getContentArea()), box);

	show();

}

HtmlOptionsDialog::~HtmlOptionsDialog() {
}

bool HtmlOptionsDialog::click(int index) {
	unsigned i;
	bool b;

	if (index == 0) {
		for (i = 0; i < SIZE(m_check); i++) {
			b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_check[i]));

			switch (SID[i]) {
			case STRING_STORE_BEST_MOVE:
				gconfig->m_htmlStoreBestMove = b;
				break;

			case STRING_STORE_NUMBER_OF_TRICKS_IN_OPTIMAL_GAME:
				gconfig->m_htmlStoreNumberOfTricks = b;
				break;

			case STRING_INVALID:
				gconfig->m_htmlPreview = b;
				break;

			case STRING_NEVER_SHOW_THIS_MESSAGE_AGAIN:
				gconfig->m_htmlShowDialog = !b;
				break;

			default:
				assert(0);
			}

		}

		getMenu().setItemAttributes(MENU_SHOW_HTML_OPTIONS);
		getMenu().setItemAttributes(MENU_PREVIEW_HTML_FILE);
	}

	return true;
}
