/*
 * DeckArrowSelectionDialog.cpp

 *
 *       Created on: 16.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "../DrawingArea.h"
#include "../ProblemSelector.h"
#include "DeckArrowSelectionDialog.h"

#ifndef FINAL_RELEASE
//	#define SVG_LOAD_SLEEP
#endif

#ifdef SVG_LOAD_SLEEP
#include <unistd.h>
const int sleepSeconds=3;
#endif

const CSize N_CARDS(4,3);
const gdouble ADJUSTMENT_STEP_INCREMENT = 1;
const gdouble ADJUSTMENT_PAGE_SIZE=0;

DeckArrowSelectionDialog*d;

static gboolean on_object_loaded(gpointer data) {
	d->onObjectLoaded(GP2INT(data));
	return G_SOURCE_REMOVE;
}

static gpointer load_thread(gpointer data) {
	d->loadThread(GP2INT(data));
	return NULL;
}

static gboolean keypress_function(GtkWidget *widget, GdkEventKey *event,
		gpointer data) {
	int i=0;
	if (event->keyval == GDK_KEY_KP_Add || event->keyval == '=') {	//GDK_KEY_KP_Add - plus on extended keyboard
		i=1;
	}
	else if(event->keyval == GDK_KEY_KP_Subtract || event->keyval == '-') {	//GDK_KEY_KP_Subtract - minus on extended keyboard
		i=-1;
	}
	if(i){
		d->keyPress(i);
	}
	return FALSE;
}

static gchar* scale_format(GtkScale *scale, gdouble value) {
	return d->scaleFormat(value);
}

void value_changed(GtkAdjustment *adjustment, gpointer user_data) {
	d->valueChanged();
}

static void draw_callback(GtkWidget *, cairo_t *cr, gpointer) {
	d->draw(cr);
}

static void toggle_radio(GtkWidget *w, gpointer) {
	d->toggle(w);
}

DeckArrowSelectionDialog::DeckArrowSelectionDialog(bool isDeck) :
		ButtonsDialog(isDeck?MENU_SELECT_DECK:MENU_SELECT_ARROW) {
	int i,j,n,w,h;
	std::string s;
	GtkWidget *radio, *vbox[2],*border[2],*box;
	GtkRadioButton *rb = NULL;

	d=this;
	m_isDeck=isDeck;
	m_lastDrawnSize={0,0};

	if(isDeck){
		CSize sz=countMaxCardSizeForY(getArrowSize());
		m_maxCardWidth=sz.cx;
		m_maxCardHeight=sz.cy;
		m_deckNumber = gconfig->m_deckNumber;
	}
	else{
		m_arrowNumber=gconfig->m_arrowNumber;
		m_maxArrowSize=countMaxArrowSizeForY(getCardHeight());
	}

	//begin init svg
	getProblemSelector().createSvgPixbufs(m_isDeck);

	//Note svg files are loading quite long, so use threads for loading
	m_number=0;
	m_maxv=getVectorSize();
	j=getNumber()-getRasterSize();
	for(i=0;i<getThreadsNumber();i++){
		m_vThread.push_back( g_thread_new("", load_thread, GP(j)));
	}

	gdouble lower=m_isDeck ? MIN_CARD_WIDTH:MIN_ARROW_SIZE;
	gdouble upper=getValueUpperBound();

	m_adjustment = gtk_adjustment_new(m_isDeck? getCardWidth():getArrowSize(), lower, upper, ADJUSTMENT_STEP_INCREMENT,
			0, ADJUSTMENT_PAGE_SIZE);
	g_signal_connect(m_adjustment, "value-changed", G_CALLBACK (value_changed),
			0);

	m_scale=gtk_scale_new (GTK_ORIENTATION_VERTICAL,m_adjustment);
	//gtk_scale_set_digits( GTK_SCALE(m_scale),0);
	g_signal_connect(m_scale, "format-value", G_CALLBACK (scale_format), 0);
	//gtk_scale_set_draw_value (GTK_SCALE(m_scale),0);
	//end init svg

	gtk_box_set_spacing(GTK_BOX(getContentArea()), 2);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	m_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

	for(i=0;i<2;i++){
		vbox[i] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
		border[i] = gtk_frame_new(getString(i==0?STRING_RASTER:STRING_VECTOR));
		gtk_container_add(GTK_CONTAINER(border[i]), vbox[i]);
		gtk_frame_set_label_align(GTK_FRAME(border[i]), 0.2, 0.5);
		gtk_container_add(GTK_CONTAINER(box), border[i]);
	}

	//Some decks, arrows couldn't be shown so use separate counters
	j=0;
	n=0;
	for (i = 0; i < (m_isDeck?N_TOTAL_DECKS:N_TOTAL_ARROWS); i++) {

		//println("%d %d %d",i, m_maxCardHeight,gconfig->getRasterArrowSize(i))

		if(i==getRasterSize()){
			j=0;
		}

		if (!isScalable(i)
				&& ((m_isDeck && m_maxCardHeight < RASTER_DECK_CARD_SIZE[i].cy)
						|| (!m_isDeck && m_maxArrowSize < RASTER_ARROW_SIZE[i]))) {
			continue;
		}

		s = getString(m_isDeck?STRING_DECK:STRING_ARROW)+format(" #%d ", j + 1);
		if (!isScalable(i)) {
			if(m_isDeck){
				w=RASTER_DECK_CARD_SIZE[i].cx;
				h=RASTER_DECK_CARD_SIZE[i].cy;
			}
			else{
				getPixbufWH(getArrowFileName(i,false),w,h);
			}
			s+=format("%dx%d", w, h);
		}
		const gchar * p = s.c_str();
		if (n == 0) {
			radio = m_radio[i] = gtk_radio_button_new_with_label(NULL, p);
			rb = GTK_RADIO_BUTTON(radio);
		}
		else {
			radio = m_radio[i] = gtk_radio_button_new_with_label_from_widget(rb, p);
		}
		if (i == (m_isDeck?gconfig->m_deckNumber:gconfig->m_arrowNumber)) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
		}
		g_signal_connect(radio, "toggled", G_CALLBACK (toggle_radio),
				gpointer(0));
		gtk_box_pack_start(GTK_BOX(vbox[ isScalable(i) ]), radio, TRUE, TRUE, 0);
		n++;
		j++;
	}

	gtk_container_add(GTK_CONTAINER(m_hbox), box);

	//drawing area
	m_area = gtk_drawing_area_new();
	if(m_isDeck){
		i=m_maxCardWidth * N_CARDS.cx;
		j=m_maxCardHeight * N_CARDS.cy;
	}
	else{
		i=j=m_maxArrowSize;
	}
	g_signal_connect(m_area, "draw", G_CALLBACK (draw_callback), gpointer(0));

	m_loading=animatedImage("run.gif");
	gtk_widget_set_size_request(m_area, i, j);
	gtk_widget_set_size_request(m_loading, i, j);

	gtk_box_pack_start(GTK_BOX(m_hbox), m_loading, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(m_hbox), m_area, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(m_hbox), m_scale);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(box), m_hbox);

	m_label = createMarkupLabel(
			m_isDeck ? STRING_DECK_SELECTION_HELP : STRING_ARROW_SELECTION_HELP,
			40);
	gtk_container_add(GTK_CONTAINER(box), m_label);

	gtk_container_add(GTK_CONTAINER(getContentArea()), box);

	gtk_widget_add_events(getContentArea(), GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT (getContentArea()), "key_press_event",
			G_CALLBACK (keypress_function), NULL);

	if (m_isDeck) {
		m_check = gtk_check_button_new_with_label(
				getString(STRING_RESIZE_MAIN_WINDOW));
		if (gconfig->m_resizeOnDeckChanged) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_check), TRUE);
		}
	}

	enableButtons();

	setHideShowWidgets();
	show(m_isDeck ? m_check : 0, false, m_hideV);
}

DeckArrowSelectionDialog::~DeckArrowSelectionDialog() {
	for (auto a:m_vThread) {
		g_thread_join(a);
	}
}

bool DeckArrowSelectionDialog::click(int index) {
	if (index == 0) {
		if(m_isDeck){
			gconfig->setDeckParameters(m_deckNumber,gtk_toggle_button_get_active(
					GTK_TOGGLE_BUTTON(m_check)), getObjectSize());
			getProblemSelector().m_deckChanged=true;
		}
		else{
			gconfig->setArrowParameters(m_arrowNumber, getObjectSize().cx);
			getProblemSelector().m_arrowChanged=true;
		}
	}
	return true;
}

CSize DeckArrowSelectionDialog::getObjectSize(){
	gdouble v=getValue();
	int i=v;
	int j=m_isDeck ? getSvgParameters().getScaledHeight(v) : i;
	return {i,j};
}

void DeckArrowSelectionDialog::draw(cairo_t* cr) {
	int i,j,k;
	std::string s;

	/*Since for some languages actual size of drawing_area can increase from requested value
	 we need center deck image vertically and horizontally
	 */
	GtkAllocation a;
	gtk_widget_get_allocation(m_area, &a);
	int w,h;
	bool sc=isScalable();
	GdkPixbuf* pb;
	if(sc){
		pb=getSvgPixbuf(m_isDeck);

		CSize cs=getObjectSize();
		w=cs.cx;
		h=cs.cy;

		if(cs!=m_lastDrawnSize){
			m_lastDrawnSize=cs;
			getProblemSelector().drawSvg(cs,getNumberIndex(),m_isDeck,getValue());
		}
	}
	else{
		pb = pixbuf(getObjectFileName(getNumber(),false));
		w = gdk_pixbuf_get_width(pb);
		h = gdk_pixbuf_get_height(pb);
		if(m_isDeck){
			w/=13;
			h/=4;
		}
	}


	if(m_isDeck){
		w*=N_CARDS.cx;
		h*=N_CARDS.cy;
	}

	i = (a.width -  w) / 2;
	j = (a.height -  h) / 2;
	k=m_isDeck?w- (13*w/N_CARDS.cx):0;

	gdk_cairo_set_source_pixbuf(cr, pb, i+k , j);
	cairo_rectangle(cr, i, j, w, h);
	cairo_fill(cr);

	if(!sc){
		g_object_unref(pb);
	}

}

