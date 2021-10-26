/*
 * SolveAllFoeDialog.cpp
 *
 *       Created on: 10.05.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "SolveAllFoeDialog.h"
#include "../DrawingArea.h"
#include "../helper/PreferansScore.h"

static SolveAllFoeDialog* d;

static void button_clicked(GtkWidget *widget, gpointer) {
	d->clickButton(widget);
}

static void close_dialog(SolveAllFoeDialog *, gint, gpointer) {
	gdraw->stopSolveAllFoeThreads();
}

static gboolean combo_changed(GtkWidget *w, gpointer) {
	d->comboChanged(w);
	return TRUE;
}

SolveAllFoeDialog::SolveAllFoeDialog(int positons) :
		ButtonsDialogWithProblem(MENU_SOLVE_ALL_FOE, false,
				BUTTONS_DIALOG_NONE),m_positions(positons) {
	int i, j,k;
	GtkWidget*g, *g1, *w;
	GList* list;
	std::string s,s1;
	bool b;
	Problem const&p = getProblem();
	VString v;
	m_labelThread.resize(getMaxRunThreads()+1);
	const int leftMargin=20;

	d=this;
	m_calculationsEnd=false;

	reset();
	m_labelTotal = gtk_label_new("");
	m_labelTotalTime = gtk_label_new("");
	m_progressBar = gtk_progress_bar_new();
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(m_progressBar), TRUE);
	m_loading =gtk_spinner_new ();
	gtk_spinner_start (GTK_SPINNER(m_loading));

	m_button=createButton(NULL, STRING_COPY_TO_CLIPBOARD);
	g_signal_connect(m_button, "clicked", G_CALLBACK(button_clicked),
			gpointer(0));

	/* for long preferans problems, leave m_copyButton is active,
	 * because user may be want to save intermediate results
	 */
	//gtk_widget_set_sensitive(m_copyButton, isBridge() );

	for (i = 0; i < 4; i++) {
		if(isBridge()){
			b=i%2==isBridgeFoeAbsentNS();
		}
		else{
			b=PLAYER[i] == p.m_player;
		}
		for (j = 0; j < 4; j++) {
			gtk_label_set_text(GTK_LABEL(m_labelPlayerSuit[i][j]),
					b ? p.getRow(j, PLAYER[i]).c_str() : " ?");
		}
	}

	g = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g), 4);
	gtk_grid_set_row_spacing(GTK_GRID(g), 4);

	for (i = 0; i < SIZEI(PLAYER); i++) {
		if (isPreferans() && PLAYER[i] == getAbsent()) {
			w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			k=0;
			for (j = 0; j < 4; j++) {
				s=p.getRow(j, CARD_INDEX_ABSENT);
				v.push_back(s);
				k+=s.length()/2;//divide by two because of spaces
			}
			if(k>14){
				for (j = 0; j < 4; j++) {
					v[j]=replaceAll(v[j]," ","");
				}
			}
			for (j = 0; j < 4; j++) {
				s=v[j];
				if (s[s.length()-1] != '-') {
					list = gtk_container_get_children(GTK_CONTAINER(w));
					if (g_list_length(list) > 0) {
						gtk_container_add(GTK_CONTAINER(w), gtk_label_new(" "));
					}
					g_list_free(list);

					gtk_container_add(GTK_CONTAINER(w),
							gtk_image_new_from_pixbuf(m_suitPixbuf[getSuitsOrder(j)]));
					gtk_container_add(GTK_CONTAINER(w), gtk_label_new(s.c_str()));
				}
			}
		}
		else {
			w = createPlayerBox(PLAYER[i], false,isPreferans() && PLAYER[i] == p.m_player);
		}
		gtk_grid_attach(GTK_GRID(g), w, EDIT_LIST_REGION_POSITION[i].x,
				EDIT_LIST_REGION_POSITION[i].y, 1, 1);

	}

	attachInnerTable(g);

	gtk_widget_set_margin_top(m_progressBar, 10);
	gtk_widget_set_margin_bottom(m_progressBar, 10);

	gtk_grid_attach(GTK_GRID(g), m_progressBar, 0, 3, 3, 1);

	g1 = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g1), 4);
	gtk_grid_set_row_spacing(GTK_GRID(g1), 4);

	gtk_grid_attach(GTK_GRID(g1), g, 0, 0, 1, resultSize()+1);

	g = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g), 2);
	gtk_grid_set_row_spacing(GTK_GRID(g), 2);

	for (i = 0; i < resultSize(); i++) {
		m_result[i] = 0;
		for (j = 0; j < 2; j++) {
			m_label[i][j] = gtk_label_new("");
		}
	}
	if (isBridge()) {
		s=getString(STRING_TRICKS1);
		s+="\n";
		i=northOrSouth(p.getVeryFirstMove());
		s1=getString(i==0 ? STRING_NORTH:STRING_EAST);
		s1+=" / ";
		s1+=getString(i==0 ? STRING_SOUTH:STRING_WEST);
		s+=utf8ToLowerCase(s1);
	}
	else {
		s=getString(STRING_PLAYER_TRICKS);
	}


	i=k=1;
	w=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
	//w=gtk_label_new("s.c_str()");
	g_object_set (w, "margin-left", leftMargin, NULL);
	gtk_grid_attach(GTK_GRID(g1), w, i++,0, 1, 1);

	w=gtk_label_new(s.c_str());
	gtk_label_set_xalign(GTK_LABEL(w), 0.5);//center full label inside grid
	gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_CENTER);//center every line of label
	gtk_grid_attach(GTK_GRID(g1), w, i++, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(g1), gtk_label_new(getString(STRING_POSITIONS)), i++,
			0, 1, 1);
	gtk_grid_attach(GTK_GRID(g1), gtk_label_new("%"), i++, 0, 1, 1);

	for (i = 0; i < resultSize(); i++) {
		gtk_grid_attach(GTK_GRID(g1), gtk_label_new(format("%d", i).c_str()), k+1,
				i + 1, 1, 1);
		for (j = 0; j < 2; j++) {
			gtk_grid_attach(GTK_GRID(g1), m_label[i][j], k+j + 2, i + 1, 1, 1);
		}
	}

	i++;
	gtk_grid_attach(GTK_GRID(g1), m_labelTotalTime, 0, i, 1, 1);
	gtk_grid_attach(GTK_GRID(g1), gtk_label_new(getString(STRING_TOTAL)), k+1,
			i, 1, 1);
	gtk_grid_attach(GTK_GRID(g1), m_labelTotal, k+2, i, 1, 1);

	i++;
	gtk_grid_attach(GTK_GRID(g1), m_button, 0, i, 2, 1);
	gtk_grid_attach(GTK_GRID(g1), m_loading, 2, i, 1, 1);

	//thread statistics
	j=k+4;
	w=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
	g_object_set (w, "margin-left", leftMargin, NULL);
	gtk_grid_attach(GTK_GRID(g1), w, j++,0, 1, 1);
	gtk_grid_attach(GTK_GRID(g1), gtk_label_new(getString(STRING_THREAD)), j,0, 1, 1);

	s=getString(STRING_POSITIONS)+std::string("\n")+getString(STRING_PER_SECOND);
	w=gtk_label_new(s.c_str());
	gtk_label_set_xalign(GTK_LABEL(w), 0.5);//center full label inside grid
	gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_CENTER);//center every line of label
	gtk_grid_attach(GTK_GRID(g1), w, j+1,0, 1, 1);
	for(i=0;i<getMaxRunThreads()+1;i++){
		if(i==getMaxRunThreads()){
			s=getString(STRING_AVERAGE);
		}
		else{
			s=format("#%d",i+1);
		}
		gtk_grid_attach(GTK_GRID(g1), gtk_label_new(s.c_str()), j,i+1, 1, 1);
		m_labelThread[i]=gtk_label_new(0);
		gtk_grid_attach(GTK_GRID(g1), m_labelThread[i], j+1,i+1, 1, 1);
	}

	if(isBridge()){
		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
		gtk_container_add(GTK_CONTAINER(w), gtk_label_new(getString(STRING_PLAYERS_WITH_UNKNOWN_CARDS)) );

		v.clear();
		for(i=0;i<2;i++){
			s=getString(i==0 ? STRING_NORTH:STRING_EAST);
			s+=" / ";
			s+=getString(i==0 ? STRING_SOUTH:STRING_WEST);
			v.push_back(utf8ToLowerCase(s));
		}
		m_combo = createTextCombobox(v);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo), !isBridgeFoeAbsentNS() );
		g_signal_connect(m_combo, "changed", G_CALLBACK(combo_changed),
				gpointer(0));

		gtk_container_add(GTK_CONTAINER(w), m_combo);

		gtk_grid_attach(GTK_GRID(g1), w, 0, resultSize()+3, 3, 1);
	}

	m_notebook = gtk_notebook_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), g1,
			label(STRING_CALCULATIONS));
	gtk_container_add(GTK_CONTAINER(getContentArea()), m_notebook);

	updateLabels();
	setInnerTable(getProblem());
	g_signal_connect(getWidget(), "response", G_CALLBACK(close_dialog), NULL);
	show();
}

