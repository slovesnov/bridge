/*
 * Menu.cpp
 *
 *       Created on: 12.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "Menu.h"
#include "Frame.h"
#include "helper/Accelerator.h"

Menu*gmenu;

#define SET_ATTRIBUTES(a) setItemAttributes(a,SIZE(a));

/*before this identifiers need to set separator in menu*/
const MENU_ID SEPARATOR_ID[] = {
		MENU_EDIT,
		MENU_SAVE_HTML,
		MENU_EXIT,
		MENU_SELECT_ARROW,
		MENU_SHOW_HTML_OPTIONS,
		MENU_SHOW_MODIFIED_WARNING,
		MENU_FIND_BEST_MOVE,
		MENU_OTHER_LANGUAGE,
		MENU_CUSTOM_SKIN };

const MENU_ID MENU_ICON_ID[] = {
		MENU_NEW,
		MENU_OPEN,
		MENU_SAVE,
		MENU_SAVE_AS,
		MENU_EDIT,
		MENU_EDIT_PROBLEM_LIST,
		MENU_EDIT_DESCRIPTION,
		MENU_PBN_EDITOR,
		MENU_ROTATE_CLOCKWISE,
		MENU_ROTATE_COUNTERCLOCKWISE, //problem

		MENU_SAVE_HTML,
		MENU_SAVE_HTML_WITH_PICTURES,
		MENU_SAVE_IMAGE,
		MENU_EXIT, //problem

		MENU_CONVERT,
		MENU_SOLVE_FOR_ALL_DECLARERS,
		MENU_CLEAR_DEAL,
		MENU_RANDOM_DEAL,
		MENU_CALCULATOR,
		MENU_SOLVE_ALL_FOE,
		MENU_ROTATE_BY_90_DEGREES_CLOCKWISE,
		MENU_ROTATE_BY_90_DEGREES_COUNTERCLOCKWISE,

		MENU_UNDOALL,
		MENU_UNDO,
		MENU_REDO,
		MENU_REDOALL,
		MENU_FIND_BEST_MOVE, //move

		MENU_OTHER_LANGUAGE, //language

		MENU_HOMEPAGE,
		MENU_ABOUT };

const char* MENU_ICON_FILE[] = {
		"new16.png",
		"folder16.png",
		"save16.png",
		"saveas16.png",
		"edit16.png",
		"editlist16.png",
		"edit-description16.png",
		"pbn16.png",
		"clockwise16.png",
		"counterclockwise16.png", //problem

		"html16.png",
		"html16.png",
		"img16.png",
		"cancel16.png", //problem

		"refresh16.png",
		"settings16.png",
		"new16.png",
		"suitsorder16.png",
		"calculator16.png",
		"settings16.png",
		"clockwise16.png",
		"counterclockwise16.png",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL, //undoall,undo,redo,redoall,best Note. Images will be setup later now need only create image menu item

		"language16.png",
		"home16.png",
		"bridge16.png" };

const MENU_ID RADIO_MENU_ID[] = {
		MENU_ESTIMATE_NONE,
		MENU_ESTIMATE_BEST_LOCAL,
		MENU_ESTIMATE_BEST_TOTAL,
		MENU_ESTIMATE_ALL_LOCAL,
		MENU_ESTIMATE_ALL_TOTAL };

const MENU_ID VARIABLE_TEXT_MENU[] = { MENU_GAME_TYPE, MENU_FIND_BEST_MOVE };

const MENU_ID MENU_ESTIMATE[] = {
		MENU_ESTIMATE_NONE,
		MENU_ESTIMATE_BEST_LOCAL,
		MENU_ESTIMATE_BEST_TOTAL,
		MENU_ESTIMATE_ALL_LOCAL,
		MENU_ESTIMATE_ALL_TOTAL };

const MENU_ID MENU_SKIN[] = {
		MENU_SKIN0,
		MENU_SKIN1,
		MENU_SKIN2,
		MENU_SKIN3,
		MENU_SKIN4,
		MENU_SKIN5,
		MENU_SKIN6,
		MENU_SKIN7 };

const MENU_ID MENU_UNDO_REDO_DEPENDENT[] = { MENU_UNDO, MENU_UNDOALL, MENU_REDO, MENU_REDOALL };

