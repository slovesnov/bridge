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

enum {
	BRIDGE_CALCULATOR_DIALOG_DECLARER,
	BRIDGE_CALCULATOR_DIALOG_CONTRACT,
	BRIDGE_CALCULATOR_DIALOG_TRUMP,
	BRIDGE_CALCULATOR_DIALOG_TRICKS,
	BRIDGE_CALCULATOR_DIALOG_VULNERABLE,
	BRIDGE_CALCULATOR_DIALOG_DOUBLE_REDOUBLE,

//	BRIDGE_CALCULATOR_DIALOG_SIZE
};

enum {
	PREFERANS_CALCULATOR_DIALOG_CONTRACT,
	PREFERANS_CALCULATOR_DIALOG_TRICKS,
	PREFERANS_CALCULATOR_DIALOG_PLAYERS,
	PREFERANS_CALCULATOR_DIALOG_WHIST_OPTION,

//	PREFERANS_CALCULATOR_DIALOG_SIZE
};

static gboolean combo_changed(GtkWidget *w, CalculatorDialog* d) {
	d->comboChanged(w);
	return TRUE;
}

CalculatorDialog::CalculatorDialog() :
		BaseDialog(MENU_CALCULATOR) {
	int i;
	GtkWidget*w, *w1,*g;
	std::string s;
	VString v, v1;
	VStringID l;
	VGtkWidgetPtr wv;
	const bool bridge=isBridge();
	const int margin = 4;
	const int hmargin = 11;

	for (auto& a:m_score) {
		a = gtk_label_new("");
		gtk_widget_set_margin_top(a, 10);
		gtk_widget_set_margin_bottom(a, 10);
	}

	if(bridge){
		const STRING_ID VS[] = { STRING_NORTH, STRING_EAST, STRING_SOUTH, STRING_WEST };

		//create comboboxes
		for (i = 0; i < 4; i++) {
			s = getString(VS[i]);
			m_player[i] = utf8ToLowerCase(s);
			v.push_back(m_player[i]);
		}

		v1.push_back("");
		for (i = 0; i < 2; i++) {
			s = format("%s / %s", getString(VS[i]), getString(VS[i + 2]));
			v1.push_back(utf8ToLowerCase(s));
		}
		v1.push_back(getString(STRING_ALL));

		m_combo = { createTextCombobox(v), createTextCombobox(1, 7),
				createImageCombobox(), createTextCombobox(0, 13),
				createTextCombobox(v1), createTextCombobox(DOUBLE_REDOUBLE,
						SIZE(DOUBLE_REDOUBLE)) };

		gtk_combo_box_set_model(GTK_COMBO_BOX(m_combo[BRIDGE_CALCULATOR_DIALOG_TRUMP]),
				createTrumpModel(false, true, BRIDGE));
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[BRIDGE_CALCULATOR_DIALOG_TRUMP]), 0);

		gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[BRIDGE_CALCULATOR_DIALOG_TRICKS]), 7);

		l = {
				STRING_DECLARER,
				STRING_CONTRACT,
				STRING_NUMBER_OF_TRICKS,
				STRING_VULNERABLE };
		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, margin);
		gtk_container_add(GTK_CONTAINER(w), m_combo[BRIDGE_CALCULATOR_DIALOG_CONTRACT]);
		gtk_container_add(GTK_CONTAINER(w), m_combo[BRIDGE_CALCULATOR_DIALOG_TRUMP]);
		gtk_container_add(GTK_CONTAINER(w),
				m_combo[BRIDGE_CALCULATOR_DIALOG_DOUBLE_REDOUBLE]);

		wv = {
				m_combo[BRIDGE_CALCULATOR_DIALOG_DECLARER],
				w,
				m_combo[BRIDGE_CALCULATOR_DIALOG_TRICKS],
				m_combo[BRIDGE_CALCULATOR_DIALOG_VULNERABLE] };

	}
	else{
		m_combo =
				{ createTextCombobox(6, 10, 1, getString(STRING_MISERE)),
						createTextCombobox(0, 10), createTextCombobox(3, 4),
						createTextCombobox(STRING_WHIST, STRING_HALF_WHIST,
								STRING_PASS) };

		l = { STRING_CONTRACT, STRING_TRICKS, STRING_PLAYERS,
				STRING_WHIST_OPTION };
	}

	g = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g), margin);
	gtk_grid_set_row_spacing(GTK_GRID(g), margin);
	gtk_widget_set_margin_start(g, hmargin);
	gtk_widget_set_margin_end(g, hmargin);
	gtk_widget_set_margin_top(g, 5);
	gtk_widget_set_margin_bottom(g, 5);

	auto &r = bridge ? wv : m_combo;
	assert(l.size() == r.size());
	i=0;
	for (auto a:l) {
		gtk_grid_attach(GTK_GRID(g), gtk_label_new(getString(a)), 0, i, 1, 1);
		gtk_grid_attach(GTK_GRID(g), r[i], 1, i, 1, 1);
		i++;
	}

	w = gtk_grid_new();
	gtk_grid_attach(GTK_GRID(w), g, 0, 0, 1, 1);

	w1 = gtk_label_new(""); //use label instead of frame to make upper/left corner rounded
	addClass(w1, ROUND_CORNER_CLASS);

	gtk_grid_attach(GTK_GRID(w), w1, 0, 0, 1, 1);

	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	for (auto a:m_score) {
		gtk_box_pack_start(GTK_BOX(w1), a, TRUE, TRUE, 6);
	}
	gtk_grid_attach(GTK_GRID(w), w1, 0, 1, 1, 1);

	if(!bridge){
		w1 = createMarkupLabel(STRING_PREFERANS_CALCULATOR_NOTES, 42);
		gtk_grid_attach(GTK_GRID(w), w1, 0, 2, 1, 1);
	}

	gtk_container_add(GTK_CONTAINER(getContentArea()), w);


	for (auto a:m_combo) {
		g_signal_connect(a, "changed", G_CALLBACK(combo_changed),
				gpointer(this));
	}

	updateScore();
	show();
}

