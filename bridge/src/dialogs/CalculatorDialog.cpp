/*
 * CalculatorDialog.cpp
 *
 *       Created on: 27.12.2016
 *           Author: alexey slovesnov
 * copyright(c/c++): 2016-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "CalculatorDialog.h"
#include "../Frame.h"

static gboolean combo_changed(GtkWidget *w, CalculatorDialog* d) {
	d->comboChanged(w);
	return TRUE;
}

CalculatorDialog::CalculatorDialog() :
		BaseDialog(MENU_CALCULATOR) {
	int i;
	GtkWidget*w, *g, *w1;
	std::string s;
	VString v;

	const STRING_ID LID[] = {
			STRING_DECLARER,
			STRING_CONTRACT,
			STRING_NUMBER_OF_TRICKS,
			STRING_VULNERABLE };
	const STRING_ID VS[] = { STRING_NORTH, STRING_EAST, STRING_SOUTH, STRING_WEST };

	v.clear();
	for (i = 0; i < 4; i++) {
		s = getString(VS[i]);
		m_player[i] = utfLower(s);
		v.push_back(m_player[i]);
	}
	m_combo[CALCULATORDIALOG_DECLARER] = createTextCombobox(v);

	m_combo[CALCULATORDIALOG_CONTRACT] = createTextCombobox(1, 7);

	m_combo[CALCULATORDIALOG_TRUMP] = createImageCombobox();
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_combo[CALCULATORDIALOG_TRUMP]),
			createTrumpModel(false, true, BRIDGE));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[CALCULATORDIALOG_TRUMP]), 0);

	m_combo[CALCULATORDIALOG_TRICKS] = createTextCombobox(0, 13);
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[CALCULATORDIALOG_TRICKS]), 7);

	v.clear();
	v.push_back("");
	for (i = 0; i < 2; i++) {
		s = format("%s / %s", getString(VS[i]), getString(VS[i + 2]));
		v.push_back(utfLower(s));
	}
	v.push_back(getString(STRING_ALL));
	m_combo[CALCULATORDIALOG_VULNERABLE] = createTextCombobox(v);

	m_combo[CALCULATORDIALOG_DOUBLE_REDOUBLE] = createTextCombobox(
			DOUBLE_REDOUBLE, SIZE(DOUBLE_REDOUBLE));

	const int margin = 4;
	const int hmargin = 11;
	for (i = 0; i < 2; i++) {
		w = m_score[i] = gtk_label_new("");
		gtk_widget_set_margin_top(w, 10);
		gtk_widget_set_margin_bottom(w, 10);
	}

	g = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g), margin);
	gtk_grid_set_row_spacing(GTK_GRID(g), margin);

	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, margin);
	gtk_container_add(GTK_CONTAINER(w), m_combo[CALCULATORDIALOG_CONTRACT]);
	gtk_container_add(GTK_CONTAINER(w), m_combo[CALCULATORDIALOG_TRUMP]);
	gtk_container_add(GTK_CONTAINER(w),
			m_combo[CALCULATORDIALOG_DOUBLE_REDOUBLE]);

	GtkWidget*pw[] = {
			m_combo[CALCULATORDIALOG_DECLARER],
			w,
			m_combo[CALCULATORDIALOG_TRICKS],
			m_combo[CALCULATORDIALOG_VULNERABLE] };
	assert(SIZE(pw)==SIZE(LID));
	for (i = 0; i < SIZEI(LID); i++) {
		gtk_grid_attach(GTK_GRID(g), gtk_label_new(getString(LID[i])), 0, i, 1, 1);
		gtk_grid_attach(GTK_GRID(g), pw[i], 1, i, 1, 1);
	}
	gtk_widget_set_margin_start(g, hmargin);
	gtk_widget_set_margin_end(g, hmargin);
	gtk_widget_set_margin_top(g, 5);
	gtk_widget_set_margin_bottom(g, 5);

	w = gtk_grid_new();
	gtk_grid_attach(GTK_GRID(w), g, 0, 0, 1, 1);

	w1 = gtk_label_new(""); //use label instead of frame to make upper/left corner rounded
	addClass(w1, ROUND_CORNER_CLASS);

	gtk_grid_attach(GTK_GRID(w), w1, 0, 0, 1, 1);

	g = gtk_box_new(GTK_ORIENTATION_VERTICAL, margin);
	gtk_container_add(GTK_CONTAINER(g), w);

	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	for (i = 0; i < 2; i++) {
		gtk_box_pack_start(GTK_BOX(w), m_score[i], TRUE, TRUE, 6);
	}
	gtk_container_add(GTK_CONTAINER(g), w);

	gtk_widget_set_margin_start(g, hmargin);
	gtk_widget_set_margin_end(g, hmargin);
	gtk_widget_set_margin_top(g, margin);
	gtk_widget_set_margin_bottom(g, margin);

	gtk_container_add(GTK_CONTAINER(getContentArea()), g);

	for (i = 0; i < CALCULATORDIALOG_SIZE; i++) {
		g_signal_connect(m_combo[i], "changed", G_CALLBACK(combo_changed),
				gpointer(this));
	}

	updateScore();
	show();
}

CalculatorDialog::~CalculatorDialog() {
}

void CalculatorDialog::updateScore() {
	/* contract 1-7
	 * result - tricks 0-13
	 * zone=true if playing pair in a zone in pbn tag (vulnerable=NS && (declare 'N' or 'S') || vulnerable=EW && (declare 'E' or 'W') )
	 * doubleRedouble=0 simple game; =1 double; =2 redouble
	 */
	const int declarer = getComboPosition(m_combo[CALCULATORDIALOG_DECLARER]);
	int i;
	std::string s;

	const int contract = getComboPosition(m_combo[CALCULATORDIALOG_CONTRACT]) + 1;

	int r = countScore(contract,
			getComboPosition(m_combo[CALCULATORDIALOG_TRUMP]),
			getComboPosition(m_combo[CALCULATORDIALOG_TRICKS]),
			getComboPosition(m_combo[CALCULATORDIALOG_DOUBLE_REDOUBLE]), declarer,
			getComboPosition(m_combo[CALCULATORDIALOG_VULNERABLE]));

	for (i = 0; i < 2; i++) {
		s = format("%s/%s %d", m_player[i].c_str(), m_player[i + 2].c_str(),
				declarer % 2 == i % 2 ? r : -r);
		gtk_label_set_text(GTK_LABEL(m_score[i]), s.c_str());
	}

}

void CalculatorDialog::comboChanged(GtkWidget* w) {
	updateScore();
}
