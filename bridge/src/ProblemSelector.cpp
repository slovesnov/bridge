/*
 * ProblemSelector.cpp
 *
 *       Created on: 27.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "ProblemSelector.h"
#include "Frame.h"
#include "dialogs/EditDescriptionDialog.h"


//USE_PIXBUF_TO_LOAD_SVG_DECK_AT_START
const char STORE_DECK_TO_PNG_FILE_NAME[]="tmpdeck.png";
const char STORE_ARROW_TO_PNG_FILE_NAME[]="tmparrow.png";

static void button_click(GtkToolItem* w, ProblemSelector* p) {
	p->clickToolbar(w);
}

static gboolean mouse_enter_event(GtkWidget *widget, GdkEventButton *event,
		ProblemSelector*d) {
	d->setCursorVisible(true);
	return TRUE;
}

static gboolean mouse_leave_event(GtkWidget *widget, GdkEventCrossing *event,
		ProblemSelector*d) {
	d->setCursorVisible(false);
	return TRUE;
}

static gboolean mouse_press_event(GtkWidget *widget, GdkEventButton *event,
		ProblemSelector*d) {
	return d->mouseClick(event->time);
}

static gboolean key_release_event(GtkWidget *widget, GdkEventButton *event,
		ProblemSelector*d) {
	d->proceedCommentChanges();
	return FALSE; //need further proceeding
}

ProblemSelector::ProblemSelector() :
		FrameItemArea(gtk_scrolled_window_new(NULL, NULL)) {
	unsigned i;

	m_current = 0;
	m_lastClickTime = 0;
	m_visible = false;
	m_deckSurface = 0;
	m_deckCairo=0;
	m_svgArrowPixbuf=0;
	m_svgDeckPixbuf=0;
	m_svgScaledPixbuf=0;
	m_deckChanged=false;
	m_arrowChanged=false;

	fillSvgParameters();
	//Frame::Frame() add m_label to menu bar, so here just create
	m_label = gtk_label_new("");

	m_commentView = gtk_text_view_new();
	gtk_text_view_set_justification(GTK_TEXT_VIEW(m_commentView),
			GTK_JUSTIFY_FILL);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_commentView), GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_commentView), FALSE);

	m_backgroundFullCairo = NULL;
	m_backgroundFullSurface = NULL;
	createNew(m_backgroundFullCairo, m_backgroundFullSurface, getMaxSize());

	/* need to create background image here when Frame calls
	 * updateAfterCreation() first item is ProblemSelector (because it manages
	 * m_vproblem data), all classes use background image to make possible
	 * draw background of ProblemSelector class need to fill
	 * background here
	 *
	 * also call setDeck, setArrows for the same reason
	 */
	setSkin();
	setDeck();

	for (i = 0; i < SIZE(m_arrow); i++) {
		m_arrow[i] = NULL;
	}
	setArrows();

	createNewGame(gconfig->m_gameType, gconfig->m_absent);
	setOriginalModified();

	gtk_container_add(GTK_CONTAINER(getWidget()), m_commentView);

	setDragDrop(m_commentView);

	//just create m_toolbar added in Frame
	m_toolbar = gtk_toolbar_new();

	gtk_toolbar_set_style(GTK_TOOLBAR(m_toolbar), GTK_TOOLBAR_ICONS);
	gtk_container_set_border_width(GTK_CONTAINER(m_toolbar), 0);

	for (i = TOOLBAR_BUTTON_UNDOALL; i < TOOLBAR_BUTTON_SIZE; i++) {
		//icons will be set later on 'enable' functions
		m_button[i] = gtk_tool_button_new(NULL, "");
		gtk_tool_item_set_expand(m_button[i], TRUE); //otherwise buttons goes to left
		gtk_toolbar_insert(GTK_TOOLBAR(m_toolbar), m_button[i], -1);
		g_signal_connect(m_button[i], "clicked", G_CALLBACK(button_click), this);
	}

	gtk_widget_set_can_focus(GTK_WIDGET(m_toolbar), FALSE);
	gtk_widget_set_size_request(GTK_WIDGET(m_toolbar), getLastTrick().getFullVisibleSize().cx,
			-1);

	gtk_widget_add_events(m_commentView,
			GDK_KEY_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK
					| GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect(m_commentView, "enter-notify-event",
			G_CALLBACK(mouse_enter_event), this);
	g_signal_connect(m_commentView, "leave-notify-event",
			G_CALLBACK(mouse_leave_event), this);
	g_signal_connect(m_commentView, "button_press_event",
			G_CALLBACK(mouse_press_event), this);
	g_signal_connect(m_commentView, "key_release_event",
			G_CALLBACK(key_release_event), this);
}

