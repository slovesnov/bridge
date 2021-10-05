/*
 * SuitsOrderDialog.cpp
 *
 *       Created on: 18.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "../Frame.h"
#include "../Toolbar.h"
#include "SuitsOrderDialog.h"

static gboolean combo_changed(GtkWidget *w, SuitsOrderDialog* d) {
	d->comboChanged(w);
	return TRUE;
}

SuitsOrderDialog::SuitsOrderDialog() :
		ButtonsDialog(MENU_SUITS_ORDER, BUTTONS_DIALOG_OK_CANCEL_RESTORE_DEFAULT) {
	unsigned i;
	char b[16];
	GtkWidget *hbox, *label, *border, *radioButton, *combobox;
	GtkWidget *sbox, *img;
	GtkRadioButton *rb = NULL;

	gtk_box_set_spacing(GTK_BOX(getContentArea()), 3);

	sbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

	for (i = 0; i < 4; i++) {
		hbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		sprintf(b, "%d", i + 1);
		label = gtk_label_new(b);
		gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 2);

		m_combo[i] = combobox = createImageCombobox();
		gtk_combo_box_set_model(GTK_COMBO_BOX(combobox), createTrumpModel(true));
		gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), getSuitsOrder(i));
		g_signal_connect(combobox, "changed", G_CALLBACK(combo_changed),
				gpointer(this));

		gtk_box_pack_start(GTK_BOX(hbox), combobox, TRUE, TRUE, 2);

		gtk_box_pack_start(GTK_BOX(sbox), hbox, TRUE, TRUE, 2);
	}

	//frame
	border = gtk_frame_new(getString(STRING_CARDS_ORDER_INSIDE_SUIT));
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	for (i = 0; i < 2; i++) {
		if (i == 0) {
			radioButton = gtk_radio_button_new(NULL);
			rb = GTK_RADIO_BUTTON(radioButton);
		}
		else {
			radioButton = gtk_radio_button_new_from_widget(rb);
		}
		m_radio[i] = radioButton;
		gtk_container_add(GTK_CONTAINER(hbox), radioButton);

		if ((i == 0 && gconfig->getAscending())
				|| (i == 1 && !gconfig->getAscending())) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioButton), TRUE);
		}

		//TODO check
		img = image( std::string(i == 0 ? "ascending" : "descending")+"32.png");
		gtk_container_add(GTK_CONTAINER(hbox), img);

	}
	gtk_container_add(GTK_CONTAINER(border), hbox);

	gtk_box_pack_start(GTK_BOX(sbox), border, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(getContentArea()), sbox, TRUE, TRUE, 2);

	show();

}

SuitsOrderDialog::~SuitsOrderDialog() {
}

bool SuitsOrderDialog::click(int index) {
	int i;
	if (index == 2) {
		for (i = 0; i < 2; i++) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radio[i]), i == 1);
		}
		for (i = 0; i < 4; i++) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[i]), i);
		}
		return false;
	}

	if (index == 0) {
		gconfig->setAscending(
				gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_radio[0])));
		int i;
		for (i = 0; i < 4; i++) {
			gconfig->setSuitsOrder(i,
					gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo[i])));
		}

		getArea().recalcRects();
		getArea().redraw();
	}
	return true;
}

void SuitsOrderDialog::comboChanged(GtkWidget* w) {
	int n = INDEX_OF(m_combo, w);
	int suit = gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo[n]));
	int i, notFound = 0;
	bool found[4];
	for (i = 0; i < 4; i++) {
		found[i] = false;
	}
	for (i = 0; i < 4; i++) {
		found[gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo[i]))] = true;
	}
	for (i = 0; i < 4; i++) {
		if (!found[i]) {
			notFound = i;
			break;
		}
	}

	for (i = 0; i < 4; i++) {
		if (i != n && gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo[i])) == suit) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[i]), notFound);
			break;
		}
	}
}

