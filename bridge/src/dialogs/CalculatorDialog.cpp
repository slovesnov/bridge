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
#include "../helper/PreferansScore.h"

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

const int WO_WHIST=0;
const int WO_HALF_WHIST=1;

#define BC(a) m_combo[BRIDGE_CALCULATOR_DIALOG_##a]
#define CB(a) getComboPosition(BC(a))

#define PC(a) m_combo[PREFERANS_CALCULATOR_DIALOG_##a]
#define CP(a) getComboPosition(PC(a))

static gboolean combo_changed(GtkWidget *w, CalculatorDialog* d) {
	d->comboChanged(w);
	return TRUE;
}

CalculatorDialog::CalculatorDialog() :
		BaseDialog(MENU_CALCULATOR) {
	int i;
	GtkWidget *w, *w1, *w2;
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

		gtk_combo_box_set_model(GTK_COMBO_BOX(BC(TRUMP)),
				createTrumpModel(false, true, BRIDGE));
		gtk_combo_box_set_active(GTK_COMBO_BOX(BC(TRUMP)), 0);

		gtk_combo_box_set_active(GTK_COMBO_BOX(BC(TRICKS)), 7);

		l = { STRING_DECLARER, STRING_CONTRACT, STRING_NUMBER_OF_TRICKS,
				STRING_VULNERABLE };
		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, margin);
		gtk_container_add(GTK_CONTAINER(w), BC(CONTRACT));
		gtk_container_add(GTK_CONTAINER(w), BC(TRUMP));
		gtk_container_add(GTK_CONTAINER(w), BC(DOUBLE_REDOUBLE));

		wv = { BC(DECLARER), w, BC(TRICKS), BC(VULNERABLE) };

	}
	else{
		//STRING_PASS
		m_combo =
				{ createTextCombobox(6, 10, 1, getString(STRING_MISERE)),
						createTextCombobox(0, 10), createTextCombobox(3, 4),
				createTextCombobox(STRING_WHIST, STRING_HALF_WHIST) };

		l = { STRING_CONTRACT, STRING_TRICKS, STRING_PLAYERS,
				STRING_WHIST_OPTION };
		setComboPosition(PC(TRICKS), 6);
	}

	w2 = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(w2), margin);
	gtk_grid_set_row_spacing(GTK_GRID(w2), margin);
	gtk_widget_set_margin_start(w2, hmargin);
	gtk_widget_set_margin_end(w2, hmargin);
	gtk_widget_set_margin_top(w2, 5);
	gtk_widget_set_margin_bottom(w2, 5);
	gtk_widget_set_halign(w2, GTK_ALIGN_CENTER);

	auto &r = bridge ? wv : m_combo;
	assert(l.size() == r.size());
	i=0;
	for (auto a:l) {
		w=gtk_label_new(getString(a));
		if(isPreferans()){
			m_label[i]=w;
		}
		gtk_grid_attach(GTK_GRID(w2), w, 0, i, 1, 1);
		gtk_grid_attach(GTK_GRID(w2), r[i], 1, i, 1, 1);
		i++;
	}

	w = gtk_grid_new();
	gtk_grid_attach(GTK_GRID(w), w2, 0, 0, 1, 1);

	w1 = gtk_label_new(""); //use label instead of frame to make upper/left corner rounded
	addClass(w1, ROUND_CORNER_CLASS);

	gtk_grid_attach(GTK_GRID(w), w1, 0, 0, 1, 1);

	if(bridge){
		w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		for (auto a:m_score) {
			gtk_box_pack_start(GTK_BOX(w1), a, TRUE, TRUE, 6);
		}
	}
	else{
		m_label[4]=gtk_label_new(0);
		setPreferansLabel();
		w1 = gtk_grid_new();
		for(i=0;i<2;i++){
			w2= i==0? gtk_label_new(getString(STRING_PLAYER_SCORE)):m_label[4];
			gtk_widget_set_halign(w2, GTK_ALIGN_END);
			gtk_grid_attach(GTK_GRID(w1), w2, 0, i, 1, 1);
			gtk_widget_set_halign(m_score[i], GTK_ALIGN_START);
			gtk_grid_attach(GTK_GRID(w1), m_score[i], 1, i, 1, 1);
		}
	}

	gtk_grid_attach(GTK_GRID(w), w1, 0, 1, 1, 1);

	gtk_container_add(GTK_CONTAINER(getContentArea()), w);

	for (auto a:m_combo) {
		g_signal_connect(a, "changed", G_CALLBACK(combo_changed),
				gpointer(this));
	}

	updateScore();
	show();
}

void CalculatorDialog::updateScore() {
	int i,j;
	std::string s, v[2];

	if(isBridge()){
		/* contract 1-7
		 * result - tricks 0-13
		 * zone=true if playing pair in a zone in pbn tag (vulnerable=NS && (declare 'N' or 'S') || vulnerable=EW && (declare 'E' or 'W') )
		 * doubleRedouble=0 simple game; =1 double; =2 redouble
		 */
		const int declarer = CB(DECLARER);

		int r = countBridgeScore(CB(CONTRACT) + 1,
				CB(TRUMP),
				CB(TRICKS),
				CB(DOUBLE_REDOUBLE), declarer,
				CB(VULNERABLE));
		for (i = 0; i < 2; i++) {
			v[i]= format("%s/%s %d", m_player[i].c_str(), m_player[i + 2].c_str(),
					declarer % 2 == i % 2 ? r : -r);
		}
	}
	else{
		const int players = CP(PLAYERS)+3;
		const int contract = getPrerefansContract();
		const int tricks = CP(TRICKS);
		PreferansScore p;
		if(isHalfWhist() && contract!=0){
			p.setHalfWhistGame(players, contract);
		}
		else{
			p.setGame(players, contract, tricks);
		}
		auto sc=p.score();
		for (i = 0; i < 2; i++) {
			s ="";
			for (j = i; j < (i == 0 ? 1 : players); j++) {
				s+=' '+normalize(format("%.2lf", sc[j]));
			}
			v[i] = s ;

		}

	}

	i=0;
	for (auto a:m_score) {
		gtk_label_set_text(GTK_LABEL(a), v[i].c_str());
		i++;
	}
}

void CalculatorDialog::setPreferansLabel() {
	gtk_label_set_text(GTK_LABEL(m_label[4]),
			getString(
					isMisere() ?
							STRING_CATCHERS_SCORE : STRING_WHISTERS_SCORE));
}

int CalculatorDialog::getPrerefansContract() {
	const int c = CP(CONTRACT) + 6;
	return c == 11 ? 0 : c;
}

bool CalculatorDialog::isMisere() {
	return getPrerefansContract()==0;
}

bool CalculatorDialog::isHalfWhist() {
	return CP(WHIST_OPTION)==WO_HALF_WHIST;
}

void CalculatorDialog::showHideRow(int row,bool show){
	showHideWidget(m_label[row], show);
	showHideWidget(m_combo[row], show);
}

void CalculatorDialog::comboChanged(GtkWidget* w) {
	updateScore();
	if(isPreferans()){
		setPreferansLabel();
		showHideRow(1,isMisere() || !isHalfWhist());//tricks
		showHideRow(3,!isMisere());//whist option
	}
}
