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
	VString v;
	VStringID l;
	VGtkWidgetPtr wv;
	const bool bridge=isBridge();
	const int margin = 4;
	const int hmargin = 11;
	const int topmargin=10;

	for (auto& a:m_score) {
		a = gtk_label_new("");
		gtk_widget_set_margin_top(a, topmargin);
		gtk_widget_set_halign(a, GTK_ALIGN_START);
		gtk_widget_set_margin_start(a, 20);//horizontal space
	}

	if(bridge){
		//create comboboxes
		m_combo = { createTextCombobox(1, 7),
				createImageCombobox(), createTextCombobox(0, 13),
				createTextCombobox(STRING_NO,STRING_YES), createTextCombobox(DOUBLE_REDOUBLE,
						SIZE(DOUBLE_REDOUBLE)) };

		gtk_combo_box_set_model(GTK_COMBO_BOX(BC(TRUMP)),
				createTrumpModel(false, true, BRIDGE));
		gtk_combo_box_set_active(GTK_COMBO_BOX(BC(TRUMP)), 0);

		gtk_combo_box_set_active(GTK_COMBO_BOX(BC(TRICKS)), 7);

		l = { STRING_CONTRACT, STRING_NUMBER_OF_TRICKS, STRING_VULNERABLE };
		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, margin);
		gtk_container_add(GTK_CONTAINER(w), BC(CONTRACT));
		gtk_container_add(GTK_CONTAINER(w), BC(TRUMP));
		gtk_container_add(GTK_CONTAINER(w), BC(DOUBLE_REDOUBLE));

		wv = { w, BC(TRICKS), BC(VULNERABLE) };

	}
	else{
		m_combo =
				{ createTextCombobox(6, 10, 1, getString(STRING_MISERE)),
						createTextCombobox(0, 10), createTextCombobox(3, 4),
				createTextCombobox(STRING_WHIST, 3) };

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

	w1 = gtk_grid_new();

	for (i = 0; i < (isBridge() ? 2 : 4); i++) {
		if(bridge){
			s=getString(i==0?STRING_DECLARING_SIDE:STRING_DEFENDERS);
		}
		else{
			s="";
		}
		w2= m_label[i+4]=label(s);
		gtk_widget_set_margin_top(w2, topmargin);
		gtk_widget_set_halign(w2, GTK_ALIGN_END);
		gtk_grid_attach(GTK_GRID(w1), w2, 0, i, 1, 1);
		gtk_grid_attach(GTK_GRID(w1), m_score[i], 1, i, 1, 1);
	}

	if(!bridge){
		setPreferansLabels();
	}

	gtk_grid_attach(GTK_GRID(w), w1, 0, 1, 1, 1);

	gtk_container_add(GTK_CONTAINER(getContentArea()), w);

	for (auto a:m_combo) {
		g_signal_connect(a, "changed", G_CALLBACK(combo_changed),
				gpointer(this));
	}

	updateScore();
	VGtkWidgetPtr ve={m_label[7],m_score[3]};
	showExclude(ve);
}

void CalculatorDialog::updateScore() {
	int i;
	std::string v[4];

	if(isBridge()){
		int r = countBridgeScore(CB(CONTRACT) + 1, CB(TRUMP), CB(TRICKS),
				CB(DOUBLE_REDOUBLE), CB(VULNERABLE));
		for (i = 0; i < 2; i++) {
			v[i]= std::to_string(i==0 ? r : -r);
		}
	}
	else{
		const int players = getPreferansPlayers();
		const int contract = getPreferansContract();
		const int tricks = CP(TRICKS);
		WHIST_OPTION wo=WHIST_OPTION(CP(WHIST_OPTION));
		PreferansScore p;
		if(isMisere() || wo==WHIST_OPTION_WHIST ){
			p.setGame(players, contract, tricks);
		}
		else{
			p.setNonPlayingGame(players, contract,wo==WHIST_OPTION_HALFWHIST);
		}
		for (i = 0; i < 4; i++) {
			v[i] = normalize(format("%.2lf",p.score()[i])) ;
		}
	}

	i=0;
	for (auto a:m_score) {
		gtk_label_set_text(GTK_LABEL(a), v[i].c_str());
		i++;
	}
}

void CalculatorDialog::setPreferansLabels() {
	int i;
	std::string s;
	gtk_label_set_text(GTK_LABEL(m_label[4]),
			getString(
					isMisere() ?
							STRING_MISERE_PLAYER : STRING_DECLARER));
	for (i = 0; i < 3; i++) {
		s = getString(isMisere() ? STRING_CATCHER : STRING_WHISTER)+std::to_string(i+1);
		gtk_label_set_text(GTK_LABEL(m_label[i + 5]), s.c_str());
	}
}

int CalculatorDialog::getPreferansContract() {
	const int c = CP(CONTRACT) + 6;
	return c == 11 ? 0 : c;
}

int CalculatorDialog::getPreferansPlayers(){
	return CP(PLAYERS)+3;
}

bool CalculatorDialog::isMisere() {
	return getPreferansContract()==0;
}

void CalculatorDialog::showHideRow(int row,bool show){
	showHideWidget(m_label[row], show);
	showHideWidget(m_combo[row], show);
}

void CalculatorDialog::comboChanged(GtkWidget* w) {
	updateScore();
	if (isPreferans()) {
		setPreferansLabels();
		showHideRow(1, isMisere() || CP(WHIST_OPTION) == WHIST_OPTION_WHIST); //tricks
		showHideRow(3, !isMisere()); //whist option
		if(w==m_combo[PREFERANS_CALCULATOR_DIALOG_PLAYERS]){
			bool players4 = getPreferansPlayers() == 4;
			showHideWidget(m_score[3], players4);
			showHideWidget(m_label[3 + 4], players4);
		}
	}
}
