/*
 * LastTrick.cpp
 *
 *       Created on: 09.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "LastTrick.h"
#include "Frame.h"

LastTrick*glasttrick;

static gboolean draw_grid_background(GtkWidget *widget, cairo_t *cr, gpointer) {
	glasttrick->drawGridBackground(cr);
	return FALSE;
}

static gboolean cell_click(GtkWidget *widget, GdkEventButton *event,gpointer data) {
	glasttrick->cellClick(GP2INT(data));
	return TRUE;
}

static gboolean mouse_enter_event(GtkWidget *widget, GdkEventButton *event,gpointer data) {
	glasttrick->gridMouseEnter(GP2INT(data));
	return TRUE;
}

static gboolean mouse_leave_event(GtkWidget *widget, GdkEventCrossing *event,
		gpointer) {
	glasttrick->hideToolTip();
	return TRUE;
}

LastTrick::LastTrick() :
		FrameItemArea() {
	int i;
	glasttrick=this;
	m_rows=m_columns=0;

	for (auto&a :m_suitPixbuf) {
		a = nullptr;
	}
	setSuitPixbufs();

	for(i=0;i<52;i++){
		m_labelCard[i] = gtk_label_new("");
		m_suit[i]= gtk_image_new();
	}

	//m_scrolled = gtk_scrolled_window_new(NULL, NULL);

	m_grid = gtk_grid_new();
	g_signal_connect(G_OBJECT (m_grid), "draw", G_CALLBACK (draw_grid_background), 0);
	//gtk_container_add (GTK_CONTAINER (m_scrolled), m_grid);

	/* couldn't call setGrid() here, because need MENU_LAST_TRICK_GAME_ANALYSIS
	 * which is not download yet. Call setGrid() in updateAfterCreation function
	 */
	m_full = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	gtk_container_add(GTK_CONTAINER(m_full), m_grid);
	gtk_container_add(GTK_CONTAINER(m_full), getWidget());

	//prevents destroy after gtk_container_remove on change show option
	g_object_ref (m_full);
}

LastTrick::~LastTrick() {
	freeSuitPixbufs();
	g_object_unref (m_full);
}

CSize LastTrick::getSize() const {
	return gconfig->m_showLastTrick ? getVisibleSize():CSize(0, 0) ;
}

CSize LastTrick::getVisibleSize() const {
	CSize a = getCardSize();
	return {std::max(2*a.cx,MIN_LAST_TRICK_WIDTH), std::max(2*a.cy,getBestLineSize().cy*getMaxHandCards())};
}

CSize LastTrick::getFullVisibleSize() const{
	CSize a=getVisibleSize();
	a.cx+=getBestLineSize().cx;
	return a;
}

void LastTrick::draw() {
	int i, j, m;
	CRect r, ir;
	int moves[4];

	CSize sz = getSize();
	if (sz.cy == 0) {
		return;
	}
	CARD_INDEX fm = getProblem().getLastTrick(moves);
	j = getBestLineSize().cx + getArea().getSize().cx;

	i = getProblemSelector().getSize().cy;
	copyFromBackground(0, 0, sz.cx, sz.cy, j, i);

	if (fm == CARD_INDEX_INVALID) {
		i=0;
		for (auto s:m_vLastTrick) {
			TextWithAttributes text(s);
			CSize ts = getTextExtents(text);
			drawText(text, (sz.cx - ts.cx) / 2, sz.cy / 2 - (i == 0 ? ts.cy : 0));
			i++;
		}
	}
	else {
		r = CRect(CPoint(0, 0), sz);
		m = gconfig->m_lastTrickMinimalMargin;

		j = 0; //area.getInsideRect(r,CARD_INDEX_WEST).topLeft().x;//find max margin

		for (i = 0; i < maxTableCards(); i++, fm = getNextPlayer(fm)) {
			ir = getInsideRect(r, fm);

			//possible last trick adjusting
			if (ir.left < m) {
				ir.left = m;
			}
			else if (ir.right > r.right - m) {
				ir.left = r.right - m - ir.width();
			}

			//last trick drawHorizontalLine(0,0,sz.cx); so need m+1
			if (ir.top < m + 1) {
				ir.top = m + 1;
			}
			else if (ir.bottom > r.bottom - m) {
				ir.top = r.bottom - m - ir.height();
			}
			copyFromDeck(m_cs.cairo(), ir.left, ir.top, moves[i]);
		}
	}
	drawBestLine();//can be called to update whole bestline
}