SolveAllFoeDialog::~SolveAllFoeDialog(){
}

void SolveAllFoeDialog::updateLabels() {
	int i,j;
	double v,va;
	std::string s;

	g_mutex_lock(&gdraw->m_solveAllMutex);

	setResults();

	for (i = 0; i < resultSize(); i++) {
		gtk_label_set_text(GTK_LABEL(m_label[i][0]),
				intToStringLocaled(m_result[i]).c_str());

		gtk_label_set_text(GTK_LABEL(m_label[i][1]),
				m_total == 0 ? "?.?%" : format("%.1lf%%", m_result[i] * 100. / m_total).c_str());
	}

	auto a=[](double v){
		return v==0?"?":(v>100 ? intToStringLocaled(int(v)) : format("%.3lf",v));
	};

	va=0;
	j=0;
	for(i=0;i<getMaxRunThreads();i++){
		auto p=gdraw->m_vSolveAll[i];
		//println("%d",p.positions)
		if(p.positions==0){
			v=0;
		}
		else{
			j++;
			v=p.positions/(double(p.end-p.begin)/CLOCKS_PER_SEC);
			va+=v;
		}
		s=a(v);
		gtk_label_set_text(GTK_LABEL(m_labelThread[i]),s.c_str());
	}
	if(j!=0){
		va/=j;
	}
	s=a(va);
	gtk_label_set_text(GTK_LABEL(m_labelThread[i]),s.c_str());

	g_mutex_unlock(&gdraw->m_solveAllMutex);


	gtk_label_set_text(GTK_LABEL(m_labelTotal), intToStringLocaled(m_total).c_str());

	gtk_label_set_text(GTK_LABEL(m_labelTotalTime),getTotalTimeLabelString().c_str());

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_progressBar), m_fraction);

	//somehow call many times with m_total == m_positions
	//not helps if (m_total == m_positions && gtk_notebook_get_n_pages(GTK_NOTEBOOK(m_notebook))) {
	if (m_total == m_positions && !m_calculationsEnd) {
		m_calculationsEnd=true;
		gtk_spinner_stop (GTK_SPINNER(m_loading));
		addContractsScoringTab();
		//gtk_widget_set_sensitive(m_copyButton, TRUE);
	}

	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(m_progressBar), getProgressBarString().c_str());
}

