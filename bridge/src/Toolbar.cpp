/*
 * Toolbar.cpp
 *
 *       Created on: 12.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "Toolbar.h"
#include "Frame.h"

const char TOOLBAR_IMAGE_STOP[] = "stop.png";
const char* TOOLBAR_IMAGE[] = { "play.png", "undoall.png", "undo.png" };

static void combo_changed(GtkWidget *comboBox, Toolbar*p) {
	p->comboboxChanged(comboBox);
}

static void button_click(GtkToolButton *toolbutton, Toolbar*p) {
	p->click((GtkToolItem*) toolbutton);
}

static gboolean draw_tooltip_background(GtkWidget *widget, cairo_t *cr, gpointer) {
	gframe->getToolbar().drawTooltipBackground(cr);
	return FALSE;
}

Toolbar::Toolbar() :
		FrameItem(gtk_toolbar_new()) {
	int i, j;

	//[play, undoall, undo, redo, redoall][small][enable]
	for (i = 0; i < SIZEI(m_pixbuf); i++) {
		if (i < SIZEI(TOOLBAR_IMAGE)) {
			m_pixbuf[i][0][1] = pixbuf(TOOLBAR_IMAGE[i]);
		}
		else {
			m_pixbuf[i][0][1] = gdk_pixbuf_flip(m_pixbuf[SIZEI(m_pixbuf) - i][0][1],
					true);
		}
		m_pixbuf[i][1][1] = gdk_pixbuf_scale_simple(m_pixbuf[i][0][1], 16, 16,
				GDK_INTERP_BILINEAR);

		for (j = 0; j < 2; j++) {
			m_pixbuf[i][j][0] = gdk_pixbuf_copy(m_pixbuf[i][j][1]);
			//desaturate image
			gdk_pixbuf_saturate_and_pixelate(m_pixbuf[i][j][1], m_pixbuf[i][j][0],
					.3f, false);
		}
	}

	m_stopPixbuf[0] = pixbuf(TOOLBAR_IMAGE_STOP);
	m_stopPixbuf[1] = gdk_pixbuf_scale_simple(m_stopPixbuf[0], 16, 16,
			GDK_INTERP_BILINEAR);


	m_trumpChanged = true;
	m_needModify = true;
	m_lock = false;

	gtk_toolbar_set_style(GTK_TOOLBAR(getWidget()), GTK_TOOLBAR_ICONS);
	gtk_container_set_border_width(GTK_CONTAINER(getWidget()), 0);

	m_contract = createImageCombobox();
	//when use select valid position and misere, so have to switch
	g_signal_connect(m_contract, "changed", G_CALLBACK(combo_changed), this);

	m_contractToolItem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(m_contractToolItem), m_contract);
	gtk_toolbar_insert(GTK_TOOLBAR(getWidget()), m_contractToolItem, -1);

	m_trump = createImageCombobox();
	g_signal_connect(m_trump, "changed", G_CALLBACK(combo_changed), this);

	m_trumpToolItem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(m_trumpToolItem), m_trump);
	gtk_toolbar_insert(GTK_TOOLBAR(getWidget()), m_trumpToolItem, -1);

	for (i = 0; i < TOOLBAR_BUTTON_SIZE; i++) {

		//icons will be set later on 'enable' functions
		m_button[i] = gtk_tool_button_new(NULL, "");

		gtk_toolbar_insert(GTK_TOOLBAR(getWidget()), m_button[i], -1);
		g_signal_connect(m_button[i], "clicked", G_CALLBACK(button_click), this);

	}

	m_tooltip = gtk_label_new("");

	gtk_label_set_xalign (GTK_LABEL(m_tooltip),0);
	g_signal_connect(G_OBJECT (m_tooltip), "draw", G_CALLBACK (draw_tooltip_background), 0);

	//prevents destroy after gtk_container_remove on change show option
	g_object_ref (m_tooltip);

	gtk_widget_set_can_focus(GTK_WIDGET(getWidget()), FALSE);
}

Toolbar::~Toolbar() {
	int i, j, k;
	for (i = 0; i < SIZEI(m_pixbuf); i++) {
		for (j = 0; j < SIZEI(*m_pixbuf); j++) {
			for (k = 0; k < SIZEI(**m_pixbuf); k++) {
				g_object_unref(m_pixbuf[i][j][k]);
			}
		}
	}
	for (i = 0; i < SIZEI(m_stopPixbuf); i++) {
		g_object_unref(m_stopPixbuf[i]);
	}
	g_object_unref (m_tooltip);
}

void Toolbar::updateGameType() {
	gint i = getComboTrumpPosition();
	if (i == -1) { //just created
		i = 0;
	}

	setGameType();

	if (isBridge() && i == MISERE) { //was misere & preferans now new game type
		i = NT;
	}
	setTrump(i); //restore old combobox position

	setContract(minContract());
}

void Toolbar::updateLanguage() {
	gint i = getComboTrumpPosition();
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_trump), createTrumpModel(false));
	setComboTrumpPosition(i); //restore old combobox position
}

void Toolbar::setMisere() {
	setTrump(MISERE);
}

void Toolbar::updateButton(TOOLBAR_BUTTON id) {
	gtk_widget_set_sensitive(GTK_WIDGET(m_button[id]), isToolbarButtonEnabled(id));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(m_button[id]),
			getToolbarImage(id, false,getToolbarButtonState(id)));
	showAll(); //DONT REMOVE
}

void Toolbar::setTrump(gint trump) {
	//prevent from m_frame->updateModified, comboboxChanged() will be called on next message so we couldn't set m_needModify here
	//m_needModify will be set later in comboboxChanged()
	m_needModify = false;
	setComboTrumpPosition(trump == NO_TRUMP_SET ? -1 : trump);
	m_needModify = true;
}

void Toolbar::setContract(gint contract) {
	setComboContractPosition(
			contract == NO_CONTRACT_SET ? -1 : contract - minContract());
}

void Toolbar::click(GtkToolItem* toolbutton) {
	int i;
	for (i = 0; i < TOOLBAR_BUTTON_SIZE && m_button[i] != toolbutton; i++)
		;
	menuClick(TOOLBAR_MENU_ID[i]);//do all calls using menuClick, because of additional functions, for example hide ProblemSelector
}

void Toolbar::comboboxChanged(GtkWidget* combo) {
	int i;

	if (m_lock) {
		return;
	}

	m_lock = true;
	if (combo == m_trump) {
		setTrumpChanged(true);

		if (getComboTrumpPosition() == MISERE) {	//misere or [PassAll game may be in future also same reaction]
			setComboContractPosition(-1);
		}
		else {
			/* 3dec2020, fixed bug. If open problem with no contract for example #1.bts
			 * then on change language, contract should not changed. So comment next 3 lines
			 */