const MENU_ID MENU_BEST_MOVE_DEPENDENT[] = {
//??		MENU_SAVE_HTML,
//??		MENU_SAVE_HTML_WITH_PICTURES,
		MENU_SOLVE_FOR_ALL_DECLARERS,
		MENU_SOLVE_ALL_FOE,
		MENU_FIND_BEST_MOVE
};

//Note updateThink() calls union MENU_UNDO_REDO_DEPENDENT+MENU_BEST_MOVE_DEPENDENT+MENU_THINK_DEPENDENT
const MENU_ID MENU_THINK_DEPENDENT[] = {
		MENU_NEW,
		MENU_OPEN,
//		MENU_SAVE,
//		MENU_SAVE_AS,
		MENU_EDIT,
//		MENU_EDIT_PROBLEM_LIST,
//		MENU_EDIT_DESCRIPTION,
//		MENU_PBN_EDITOR,
		MENU_ROTATE_CLOCKWISE,
		MENU_ROTATE_COUNTERCLOCKWISE,
		MENU_SAVE_HTML,
		MENU_SAVE_HTML_WITH_PICTURES,
		MENU_SAVE_IMAGE,
//set in function		//MENU_RECENT...
//		MENU_EXIT,

		MENU_CONVERT,
		MENU_SOLVE_FOR_ALL_DECLARERS,
		MENU_CLEAR_DEAL,
		MENU_RANDOM_DEAL,
		MENU_GAME_TYPE,
//		MENU_CALCULATOR,
		MENU_SOLVE_ALL_FOE,
		MENU_ROTATE_BY_90_DEGREES_CLOCKWISE,
		MENU_ROTATE_BY_90_DEGREES_COUNTERCLOCKWISE,

		MENU_ESTIMATE_NONE,
		MENU_ESTIMATE_BEST_LOCAL,
		MENU_ESTIMATE_BEST_TOTAL,
		MENU_ESTIMATE_ALL_LOCAL,
		MENU_ESTIMATE_ALL_TOTAL,
//		MENU_SHOW_HTML_OPTIONS,
//		MENU_SAVE_HTML_FILE_WITH_IMAGES,
//		MENU_PREVIEW_HTML_FILE,
//		MENU_SHOW_MODIFIED_WARNING,
//		MENU_AUTOPLAY_SEQUENCE,
		MENU_SPLIT_EVERY_FILE,
		MENU_ONLY_ONE_INSTANCE,

		MENU_UNDOALL,
		MENU_UNDO,
		MENU_REDO,
		MENU_REDOALL,
		MENU_FIND_BEST_MOVE
};

static void menu_activate(GtkWidget*, MENU_ID id) {
	/* need to hide tool tip, because if user click for example
	 * Ctrl+w and tool tip is active need to hide,
	 * don't need menu->click because of mouse moving (tool tip already hided)
	 */
	gmenu->hideToolTip();
	gmenu->click(id);
}

Menu::Menu() :
		FrameItem(gtk_menu_bar_new()) {
	gmenu = this;
	m_lastRecentSize = 0;

//to suppress eclipse warnings
#ifndef NDEBUG
	m_signals = false;
	radioGroup = 0;
#endif
	assert(SIZE(MENU_ICON_ID)==SIZE(MENU_ICON_FILE));

	for (auto& m: gconfig->m_vectorMenuString) {
		assert(m_map.find(m.first) == m_map.end());
		if ( m.first >= MENU_PROBLEM && m.first < MENU_LANGUAGE_FIRST) {
			insertTopMenu(m);
		}
		else {
			insertSubMenu(m);
		}
	}

}

Menu::~Menu() {
}

GtkWidget* Menu::createImageMenuItem(const char* s, GdkPixbuf *p) {
	GtkWidget*w, *w1;
	GtkWidget* item = gtk_menu_item_new();

	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

	w1 = p == NULL ? gtk_image_new() : gtk_image_new_from_pixbuf(p);
	gtk_container_add(GTK_CONTAINER(w), w1);

	w1 = gtk_accel_label_new(s);
	gtk_label_set_use_underline(GTK_LABEL(w1), TRUE);
	gtk_label_set_xalign(GTK_LABEL(w1), 0.0);

	gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(w1), item);

	gtk_box_pack_end(GTK_BOX(w), w1, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(item), w);
	return item;
}