void DeckArrowSelectionDialog::toggle(GtkWidget* w) {
	/* Note click on radio send two signals
	 * for old radio with active=0
	 * and new with active=1
	 */
	if (!(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))) {
		return;
	}

	getNumber()=INDEX_OF(w,m_radio);
	if(isScalable()){
		/* if new maximum value is lower than new maximum correct it
		 * If just call gtk_adjustment_set_upper then value is not changed
		 */
		int i=getValueUpperBound();

		if(i<gtk_adjustment_get_value(m_adjustment)){
			gtk_adjustment_set_value(m_adjustment,i);
		}
		gtk_adjustment_set_upper(m_adjustment,i);

	}
	onObjectChanged();
}

int DeckArrowSelectionDialog::countMaxSvgWidthForDeck(){
	if(isScalable()){
		auto& p=getSvgParameters();
		return int(m_maxCardHeight*p.cw/p.ch);
	}
	else{
		/* need to use some value if current deck is not scalable
		 * when user select scalable deck value will change
		 */
		return m_maxCardWidth;
	}
}

void DeckArrowSelectionDialog::onObjectLoaded(int i){
	if(i==getNumberIndex()){
		onObjectChanged();
	}
}

void DeckArrowSelectionDialog::loadThread(const int n){
	/* Note run user selected deck first thread not needed
	 * because this deck already loaded in ProblemSelector
	 */
	int i;
	const int MAXV=m_maxv;
	std::string s;

	while ( (i = m_number++)<MAXV) {
#ifdef SVG_LOAD_SLEEP
		usleep(sleepSeconds*1000*1000);
#endif
		//load current deck first swap i=0 & i=j if j>0
		if( (i==0 || i==n) && n>0){
			i = i == 0 ? n : 0;
		}

		s=getObjectFileName(i,true);
		getSvgParameters(i).loadPixbuf(s);
		/* Note cann't do check i==m_deck-N_DECKS (when need to redraw deck)
		 * here because user can change m_deck from main thread, so just use
		 * gdk_threads_add_idle with parameter and do check i==m_deck-N_DECKS
		 * in main thread
		 */
		gdk_threads_add_idle(on_object_loaded, GP(i));
	}

}

