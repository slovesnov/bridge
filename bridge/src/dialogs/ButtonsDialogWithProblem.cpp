/*
 * ButtonsDialogWithProblem.cpp
 *
 *       Created on: 10.05.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "ButtonsDialogWithProblem.h"

const CPoint REGION_INNER[] = {
//inner card position
		CPoint(2, 1),
		CPoint(3, 2),
		CPoint(2, 3),
		CPoint(1, 2),

		//arrow position (on full table)
		CPoint(2, 0),
		CPoint(4, 2),
		CPoint(2, 4),
		CPoint(0, 2) };

const CPoint ButtonsDialogWithProblem::EDIT_LIST_REGION_POSITION[] = { CPoint(1,
		0), CPoint(2, 1), CPoint(1, 2), CPoint(0, 1) };

ButtonsDialogWithProblem::ButtonsDialogWithProblem(MENU_ID id, bool modal,
		BUTTONS_DIALOG_TYPE type, Widget* parent, bool resizable) :
		ButtonsDialog(id, modal, type, parent, resizable) {

	int i, j;
	GdkPixbuf*px;

	assert(SIZE(m_suit)==SIZE(m_labelCard));

	for (i = 0; i < SIZEI(m_suitPixbuf); i++) {
		m_suitPixbuf[i] = getSuitPixbuf(i, getFontHeight());
	}

	px = pixbuf(getArrowFileName(1,false));//48x48
	m_arrowPixbuf[0] = gdk_pixbuf_scale_simple(px, getFontHeight(),
			getFontHeight(), GDK_INTERP_BILINEAR);
	g_object_unref(px);
	for (i = 1; i < SIZEI(m_arrowPixbuf); i++) {
		m_arrowPixbuf[i] = gdk_pixbuf_rotate_simple(m_arrowPixbuf[i - 1],
				GDK_PIXBUF_ROTATE_CLOCKWISE);
	}

	for (i = 0; i < SIZEI(m_arrow); i++) {
		m_arrow[i] = gtk_image_new();
	}

	for (i = 0; i < SIZEI(m_labelCard); i++) {
		m_labelCard[i] = gtk_label_new("");
		m_suit[i] = gtk_image_new();
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			m_labelPlayerSuit[i][j] = gtk_label_new("");
		}
	}

}

ButtonsDialogWithProblem::~ButtonsDialogWithProblem() {
	int i;
	for (i = 0; i < SIZEI(m_suitPixbuf); i++) {
		g_object_unref(m_suitPixbuf[i]);
	}

	for (i = 0; i < SIZEI(m_arrowPixbuf); i++) {
		g_object_unref(m_arrowPixbuf[i]);
	}
}

GtkWidget* ButtonsDialogWithProblem::createPlayerBox(CARD_INDEX ci,
		PLAYERBOX_NAME_TYPE type /*= PLAYERBOX_NAME_TYPE_SIMPLE*/) {
	//printl(ci)
	GtkWidget *w, *w1, *w2,*w3;
	int i;
	std::string s;

	w = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	w1 =m_playerNameBox[indexOfPlayer(ci)]= gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	if(type==PLAYERBOX_NAME_TYPE_EMPTY_BOX){
//TODO
//		w3 = gtk_check_button_new();
//		gtk_container_add(GTK_CONTAINER(w1), w3);
//		w2=label(getPlayerString(ci));
	}
	else{
		w2 = type==PLAYERBOX_NAME_TYPE_UNDERLINED ?
				createUnderlinedLabel(ci) : gtk_label_new(getPlayerString(ci));
		gtk_label_set_xalign(GTK_LABEL(w2), 0);
		gtk_label_set_yalign(GTK_LABEL(w2), 0.5);
		gtk_container_add(GTK_CONTAINER(w1), w2);
	}
	gtk_container_add(GTK_CONTAINER(w), w1);

	for (i = 0; i < 4; i++) {
		w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		//need to create new image every time, because one image should be in one container only
		gtk_container_add(GTK_CONTAINER(w1),
				gtk_image_new_from_pixbuf(m_suitPixbuf[getSuitsOrder(i)]));

		w3=w2 = m_labelPlayerSuit[indexOfPlayer(ci)][i] = gtk_label_new("");

		gtk_label_set_xalign(GTK_LABEL(w2), 0);
		gtk_label_set_yalign(GTK_LABEL(w2), 0.5);

		w2 = m_playerBox[indexOfPlayer(ci)][i]=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_container_add(GTK_CONTAINER(w2), w3);
		gtk_container_add(GTK_CONTAINER(w1), w2);

		gtk_container_add(GTK_CONTAINER(w), w1);
	}

	return w;
}

void ButtonsDialogWithProblem::attachInnerTable(GtkWidget*g) {
	int i;
	GtkWidget*w;
	GtkWidget*w1 = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(w1), 0);
	gtk_grid_set_row_spacing(GTK_GRID(w1), 0);
	gtk_grid_set_column_homogeneous(GTK_GRID(w1), TRUE);
	gtk_grid_set_row_homogeneous(GTK_GRID(w1), TRUE);

	for (i = 0; i < 4; i++) {
		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_container_add(GTK_CONTAINER(w), m_labelCard[i]);
		gtk_container_add(GTK_CONTAINER(w), m_suit[i]);
		gtk_widget_set_halign(w, GTK_ALIGN_CENTER);
		//gtk_widget_set_hexpand(w, TRUE);
		//gtk_widget_set_vexpand(w, TRUE);
		gtk_grid_attach(GTK_GRID(w1), w, REGION_INNER[i].x, REGION_INNER[i].y, 1,
				1);
	}

	//attach some m_arrow[], to the same cells with m_suit & m_labelCard, for good horizontal center in cell
	for (i = 0; i < 8; i++) {
		w = m_arrow[i];
		gtk_widget_set_hexpand(w, TRUE);
		gtk_widget_set_vexpand(w, TRUE);
		gtk_grid_attach(GTK_GRID(w1), w, REGION_INNER[i].x, REGION_INNER[i].y, 1,
				1);
	}

	gtk_grid_attach(GTK_GRID(g), w1, 1, 1, 1, 1);

	//cann't add border to inner grid w1, so add label to the same cell grid for label is working fine
	w = gtk_label_new("");
	addClass(w, ROUND_CORNER_CLASS);
	gtk_grid_attach(GTK_GRID(g), w, 1, 1, 1, 1);

}

void ButtonsDialogWithProblem::setInnerTable(const Problem& p) {
	int i, j, k, a[4];

	p.fillInner4NorthFirst(a);
	for (i = 0; i < SIZEI(PLAYER); i++) {
		j = a[i];
		if (j ==-1) {
			gtk_label_set_text(GTK_LABEL(m_labelCard[i]), "");
			gtk_image_clear(GTK_IMAGE(m_suit[i]));
		}
		else {
			gtk_label_set_text(GTK_LABEL(m_labelCard[i]),
					getCardRankString(j).c_str());
			gtk_image_set_from_pixbuf(GTK_IMAGE(m_suit[i]), m_suitPixbuf[j / 13]);
		}
	}

	k = j = p.getNextMove() - 1;
	if (a[j] != -1) {
		k += 4;
	}
	gtk_image_set_from_pixbuf(GTK_IMAGE(m_arrow[k]), m_arrowPixbuf[j]);

	for (i = 0; i < SIZEI(m_arrow); i++) {
		if (i != k) {
			gtk_image_clear(GTK_IMAGE(m_arrow[i]));
		}
	}

}
