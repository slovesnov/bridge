/*
 * EditListDialog.cpp
 *
 *       Created on: 04.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "EditListDialog.h"
#include "InsertProblemDialog.h"
#include "DeleteProblemDialog.h"
#include "../ProblemSelector.h"
#include "../DrawingArea.h"

static EditListDialog*dialog;

const STRING_ID SID[] = {
		STRING_MOVE_LEFT,
		STRING_MOVE_RIGHT,
		STRING_INSERT,
		STRING_DELETE };

const char* IMG[] = { "undo24.png", NULL, "insert24.png", "cancel24.png" };

static void button_clicked(GtkWidget *widget, int i) {
	dialog->click(EDIT_LIST_BUTTON(i)); //click(int) used for ButtonDialog
}

static gboolean mouse_press_event(GtkWidget *widget, GdkEventButton *event,
		int i) {
	//event->button =1 left mouse button
	if (event->button == 1) {
		dialog->click(EDIT_LIST_BUTTON(i)); //click(int) used for ButtonDialog
	}
	return TRUE;
}

EditListDialog::EditListDialog() :
		ButtonsDialogWithProblem(MENU_EDIT_PROBLEM_LIST), ProblemVectorModified(
				getProblemSelector()) {
	GtkWidget*g, *w, *w1;
	int i;
	GdkPixbuf*px, *px1;

	assert(SIZE(SID)==SIZE(IMG));
	assert(SIZE(SID)==EDIT_LIST_MOVE_0);

	dialog = this;

	m_lastHide = CARD_INDEX_INVALID;

	m_grid = g = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g), 0);
	gtk_grid_set_row_spacing(GTK_GRID(g), 0);

	attachInnerTable(g);

	for (i = 0; i < SIZEI(PLAYER); i++) {
		w = createPlayerBox(PLAYER[i]);
		gtk_grid_attach(GTK_GRID(g), w, EDIT_LIST_REGION_POSITION[i].x,
				EDIT_LIST_REGION_POSITION[i].y, 1, 1);
	}

	//navigate buttons, after commands, add to vertical box because sometimes need to remove east/west player
	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	for (; i < EDIT_LIST_DIALOG_BUTTON_SIZE; i++) {
		m_button[i] = gtk_image_new();
		gtk_widget_set_hexpand(m_button[i], TRUE);
		w1 = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(w1), m_button[i]);
		gtk_widget_add_events(w1, GDK_BUTTON_PRESS_MASK);
		g_signal_connect(w1, "button_press_event", G_CALLBACK(mouse_press_event),
				GP(i));
		gtk_container_add(GTK_CONTAINER(w), w1);

	}
	auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(box), g);
	gtk_container_add(GTK_CONTAINER(box), w);

	//move right/left+insert/remove commands, add to horizontal box because sometimes need to remove north/south player
	w = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(w), 3);
	gtk_grid_set_row_spacing(GTK_GRID(w), 3);
	for (i = 0; i < EDIT_LIST_DIALOG_BUTTON_SIZE - 4; i++) {
		w1 = gtk_label_new(getString(SID[i]));
		gtk_label_set_xalign(GTK_LABEL(w1), 1);
		gtk_label_set_yalign(GTK_LABEL(w1), 0.5);
		gtk_grid_attach(GTK_GRID(w), w1, 0, i, 1, 1);

		m_button[i] = createButton(IMG[i]);
		if (IMG[i] == NULL) {
			px = pixbuf(IMG[0]);
			px1 = gdk_pixbuf_flip(px, true);
			gtk_button_set_image(GTK_BUTTON(m_button[i]),
					gtk_image_new_from_pixbuf(px1));
			g_object_unref(px1);
			g_object_unref(px);
		}
		gtk_grid_attach(GTK_GRID(w), m_button[i], 1, i, 1, 1);

		g_signal_connect(m_button[i], "clicked", G_CALLBACK(button_clicked),
				GP(i));
	}
	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(w1), box);
	gtk_container_add(GTK_CONTAINER(w1), w);

	gtk_container_add(GTK_CONTAINER(getContentArea()), w1);

	redrawProblem();

	show();
}

void EditListDialog::redrawProblem() {
	int i, j;
	const Problem& p = getProblem();

	std::string s = getString(MENU_EDIT_PROBLEM_LIST);
	s += " [";
	s += getString(MENU_PROBLEM);
	s += format(" %d / %d ]", m_current + 1, size());
	gtk_window_set_title(GTK_WINDOW(getWidget()), s.c_str());

	for (i = 0; i < 4; i++) {
		gtk_image_set_from_pixbuf(GTK_IMAGE(m_button[EDIT_LIST_MOVE_0 + i]),
				getToolbarPixbuf(TOOLBAR_BUTTON_ARRAY[i + 1], false, boolToButtonState(isMovePossible(i))));
	}

	gtk_widget_set_sensitive(m_button[EDIT_LIST_MOVE_LEFT], isMovePossible(1));
	gtk_widget_set_sensitive(m_button[EDIT_LIST_MOVE_RIGHT], isMovePossible(2));
	gtk_widget_set_sensitive(m_button[EDIT_LIST_DELETE], size() > 1);

	//show last hide player
	if (m_lastHide != CARD_INDEX_INVALID) {
		if (northOrSouth(m_lastHide)) {
			gtk_grid_insert_row(GTK_GRID(m_grid), north(m_lastHide) ? 0 : 2);
		}
		else {
			gtk_grid_insert_column(GTK_GRID(m_grid), west(m_lastHide) ? 0 : 2);
		}

		i = indexOfPlayer(m_lastHide);
		auto w = createPlayerBox(m_lastHide);
		gtk_grid_attach(GTK_GRID(m_grid), w, EDIT_LIST_REGION_POSITION[i].x,
				EDIT_LIST_REGION_POSITION[i].y, 1, 1);
	}

	if (p.isBridge()) {
		m_lastHide = CARD_INDEX_INVALID;
	}
	else {
		m_lastHide = p.m_absent;
		if (northOrSouth(m_lastHide)) {
			gtk_grid_remove_row(GTK_GRID(m_grid), north(m_lastHide) ? 0 : 2);
		}
		else {
			gtk_grid_remove_column(GTK_GRID(m_grid), west(m_lastHide) ? 0 : 2);
		}
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (isPreferans() && i == indexOfPlayer(p.m_absent)) {
				continue;
			}
			gtk_label_set_text(GTK_LABEL(m_labelPlayerSuit[i][j]),
					p.getRow(j, PLAYER[i]).c_str());
		}
	}

	//use non standard getProblem() functions it's redefined in this class so pass reference
	setInnerTable(p);

	//need to call after insert row or column to grid
	gtk_widget_show_all(m_grid);

}

void EditListDialog::click(EDIT_LIST_BUTTON e) {
	bool redraw = true;

	switch (e) {
	case EDIT_LIST_MOVE_LEFT:
	case EDIT_LIST_MOVE_RIGHT:
		move(e == EDIT_LIST_MOVE_LEFT);
		break;

	case EDIT_LIST_INSERT:
		redraw = InsertProblemDialog(this).getReturnCode() == GTK_RESPONSE_OK;
		break;

	case EDIT_LIST_DELETE:
		redraw = DeleteProblemDialog(this).getReturnCode() == GTK_RESPONSE_OK;
		break;

	case EDIT_LIST_MOVE_0:
	case EDIT_LIST_MOVE_1:
	case EDIT_LIST_MOVE_2:
	case EDIT_LIST_MOVE_3:
		redraw = isMovePossible(e - EDIT_LIST_MOVE_0);
		if (redraw) {
			move(e - EDIT_LIST_MOVE_0);
		}
		break;

	default:
		assert(0);
	}

	if (redraw) {
		redrawProblem();
	}

}

