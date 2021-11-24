/*
 * SolveAllDealsDialog.cpp
 *
 *       Created on: 10.05.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "SolveAllDealsDialog.h"

#include "../DrawingArea.h"
#include "../helper/PreferansScore.h"

static SolveAllDealsDialog* d;

const int PREFERANS_PLAYERS_COMBO=1;
const int PREFERANS_WHIST_OPTION_COMBO=2;
const int BRIDGE_DOUBLE_REDOUBLE_COMBO=1;
const int BRIDGE_VULNERABLE_COMBO=2;
const int TAB1=0;
const int TAB2=1;
const int EXPORT_THE_RESULTS_OF_ALL_DEALS_TO_CSV_BUTTON=2;
//2nd tab
const int TITLE_ROWS=2;

enum{
	WHIST_PROFITABLE_DEALS_ELSE_PASS
	,WHIST_PROFITABLE_DEALS_ELSE_HALFWHIST
	,ALWAYS_WHIST
	,ALWAYS_HALFWHIST
	,ALWAYS_PASS
};

static void button_clicked(GtkWidget *widget, gpointer) {
	d->clickButton(widget);
}

static void close_dialog(SolveAllDealsDialog *, gint, gpointer) {
	d->close();
}

static gboolean combo_changed(GtkWidget *w, gpointer) {
	d->comboChanged(w);
	return TRUE;
}

static void check_changed(GtkWidget *check, GtkWidget* w) {
	d->checkChanged(check,w);
}

static gboolean label_click(GtkWidget *widget, GdkEventButton *event,
		GtkWidget* w) {
	d->labelClick(w);
	return TRUE;
}

SolveAllDealsDialog::SolveAllDealsDialog(int positons) :
		ButtonsDialogWithProblem(MENU_SOLVE_ALL_DEALS, false,
				BUTTONS_DIALOG_NONE),m_positions(positons) {
	int i, j,k;
	GtkWidget*g, *g1, *w,*w1,*wa;
	GList* list;
	std::string s;
	Problem const&p = getProblem();
	VString v;
	const int leftMargin=20;
	STRING_ID sid;

	d=this;
	m_labelThread.resize(getMaxRunThreads()+1);
	g_mutex_init(&m_mutex);

	reset();
	m_labelTotal = gtk_label_new("");
	m_labelTotalTime = gtk_label_new("");
	m_progressBar = gtk_progress_bar_new();
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(m_progressBar), TRUE);
	for(auto& a:m_loading){
		a =gtk_spinner_new ();
		gtk_widget_set_size_request(a, 32, 32);//otherwise small
		gtk_spinner_start (GTK_SPINNER(a));
	}

	i=0;
	for(auto& a:m_button){
		if(i==EXPORT_THE_RESULTS_OF_ALL_DEALS_TO_CSV_BUTTON){
			sid=STRING_EXPORT_THE_RESULTS_OF_ALL_DEALS_TO_CSV;
		}
		else{
			sid=STRING_COPY_TO_CLIPBOARD;
		}
		a=createTextButton(sid);
		g_signal_connect(a, "clicked", G_CALLBACK(button_clicked),
				gpointer(0));
		i++;
	}

	m_entry=gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(m_entry), 1);
	gtk_entry_set_text(GTK_ENTRY(m_entry), csvSeparator().c_str());


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

	setPlayersCards();

	gtk_widget_set_margin_top(m_progressBar, 10);
	gtk_widget_set_margin_bottom(m_progressBar, 10);

	gtk_grid_attach(GTK_GRID(g), m_progressBar, 0, 3, 3, 1);

	g1 = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g1), 4);
	gtk_grid_set_row_spacing(GTK_GRID(g1), 0);

	gtk_grid_attach(GTK_GRID(g1), g, 0, 0, 1, resultSize()+1);

	g = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(g), 2);
	gtk_grid_set_row_spacing(GTK_GRID(g), 2);

	for (i = 0; i < resultSize(); i++) {
		m_result[i] = 0;//no multithread calls
		for (j = 0; j < 2; j++) {
			m_label[i][j] = gtk_label_new("");
		}
	}
	if (isBridge()) {
		s=getString(STRING_TRICKS1)+("\n"+getNSEWString(isDeclarerNorthOrSouth()));
	}
	else {
		s=getString(STRING_PLAYER_TRICKS);
	}

	i=k=1;
	w=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
	g_object_set (w, "margin-left", leftMargin, NULL);
	gtk_grid_attach(GTK_GRID(g1), w, i++,0, 1, 1);

	w=gtk_label_new(s.c_str());
	gtk_label_set_xalign(GTK_LABEL(w), 0.5);//center full label inside grid
	gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_CENTER);//center every line of label
	gtk_grid_attach(GTK_GRID(g1), w, i++, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(g1), gtk_label_new(getString(STRING_DEALS)), i++,
			0, 1, 1);
	gtk_grid_attach(GTK_GRID(g1), gtk_label_new("%"), i++, 0, 1, 1);

	for (i = 0; i < resultSize(); i++) {
		gtk_grid_attach(GTK_GRID(g1), gtk_label_new(std::to_string(i).c_str()), k+1,
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

	wa=gtk_box_new(GTK_ORIENTATION_VERTICAL,4);
	gtk_container_add(GTK_CONTAINER(wa), g1 );

	w=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
	gtk_container_add(GTK_CONTAINER(w), m_button[TAB1] );
	gtk_container_add(GTK_CONTAINER(w), m_loading[TAB1] );
	gtk_container_add(GTK_CONTAINER(wa), w );

	w=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
	gtk_container_add(GTK_CONTAINER(w), m_button[EXPORT_THE_RESULTS_OF_ALL_DEALS_TO_CSV_BUTTON] );
	s="csv ";
	s+=getString(STRING_SEPARATOR);
	w1=label(s);
	g_object_set (w1, "margin-left", leftMargin, NULL);
	gtk_container_add(GTK_CONTAINER(w), w1 );
	gtk_box_pack_start(GTK_BOX(w), m_entry, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(wa), w );

	if(isBridge()){
		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
		g_object_set (w, "margin-top", 4, NULL);
		gtk_container_add(GTK_CONTAINER(w), gtk_label_new(getString(STRING_PLAYERS_WITH_UNKNOWN_CARDS)) );

		v.clear();
		for(j=0;j<2;j++){
			v.push_back(getNSEWString(j==0));
		}
		m_combo[TAB1] = createTextCombobox(v);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[TAB1]), !isBridgeSolveAllDealsAbsentNS() );

		gtk_container_add(GTK_CONTAINER(w), m_combo[TAB1]);

		gtk_container_add(GTK_CONTAINER(wa), w );
	}

	w = createMarkupLabel(STRING_FIX_CARDS_HELP, 60);
	gtk_container_add(GTK_CONTAINER(wa), w );

	//thread statistics
	j=k+4;
	w=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
	g_object_set (w, "margin-left", leftMargin, NULL);
	gtk_grid_attach(GTK_GRID(g1), w, j++,0, 1, 1);
	gtk_grid_attach(GTK_GRID(g1), gtk_label_new(getString(STRING_THREAD)), j,0, 1, 1);

	s=getString(STRING_DEALS)+std::string("\n")+getString(STRING_PER_SECOND);
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


	m_notebook = gtk_notebook_new();

	for (auto w : { wa, createTab2() }) {
		gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), w,
				label(w == wa ? STRING_TRICKS1 : STRING_CONTRACTS));
	}

	gtk_container_add(GTK_CONTAINER(getContentArea()), m_notebook);

	updateData();
	setInnerTable(getProblem());
	g_signal_connect(getWidget(), "response", G_CALLBACK(close_dialog), NULL);

	i=0;
	for (auto a : m_combo) {
		if(isBridge() || i!=TAB1){
			g_signal_connect(a, "changed", G_CALLBACK(combo_changed),
					gpointer(0));
		}
		i++;
	}

	VGtkWidgetPtr e;
	if(isPreferans()){
		e=getLastWhisterWidgets();
	}
	showExclude(e);

	//gtk_notebook_next_page(GTK_NOTEBOOK(m_notebook));
}

SolveAllDealsDialog::~SolveAllDealsDialog(){
	g_mutex_clear(&m_mutex);
}

void SolveAllDealsDialog::updateData() {
	int i,j;
	double v,va;
	std::string s;
	int result[MAX_RESULT_SIZE];

	g_mutex_lock(&m_mutex);
	std::copy(m_result,m_result+MAX_RESULT_SIZE,result);//multithread ok
	g_mutex_unlock(&m_mutex);


	for (i =m_total= 0; i < resultSize(); i++) {
		m_total += result[i];
	}
	m_fraction = double(m_total)/m_positions;

	for (i = 0; i < resultSize(); i++) {
		gtk_label_set_text(GTK_LABEL(m_label[i][0]),
				intToStringLocaled(result[i]).c_str());

		gtk_label_set_text(GTK_LABEL(m_label[i][1]),
				m_total == 0 ? "?.?%" : format("%.1lf%%", result[i] * 100. / m_total).c_str());
	}

	auto a=[](double v){
		return v==0?"?":(v>100 ? intToStringLocaled(int(v)) : format("%.3lf",v));
	};

	va=0;
	j=0;
	for(i=0;i<getMaxRunThreads();i++){
		auto& p=gdraw->m_vSolveAll[i];
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

	gtk_label_set_text(GTK_LABEL(m_labelTotal), intToStringLocaled(m_total).c_str());
	gtk_label_set_text(GTK_LABEL(m_labelTotalTime),getTotalTimeLabelString().c_str());
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_progressBar), m_fraction);

	if (m_total == m_positions) {
		for(i=0;i<getMaxRunThreads();i++){
			s=gtk_label_get_text(GTK_LABEL(m_labelThread[i]));
			if(s=="?"){
				gtk_label_set_text(GTK_LABEL(m_labelThread[i]),"-");
			}
		}
		for(auto a:m_loading){
			gtk_spinner_stop (GTK_SPINNER(a));
		}
	}

	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(m_progressBar), getProgressBarString().c_str());

	updateTab2();
}

void SolveAllDealsDialog::clickButton(GtkWidget* w) {
	int i,j;
	std::string s;
	const int trump = getTrump();

	if (w == m_button[EXPORT_THE_RESULTS_OF_ALL_DEALS_TO_CSV_BUTTON]) {
		FileChooserResult r = fileChooserSave(FILE_TYPE_CSV);
		if (r.ok()) {
			VDealResult v;
			auto &sa = gdraw->m_vSolveAll;
			i = 0;
			for (auto &a : sa) {
				i += a.dealResultSize();
			}
			v.reserve(i);
			for (auto &a : sa) {
				a.add(v);
			}

			std::sort(v.begin(), v.end());

			/* try to make output file as small as possible
			 * because for bridge
			 * C^13_26 = 10'400'600
			 * one row 16chars(13+3) for deals+2chars for result+2separator+"\n" so 16*2+2+2+1=37 chars
			 * max file size without title 10'400'600*37=384'822'200
			 *
			 * for preferans max 184'756
			 * one row 13chars(10+3) for deals+2chars for result+2separator+"\n" so 13*2+2+2+1=31 chars
			 * max file size without title 184'756*31=5'727'436
			 */
			std::ofstream f(r.file());
			if (!f.is_open()) {
				showOpenFileError();
				return;
			}

			//BOM
			f << char(0xef) << char(0xbb) << char(0xbf);

			for (i = 0; i < 2; i++) {
				f << getLowercasedPlayerString(sa[0].p[i]) << csvSeparator();
			}
			if (isBridge()) {
				s = getString(STRING_TRICKS1)
						+ (" " + getNSEWString(isDeclarerNorthOrSouth()));
			} else {
				s = getString(STRING_PLAYER_TRICKS);
				s = replaceAll(s, "\n", " ");
			}
			f << s << "\n";

			for (auto &a : v) {
				for (i = 0; i < 2; i++) {
					f << a.a[i] << csvSeparator();
				}
				//not a.result but int(a.result)
				f << int(a.result) << "\n";
			}
		}
		return;
	}
	else if(w==m_button[TAB2]){
		int columns = isBridge() ? 4 : getPreferansPlayers()+2;

		for (j = 0; j < getTableRowsTab2(); j++) {
			if(j){
				s += "\n";
			}
			for (i = 0; i < columns; i++) {
				//first row and first column has spanned on two columns so i>1
				if (i>1) {
					s += "\t";
				}
				//first column has spanned on two columns for first row
				//in case of misere first column has spanned on two columns for all rows
				if((j==0 || isMisere()) && i==1){
					continue;
				}
				auto w = gtk_grid_get_child_at(GTK_GRID(m_grid), i, j);
				if(GTK_IS_LABEL(w)){
					/* "6NT" -> "6 NT", for suits game not add
					 * space before suit symbol it looks good
					 */
					if(i==1 && trump==NT){
						s+=' ';
					}
					s += gtk_label_get_text(GTK_LABEL(w));
					//printzn(i,j,'[',utf8ToLocale(gtk_label_get_text(GTK_LABEL(w))),']')
				}
				else{
					assert(trump!=NT);
					s+=UTF8SUITS[trump];
				}
			}
		}
	}
	else{
		i = getTrump();
		if (i == NT) {
			s += getString(isMisere() ?  STRING_MISERE :STRING_NT);
		}
		else {
			s+=getString(STRING_TRUMP)+ (" "+UTF8SUITS[i]);
		}
		s+="\n";

		g_mutex_lock(&m_mutex);
		for (i = 0; i < resultSize(); i++) {
			s+=std::to_string(i)+"\t"+std::to_string(m_result[i])+"\n";//multithread ok
		}
		g_mutex_unlock(&m_mutex);

		if(m_total != m_positions){
			s+=getTotalTimeLabelString()+"\n"+getProgressBarString(false);
		}
	}
	GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(clipboard, s.c_str(), s.length());
}