void DeckArrowSelectionDialog::valueChanged(){
	gtk_widget_queue_draw(m_area);
}

void DeckArrowSelectionDialog::keyPress(int i){
	if (isScalable()) {
		gtk_adjustment_set_value(m_adjustment, getValue() + i);
	}
}

void DeckArrowSelectionDialog::onObjectChanged(){
	setHideShowWidgets();
	for(auto a:m_showV){
		gtk_widget_show(a);
	}
	for(auto a:m_hideV){
		gtk_widget_hide(a);
	}
	//set m_lastDrawnSize.cx=0 to indicate redraw
	m_lastDrawnSize.cx=0;
	gtk_widget_queue_draw(m_area);
	enableButtons();
}

SvgParameters& DeckArrowSelectionDialog::getSvgParameters() {
	return getSvgParameters(getNumberIndex());
}

SvgParameters& DeckArrowSelectionDialog::getSvgParameters(int i){
	return getProblemSelector().getSvgParameters(i,m_isDeck);
}

int DeckArrowSelectionDialog::getThreadsNumber(){
	return std::min(getVectorSize(),getMaxRunThreads());
}

int DeckArrowSelectionDialog::getRasterSize()const{
	return m_isDeck ? N_RASTER_DECKS : N_RASTER_ARROWS;
}