ProblemSelector::~ProblemSelector() {
	int i;
	if(m_arrowChanged && isScalableArrow()){
		gdk_pixbuf_save(m_arrow[0], getWritableFilePath(STORE_ARROW_TO_PNG_FILE_NAME).c_str(), "png", NULL, NULL);
	}
	if(m_deckChanged && isScalableDeck()){
		CSize c=getCardSize();
		c.cx*=13;
		c.cy*=4;

		/* Note m_svgDeckPixbuf can have invalid data, because user can select deck make some
		 * manipulations and press cancel
		 */
		GdkPixbuf *p = gdk_pixbuf_get_from_surface(m_deckSurface, 0, 0, c.cx, c.cy);
		gdk_pixbuf_save(p, getWritableFilePath(STORE_DECK_TO_PNG_FILE_NAME).c_str(), "png", NULL, NULL);
		free(p);
	}

	for (i = 0; i < SIZEI(m_arrow); i++) {
		free(m_arrow[i]);
	}

	destroy(m_backgroundFullSurface);
	destroy(m_backgroundFullCairo);
	destroy(m_deckSurface);
	destroy(m_deckCairo);
	free(m_svgArrowPixbuf);
	free(m_svgDeckPixbuf);
	free(m_svgScaledPixbuf);
}

void ProblemSelector::setAreaProblem() {
	Problem const& p = getProblem();
	setComment(p.m_comment);

	if (getToolbar().getCurrentGameType() != p.m_gameType) {
		getToolbar().setGameType();
		getMenu().updateGameType();
	}

	getArea().updateProblem();
	updateFindBestStateAll();
	updateLabel();
	updateToolbar();
	updateModified();

/*
#ifndef FINAL_RELEASE
	static std::string sap;
	//used in BridgeTest project to get deal
	sap+=p.getForBridgeTestDealClass(m_current);
	printl(sap);
#endif
*/

}

void ProblemSelector::draw() {
	CSize sz = getSize();
	copyFromBackground(0, 0, sz.cx, sz.cy, getArea().getSize().cx, 0);
}

CSize ProblemSelector::getSize() const {
	int h = getArea().getSize().cy;

#ifdef TOOLTIP_IN_STATUSBAR
	if(gconfig->m_showToolTips){
		h+=getBestLineHeight();
	}
#endif

	auto& a=getLastTrick();
	if (gconfig->m_showLastTrick) {
		h -= a.getSize().cy;
	}
	return CSize(a.getFullVisibleSize().cx, h);
}

void ProblemSelector::updateLanguage() {
	updateLabel();
	if (m_comment.length() == 0) {
		setComment(m_comment);
	}
}

bool ProblemSelector::setCheckLabelFit(int option, int width, std::string& s) {
	gint a;
	assert(option >= 0 && option < 4);
	s = m_vproblem[m_current].getShortFileName();
	if (option & 1) {
		s += " ";
		s += getString(MENU_PROBLEM);
	}
	if (option & 2) {
		s += format(" %d / %d", m_current + 1, size());
	}
	else {
		s += format(" %d/%d", m_current + 1, size());
	}

	gtk_label_set_text(GTK_LABEL(m_label), s.c_str());
	gtk_widget_get_preferred_width(m_label, &a, NULL);

	return a <= width;
}

void ProblemSelector::updateLabel() {
	int i; //not unsigned because for(i=3;i>=0;i--){
	std::string s;
	gint a, width;

	if (m_vproblem.size() <= 1) {
		gtk_label_set_text(GTK_LABEL(m_label), ""); //clear previous label
		return;
	}

	gtk_widget_get_size_request(GTK_WIDGET(m_toolbar), &width, NULL);

	for (i = 3; i >= 0; i--) {
		/* i=3 "problem" + spaces
		 * i=2 spaces only
		 * i=0 most tight string
		 */
		if (i == 1) {
			continue;
		}
		if (setCheckLabelFit(i, width, s)) {
			return;
		}
	}

	const char*begin = "..";

	for (i = 1; i < int(s.length()); i++) {
		s = s.substr(1);
		gtk_label_set_text(GTK_LABEL(m_label), (begin + s).c_str());
		gtk_widget_get_preferred_width(m_label, &a, NULL);
		if (a <= width) {
			break;
		}
	}

}