void Menu::insertTopMenu(MenuString menuString) {
	std::string g = "_" + menuString.second;
	m_top.push_back(gtk_menu_item_new_with_mnemonic(g.c_str()));
	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget()), m_top.back());
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(m_top.back()), gtk_menu_new());
	m_map[menuString.first] = m_top.back();
}

void Menu::insertSubMenu(MenuString menuString) {
	int i;
	GtkWidget*w;
	const gchar *g = menuString.second.c_str();
	const int recentIndex =
			menuString.first < MENU_PROBLEM ? menuString.first - MENU_RECENT : -1;
	GtkMenuShell* sub = getSubMenu(recentIndex == -1);
	GdkPixbuf*p;

	if (menuString.first == MENU_EXIT) { //add recent menu items
		m_lastRecentSize = recentSize();
		if (m_lastRecentSize != 0) {
			insertSeparator();
			for (i = 0; i < recentSize(); i++) {
				insertRecentMenu(i);
			}
		}
	}

	if (ONE_OF(menuString.first,SEPARATOR_ID)) {
		insertSeparator();
	}

	//set up icon if needed
	if (menuString.first == MENU_GAME_TYPE || menuString.first == MENU_CUSTOM_SKIN
			|| (menuString.first >= MENU_SKIN0 && menuString.first <= MENU_SKIN7)
			|| (menuString.first >= MENU_LANGUAGE_FIRST && menuString.first < MENU_NEW)) {

		if (menuString.first == MENU_GAME_TYPE) {
			p = pixbuf("backv.gif");
			p = gdk_pixbuf_scale_simple(p, 12, 16, GDK_INTERP_BILINEAR);
		}
		else if (menuString.first == MENU_CUSTOM_SKIN) {
			p = pixbuf("empty16.png");
		}
		else if (menuString.first >= MENU_SKIN0 && menuString.first <= MENU_SKIN7) {
			p = pixbuf(getBgImageName(menuString.first - MENU_SKIN0), 0, 0, 16, 16);
		}
		else {
			p = gconfig->languagePixbuf(menuString.first);
		}
		w = createImageMenuItem(g, p);
		g_object_unref(p);
	}
	else if ((i = INDEX_OF(menuString.first,MENU_ICON_ID)) == -1) {
		if (ONE_OF(menuString.first,CHECKED_MENU)) {
			w = gtk_check_menu_item_new_with_label(g);
		}
		else if (ONE_OF(menuString.first,RADIO_MENU_ID)) {
			if (menuString.first == RADIO_MENU_ID[0]) {
				radioGroup = NULL;
			}
			w = gtk_radio_menu_item_new_with_label(radioGroup, g);
			radioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(w));
		}
		else {
			w = gtk_menu_item_new_with_label(g);
		}
	}
	else {
		p = MENU_ICON_FILE[i] == NULL ? NULL : pixbuf(MENU_ICON_FILE[i]);
		w = createImageMenuItem(g, p);
		if (p) {
			g_object_unref(p);
		}
	}

	for (i = 0; i < SIZEI(ACCELERATOR); i++) {
		if (menuString.first == ACCELERATOR[i].id) {
			break;
		}
	}
	if (i != SIZEI(ACCELERATOR)) {
		m_accelerator[i] = gtk_accel_group_new();

		/* this function is call from Menu::Menu() therefore gframe->getWidget() is invalid
		 * so call gtk_window_add_accel_group(GTK_WINDOW(gframe->getWidget()), m_accelerator[i]);
		 * in Menu.updateAfterCreation()
		 */
		Accelerator a = ACCELERATOR[i];
		gtk_widget_add_accelerator(w, "activate", m_accelerator[i], a.key, a.mask,
				GTK_ACCEL_VISIBLE);
	}

	if (recentIndex == -1) {
		gtk_menu_shell_append(sub, w);
	}
	else {
		gtk_menu_shell_insert(sub, w, m_firstRecentPosition + recentIndex);
	}

	m_map[menuString.first] = w;

	g_signal_connect(w, "activate", G_CALLBACK (menu_activate),
			gpointer(menuString.first));
}

void Menu::updateSkin() {
	//disable active skin, enable all others
	SET_ATTRIBUTES(MENU_SKIN)
}