void SolveAllFoeDialog::clickButton(GtkWidget* w) {
	int i;
	std::string s;
	//i=INDEX_OF(w, m_button);
	i = getTrump();
	if (i == NT) {
		s += getString(isMisere() ?  STRING_MISERE :STRING_NT);
	}
	else {
		s+=getString(STRING_TRUMP);
		s += ":";
		s+=getString(STRING_ID(STRING_SPADES+i));
	}

	s+="\n";

	for (i = 0; i < resultSize(); i++) {
		s += format("%d %d\n", i, m_result[i]);
	}

	if(m_total != m_positions){
		s+=getTotalTimeLabelString()+"\n"+getProgressBarString(false);
	}

	GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(clipboard, s.c_str(), s.length());
}

int SolveAllFoeDialog::resultSize()const{
	return 1+ getMaxHandCards();
}

void SolveAllFoeDialog::comboChanged(GtkWidget *w){
	if(w==m_combo1[0] || w==m_combo1[1]){
		recountScores();
		return;
	}
	setBridgeFoeAbsentNS(!gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo)));
	int i,j;
	Problem const&p = getProblem();
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			gtk_label_set_text(GTK_LABEL(m_labelPlayerSuit[i][j]),
					i % 2 == isBridgeFoeAbsentNS() ? p.getRow(j, PLAYER[i]).c_str() : " ?");
		}
	}


	gdraw->stopSolveAllFoeThreads();

	//Call reset() only after stopSolveAllFoeThreads, because need set m_id
	reset();

	//stopSolveAllFoeThreads
	gdraw->m_solveAllFoeDialog=this;
	gdraw->solveAllFoe(false);//here new number of positions is set
}