int SolveAllDealsDialog::resultSize()const{
	return 1+ getMaxHandCards();
}

void SolveAllDealsDialog::comboChanged(GtkWidget *w){
	int i=INDEX_OF(w,m_combo);

	if(i==TAB1){//only bridge
		setBridgeSolveAllDealsAbsentNS(!gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo[TAB1])));
		setPlayersCards();
		gdraw->stopSolveAllDealsThreads();

		//Call reset() only after stopSolveAllDealsThreads, because need set m_id
		reset();
		for(auto a:m_loading){
			gtk_spinner_start (GTK_SPINNER(a));
		}

		//stopSolveAllDealsThreads
		gdraw->m_solveAllDealsDialog=this;
		gdraw->solveAllDeals(false);//here new number of positions is set
	}
	else{
		if(isPreferans() && i==PREFERANS_PLAYERS_COMBO){
			updateNumberOfPreferansPlayers();
		}
		updateTab2();
	}

}


void SolveAllDealsDialog::reset(){
	m_id=g_get_real_time();
	m_begin = clock();
	for (int& a :m_result) {//no multithread calls
		a = 0;
	}
}

void SolveAllDealsDialog::setPositions(int positions){
	m_positions=positions;
	updateData();
}

std::string SolveAllDealsDialog::getTotalTimeLabelString() {
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

std::string SolveAllDealsDialog::getProgressBarString(bool b) {
	return getPercentString() + (b ? "   " : " ") + intToStringLocaled(m_total)
			+ "/" + intToStringLocaled(m_positions);
}

GtkWidget* SolveAllDealsDialog::createTab2() {
	int i,j;
	GtkWidget *w,*w1,*w2;
	std::string s;
	VString vs;
	STRING_ID id;
	bool b;
	const int trump=getTrump();
	const bool bridge=isBridge();

	//m_button,m_loading are created
	m_grid=gtk_grid_new();
	//gtk_grid_set_column_spacing(GTK_GRID(m_grid1), 4);
	gtk_grid_set_row_spacing(GTK_GRID(m_grid), 4);
	m_labelPercentTab2=label();

	if(bridge){
		m_combo[BRIDGE_DOUBLE_REDOUBLE_COMBO]=createTextCombobox(STRING_UNDOUBLED,3);
		m_combo[BRIDGE_VULNERABLE_COMBO]=createTextCombobox(STRING_NO,STRING_YES);
	}
	else{
		m_combo[PREFERANS_PLAYERS_COMBO]=createTextCombobox(3, 4);
		s=getString(STRING_WHIST_OPTIONS_COMBO);
		m_combo[PREFERANS_WHIST_OPTION_COMBO]=createTextCombobox(split(s,'#'));
	}

	w2=gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

	gtk_container_add(GTK_CONTAINER(w2),
			createBoldLabel(STRING_EXPECTED_VALUE_TABLE));

	for(j=0;j<TITLE_ROWS;j++){
		if(j==0){
			vs.push_back(getString(STRING_CONTRACT));
			if(bridge){
				for(auto a:{STRING_DECLARING_SIDE,STRING_DEFENDERS}){
					s=getString(a);
					vs.push_back(s);
				}
			}
			else{
				VStringID vi={isMisere() ? STRING_MISERE_PLAYER : STRING_DECLARER};
				for (i = 0; i < 3; i++) {
					vi.push_back(isMisere() ? STRING_CATCHER : STRING_WHISTER);
				}

				i=0;
				for(auto a:vi){
					s=getString(a);
					if(i>0){
						s+=std::to_string(i);
					}
					vs.push_back(s);
					i++;
				}
			}
		}
		else{//j==1
			vs={""};
			if(bridge){
				b=isDeclarerNorthOrSouth();
				for(auto a:{b,!b}){
					vs.push_back(getNSEWString(a));
				}

			}
			else{
				//changeable
				for(i=0;i<4;i++){//always 4
					vs.push_back("");
				}
			}
		}

		i=0;
		for(auto a:vs){
			w=createBoldLabel(a);
			gtk_widget_set_hexpand(w, 1);//to stretch grid
			gtk_grid_attach(GTK_GRID(m_grid), w, i + (i != 0), j,
					i == 0 ? 2 : 1, 1);
			i++;
		}
	}



	if(!bridge){
		setPreferans2ndTitleRow();
	}

	if(isMisere()){//game type checked inside
		addGridRow(label(STRING_MISERE),nullptr,j);
	}
	else{
		for (i = minContract(); i <= maxContract(); i++) {
			w = label(std::to_string(i));
			w1 = trump == NT ?
					label(getNTString()) :
					gtk_image_new_from_pixbuf(m_suitPixbuf[trump]);
			addGridRow(w, w1, j++);
		}
	}
	gtk_container_add(GTK_CONTAINER(w2), m_grid);

	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
	gtk_container_add(GTK_CONTAINER(w), m_button[TAB2]);
	gtk_container_add(GTK_CONTAINER(w), m_loading[TAB2]);
	gtk_container_add(GTK_CONTAINER(w), m_labelPercentTab2);
	gtk_container_add(GTK_CONTAINER(w2), w);


	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	for(i=1;i<3;i++){
		auto a=m_combo[i];
		if(isMisere() && i==PREFERANS_WHIST_OPTION_COMBO){
			continue;
		}
		w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
		if(bridge || (!bridge && i==PREFERANS_PLAYERS_COMBO) ){
			if(bridge){
				id=i==BRIDGE_DOUBLE_REDOUBLE_COMBO? STRING_CONTRACT:STRING_VULNERABLE;
			}
			else{
				id=STRING_PLAYERS;
			}
			gtk_container_add(GTK_CONTAINER(w1), label(id));
		}
		gtk_container_add(GTK_CONTAINER(w1), a);
		gtk_widget_set_halign(w1, GTK_ALIGN_CENTER);
		gtk_box_pack_start(GTK_BOX(w), w1, TRUE, TRUE, 0);

	}
	w1= gtk_frame_new(getString(MENU_OPTIONS));
	gtk_container_add(GTK_CONTAINER(w1), w);
	gtk_frame_set_label_align(GTK_FRAME(w1), 0.03, 0.5);
	gtk_container_add(GTK_CONTAINER(w2), w1);

	w1=createMarkupLabel(STRING_CONTRACTS_SCORING_HELP,60);
	gtk_container_add(GTK_CONTAINER(w2), w1);

	return w2;
}

void SolveAllDealsDialog::updateTab2() {
	int contract, tricks;
	VDouble rv;
	double probability[MAX_RESULT_SIZE];
	int i, t;
	std::string s;
	double ev;
	const int MIN_WHIST_TRICKS[] = { 4, 2, 1, 1, 0 };

	g_mutex_lock(&m_mutex);
	for (i = 0; i < MAX_RESULT_SIZE; i++) {
		probability[i] = double(m_result[i]) / m_total;//multithread ok
	}
	g_mutex_unlock(&m_mutex);

	if(isBridge()){
		const int trump=getTrump();
		const int doubleRedouble=getComboPosition(m_combo[BRIDGE_DOUBLE_REDOUBLE_COMBO]);
		const bool vulnerable=getComboPosition(m_combo[BRIDGE_VULNERABLE_COMBO]);
		for (contract = 1; contract <= 7; contract++) {
			ev=0;
			for (tricks = 0; tricks <= 13; tricks++) {
				ev += probability[tricks]
						* countBridgeScore(contract, trump, tricks,
								doubleRedouble, vulnerable);
			}
			rv={ev,-ev};
			setGridLabels(contract, rv);
		}
	}
	else{
		const int players = getPreferansPlayers();
		const int option = getComboPosition(m_combo[PREFERANS_WHIST_OPTION_COMBO]);
		PreferansScore p;
		VDouble v[11][11][WHIST_OPTION_SIZE];
		WHIST_OPTION wo;
		int whisterstricks;

		for (tricks = 0; tricks <= 10; tricks++) {
			for (auto c :{0,6,7,8,9,10}) {
				for(i=0;i<WHIST_OPTION_SIZE;i++){
					wo=WHIST_OPTION(i);
					if(wo==WHIST_OPTION_WHIST || c==tricks){
						v[tricks][c][i]=p.getGameScore(players, c, tricks,wo);
					}
				}
			}
		}

		if(isMisere()){
			contract=0;
			rv.clear();
			for(i=0;i<players;i++){
				ev=0;
				for (tricks = 0; tricks <= 10; tricks++) {
					wo = WHIST_OPTION_WHIST;
					t = tricks;
					assert(i < int(v[t][contract][wo].size()));
					ev += probability[tricks] * v[t][contract][wo][i];
				}
				rv.push_back(ev);
			}
			setGridLabels(contract, rv);
		}
		else{
			if (option==ALWAYS_HALFWHIST || option==ALWAYS_PASS) {
				wo = option == ALWAYS_HALFWHIST ?
						WHIST_OPTION_HALFWHIST : WHIST_OPTION_ALLPASS;
				for (contract = 6; contract <= 10; contract++) {
					setGridLabels(contract, v[contract][contract][wo]);
				}
			}
			else{
				for (contract = 6; contract <= 10; contract++) {
					rv.clear();
					for(i=0;i<players;i++){
						ev=0;
						for (tricks = 0; tricks <= 10; tricks++) {
							wo = WHIST_OPTION_WHIST;
							t = tricks;
							if (option != ALWAYS_WHIST) {
								whisterstricks = 10 - tricks;
								if (whisterstricks
										< MIN_WHIST_TRICKS[contract - 6]) {
									wo =
											option
													== WHIST_PROFITABLE_DEALS_ELSE_HALFWHIST ?
													WHIST_OPTION_HALFWHIST :
													WHIST_OPTION_ALLPASS;
									if (tricks > contract) {
										t = contract;
									}
								}
							}
							assert(i < int(v[t][contract][wo].size()));
							ev += probability[tricks] * v[t][contract][wo][i];
						}
						rv.push_back(ev);
					}
					setGridLabels(contract, rv);
				}
			}
		}
	}
	s=m_total == m_positions ?"":getPercentString();
	gtk_label_set_text(GTK_LABEL(m_labelPercentTab2), s.c_str());

}

void SolveAllDealsDialog::addGridRow(GtkWidget *w,GtkWidget *w1, int row) {
	gtk_grid_attach(GTK_GRID(m_grid), w, 0, row, w1?1:2, 1);
	if(w1){
		gtk_grid_attach(GTK_GRID(m_grid), w1, 1, row, 1, 1);
		gtk_widget_set_halign(w, GTK_ALIGN_END);
		gtk_widget_set_halign(w1, GTK_ALIGN_START);
		const int margin=2;
		gtk_widget_set_margin_end(w, margin);
		gtk_widget_set_margin_start(w1, margin);
	}

	for (int i = 0; i < 4; i++) {
		gtk_grid_attach(GTK_GRID(m_grid), label(), i+2, row, 1, 1);
	}
}

void SolveAllDealsDialog::setGridLabels(int contract,const VDouble& v) {
	std::string s;
	int i=2;
	const bool empty=m_total==0;
	int r=isMisere()? 0:-minContract();

	for (auto a:v) {
		s = empty ? "?":normalize(format("%.2lf", a));
		setGridLabel(s, i++, TITLE_ROWS+contract+r);
	}
}

int SolveAllDealsDialog::getPreferansPlayers() {
	return getComboPosition(m_combo[PREFERANS_PLAYERS_COMBO]) + 3;
}

void SolveAllDealsDialog::updateNumberOfPreferansPlayers() {
	int players = getPreferansPlayers();
	for(auto a:getLastWhisterWidgets()){
		showHideWidget(a, players == 4);
	}
	setPreferans2ndTitleRow();
}

VGtkWidgetPtr SolveAllDealsDialog::getLastWhisterWidgets() {
	VGtkWidgetPtr v;
	for (int i = 0; i < getTableRowsTab2(); i++) {
		v.push_back( gtk_grid_get_child_at(GTK_GRID(m_grid), 5, i));
	}
	return v;
}

void SolveAllDealsDialog::updateResult(int *result, int size) {
	int i;

	g_mutex_lock(&m_mutex);
	for (i = 0; i < size; i++) {
		if(i>getMaxHandCards()){
			assert(result[i]==0);
#ifdef NDEBUG
			break;
#endif
		}
		else{
			m_result[i] += result[i];//multithread ok
		}
	}
	g_mutex_unlock(&m_mutex);

}

std::string SolveAllDealsDialog::getPercentString() {
	return format(isBridge() ? "%.3lf%%" : "%.0lf%%", m_fraction * 100);
}

void SolveAllDealsDialog::setPreferans2ndTitleRow(){
	Problem const &p = getProblem();
	auto pl = p.m_player;
	VCardIndex vc;
	CARD_INDEX c;
	bool b = getNextBridgePlayer(pl) == p.m_absent;

	int i, k = getPreferansPlayers();
	for (i = 0; i < k; i++) {
		if (k == 3) {
			c = getPlayer(pl, true, i);
		} else {
			/* first whister 2nd table column should be always active
			 * in case of four players if absent player is next after player (b=true)
			 * he/she couldn't be active whister so invert whisters name order
			 */
			c = getBridgePlayer(pl, !b, i);
		}
		vc.push_back(c);
	}

	i = 2;
	for (auto a : vc) {
		setGridLabel(getLowercasedPlayerString(a), i, 1);
		i++;
	}
}

void SolveAllDealsDialog::setGridLabel(const std::string &s, int left,
		int top) {
	auto w = gtk_grid_get_child_at(GTK_GRID(m_grid), left, top);
	if(top<TITLE_ROWS){
		auto q="<b>"+s+"</b>";
		gtk_label_set_markup(GTK_LABEL(w), q.c_str());
	}
	else{
		gtk_label_set_text(GTK_LABEL(w), s.c_str());
	}
}

int SolveAllDealsDialog::getTableRowsTab2() {
	return TITLE_ROWS + (isMisere() ? 1 : maxContract()-minContract()+1);
}

std::string SolveAllDealsDialog::getNSEWString(bool ns) {
	auto i=ns ? CARD_INDEX_NORTH:CARD_INDEX_EAST;
	return getLowercasedPlayerString(i)+" / "+getLowercasedPlayerString(getBridgePartner(i));
}

void SolveAllDealsDialog::close() {
	csvSeparator()=gtk_entry_get_text(GTK_ENTRY(m_entry));
	gdraw->stopSolveAllDealsThreads();
}

void SolveAllDealsDialog::setPlayersCards() {
	int i,j;
	bool b;
	GtkWidget *w, *q,*w1,*q1;
	std::string s;
	Problem const&p = getProblem();
	m_map.clear();
	for (i = 0; i < 4; i++) {
		if(isPreferans() && PLAYER[i] == p.m_absent){
			continue;
		}
		if(isBridge()){
			b = i % 2 == isBridgeSolveAllDealsAbsentNS();
		}
		else{
			b=PLAYER[i] == p.m_player;
		}
		for (j = 0; j < 4; j++) {
			w=m_playerBox[i][j];
			clearContainer(w);
			auto v=p.getRowVectorIndex(j, PLAYER[i]);
			if (b || v.empty() ) {
				gtk_container_add(GTK_CONTAINER(w),label(b ? p.getRow(j, PLAYER[i]) : " ?"));
			}
			else{
				for(auto& a:v){
					s=getCardRankString(a%13);
					w1=label(s);
					//allow click on label
				    q1 = gtk_event_box_new ();
				    gtk_container_add (GTK_CONTAINER (q1), w1);
					g_signal_connect(q1, "button-press-event", G_CALLBACK(label_click), w1);

					q=gtk_check_button_new();
					gtk_container_add(GTK_CONTAINER(w),q);
					g_signal_connect(q, "toggled", G_CALLBACK(check_changed), w1);

					m_map[w1]={q,a,PLAYER[i]};

					gtk_container_add(GTK_CONTAINER(w),q1);
				}
			}
			gtk_widget_show_all(w);
		}
	}

}

void SolveAllDealsDialog::checkChanged(GtkWidget* check,GtkWidget *w) {
	std::string s;
	int i,j;
	VInt p[2][2];

	s=gtk_label_get_text(GTK_LABEL(w));

	//for stroke text also return '9' not "<s>9</s>"
	//printl(s);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check))) {
		s="<s>"+s+"</s>";
	}
	gtk_label_set_markup(GTK_LABEL(w), s.c_str());

	auto& pl=gdraw->m_vSolveAll[0].p;

	for (auto const& [key, a] : m_map){
		gboolean c=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(a.check));
		for(j=0;j<2 && pl[j]!=a.player;j++);
		if(j==2){
			continue;
		}
		p[j][c].push_back(a.card);
	}

	//printl(LEADER[indexOfPlayer(sa.p[0])] , LEADER[indexOfPlayer(sa.p[1])] )

	for(i=0;i<2;i++){
		s="";
		for(auto&a:p[i][1]){
			s+=" "+getCardString(a);
		}
		if(!s.empty()){
			printl(LEADER[indexOfPlayer(pl[i])], s);
		}
	}
	printi

}

void SolveAllDealsDialog::labelClick(GtkWidget *w) {
	GtkWidget* check=m_map[w].check;
	gboolean c;
	c=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),!c);
	checkChanged(check,w);
}
