/*
 * InsertProblemDialog.cpp
 *
 *       Created on: 16.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "InsertProblemDialog.h"
#include "EditListDialog.h"

const int MIN_QUANTITY = 1;

static void toggle_radio(GtkWidget *w, InsertProblemDialog*d) {
	d->toggle(w);
}

InsertProblemDialog::InsertProblemDialog(EditListDialog* eld) :
		ButtonsDialog(STRING_INSERT, BUTTONS_DIALOG_OK_CANCEL, eld) {
	int i;
	GtkWidget *w, *w1;
	VString v;
	std::string s;
	GtkRadioButton *rb = NULL;

	m_pvm = eld;

	m_combo[INSERT_PROBLEM_DIALOG_COMBO_BEFORE_AFTER] = createTextCombobox(
			STRING_BEFORE, STRING_AFTER);
	m_combo[INSERT_PROBLEM_DIALOG_COMBO_GAMETYPE] = createTextCombobox(
			STRING_BRIDGE, STRING_PREFERANS);
	m_combo[INSERT_PROBLEM_DIALOG_COMBO_QUANTITY] = createTextCombobox(
			MIN_QUANTITY, 10);

	m_grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(m_grid), 15);

	for (i = 0; i < SIZEI(m_radio); i++) {
		if (i == 0) {
			m_radio[i] = gtk_radio_button_new_with_label(NULL,
					getString(INSERT_PROBLEM_DIALOG_RID[i]));
			rb = GTK_RADIO_BUTTON(m_radio[i]);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radio[i]), TRUE);
		}
		else {
			m_radio[i] = gtk_radio_button_new_with_label_from_widget(rb,
					getString(INSERT_PROBLEM_DIALOG_RID[i]));
		}
		g_signal_connect(m_radio[i], "toggled", G_CALLBACK (toggle_radio),
				gpointer(this));

		gtk_grid_attach(GTK_GRID(m_grid), m_radio[i], 0, i, 1, 1);
	}

	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 13);

	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_container_add(GTK_CONTAINER(w1),
			gtk_label_new(getString(STRING_QUANTITY)));
	gtk_container_add(GTK_CONTAINER(w1),
			m_combo[INSERT_PROBLEM_DIALOG_COMBO_QUANTITY]);
	gtk_container_add(GTK_CONTAINER(w), w1);

	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_container_add(GTK_CONTAINER(w1), gtk_label_new(getString(STRING_TYPE)));

	gtk_container_add(GTK_CONTAINER(w1),
			m_combo[INSERT_PROBLEM_DIALOG_COMBO_GAMETYPE]);
	gtk_container_add(GTK_CONTAINER(w), w1);

	gtk_grid_attach(GTK_GRID(m_grid), w, 1, 0, 1, 2);

	//main box
	w = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 13);
	gtk_container_add(GTK_CONTAINER(w1),
			m_combo[INSERT_PROBLEM_DIALOG_COMBO_BEFORE_AFTER]);
	s = getString(MENU_PROBLEM) + format(" # %d", m_pvm->m_current + 1);
	gtk_container_add(GTK_CONTAINER(w1), gtk_label_new(s.c_str()));
	gtk_container_add(GTK_CONTAINER(w), w1);

	w1 = gtk_frame_new(getString(STRING_INSERT));
	gtk_container_add(GTK_CONTAINER(w1), m_grid);
	gtk_container_add(GTK_CONTAINER(w), w1);
	gtk_widget_set_margin_bottom(w1, 5);

	gtk_container_add(GTK_CONTAINER(getContentArea()), w);

	show();
}

InsertProblemDialog::~InsertProblemDialog() {
}

bool InsertProblemDialog::click(int index) {
	int i;
	bool before;
	if (index == 0) {
		for (i = 0; i < SIZEI(m_radio); i++) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_radio[i]))) {
				break;
			}
		}
		assert(i<SIZEI(m_radio));

		before = getComboPosition(m_combo[INSERT_PROBLEM_DIALOG_COMBO_BEFORE_AFTER])
				== 0;

		if (i == 2) {
			FileChooserResult r = fileChooser(MENU_OPEN, FILE_TYPE_ANY,
					CHOOSER_OPTION_OPEN_ALL_SUPPORTED, "");
			if (r.ok()) {
				assert(r.m_response == GTK_RESPONSE_OK);
				m_pvm->insert(r.m_files, before);
			}
			else {
				return false;
			}

		}
		else {
			m_pvm->insert(i == 0, before,
					getComboPosition(m_combo[INSERT_PROBLEM_DIALOG_COMBO_QUANTITY])
							+ MIN_QUANTITY,
					GAME_TYPE(
							getComboPosition(m_combo[INSERT_PROBLEM_DIALOG_COMBO_GAMETYPE])));
		}
	}
	return true;
}

void InsertProblemDialog::toggle(GtkWidget* w) {
	int i;
	bool fromFile;
	GtkWidget*w1;
	if ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))) {
		fromFile = INDEX_OF(w,m_radio) == 2;
		for (i = 0; i < 2; i++) {
			w1 = gtk_grid_get_child_at(GTK_GRID(m_grid), 1, i);
			if (fromFile) {
				gtk_widget_hide(w1);
			}
			else {
				gtk_widget_show_all(w1);
			}
		}
	}
}