void ProblemSelector::updateSkin() {
	setSkin();
	redraw();
}

void ProblemSelector::updateDeckSelection() {
	setDeck();
	initResizeRedraw();
}

void ProblemSelector::updateFontSelection() {
	//setBestLineHeight();
	//TODO
	TextWithAttributes text("1");
	setBestLineHeight (getTextExtents(text).cy);

	initResizeRedraw();
}

void ProblemSelector::createNewGame(GAME_TYPE t, CARD_INDEX a) {
	m_current = 0;
	m_vproblem.push_back(Problem(t, a, 1));
	m_filepath = "";
}

void ProblemSelector::fullUpdate() {
	setComment("");
	updateLabel();
	//DrawingArea size is changed so Description size is changed -> need setup background image, resize, and redraw anyway
	//need call init() because of new size of background
	initResizeRedraw();

	updateToolbar();
}

void ProblemSelector::updateToolbarButtons(){
	for (int i = TOOLBAR_BUTTON_UNDOALL; i < TOOLBAR_BUTTON_SIZE; i++) {
		bool e = !think() && isMovePossible(i - TOOLBAR_BUTTON_UNDOALL);
		gtk_widget_set_sensitive(GTK_WIDGET(m_button[i]), e);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(m_button[i]),
				getToolbarImage(TOOLBAR_BUTTON(i), false, boolToButtonState(e)));
	}
	gtk_widget_show_all(m_toolbar);	//DONT REMOVE
}

void ProblemSelector::updateToolbar() {
	//update back/forward
	int i;
	if (size() <= 1) {
		for (i = TOOLBAR_BUTTON_UNDOALL; i < TOOLBAR_BUTTON_SIZE; i++) {
			gtk_widget_hide(GTK_WIDGET(m_button[i]));
		}
	}
	else {
		updateToolbarButtons();
	}
}

void ProblemSelector::clickToolbar(GtkToolItem* w) {
	int i = INDEX_OF(w,m_button);
	assert(i != -1);
	assert(i != 0);

	move(i - 1);
	setAreaProblem();
}

void ProblemSelector::setCursorVisible(bool visible) {
	//prevents two calls of getMenu().addAccelerators() with same visibility which produces errors if try to remove accelerators more than one time
	if (m_visible == visible) {
		return;
	}

	m_visible = visible;
	getMenu().addAccelerators(!visible);//enable/disable hotkeys proceeding for Frame
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_commentView), visible);

	if (m_comment.length() == 0) {
		updateCommentTextView();
	}
	if (visible) {
		//set focus on m_comment. if m_comment has no focus then cursor isn't visible even if gtk_text_view_set_cursor_visible(*,true) was called
		gtk_widget_grab_focus(m_commentView);
	}

	gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(m_commentView));//make cursor is visible
}

void ProblemSelector::setComment(std::string s, bool updateTextView) {
	m_comment = s;
	if (updateTextView) {
		updateCommentTextView();
	}
	getProblem().m_comment = s;
	updateModified();
}

void ProblemSelector::proceedCommentChanges() {
	//if user click hotkey from menu not need proceedCommentChanges()
	bool cursorVisible = gtk_text_view_get_cursor_visible(
			GTK_TEXT_VIEW(m_commentView));
	if (cursorVisible) {
		/* call updateCommentTextView() move cursor at the end of textview
		 * that is not good for cursor keys, backspace etc.
		 */
		setComment(getCommentTextView(), false);
	}
}

std::string ProblemSelector::getCommentTextView() const {
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(
			GTK_TEXT_VIEW(m_commentView));
	GtkTextIter start;
	GtkTextIter end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	return gtk_text_buffer_get_text(buffer, &start, &end, true);
}

