/*
 * DeleteProblemDialog.cpp
 *
 *       Created on: 09.03.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "DeleteProblemDialog.h"
#include "EditListDialog.h"

DeleteProblemDialog::DeleteProblemDialog(EditListDialog* eld) :
		ButtonsDialog(STRING_DELETE, BUTTONS_DIALOG_OK_CANCEL, eld) {
	int i;
	GtkWidget *w, *grid;

	GtkRadioButton *rb = NULL;

	m_pvm = eld;

	m_combo = createTextCombobox(STRING_BEFORE, STRING_AFTER);

	grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
	gtk_widget_set_margin_bottom(grid, 5);

	for (i = 0; i < SIZEI(m_radio); i++) {
		if (i == 0) {
			m_radio[i] = gtk_radio_button_new_with_label(NULL,
					getString(DELETE_PROBLEM_DIALOG_RID[i]));
			rb = GTK_RADIO_BUTTON(m_radio[i]);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radio[i]), TRUE);
		}
		else {
			m_radio[i] = gtk_radio_button_new_with_label_from_widget(rb,
					getString(DELETE_PROBLEM_DIALOG_RID[i]));
		}

		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_container_add(GTK_CONTAINER(w), m_radio[i]);
		if (i == 1) {
			gtk_container_add(GTK_CONTAINER(w), m_combo);
			gtk_container_add(GTK_CONTAINER(w),
					gtk_label_new(format(" # %d", m_pvm->m_current + 1).c_str()));
		}

		gtk_grid_attach(GTK_GRID(grid), w, 0, i, 1, 1);
	}

	gtk_container_add(GTK_CONTAINER(getContentArea()), grid);

	show();

}

DeleteProblemDialog::~DeleteProblemDialog() {
}

bool DeleteProblemDialog::click(int index) {
	int i;
	if (index == 0) {
		for (i = 0; i < SIZEI(m_radio); i++) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_radio[i]))) {
				break;
			}
		}
		assert(i<SIZEI(m_radio));

		if (i == 0) {
			m_pvm->deleteCurrent();
		}
		else {
			m_pvm->deleteAllBeforeAfter(getComboPosition(m_combo) == 0);
		}

	}
	return true;
}