int DeckArrowSelectionDialog::getVectorSize(){
	return m_isDeck ? N_VECTOR_DECKS : N_VECTOR_ARROWS;
}

bool DeckArrowSelectionDialog::isScalable(int i)const{
	return i>=getRasterSize();
}

bool DeckArrowSelectionDialog::isScalable(){
	return isScalable(getNumber());
}

int& DeckArrowSelectionDialog::getNumber(){
	return m_isDeck?m_deckNumber:m_arrowNumber;
}

int DeckArrowSelectionDialog::getNumberIndex(){
	return getNumber()-getRasterSize();
}

std::string DeckArrowSelectionDialog::getObjectFileName(int n,bool svg){
	return m_isDeck?getDeckFileName(n,svg):getArrowFileName(n,svg);
}

gchar* DeckArrowSelectionDialog::scaleFormat(gdouble value){
	/* Note even g_strdup_printf request g_free() this sample was taken
	 * from gtk documentation so it seems that scale_format signal
	 * frees the string
	 */
	if (m_isDeck) {
		int i = getSvgParameters().getScaledHeight(value);
		return g_strdup_printf("%dx%d", int(value), i);
	}
	else{
		return g_strdup_printf("%d", int(value));
	}
}

gdouble DeckArrowSelectionDialog::getValue(){
	return gtk_adjustment_get_value (m_adjustment);
}

int DeckArrowSelectionDialog::getValueUpperBound(){
	return m_isDeck?countMaxSvgWidthForDeck():m_maxArrowSize;
}

void DeckArrowSelectionDialog::setHideShowWidgets(){
	m_showV.clear();
	if(isScalable() ){
		m_showV.push_back(m_label);
		if(getSvgParameters().isLoaded()){
			m_showV.push_back(m_area);
			m_showV.push_back(m_scale);
		}
		else{
			m_showV.push_back(m_loading);
		}
	}
	else{
		m_showV.push_back(m_area);
	}

	GtkWidget*a[]={
			m_area,m_scale,m_loading,m_label
	};

	m_hideV.clear();
	for(auto b:a){
		if(!oneOf(b,m_showV)){
			m_hideV.push_back(b);
		}
	}
	//println("%llu %llu",m_showV.size(),m_hideV.size())
}

void DeckArrowSelectionDialog::enableButtons() {
	//cann't click ok while loading deck or arrow
	bool e = !isScalable() || getSvgParameters().isLoaded();
	gtk_widget_set_sensitive(m_button[0], e);
}