void SolveAllFoeDialog::reset(){
	m_id=g_get_real_time();
	m_begin = clock();
	for (int& q :m_result) {
		q = 0;
	}
}

void SolveAllFoeDialog::setPositions(int positions){
	m_positions=positions;
	updateLabels();
}

void SolveAllFoeDialog::setResults() {
	int i;
	for (i =m_total= 0; i < resultSize(); i++) {
		m_total += m_result[i];
	}
	m_fraction = double(m_total)/m_positions;
}

std::string SolveAllFoeDialog::getTotalTimeLabelString() {
	m_end = clock();
	std::string s=getString(STRING_TIME);
	s+=" ";
	s+=secondsToString( m_end, m_begin);
	s+=" ";
	s+=getString(STRING_LEFT);
	s+=" ";
	auto t=double(m_end - m_begin) / CLOCKS_PER_SEC;
	/* t - j positions
	 * tall - af positions
	 * tall=af/j*t
	 *
	 * timeLeft=tall-t=af/j*t-t=t(af/j-1)
	 */
	s+=m_total==0? "??:??:??" : secondsToString(t*(1/m_fraction-1));
	return s;
}

std::string SolveAllFoeDialog::getProgressBarString(bool b) {
	return format(isBridge() ? "%.3lf%%" : "%.0lf%%", m_fraction * 100) + (b?"   ":" ")
			+ intToStringLocaled(m_total) + "/" + intToStringLocaled(m_positions);
}

void SolveAllFoeDialog::addContractsScoringTab() {
	int i,j;
	GtkWidget*g,*w,*w1;
	std::string s;

	m_grid1=g=gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g), 4);
	gtk_grid_set_row_spacing(GTK_GRID(g), 4);

	j=0;
	const int columns=3;
	const int trump=getTrump();

	gtk_grid_attach(GTK_GRID(g),
			createBoldLabel(
					isMisere() ?
							STRING_TABLE_EV_CAPTION_MISERE : STRING_TABLE_EV_CAPTION),
			0, j++, columns, 1);

	gtk_grid_attach(GTK_GRID(g), createBoldLabel(STRING_CONTRACT), 0, j, 1, 1);
	gtk_grid_attach(GTK_GRID(g), createBoldLabel(STRING_PLAYER_SCORE), 1, j, 1, 1);
	gtk_grid_attach(GTK_GRID(g),
			createBoldLabel(
					isMisere() ? STRING_CATCHERS_SCORE : STRING_WHISTERS_SCORE),
			2, j, 1, 1);
	j++;

	if(isMisere()){
		addGridRow(label(STRING_MISERE),j);
		j++;
	}
	else{
		for (i = 6; i <= 10; i++) {
			s = std::to_string(i);
			if(trump==NT){
				s+=" "+getNTString();
				w=label(s);
			}
			else{
				w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
				gtk_container_add(GTK_CONTAINER(w), label(s));
				gtk_container_add(GTK_CONTAINER(w),
						gtk_image_new_from_pixbuf(m_suitPixbuf[trump]));
				gtk_widget_set_halign(w, GTK_ALIGN_CENTER);

			}
			addGridRow(w,j);
			j++;
		}
	}

	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	m_combo1[0]=createTextCombobox(3, 4);
	s=getString(STRING_WHIST_OPTIONS_COMBO);
	m_combo1[1]=createTextCombobox(split(s,'#'));

	i=0;
	for(auto a:m_combo1){
		w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
		if(i==0){
			gtk_container_add(GTK_CONTAINER(w1), label(STRING_PLAYERS));
		}
		gtk_container_add(GTK_CONTAINER(w1), a);
		gtk_widget_set_halign(w1, GTK_ALIGN_CENTER);
		gtk_box_pack_start(GTK_BOX(w), w1, TRUE, TRUE, 0);

		g_signal_connect(a, "changed", G_CALLBACK(combo_changed),
				gpointer(0));
		i++;
	}

	auto f= gtk_frame_new(getString(STRING_OPTIONS));
	gtk_container_add(GTK_CONTAINER(f), w);
	gtk_frame_set_label_align(GTK_FRAME(f), 0.03, 0.5);

	gtk_grid_attach(GTK_GRID(g), f, 0, j++, columns, 1);

	m_label1 = createMarkupLabel(STRING_CONTRACTS_SCORING_HELP,60);
	gtk_grid_attach(GTK_GRID(g), m_label1, 0, j++, columns, 1);


	gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), g,
			label(STRING_CONTRACTS_SCORING));

	gtk_widget_show_all(m_notebook);

	recountScores();

	//TODO remove
	gtk_notebook_next_page(GTK_NOTEBOOK(m_notebook));
}