void Menu::updateLanguage() {
	unsigned i;

	for (auto& m:gconfig->m_vectorMenuString) {
		updateMenu(m);
	}

	//disable active language
	for (i = 0; i < gconfig->m_language.size(); i++) {
		setItemAttributes(MENU_ID(MENU_LANGUAGE_FIRST + i));
	}

	SET_ATTRIBUTES(VARIABLE_TEXT_MENU)
}

void Menu::updateEstimationType() {
	SET_ATTRIBUTES(MENU_ESTIMATE)
}

void Menu::updateUndoRedo() {
	for(auto& v:MENU_UNDO_REDO_DEPENDENT){
		setItemAttributes(v);
	}
}

void Menu::updateFindBestState() {
	for(auto& v:MENU_BEST_MOVE_DEPENDENT){
		setItemAttributes(v);
	}
}

void Menu::updateGameType() {
	setItemAttributes(MENU_GAME_TYPE);
	setItemAttributes(MENU_PBN_EDITOR); //disable if only preferans problems, enable otherwise
}

void Menu::click(const MENU_ID id) {
	if (!m_signals) {
		return;
	}
	menuClick(id);
}

GtkLabel* Menu::getLabel(const MENU_ID id) const {
	GList*list;
	GtkWidget*w;

	auto it = m_map.find(id);

	assert(it != m_map.end());

	w = gtk_bin_get_child(GTK_BIN(it->second));
	if (!GTK_IS_LABEL(w)) {
		list = gtk_container_get_children(GTK_CONTAINER(w));
		assert(g_list_length(list) == 2);
		w = GTK_WIDGET(g_list_nth(list, 1)->data);
		assert(GTK_IS_LABEL(w));
		g_list_free(list);
	}
	return GTK_LABEL(w);
}

void Menu::setItemAttributes(const MENU_ID id[], unsigned size) {
	unsigned i;
	for (i = 0; i < size; i++) {
		setItemAttributes(id[i]);
	}
}

void Menu::setItemAttributes(const MENU_ID id) {
	int i;
	bool b, radio;
	GtkWidget*w = m_map[id];
	STRING_ID sid = STRING_INVALID; //prevents warning
	GList *list;

	//set check
	if ((i = INDEX_OF(id,CHECKED_MENU)) != -1) {
		blockSignals();
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),
				gconfig->isChecked(i));
		unblockSignals();
	}

	//set caption for variable text items
	i = INDEX_OF(id,VARIABLE_TEXT_MENU);
	if (i != -1) {

		if (id == MENU_GAME_TYPE) {
			sid = isBridge() ? STRING_PREFERANS : STRING_BRIDGE;
		}
		else if (id == MENU_FIND_BEST_MOVE) {
			sid =
					getFindBestState() == BUTTON_STATE_FIND_BEST_STOP ?
							STRING_STOP_SEARHING_BEST_MOVE : STRING_FIND_BEST_MOVE;
		}

		updateMenu(id, ::getString(sid));
	}

	//set radio
	b = false;
	radio = INDEX_OF(id,RADIO_MENU_ID) != -1;
	if (radio) {
		if (id == MENU_ESTIMATE_NONE || id == MENU_ESTIMATE_BEST_LOCAL
				|| id == MENU_ESTIMATE_BEST_TOTAL || id == MENU_ESTIMATE_ALL_LOCAL
				|| id == MENU_ESTIMATE_ALL_TOTAL) {
			b = id == getEstimateType() + MENU_ESTIMATE_NONE;
		}
		else { //should not happens
			assert(0);
			b = false;
		}
		blockSignals();
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), b);
		unblockSignals();
	}

	//set enable
	if (radio) {
		b = !b; //if active -> disable
	}
	else if (id == MENU_SAVE) {
		if (gframe->proceedSaveAsSaveAs()) {
			b = true;
		}
		else {
			b = isModified();
		}
	}
	else if (id == MENU_EDIT) {
		b = !isEditEnable();
	}
	else if (id == MENU_PBN_EDITOR) {
		b = !onlyPreferansProblems();
	}
	else if (id >= MENU_SKIN0 && id <= MENU_SKIN7) {
		b = MENU_SKIN0 + gconfig->m_skin != id;
	}
	else if (id >= MENU_LANGUAGE_FIRST && id < MENU_NEW) {
		//if gconfig->getLanguageIndex()==-1 always enable
		b = id != MENU_LANGUAGE_FIRST + gconfig->getLanguageIndex();
	}
	else if (id == MENU_UNDOALL || id == MENU_UNDO) {
		b = isUndoEnable();
	}
	else if (id == MENU_REDOALL || id == MENU_REDO) {
		b = isRedoEnable();
	}
	else if (id == MENU_FIND_BEST_MOVE) {
		b = isToolbarButtonEnabled(TOOLBAR_BUTTON_FIND_BEST);
	}
	else if (id == MENU_SOLVE_FOR_ALL_DECLARERS) {
		//in case of id==MENU_SOLVE_FOR_ALL_DECLARERS even if trump not set possible solve for all declarers Gboplo27.pbn problem 47
		b = getFindBestState(false) != BUTTON_STATE_DISABLED; //DO NOT CHANGE
	}
	else if (id == MENU_SOLVE_ALL_FOE) {
		b = getProblem().isSolveAllFoeEnable();
	}
	else {
		b = true;
	}

	//all think dependent became disabled
	if(think() && id!=MENU_FIND_BEST_MOVE &&  ((id>=MENU_RECENT && id<MENU_RECENT+m_lastRecentSize) || ONE_OF(id,MENU_THINK_DEPENDENT)) ){
		b=false;
	}
	gtk_widget_set_sensitive(w, b);

	//set icon
	i = INDEX_OF(id,TOOLBAR_MENU_ID);
	if (i != -1) {
		auto e=TOOLBAR_BUTTON_ARRAY[i];
		w = gtk_bin_get_child(GTK_BIN(w));
		list = gtk_container_get_children(GTK_CONTAINER(w));
		assert(g_list_length(list) > 0);
		gtk_image_set_from_pixbuf(GTK_IMAGE(g_list_nth(list, 0)->data),
				getToolbarPixbuf(e, true,getToolbarButtonState(e)));
		g_list_free(list);
	}
}