void LastTrick::newGame() {
	//DrawingArea size is changed so LastTrick size is changed -> need setup background image, resize, and redraw anyway
	//need call init() because of new size of background
	initResizeRedraw();
}

void LastTrick::updateLanguage() {
	setLastTrickGameAnalysisStrings();
	if (isEmpty()) {
		redraw();
	}
	drawBestLine();
}

bool LastTrick::isEmpty() {
	return getProblem().getLastTrick(NULL) == CARD_INDEX_INVALID;
}

void LastTrick::setLastTrickGameAnalysisStrings(){
	VString v = split(getString(MENU_LAST_TRICK_GAME_ANLYSIS), " ");
	m_vLastTrick.clear();
	m_vGameAnalysis.clear();

	auto p=&m_vLastTrick;
	for(auto s:v){
		if(s=="/"){
			p=&m_vGameAnalysis;
			continue;
		}
		p->push_back(s);
	}
}

void LastTrick::updateAfterCreation() {
	//this function is call when user click show/hide last trick
	init();

	setLastTrickGameAnalysisStrings();

	drawBestLine();
	if (getSize().cy == 0) {
		gtk_widget_hide(m_full);
	}
	else {
		gtk_widget_show_all(m_full);
		resize();
		redraw();
	}
}

void LastTrick::updateEdit() {
}

void LastTrick::setDeal(bool random) {
	drawBestLine();
	redraw();
}

void LastTrick::drawBestLine(){
	int i, j, rows, cols;
	GtkWidget*w, *w1;
	const int d = maxTableCards();
	VInt& v = getState().m_bestLine;

	if (v.empty()) {
		rows = cols = 1;
	}
	else {
		i = v.size();
		rows = i / d;
		if(i%d!=0){
			rows++;
		}
		cols = d * 2;
	}

	bool b;
	/* b=1 program hangs if refill grid every time and do many calls of
	 * drawBestLine() by clicking right mouse button on problem may be it's
	 * gtk bug. Refill grid as rarely as possible allows to avoid hangs
	 */
	if (m_columns == cols) {
		if (rows == m_rows) {
			b = 0;
		}
		else if (rows == m_rows - 1) {	//just remove last row
			b = 0;
			gtk_grid_remove_row(GTK_GRID(m_grid), m_rows - 1);
		}
		else {
			b = 1;
		}
	}
	else {
		b = 1;
	}
	if (b) {
		for (i = 0; i < m_columns; i++) {
			gtk_grid_remove_column(GTK_GRID(m_grid), 0);
		}
	}

	if (v.empty()) {
		i = 0;
		std::string s;
		for (auto a : m_vGameAnalysis) {
			if (i) {
				s += "\n";
			}
			s += a;
			i++;
		}
		if (b) {
			m_labelCard[0] = gtk_label_new("");
		}
		w = m_labelCard[0];
		addClass(w,"fontcolor");
		gtk_label_set_text(GTK_LABEL(w), s.c_str());
		g_object_set(w, "expand", TRUE, NULL);
		gtk_label_set_xalign(GTK_LABEL(w), 0.5);//center full label inside grid
		gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_CENTER);//center every line of label
		if (b) {
			gtk_grid_attach(GTK_GRID(m_grid), w, 0, 0, 1, 1);
			gtk_widget_show_all(w);
		}

	}
	else{
		int l = 0;
		int m = v.size();
		int n=0;
		if (m % d != 0) {
			l = d - m % d;
			m += l;
		}
		bool empty;

		for (i = 0; i < m; i++) {
			empty = i < l;
			if (!empty) {
				n = v[i - l];
			}
			for (j = 0; j < 2; j++) {
				if (b) {
					if (j == 0) {
						w = m_labelCard[i] = gtk_label_new("");
					}
					else {
						w = m_suit[i] = gtk_image_new();
					}
					/* make gtk image clickable
					 * https://stackoverflow.com/questions/24386412/how-to-detect-mouse-click-over-an-image-in-gtk
					 *
					 * connect signal always, because empty cell, after several turns can became not empty
					 */
					w1 = gtk_event_box_new();
					gtk_container_add(GTK_CONTAINER(w1), w);
					g_signal_connect(G_OBJECT (w1), "button_press_event",
							G_CALLBACK (cell_click), GP(i));

					gtk_widget_add_events(w1,
							GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
					g_signal_connect(G_OBJECT (w1), "leave-notify-event",
							G_CALLBACK(mouse_leave_event), GP(i));
					g_signal_connect(G_OBJECT (w1), "enter-notify-event",
							G_CALLBACK(mouse_enter_event), GP(i));
				}
				else {
					w = w1 = j == 0 ? m_labelCard[i] : m_suit[i];
				}

				if(j==0){
					gtk_label_set_text(GTK_LABEL(w),
							empty ? "" : getCardRankString(n % 13).c_str());
					gtk_label_set_xalign(GTK_LABEL(w), 1);	//align card rank right
				}
				else{
					if (empty) {
						gtk_image_clear(GTK_IMAGE(w));
					}
					else {
						gtk_image_set_from_pixbuf(GTK_IMAGE(w), m_suitPixbuf[n / 13]);
					}
				}

				g_object_set(w, "hexpand", TRUE, NULL);
				if (b) {
					gtk_grid_attach(GTK_GRID(m_grid), w1, 2 * (i % d) + j, i/d, 1, 1);
					gtk_widget_show_all(w1);
				}

			}
		}
	}

	m_rows=rows;
	m_columns=cols;
}