void CalculatorDialog::updateScore() {
	int i;
	std::string s;
	VString v;

	if(isBridge()){
		/* contract 1-7
		 * result - tricks 0-13
		 * zone=true if playing pair in a zone in pbn tag (vulnerable=NS && (declare 'N' or 'S') || vulnerable=EW && (declare 'E' or 'W') )
		 * doubleRedouble=0 simple game; =1 double; =2 redouble
		 */
		const int declarer = getComboPosition(m_combo[BRIDGE_CALCULATOR_DIALOG_DECLARER]);
		const int contract = getComboPosition(m_combo[BRIDGE_CALCULATOR_DIALOG_CONTRACT]) + 1;

		int r = countBridgeScore(contract,
				getComboPosition(m_combo[BRIDGE_CALCULATOR_DIALOG_TRUMP]),
				getComboPosition(m_combo[BRIDGE_CALCULATOR_DIALOG_TRICKS]),
				getComboPosition(m_combo[BRIDGE_CALCULATOR_DIALOG_DOUBLE_REDOUBLE]), declarer,
				getComboPosition(m_combo[BRIDGE_CALCULATOR_DIALOG_VULNERABLE]));

		for (i = 0; i < 2; i++) {
			v.push_back( format("%s/%s %d", m_player[i].c_str(), m_player[i + 2].c_str(),
					declarer % 2 == i % 2 ? r : -r));
		}
	}
	else{
		double score[] = { 1.333, -4.0 };
		for (i = 0; i < 2; i++) {
			v.push_back(
					getString(
							i == 0 ? STRING_PLAYER_SCORE : STRING_WHISTER_SCORE)
							+ normalize(format(" %.2lf", score[i])));
		}
	}

	i=0;
	for (auto a:v) {
		gtk_label_set_text(GTK_LABEL(m_score[i]), v[i].c_str());
		i++;
	}
}

void CalculatorDialog::comboChanged(GtkWidget* w) {
	updateScore();
}