void ProblemSelector::updateCommentTextView() {
	bool cursorVisible = gtk_text_view_get_cursor_visible(
			GTK_TEXT_VIEW(m_commentView));
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(
			GTK_TEXT_VIEW(m_commentView));
	gtk_text_buffer_set_text(buffer,
			m_comment.empty() && !cursorVisible ?
					getString(STRING_PROBLEM_DESCRIPTION) : m_comment.c_str(), -1);
}

void ProblemSelector::changeShowOption() {//don't know why if not redefine function it not called from FrameArea class
	FrameItemArea::init();
	if (getSize().cy == 0) {
		hide();
	}
	else {
		Widget::show();
		resize();
		redraw();
	}
}

bool ProblemSelector::mouseClick(guint32 time) {
	//double click when two clicks with time difference lower than 250 milliseconds
	//taken from gdkevents.h file where defined _GdkEventButton
	if (time - m_lastClickTime < 250) {
		EditDescriptionDialog dialog;
		return TRUE;
	}
	m_lastClickTime = time;
	return FALSE;

	/*call from mouse_press_event
	 * return FALSE; - need further proceeding if user want to select text by mouse
	 * return TRUE; - no need further proceeding. Selection by mouse is not possible
	 *
	 * if do some changed here check how ok/cancel buttons is working for EditDescriptionDialog
	 * if return FALSE after creation of EditDescriptionDialog, then ok/cancel buttons close dialog after second click
	 */
}

void ProblemSelector::openUris(char** uris) {
	set(getValidFilesList(uris), false);
}

void ProblemSelector::openFiles(const char*files) {
	if (strlen(files) == 0) {
		//new game
		getMenu().updateEdit();
		getLastTrick().updateEdit();
	}
	else {
		VString v = split(files, G_SEARCHPATH_SEPARATOR_S);
		set(v, false);
	}
}

void ProblemSelector::set(VString const& v, bool add) {
	if (v.size() == 0) {
		return;
	}

	if (!add) {
		if (!gframe->saveIfModified()) {
			return;
		}
	}

	if (!m_vproblem.set(v, add)) {
		//no new problems recognized, prevents enableEdit(false);
		return;
	}

	if (!add) {
		m_current = 0;
		if (m_vproblem.size() == 0) {	//added file(s) recognized only errors
			return;
		}
	}

	//if many files then no name
	setFilepathUpdateRecent(add || v.size() > 1 ? "" : v[0]);

	setOriginalModified();
	setAreaProblem();
	redraw();
}

void ProblemSelector::save(std::string filepath, bool split) {
	assert(m_vproblem.size() > 0);

	FILE_TYPE t = getFileType(filepath);

	if (onlyPreferansProblems() && (t == FILE_TYPE_DF || t == FILE_TYPE_PBN)) {
		message(MESSAGE_ICON_ERROR,
				STRING_ERROR_IMPOSSIBLE_TO_SAVE_PROBLEMS_LIST_TO_PBN_DF_FILE_BECAUSE_IT_HAS_ONLY_PREFERANS_PROBLEMS);
		return;
	}

	if (m_vproblem.size() == m_vproblemOriginal.size()) {
		m_vproblemOriginal[m_current] = m_vproblem[m_current];	//save changes
		m_modified[m_current] = false;
	}
	bool m = m_vproblem.save(filepath, split);
	if (!split) {
		if (m) {
			//if some of preferans problems were omitted
			m_current = 0;
			m_vproblem.removeAllPreferansProblems();
			setOriginalModified();
			updateLabel();
			setAreaProblem();
			redraw();
		}
		else {
			/* if user change some problem then move to another problem
			 * so have to change whole m_vproblemOriginal, m_modified vectors
			 */
			setOriginalModified();
		}
		setFilepathUpdateRecent(filepath);
	}
}

bool ProblemSelector::isModified() const {
	VBoolCI it;
	int i;

	assert(m_current >= 0 && m_current < m_vproblem.size());

	if (m_vproblem.size() != m_vproblemOriginal.size()) {
		return true;
	}

	for (i = 0, it = m_modified.begin(); it != m_modified.end(); it++, i++) {
		if (i == m_current) {
			if (m_vproblemOriginal[i] != m_vproblem[i]) {
				return true;
			}
		}
		else if (*it) {
			return true;
		}
	}
	return false;
}

Problem& ProblemSelector::getProblem() {
	assert(m_current >= 0 && m_current < m_vproblem.size());
	return m_vproblem[m_current];
}