void LastTrick::drawGridBackground(cairo_t *cr){
	auto& p=getProblemSelector();//DO NOT change "auto&" to "auto"
	int sourcex = getArea().getSize().cx;
	int sourcey = p.getSize().cy;
	int h = std::max(getBestLineSize().cy*13,getVisibleSize().cy);

	/* more than GRID_SIZE width / height because of scrolling
	 * visible 13 tricks need m_bestLineHeight*13
	 */
	copy(getBackgroundFullSurface(),cr,0,0, getBestLineSize().cx, h,sourcex,sourcey);
}

void LastTrick::cellClick(int n){
	if(think()){
		return;
	}
	const int d = maxTableCards();
	auto& v=getState().m_bestLine;
	int i, j = v.size() % d;

	if(j==0){
		i=0;
	}
	else{
		i=d-j;
	}
	for (j=0; j <n-i+1; j++) {
		gdraw->makeMove(v[j]);
	}
	hideToolTip();

	/*	update undo, redo, think (if very last item in list, buttons should became disabled)
	 * toolbar & menu
	 * */
	updateThinkAll();
}

void LastTrick::gridMouseEnter(int n) {
	if(think()){
		return;
	}

	const int d = maxTableCards();
	auto& v = getState().m_bestLine;
	int j = v.size() % d;

	//avoid tooltip for empty grid cells
	if (j != 0 && d - j > n) {
		return;
	}

	showToolTip( (j==0? n==0 : d-j==n) ? STRING_CLICK_TO_MAKE_MOVE : STRING_CLICK_TO_MAKE_MOVES);

}

void LastTrick::updateDeckSelection(){
	initResizeRedraw();
}

void LastTrick::updateFontSelection(){
	setSuitPixbufs();
	initResizeRedraw();
}

void LastTrick::setSuitPixbufs() {
	int i=0;
	for (auto&a :m_suitPixbuf) {
		free(a);
		a = getSuitPixbuf(i, getFontHeight());
		i++;
	}
}

void LastTrick::freeSuitPixbufs() {
	for (auto&a :m_suitPixbuf) {
		free(a);
	}
}

void LastTrick::updateSkin() {
	gtk_widget_queue_draw(m_grid);
	redraw();
}