void Menu::updateAfterCreation() {
	int i;
	for (i = 0; i < SIZEI(ACCELERATOR); i++) {
		gtk_window_add_accel_group(GTK_WINDOW(gframe->getWidget()),
				m_accelerator[i]);
	}

	updateLanguage();
	updateGameType();
	updateSkin();

	//disable active radio menu items
	SET_ATTRIBUTES(RADIO_MENU_ID)
	SET_ATTRIBUTES(CHECKED_MENU)
}

MenuString Menu::recentMenuString(int index) {
	gchar buff[16];
	std::string s = gconfig->m_recent[index];
	if (s.length() > unsigned(gconfig->m_maxRecentLength)) {
		const std::string dots = "..";
		s = dots
				+ s.substr(s.length() - gconfig->m_maxRecentLength + dots.length());
	}

	sprintf(buff, "%d ", index + 1);
	return {MENU_ID(MENU_RECENT + index), buff + s}; //do not use Helper::encodeString, because we call addSubMenu() and this function do encoding
}

void Menu::updateRecent() {
	int i;
	for (i = 0; i < m_lastRecentSize; i++) {
		updateMenu(recentMenuString(i));
	}
	if (recentSize() > m_lastRecentSize) {
		assert(int(gconfig->m_recent.size()) == m_lastRecentSize + 1);
		insertRecentMenu(m_lastRecentSize);
		if (m_lastRecentSize == 0) {
			gtk_menu_shell_insert(getSubMenu(false), gtk_separator_menu_item_new(),
					m_firstRecentPosition + 1);
		}
		m_lastRecentSize = recentSize();
		gtk_widget_show_all(m_top[0]); //DONT REMOVE
	}
}

void Menu::addAccelerators(bool add) {
	int i;
	for (i = 0; i < SIZEI(m_accelerator); i++) {
		if (add) {
			gtk_window_add_accel_group(GTK_WINDOW(gframe->getWidget()),
					m_accelerator[i]);
		}
		else {
			gtk_window_remove_accel_group(GTK_WINDOW(gframe->getWidget()),
					m_accelerator[i]);
		}
	}
}

void Menu::updateThink(){
	for(auto& v:MENU_THINK_DEPENDENT){
		setItemAttributes(v);
	}
	for(int i=0;i<m_lastRecentSize;i++){
		setItemAttributes(MENU_ID(MENU_RECENT+i));
	}
}