const Problem& ProblemSelector::getProblem() const {
	assert(m_current >= 0 && m_current < m_vproblem.size());
	return m_vproblem[m_current];
}

void ProblemSelector::setOriginalModified() {
	VBoolI it;

	m_vproblemOriginal = m_vproblem;
	m_modified.resize(size());
	for (it = m_modified.begin(); it != m_modified.end(); it++) {
		*it = false;
	}
}

void ProblemSelector::setFilepathUpdateRecent(std::string filepath) {
	m_filepath = filepath;
	if (filepath.length() > 0) {
		gframe->updateRecent(filepath);
	}
}

std::string ProblemSelector::getTitle() const {
	std::string s =
			isEmptyFilepath() ?
					"(" + std::string(getString(STRING_UNTITLED)) + ")" :
					getFileInfo(m_filepath, FILEINFO::NAME);
	s += format(" %c ", isModified() ? '*' : '-') + gconfig->getTitle();
	return s;
}

void ProblemSelector::setFrom(ProblemVectorModified const& pvm) {
	m_vproblem.m_problems = pvm.m_vproblem.m_problems;
	m_current = pvm.m_current;
	setModifiedIfSameSize();

	setAreaProblem();

	/* setAreaProblem() calls getMenu().updateGameType() only if gametype is changed
	 * but needs to call getMenu().updateGameType() even if gametype is not changed because need to
	 * set enable/disable save pbn, df anyway
	 */
	getMenu().updateGameType();
}

void ProblemSelector::setSkin() {
	GtkStyleContext *context;
	GtkWidgetPath *path;

	path = gtk_widget_path_new();
	gtk_widget_path_append_type(path, GTK_TYPE_DIALOG);
	context = gtk_style_context_new();
	gtk_style_context_set_path(context, path);
	CSize a=getMaxSize();
	gtk_render_background(context, m_backgroundFullCairo, 0, 0, a.cx,
			a.cy);
}

SvgParameters& ProblemSelector::getSvgParameters(int n,bool isDeck){
	return isDeck?m_svgDeckParameters[n]:m_svgArrowParameters[n];
}

void ProblemSelector::drawSvg(CSize const& size,int n,bool isDeck,gdouble value){
	int i,j,k,l;
	int w=size.cx;
	int h=size.cy;
	int width = 13*w;
	int height = 4*h;
	double sc;

	auto&p= getSvgParameters(n,isDeck);
	if(isDeck){
		sc = p.getScale(value);
		gdk_pixbuf_scale(p.p, m_svgScaledPixbuf, 0, 0, width+12*sc*p.addx, height+3*sc*p.addy, -p.startx * sc, -p.starty * sc, sc, sc,
				GDK_INTERP_BILINEAR);

		for (i = 0; i < 4; i++) {
			k = p.suitsOrder[i] * (h+p.addy*sc);
			for (j = 0; j < 13; j++) {
				l=p.cardsOrder[j] * (w+p.addx*sc);
				gdk_pixbuf_copy_area(m_svgScaledPixbuf, l, k, w, h, m_svgDeckPixbuf, j* w, i * h);
			}
		}
	}
	else{
		sc=double(size.cx)/gdk_pixbuf_get_width(p.p);
		gdk_pixbuf_scale(p.p, m_svgArrowPixbuf, 0, 0, w, h, 0, 0, sc, sc,
				GDK_INTERP_BILINEAR);
	}

}

double ProblemSelector::getSvgMaxWHRatio()const{
	double v=0,v1;
	for(auto& p:m_svgDeckParameters){
		v1=p.cw/p.ch;
		if(v1>v){
			v=v1;
		}
	}
	return v;
}

void ProblemSelector::createSvgPixbufs(bool isDeck){
	int i,j;

	if(isDeck){
		int x=0;
		int y=0;
		for(auto& p:m_svgDeckParameters){
			x=std::max(x,p.addx);
			y=std::max(y,p.addy);
		}

		CSize sz=countMaxCardSizeForY(MIN_ARROW_SIZE);
		i=sz.cx*13;
		j=sz.cy*4;

		createNew(m_svgDeckPixbuf,  i, j);
		createNew(m_svgScaledPixbuf, i+12*x, j+3*y);
	}
	else{
		i=countMaxArrowSizeForY(MIN_CARD_HEIGHT);
		createNew(m_svgArrowPixbuf, i, i);
	}
}