enum{
	WHIST_PROFITABLE_DEALS_ELSE_PASS
	,WHIST_PROFITABLE_DEALS_ELSE_HALFWHIST
	,ALWAYS_WHIST
	,ALWAYS_HALFWHIST
	,ALWAYS_PASS
};

void SolveAllFoeDialog::recountScores() {
	auto players = getComboPosition(m_combo1[0]) + 3;
	int option=getComboPosition(m_combo1[1]);
	int contract,tricks;
	PreferansScore p;
	WHIST_OPTION wo;
	VDouble v[11][11][WHIST_OPTION_SIZE],r;
	double probability[MAX_RESULT_SIZE];
	GtkWidget*w;
	int i,j;
	std::string s;
	double ev;

	for(i=0;i<MAX_RESULT_SIZE;i++){
		probability[i]=double(m_result[i]) / m_total;
	}
/*
	for (tricks = 0; tricks <= 10; tricks++) {
		for(j=0;j<11;j++){
			for(i=0;i<WHIST_OPTION_SIZE;i++){
				v[tricks][j][i]={10e+100,10e+100,10e+100,10e+100};
			}
		}
	}
*/

	for (tricks = 0; tricks <= 10; tricks++) {
		for (auto c :{0,6,7,8,9,10}) {
			for(i=0;i<WHIST_OPTION_SIZE;i++){
				wo=WHIST_OPTION(i);
				if(wo!=WHIST_OPTION_WHIST && c!=tricks){
					continue;
				}
				v[tricks][c][i]=p.getGameScore(players, c, tricks,wo);
			}
		}
	}

	if(isMisere()){
		w=gtk_grid_get_child_at(GTK_GRID(m_grid1), 1, 1);
		gtk_label_set_text(GTK_LABEL(w), "misere");
	}
	else{
/*
		WHIST_PROFITABLE_DEALS_ELSE_PASS
		,WHIST_PROFITABLE_DEALS_ELSE_HALFWHIST
*/
		if (option == ALWAYS_WHIST) {
			for (contract = 6; contract <= 10; contract++) {
				r.clear();
				for(i=0;i<players;i++){
					ev=0;
					for (tricks = 0; tricks <= 10; tricks++) {
						ev += probability[tricks]
								* v[tricks][contract][WHIST_OPTION_WHIST][i];
					}
					r.push_back(ev);
				}
				setGridLabels(contract, r);
			}
		}
		else if (option==ALWAYS_HALFWHIST || option==ALWAYS_PASS) {
			wo = option == ALWAYS_HALFWHIST ?
					WHIST_OPTION_HALFWHIST : WHIST_OPTION_ALLPASS;
			for (contract = 6; contract <= 10; contract++) {
				setGridLabels(contract, v[contract][contract][wo]);
			}
		}
	}
}

void SolveAllFoeDialog::addGridRow(GtkWidget *w, int row) {
	gtk_grid_attach(GTK_GRID(m_grid1), w, 0, row, 1, 1);
	for(int k=1;k<3;k++){
		gtk_grid_attach(GTK_GRID(m_grid1), label(), k, row, 1, 1);
	}
}

void SolveAllFoeDialog::setGridLabels(int contract,const VDouble& v) {
	for(int i=0;i<2;i++){
		auto w=gtk_grid_get_child_at(GTK_GRID(m_grid1), i+1, contract-4);
		auto s = getScoreString(i,v);
		gtk_label_set_text(GTK_LABEL(w),s.c_str());
	}
}