//			if (getComboContractPosition() == -1) {
//				setComboContractPosition(0);
//			}
		}

	}

	else if (combo == m_contract) {
		if (getComboContractPosition() == -1) {

		}
		else if (getComboTrumpPosition() == MISERE) {
			setComboTrumpPosition(0);
		}

	}
	else {
		assert(0);
	}
	m_lock = false;

	if (m_needModify) {	//m_frame->updateModified() only if user change combobox
		i = getComboTrumpPosition();
		if (i == MISERE) {
			getProblem().m_trump = NT;
			getProblem().m_misere = true;
		}
		else {
			getProblem().m_trump = i == -1 ? NO_TRUMP_SET : i;
			getProblem().m_misere = false;
		}

		i = getComboContractPosition();
		getProblem().m_contract = i == -1 ? NO_CONTRACT_SET : minContract() + i;

		updateModified();
	}

}

void Toolbar::updateEdit() {
	changeEnableEdit(isEditEnable());
}

GAME_TYPE Toolbar::getCurrentGameType() const {
	GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(m_trump));
	gint i = gtk_tree_model_iter_n_children(model, NULL);
	i = INDEX_OF(i,TRUMP_MODEL_SIZE);
	assert(i != -1);
	return GAME_TYPE(i);
}

void Toolbar::setGameType() {
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_contract), createContractModel());
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_trump), createTrumpModel(false));
}

void Toolbar::newGame() {
	setComboTrumpPosition(0);
	setComboContractPosition(0);
}

gint Toolbar::getComboTrumpPosition() const {
	return gtk_combo_box_get_active(GTK_COMBO_BOX(m_trump));
}

void Toolbar::setComboTrumpPosition(gint p) const {
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_trump), p);
}

gint Toolbar::getComboContractPosition() const {
	return gtk_combo_box_get_active(GTK_COMBO_BOX(m_contract));
}

void Toolbar::setComboContractPosition(gint p) const {
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_contract), p);
}

void Toolbar::updateUndoRedo() {
	updateButton(TOOLBAR_BUTTON_UNDOALL);
	updateButton(TOOLBAR_BUTTON_UNDO);
	updateButton(TOOLBAR_BUTTON_REDOALL);
	updateButton(TOOLBAR_BUTTON_REDO);
}

void Toolbar::updateFindBestState() {
	updateButton(TOOLBAR_BUTTON_FIND_BEST);
}

void Toolbar::changeEnableEdit(bool enable) {
	gtk_widget_set_sensitive(GTK_WIDGET(m_trump), enable);
	gtk_widget_set_sensitive(GTK_WIDGET(m_contract), enable);
}

void Toolbar::updateThink(){
	updateUndoRedo();
	updateFindBestState();
}

void Toolbar::showToolTip(const gchar *s) {
	gtk_label_set_text(GTK_LABEL(m_tooltip), s);
}

GdkPixbuf * Toolbar::getPixbuf(TOOLBAR_BUTTON id, bool small,
		BUTTON_STATE state) {
//	GdkPixbuf *m_pixbuf[TOOLBAR_BUTTON_SIZE][2][2]; //[...][small][enable]
//	GdkPixbuf *m_stopPixbuf[2]; //[small] enable always true
	if(id==TOOLBAR_BUTTON_FIND_BEST && state==BUTTON_STATE_FIND_BEST_STOP){
		return m_stopPixbuf[small];
	}
	else{
		return m_pixbuf[id][small][state==BUTTON_STATE_ENABLED];
	}

}

void Toolbar::drawTooltipBackground(cairo_t *cr){
	auto sz=getArea().getSize();
	int w = sz.cx;
	int sourcey=sz.cy;
	int h=gtk_widget_get_allocated_height(m_tooltip);
	copy(getBackgroundFullSurface(),cr,0,0, w, h,0,sourcey);
}

void Toolbar::changeShowOption(){
	if(gconfig->m_showToolTips){
		gtk_widget_show(m_tooltip);
	}
	else{
		gtk_widget_hide(m_tooltip);
	}
}

void Toolbar::updateResetSettings() {
}