void ProblemSelector::setDeck() {
	if(isScalableDeck()){
		bool start=m_svgDeckPixbuf==0;
		CSize c=getCardSize();
		c.cx*=13;
		c.cy*=4;
		createNew(m_deckCairo,m_deckSurface, c);

		if(start){
			m_svgDeckPixbuf=writablePixbuf(STORE_DECK_TO_PNG_FILE_NAME);
		}
		copyFromPixbuf(m_svgDeckPixbuf, m_deckCairo,CRect(CPoint(0,0),c));
	}
	else{
		createNew(m_deckSurface, surface(getDeckFileName()));
		getCardWidth() = cairo_image_surface_get_width(m_deckSurface) / 13;
		getCardHeight() = cairo_image_surface_get_height(m_deckSurface) / 4;
	}
}

void ProblemSelector::init() {
	getArea().countSize();//size of ProblemSelector based on getArea().getSize() so need to recount inner variables
	FrameItemArea::init();
}

void ProblemSelector::setArrows() {
	bool start=m_arrow[0]==0;
	int i=getArrowNumber();
	if(isScalableArrow(i)){
		if(start){
			createNew(m_arrow[0], writablePixbuf(STORE_ARROW_TO_PNG_FILE_NAME ));
		}
		else{
			i=getArrowSize();
			createNew(m_arrow[0], i,i);

			gdk_pixbuf_copy_area(m_svgArrowPixbuf, 0, 0, i, i, m_arrow[0], 0, 0);
		}
	}
	else{
		createNew(m_arrow[0], pixbuf(getArrowFileName(i,false) ));
		getArrowSize() = gdk_pixbuf_get_width(m_arrow[0]);
	}
	for (i = 1; i < SIZEI(m_arrow); i++) {
		createNew(m_arrow[i],
				gdk_pixbuf_rotate_simple(m_arrow[i - 1], GDK_PIXBUF_ROTATE_CLOCKWISE));
	}
}

void ProblemSelector::currentProblemChanged(bool gameType) {
	setComment(getProblem().m_comment);
	if (m_vproblem.size() == m_vproblemOriginal.size()) {
		countModified();
	}

	if (gameType) {
		//DrawingArea size is changed so Description size is changed -> need setup background image, resize, and redraw anyway
		//need call init() because of new size of background
		initResizeRedraw();
	}
	updateUndoRedoAll();
	updateFindBestStateAll();
}

void ProblemSelector::newGame() {
	auto& p = getProblem();
	m_vproblem.clear();
	createNewGame(p.m_gameType, p.m_absent);
	setOriginalModified();
	fullUpdate();
}

void ProblemSelector::resize(){
	gtk_widget_set_size_request(GTK_WIDGET(m_toolbar), getLastTrick().getFullVisibleSize().cx, -1);
	FrameItemArea::resize();
}

void ProblemSelector::fillSvgParameters(){
	int i,j,k;
	j=0;
	for(auto& p:m_svgDeckParameters){
		for(i=0;i<4;i++){
			if(j==1){
				k=3-i;
			}
			else if(j==3){
				int a[]={3,1,0,2};
				k=a[i];
			}
			else if(j==2){
				k=i==0?3:i-1;
			}
			else{
				k=i;
			}
			p.suitsOrder[i]=k;
		}

		if(j==1){
			p.cw=2178/13.;
			p.ch=1216/5.;
		}
		else if(j==2){
			//x 30, 420, endcard 390
			//y 30, 600, 570
			p.addx=p.addy=p.startx=p.starty=30;
			p.cw = 390-p.startx;
			p.ch = 570-p.starty;
			int k=0;
			p.addx-=k;
			p.cw+=k;
		}
		else if(j==3){
			//0,54 2897,1299
			p.cw = 2897./13;
			p.starty=54;
			p.ch = (1299-p.starty)/4;
		}
		else{
			//x 30, 420, endcard 390
			//y 30, 600, 570
			p.addx=p.addy=p.startx=p.starty=30;
			p.cw = 390-p.startx;
			p.ch = 570-p.starty;
		}

		j++;
	}

}

void ProblemSelector::updateThink(){
	if (size() > 1) {
		updateToolbarButtons();
	}
}
